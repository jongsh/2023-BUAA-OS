/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		if (syscall_write_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_ID), 4) < 0) {
			user_panic("Wrong\n");
		}
		temp = begin + off;
		if (syscall_write_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS), 4) < 0) {
			user_panic("Wrong\n");
		}
		temp = 0;
		if (syscall_write_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION), 4) < 0) {
			user_panic("Wrong\n");
		}
		if (syscall_read_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_STATUS), 4) < 0) {
			user_panic("Wrong\n");
		}
		if (temp == 0) {
			user_panic("ide_read failed!\n");
		} else {
			if (syscall_read_dev(dst + off, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_BUFFER), BY2SECT) < 0) {
				user_panic("Wrong\n");
			}
		}

	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		if (syscall_write_dev(src + off, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_BUFFER), BY2SECT) < 0) {
			user_panic("Wrong");
		}
		if (syscall_write_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_ID), 4) < 0) {
			user_panic("Wrong");
		}
		temp = begin + off;
		if (syscall_write_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS), 4) < 0) {
			user_panic("Wrong");
		}
		temp = 1;
		if (syscall_write_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION), 4) < 0) {
			user_panic("Wrong");
		}
		if (syscall_read_dev((void *)&temp, (u_int)(DEV_DISK_ADDRESS + DEV_DISK_STATUS), 4) < 0) {
			user_panic("Wrong");
		}
		if (temp == 0) {
			user_panic("ide_write failed!\n");
		}
	}
}

struct phyblk {
	int writable;
	int count;
} blks[40];
int map[40];
char zero[512] = {0};

void flash(u_int physic_no) {
	ide_write(0, physic_no, zero, 1);
	blks[physic_no].count += 1;
	blks[physic_no].writable = 1;
}

int alloc_physic_blk() {
	u_int able_blk, unable_blk;
	u_int able_min = 999999;
	u_int unable_min = 99999999;
	for (int i = 0; i < 32; ++i) {
		if (blks[i].writable == 1) {
			if (blks[i].count < able_min) {
				able_min = blks[i].count;
				able_blk = i;
			}
		} else {
			if (blks[i].count < unable_min) {
				unable_min = blks[i].count;
				unable_blk = i;
			}
		}
	}

	if (blks[able_blk].count < 5) {
		return able_blk;
	} else {
		char temp[512];
		ide_read(0, unable_blk, temp, 1);
		ide_write(0, able_blk, temp, 1);
		blks[able_blk].writable = 0;
		for (int i = 0; i < 32; ++i) {
			if (map[i] == unable_blk) {
				map[i] = able_blk;
				break;
			}
		}
		flash(unable_blk);
		return unable_blk;
	}
}
void ssd_init() {
	for (int i = 0; i < 32; ++i) {
		map[i] = -1;
		blks[i].writable = 1;
		blks[i].count = 0;
	}
}

int ssd_read(u_int logic_no, void *dst) {
	if (map[logic_no] == -1) {
		return -1;
	}
	u_int physic_no = map[logic_no];
	ide_read(0, physic_no, dst, 1);
	return 0;

}

void ssd_write(u_int logic_no, void *src) {
	u_int physic_no;
	if (map[logic_no] != -1) {
		physic_no = map[logic_no];
		flash(physic_no);
		
        }
	physic_no = alloc_physic_blk();
	map[logic_no] = physic_no;
	ide_write(0, physic_no, src, 1);
	blks[physic_no].writable = 0;
}

void ssd_erase(u_int logic_no) {
	if (map[logic_no] == -1) {
		return;
	}
	flash(map[logic_no]);
	map[logic_no] = -1;
	return;
}
