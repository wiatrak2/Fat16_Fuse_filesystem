/*
    Wojciech Pratkowiecki II UWr
    FAT16 FUSE driver
*/
#include "fat16.h"

void open_filesystem ( fat16_filesystem *fs )
{   
    if(!fread(&fs->fs_boot_block, sizeof(fs->fs_boot_block), 1, fs->fs))
        perror("unable to read boot sector");
    
    boot_block *boot_sec = malloc( sizeof( boot_block ) );
    boot_sec = &fs->fs_boot_block;
    fs->fat_offset = boot_sec->bytes_per_block * boot_sec->reserved_blocks;
    fs->fat_size = boot_sec->blocks_per_fat * boot_sec->bytes_per_block;
    fs->root_dir_start = fs->fat_offset + fs->fat_size * boot_sec->fats;
    fs->root_dir_entries = boot_sec->root_entries;
    fs->root_dir_size = fs->root_dir_entries * sizeof( root_directory );
    fs->root_dir_blocks = fs->root_dir_size / boot_sec->bytes_per_block;
    fs->data_offset = fs->root_dir_start + fs->root_dir_size;
    fs->cluster_size = boot_sec->bytes_per_block * boot_sec->blocks_per_claster;

    fseek( fs->fs, fs->fat_offset, SEEK_SET );
    fs->file_allocation_table = malloc( fs->fat_size );
    fread( fs->file_allocation_table, sizeof( uint16_t ), fs->fat_size / 2, fs->fs );

    init_vector( &fs->vector );
    fs->inode_index = 0;
}

fat16_attr *read_entry_attr ( root_directory *dir )
{
    fat16_attr *attr = malloc( sizeof( fat16_attr ) );
    uint8_t a = dir->file_attributes;
    attr->read_only     = a & 0x01;
    attr->hidden        = a & 0x02;
    attr->system        = a & 0x04;
    attr->volume_name   = a & 0x08;
    attr->directory     = a & 0x10;
    attr->achieve_flag  = a & 0x20;
    return attr;
}

inode_entry *init_inode ( uint64_t inode_num, root_directory *dir )
{
    inode_entry *entry  = malloc( sizeof( inode_entry ) ); 
    entry->inode        = inode_num;
    entry->inode_attr   = read_entry_attr( dir );
    entry->dir          = dir;
    entry->visited      = 0;
    entry->entries_list = NULL;
    return entry;
}

int seek_to_root_dir ( fat16_filesystem *fs )
{
    uint32_t offset = fs->fat_offset + fs->fs_boot_block.bytes_per_block * fs->fs_boot_block.blocks_per_fat * fs->fs_boot_block.fats;
    return fseek(fs->fs, offset, SEEK_SET);
}

int seek_to_cluster ( fat16_filesystem * fs, uint16_t cluster )
{
    uint32_t offset = fs->fat_offset + fs->fs_boot_block.bytes_per_block * fs->fs_boot_block.blocks_per_fat * fs->fs_boot_block.fats + fs->root_dir_size;
    return fseek(fs->fs, offset + (cluster - 2) * fs->cluster_size, SEEK_SET );
}

void init_vector ( inode_vector *vector )
{
    vector->capacity   = INITIAL_CAPACITY;
    vector->vector     = malloc( INITIAL_CAPACITY * sizeof(inode_entry) );

    if ( ! vector->vector )
    {
        perror("allocation fault");
        return;
    }
    vector->size = 0;

}

inode_entry *get_inode_entry ( uint64_t ino, fat16_filesystem *fs )
{
    inode_entry *entry = &(fs->vector.vector[ ino ]);
    return entry;
}

void vector_push ( inode_vector *vector, inode_entry *entry )
{

    if ( vector->capacity  <= vector->size )
    {
        inode_entry *tmp;

        if ( ( tmp = realloc ( vector->vector, 2 * vector->capacity * sizeof(inode_entry) ) ) == NULL )
        {
            perror("reallocation fault");
            return;
        }

        vector->vector = tmp;

        vector->capacity *= 2;
    }

    vector->vector [ ++(vector->size) ] = *entry;
}

inode_list *list_push ( inode_list *list, inode_entry *ino )
{
    inode_list *li = malloc(sizeof(inode_list));
    li->inode = ino;
    li->next = list;
    return li;
}

inode_entry *inode_lookup ( inode_entry *parent, const char *name )
{

    inode_list *list = parent->entries_list;
    while ( list )
    {
        char *filename = format_filename( list->inode->dir );
        if ( strcmp( name, filename ) == 0 )
                return list->inode;
            
        list = list->next;
    }

    return NULL;
}

void set_readdir_entries ( fat16_filesystem *fs, inode_entry *dir_entry )
{
    dir_entry->inode == 1 ? seek_to_root_dir( fs ) : seek_to_cluster( fs, dir_entry->dir->starting_cluster );
    
    dir_entry->visited = 1;   

    int entries_num =  dir_entry->inode == 1 ? fs->root_dir_entries : fs->cluster_size / sizeof( root_directory );

    inode_list *list = NULL;  
    
    for ( int i = 0 ; i < entries_num ; ++ i )
    {
        root_directory *dir = malloc( sizeof( root_directory ) );
        fread(dir, sizeof( root_directory ), 1, fs->fs);

        if( dir->filename[0] == 0x00 ||  (unsigned char) dir->filename[0] == 0xE5 )
            continue;
        if( ( read_entry_attr( dir ) )->volume_name )
            continue;

        inode_entry *entry = init_inode( ++(fs->inode_index), dir );
        vector_push( &fs->vector, entry );
        list = list_push( list, entry );
    } 

    dir_entry->entries_list = list;
}

void read_date ( struct tm *tm, uint16_t fat16_date )
{
    tm->tm_mday = fat16_date & 0x1F;
    tm->tm_mon  = ((fat16_date >> 5) & 0x0F) - 1;
    tm->tm_year = ((fat16_date >> 9) & 0x7F) + 80;
}

void read_time ( struct tm *tm, uint16_t fat16_time )
{
    tm->tm_sec  = (fat16_time & 0x1F) * 2;
    tm->tm_min  = (fat16_time >> 5) & 0x3F;
    tm->tm_hour = (fat16_time >> 11) & 0x1F;
}

void get_date ( struct stat *st, root_directory *dir )
{
    struct tm atime;
    struct tm mtime;
    struct tm ctime;
    memset(&atime, 0, sizeof(atime));
    memset(&mtime, 0, sizeof(mtime));
    memset(&mtime, 0, sizeof(ctime));
    
    read_date(&atime, dir->access_date);
    st->st_atime = mktime(&atime);
    
    read_date(&mtime, dir->modify_date);
    read_time(&mtime, dir->modify_time);
    st->st_mtime = mktime(&mtime);

    read_date(&ctime, dir->create_date);
    read_time(&ctime, dir->create_time);
    st->st_ctime = mktime(&ctime);
}

nlink_t set_nlink( inode_entry *entry )
{
    nlink_t ref = 0;
    inode_list *list = entry->entries_list;
    while( list )
    {
        if(list->inode->inode_attr->directory)
            ++ ref;

        list = list->next;
    }
    return ref;
} 

char* format_filename( root_directory *dir )
{
    char *filename = calloc( 12, sizeof( char ) );
    int index = 8;

    fat16_attr *attr = read_entry_attr( dir );

    while( dir->filename[ index - 1 ] == 0x20 && index > 0 )
        -- index;
    strncat( filename, dir->filename, index );

    if(! attr->directory)
    {
        int dot = index;

        index = 3;

        while( dir->filename_extension[ index - 1 ] == 0x20 && index > 0 ) 
            -- index;

        if( index )
            filename[ dot ] = '.';
        strncat( filename, dir->filename_extension, index );
    }
    
    for ( int i = 0 ; filename[ i ] ; ++ i )
        if( filename[ i ] >= 65 && filename[ i ] <= 90  )
            filename[ i ] += 32; 

    return filename;
}


void read_file ( fat16_filesystem *fs, inode_entry *entry, char *buff, size_t size, off_t off )
{
    uint16_t cluster = entry->dir->starting_cluster;

    for ( int i = 0 ; i < off / fs->cluster_size ; ++ i )
        cluster = fs->file_allocation_table[ cluster ];

    seek_to_cluster( fs, cluster );

    size_t new_off = off % fs->cluster_size;
    fseek( fs->fs, new_off, SEEK_CUR );

    size_t new_size = min( fs->cluster_size - new_off, size );
    fread( buff, new_size, 1, fs->fs );

    while( new_size < size )
    {
        cluster = fs->file_allocation_table[ cluster ];
        seek_to_cluster( fs, cluster );
        fread( buff + new_size, min( fs->cluster_size, size - new_size ), 1, fs->fs );
        new_size += min( fs->cluster_size, size - new_size );
    }
}
