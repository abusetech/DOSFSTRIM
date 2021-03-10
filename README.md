# DOSFSTRIM
MS-DOS Compatible TRIM Utility for FAT16/FAT32 Filesystems

This project aims to create a DOS utility that can be used to [TRIM](https://en.wikipedia.org/wiki/Trim_\(computing\)) unallocated sectors on SSDs containing FAT16 or FAT32 filesystems. The main use for this utility is periodic maintenance of SSDs installed in vintage computers running legacy operating systems such as DOS or Windows 9x. This project is very much a work in progress and is unlikely to compile correctly at the moment.

**This utility is EXPERIMENTAL. It has not been extensively tested and is __VERY__ likely to cause corruption or data loss if something goes wrong. Use at your own risk.**

This utility must be run from DOS or DOS mode and only supports I/O port connected ATA disks (ie. IDE drives or SATA drives with BIOS emulation) as it works by sending ATA commands directly to drive controller through the standard I/O ports, thus bypassing the operating system.

This utility could possibly be run automatically on startup by placing a line in the AUTOEXEC.BAT file, for automatic maintenance of SSDs in these systems.

