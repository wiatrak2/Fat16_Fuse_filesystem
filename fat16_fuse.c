/*
    Wojciech Pratkowiecki II UWr
    FAT16 FUSE driver
*/
#include "fat16_fuse.h"

/* ------------------------------------------- */
/*
	Poniższe funkcje pochodzą z programu 'hello_ll.c' będącego przykładem użycia interfejsu fuse  
*/
struct dirbuf {
    char *p;
    size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
                       fuse_ino_t ino)
{
    struct stat stbuf;
    size_t oldsize = b->size;
    b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
    b->p = (char *) realloc(b->p, b->size);
    memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
    fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
                      b->size);
}

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
                             off_t off, size_t maxsize)
{
    if (off < bufsize)
    return fuse_reply_buf(req, buf + off,
                          min(bufsize - off, maxsize));
    else
    return fuse_reply_buf(req, NULL, 0);
}
/* ------------------------------------------- */

void fat16_init( void *userdata, struct fuse_conn_info *conn )
{

	fat16_filesystem *fs = (fat16_filesystem *) userdata;
	
	open_filesystem( fs );

    fseek( fs->fs, fs->fat_offset, SEEK_SET );
    fs->file_allocation_table = malloc( fs->fat_size );
    fread( fs->file_allocation_table, sizeof( uint16_t ), fs->fat_size / 2, fs->fs );

	if( seek_to_root_dir( fs ) )
		perror("seek error");

    root_directory *root_dir = malloc( sizeof( root_directory ) );
    fread(root_dir, sizeof(root_directory), 1, fs->fs);

    inode_entry *root_entry = init_inode( 1, root_dir );
    root_entry->inode_attr->directory = 1;
    root_entry->dir->starting_cluster = 0;
    vector_push( &(fs->vector), root_entry );
    
    ++ (fs->inode_index);

}

void fat16_destroy( void *userdata )
{
	fat16_filesystem *fs = (fat16_filesystem *) userdata;
	inode_entry *entry;
	for ( uint64_t i = 1 ; i <= fs->inode_index ; ++ i )
	{
		entry = get_inode_entry( i, fs );
		if(  entry->visited && entry->entries_list != NULL  )
		{
			inode_list *list = entry->entries_list;
			while( list )
			{
				inode_list *next = list->next;
				free(list);
				list = next;
			}
		}
	}
	free( &fs->vector );
	fclose( fs->fs );
}

void fat16_stat( fat16_filesystem *fs, inode_entry *entry, struct stat *st)
{
	
	st->st_ino = entry->inode;
	st->st_blksize = fs->cluster_size;

	get_date ( st, entry->dir );

	if ( entry->inode_attr->directory )
    {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = entry->inode == 1 ? set_nlink( entry ) + 2 : set_nlink( entry ) ;

    }
    else
    {
        st->st_mode 	 = S_IFREG;
        st->st_mode 	|= entry->inode_attr->read_only ? 0444 : 0666 ;
        st->st_nlink 	 = 1;
        st->st_size 	 = entry->dir->file_size;
        st->st_blocks 	 = ( st->st_size + ( st->st_blksize >> 1 ) ) / st->st_blksize;
    }

}

void fat16_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	
	fat16_filesystem *fs = (fat16_filesystem *) fuse_req_userdata (req);
	inode_entry *parent_entry = get_inode_entry( parent, fs );
	uint64_t ino;

	 if(! parent_entry->inode_attr->directory )
	 {
	 	fuse_reply_err(req, ENOENT);
	 	return;
	 }

	 inode_entry *entry = inode_lookup( parent_entry, name );

	 if( ! entry )
	 {
	 	fuse_reply_err(req, ENOENT);
	 	return;
	 }

	 
	struct fuse_entry_param *e = calloc( 1, sizeof( struct fuse_entry_param ) );
	e->attr_timeout = 0;
	e->entry_timeout = 0;
	e->ino = entry->inode;

	struct stat *st = calloc( 1, sizeof( struct stat ) );
	fat16_stat( fs, entry, st );
	e->attr = *st;

	fuse_reply_entry( req, e );

	free(st);
	free(e);

}

void fat16_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	
	fat16_filesystem *fs = (fat16_filesystem *) fuse_req_userdata( req );
	inode_entry *entry = get_inode_entry( ino, fs );

	if( ino > fs->inode_index || ino < 0 || entry == NULL )
	{
		fuse_reply_err(req, ENOENT);
		return;
	}
	
	struct stat *st = calloc( 1, sizeof( struct stat ) );
	fat16_stat( fs, entry, st ); 
	fuse_reply_attr(req, st, 1.0);
	
	free( st );
}
void fat16_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{

	fat16_filesystem *fs = (fat16_filesystem *) fuse_req_userdata( req );
	inode_entry *parent_entry = get_inode_entry( ino, fs );

	if( ! parent_entry->inode_attr->directory )
	{
		fuse_reply_err(req, ENOTDIR);
		return;
	}
	struct dirbuf *b = calloc(1, sizeof(struct dirbuf));
	inode_list *list = NULL;

	if(! parent_entry->visited)
	{
		set_readdir_entries(fs, parent_entry);
	}

	list = parent_entry->entries_list;

	while(list)
	{
		inode_entry *entry = list->inode;
		char *filename = format_filename( entry->dir );
	    dirbuf_add( req, b, filename, entry->inode );
	    list = list->next;
	}


	reply_buf_limited(req, b->p, b->size, off, size);
    free(b->p);
    free(b);
}

void fat16_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	fuse_reply_open( req, fi );
}
void fat16_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	fuse_reply_err(req, ENOENT);
}
void fat16_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	fat16_filesystem *fs = (fat16_filesystem *) fuse_req_userdata( req );
	inode_entry *entry = get_inode_entry( ino, fs );

	entry == NULL ? fuse_reply_err( req, ENOENT ) : ( entry->inode_attr->directory ? fuse_reply_err( req, EISDIR ) : ( fi->flags & 3 != O_RDONLY ? fuse_reply_err( req, EACCES ) : fuse_reply_open( req, fi ) ) );
}
void fat16_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{
	fat16_filesystem *fs = (fat16_filesystem *) fuse_req_userdata( req );
	inode_entry *entry = get_inode_entry( ino, fs );

	if( ! entry )
	{
		fuse_reply_err( req, ENOENT );
		return;
	}

	char *buff = malloc( size * sizeof ( char ) );

	read_file( fs, entry, buff, size, off );

	reply_buf_limited( req, buff, size, 0, size );

	free( buff );

}
void fat16_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	fuse_reply_err(req, ENOENT);
}
void fat16_statfs(fuse_req_t req, fuse_ino_t ino)
{
	struct statvfs *statfs = calloc( 1, sizeof( struct statvfs ) );
	fuse_reply_statfs( req, statfs );
	free( statfs );
}

int main(int argc, char *argv[]) {


    struct fuse_args args = FUSE_ARGS_INIT(argc-1, &argv[1]);
    struct fuse_session *se;
    struct fuse_cmdline_opts opts;
    int ret = -1;

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    
    fat16_filesystem *fs = malloc(sizeof(fat16_filesystem));
    fs->fs = fopen( argv[1], "rb" );

    se = fuse_session_new(&args, &fat16_ops, sizeof(fat16_ops), fs);



    if (se != NULL) {
        if (fuse_set_signal_handlers(se) == 0) {
            if (fuse_session_mount(se, opts.mountpoint) == 0) {
                fuse_daemonize(opts.foreground);
                ret = fuse_session_loop(se);
                fuse_session_unmount(se);
            }
            fuse_remove_signal_handlers(se);
        }
        fuse_session_destroy(se);
    }

    free(opts.mountpoint);
    fuse_opt_free_args(&args);
    
    return ret ? 1 : 0;
  }  