#ifndef _INDIRECT_H_

#define _INDIRECT_H_

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "myFS.h"

#define MAX_FUSE_NARGS 64


extern MyFileSystem myFileSystem;

IBlockStruct* getIndirectBlockTable(NodeStruct *node);
int initIndirectBlockTable(NodeStruct *node);
DISK_LBA getBF_from_BL(NodeStruct *node, int bl);
DISK_LBA getBF_of_last_BL(NodeStruct *node);
void assignBF_to_BL(NodeStruct *node, int bl, DISK_LBA bf);

#endif
