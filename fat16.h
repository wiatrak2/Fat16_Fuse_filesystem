/*
    Wojciech Pratkowiecki II UWr
    FAT16 FUSE driver
*/
#ifndef fat16_h
#define fat16_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#define min(x, y) ((x) < (y) ? (x) : (y))
#define INITIAL_CAPACITY 512
/*
    FAT16 tutorials:
        http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html
        http://www.tavi.co.uk/phobos/fat.html

    boot_block - struktura opisująca boot block systemu plików
*/
typedef struct boot_block
{
    uint8_t     bootstrap_prog [3];
    char        manufacturer [8];
    uint16_t    bytes_per_block;
    uint8_t     blocks_per_claster;
    uint16_t    reserved_blocks;
    uint8_t     fats;
    uint16_t    root_entries;
    uint16_t    blocks_in_disk;
    uint8_t     media_descriptor;
    uint16_t    blocks_per_fat;
    uint16_t    blocks_per_track;
    uint16_t    num_heads;
    uint32_t    num_hidden_sectors;
    uint32_t    blocks_in_disk_large;
    uint16_t    drive_num;
    uint8_t     extended_boot_rec;
    uint32_t    vol_serial_num;
    char        volume_label [11];
    char        file_sys_id [8];
    uint8_t     bootstrap_code[448];
    uint8_t     boot_block_signature [2];
    
}__attribute__((packed)) boot_block;

/*
    root_directory - struktura opsiująca directory entry
*/

typedef struct root_directory
{
    char        filename [8];
    char        filename_extension [3];
    uint8_t     file_attributes;
    char        nt_bits;
    uint8_t     create_time_ms;
    uint16_t    create_time;
    uint16_t    create_date;
    uint16_t    access_date;
    uint16_t    reserved_fat;
    uint16_t    modify_time;
    uint16_t    modify_date;
    uint16_t    starting_cluster;
    uint32_t    file_size;

}__attribute__((packed)) root_directory;

/*
    fat16_attr - atrybuty pliku
*/

typedef struct fat16_attr
{
    uint8_t     read_only;
    uint8_t     hidden;
    uint8_t     system;
    uint8_t     volume_name;
    uint8_t     directory;
    uint8_t     achieve_flag;

} fat16_attr;

typedef struct inode_list inode_list;
/*
    inode_entry - struktura trzymająca główne informacje o danym pliku:
    - numer inode
    - atrybuty
    - directory entry
    - iformacje o wcześniejszym odwiedzeniu (dotyczy katalgów)
    - listę plików w katalogu (dotyczy katalogów)
*/
typedef struct inode_entry
{
    uint64_t         inode;
    fat16_attr       *inode_attr;
    root_directory   *dir;

    uint8_t          visited;
    inode_list       *entries_list;

} inode_entry;

/*
    inode_list - lista struktur inode_entry
*/

struct inode_list
{
    inode_entry *inode;
    struct inode_list *next;
};

/*
    inode_vector - struktura reprezentująca wektor inode'ów.
    W wektorze tym przechowywane są wszystkie odwiedzone pliki
*/

typedef struct inode_vector
{
    uint32_t    capacity;
    uint32_t    size;
    inode_entry *vector;

} inode_vector; 

/*
    fat16_filesystem - główna struktura programu trzymająca:
    - wskaźnik na system plików
    - wygląd struktury boot_block
    - FATs
    - dane do wyliczenia z boot blocka
    - wektor inode'ów
    - inode_index - numer ostatnio przypisanego inode'a
*/

typedef struct fat16_filesystem
{
    FILE         *fs;
    boot_block   fs_boot_block;
    uint16_t     fat_offset;
    uint16_t     fat_size;
    uint16_t     root_dir_start;
    uint16_t     root_dir_entries;
    uint16_t     root_dir_size;
    uint16_t     root_dir_blocks;
    uint16_t     data_offset;
    uint16_t     cluster_size;

    uint16_t     *file_allocation_table;

    inode_vector vector;
    uint64_t     inode_index;

} fat16_filesystem;

/*
    Funkcje początkowe, służące do inicjacji i uzupełnienia struktur trzymających informacje o systemie do zamontowania
*/
void open_filesystem ( fat16_filesystem *fs );
fat16_attr *read_entry_attr ( root_directory *dir );
/*
    Uzupełnienie informacji o wskaźnikach na dany plik
*/
nlink_t set_nlink( inode_entry *entry );
/*
    Obsługa wektora sturktur inode_entry oraz pojedynczych instancji struktury
*/
inode_entry *init_inode ( uint64_t inode_num, root_directory *dir );
void init_vector ( inode_vector *vector );
void vector_push ( inode_vector *vector, inode_entry *entry );
inode_entry *get_inode_entry ( uint64_t ino, fat16_filesystem *fs );
inode_entry *inode_lookup ( inode_entry *parent, const char *name );
void set_readdir_entries ( fat16_filesystem *fs, inode_entry *dir_entry );
inode_list *list_push ( inode_list *list, inode_entry *ino );
/*
    Poruszanie się po pliku będącym systemem fat16
*/
int seek_to_root_dir ( fat16_filesystem *fs );
int seek_to_cluster ( fat16_filesystem *fs, uint16_t cluster );

void read_file ( fat16_filesystem *fs, inode_entry *entry, char *buff, size_t size, off_t off );
/*
    Obsługa formatów zgodnych ze standardem fat16
*/
void read_date ( struct tm *tm, uint16_t fat16_date );
void read_time ( struct tm *tm, uint16_t fat16_time );
void get_date ( struct stat *st, root_directory *dir );
char* format_filename( root_directory *dir );


#endif /* fat16_h */
