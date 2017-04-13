System plików z użyciem FUSE
=======================

Wojciech Pratkowiecki

Systemy Operacyjne - P2 - semestr zimowy 2016/17

-----------------

Repozytorium poświęcone implementacji sterownika do obsługi systemu plików FAT16. Sterownik działa przy użyciu interfejsu **Fuse** (Filesystem in Userspace), umożliwiającego tworzenie własnych systemów plików w trybie użytkownika.  

---------------

Użycie sterownika
-------

Repozytorium zawiera plik `Makefile`, umożliwiający kompilację projektu przy pomocy polecenia `make`. 

Jako, że celem programu jest obsługa systemu plików FAT16, potrzebny jest obraz owego systemu. Repozytorium zawiera skrypt `make-fat16.sh`, który tworzy plik `fs-image.raw` systemu plików FAT16 rozmiaru 100MB o skomplikowanej strukturze.

Następnie należy uruchomić program `fusermount` podając mu jako argumenty ścieżkę do systemu plików sformatowanego w FAT16 oraz ścieżkę katalogu, w którym zostanie zamontowany obraz systemu, np:

`./filesystem fs-image.raw mountpoint/`

Wskazany katalog zawiera obraz systemu pliku. Można się po nim poruszać jak po typowym katalogu przy użyciu typowych poleceń, takich jak `ls, cd, stat, cat`

Aby odmontować system plików należy skorzystać z jednego z gotowych narzędzi. Przykładowo:

`fusermount -u mountpoint`  

Katalog można oczyścić z plików powstałych podczas kompilacji przy użyciu polecenia `make clean`

------------------------
Przydatne linki:

[Fuse na GitHubie](https://github.com/libfuse/libfuse)

[Dokumentacja FAT16](http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html)

[Tutorial FAT16](http://www.tavi.co.uk/phobos/fat.html)
