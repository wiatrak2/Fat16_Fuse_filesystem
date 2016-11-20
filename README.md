System plików z użyciem FUSE
=======================

Wojciech Pratkowiecki

Systemy Operacyjne - P2 - semestr zimowy 2016/17

----------

Opis problemu
-------------

Implementacja sterownika obsługi systemu plików FAT16 przy użyciu interfejsu FUSE, który to będzie odczytywał dane z obrazu. Należy zaimplementować następujące operacje:
 
`open , read ,  release ,  getattr ,  lookup ,  opendir ,  readdir ,  releasedir ,  statfs`
a także zadbać o zwracanie odpowiednich kodów błędów w razie niepowodzenia którejś z nich.
