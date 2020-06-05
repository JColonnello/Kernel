BMFS_DIR = ../../BMFS

all: bmfs.bin bmfs

bmfs.bin : bmfs.c
	gcc -std=c99 -o bmfs.bin $^

bmfs : bmfs_new.c filedisk.c stdhost.c
	gcc -std=c99 -o bmfs $^ -I$(BMFS_DIR)/include -L$(BMFS_DIR) -l:libbmfs.a

clean:
	rm -f bmfs.bin bmfs
