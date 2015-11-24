#include "indirect.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/kdev_t.h>

/**
* @brief Write to disk the content of the block pointed to the indirect pointer from the Inode
**/
int setIndirectBlockTable(NodeStruct *node, IBlockStruct* iblock) {
	if ( node->indirecto < 1 ) {
		fprintf(stderr,"----> Error getting indirect table!!! LBA requested %d\n", node->indirecto);
		return -1;
	}

	if((lseek(myFileSystem.fdVirtualDisk, (node->indirecto) * BLOCK_SIZE_BYTES, SEEK_SET) == (off_t) - 1) ||
	        (write(myFileSystem.fdVirtualDisk, iblock, BLOCK_SIZE_BYTES) == -1)) {
				perror("Failed lseek/read in getIndirectBlockTable");
				return -1;
	}
	return 0;
}


/**
* @brief Reads from disk the data block with the indrect table for the node. It mallocs the table, so someone should free it...
**/
IBlockStruct* getIndirectBlockTable(NodeStruct *node) {
	IBlockStruct* block;
	if ( node->indirecto < 1 ) {
		fprintf(stderr,"----> Error getting indirect table!!! LBA requested %d\n", node->indirecto);
		return NULL;
	}
	block = (IBlockStruct*) malloc(sizeof(IBlockStruct));

	if((lseek(myFileSystem.fdVirtualDisk, (node->indirecto) * BLOCK_SIZE_BYTES, SEEK_SET) == (off_t) - 1) ||
	        (read(myFileSystem.fdVirtualDisk, block, BLOCK_SIZE_BYTES) == -1)) {
				perror("Failed lseek/read in getIndirectBlockTable");
				return NULL;
	}
	return block;
}


/**
* @brief Called when a i-node uses the indirect poitner for the first time.
* 	 Looks for a free block and inits it to be used as a table of direct pointer
**/
int initIndirectBlockTable(NodeStruct *node) {
	int freeBlock=-1;
	int i =0;
	while (freeBlock==-1 && i<NUM_BITS) {
		if(myFileSystem.bitMap[i] == 0) {
			myFileSystem.bitMap[i] = 1;
			freeBlock = i;
			node->indirecto = freeBlock;
		}
		i++;
	} 
	if (freeBlock == -1 ) {
		fprintf(stderr,"Error finding free block in bitmap when init indirect block\n");
		return -1;
	}
	else {
		char block[BLOCK_SIZE_BYTES];
		memset(block, 0, sizeof(char)*BLOCK_SIZE_BYTES);
		if((lseek(myFileSystem.fdVirtualDisk, freeBlock * BLOCK_SIZE_BYTES, SEEK_SET) == (off_t) - 1) ||
		   (write(myFileSystem.fdVirtualDisk, &block, BLOCK_SIZE_BYTES) == -1)) {
				perror("Failed lseek/write in initIndirectBlockTable");
				return -EIO;
		}

	}
	return 0;
}



/**
* @brief Devuelve el LBA (BLOQUE FISICO) donde esta el bloque logico (bl) de un nodo-i
**/
DISK_LBA getBF_from_BL(NodeStruct *node, int bl) {

	// 1. Si se debe usar puntero directo, la traduccion es directa: node->blocks[bl]
	// 2 Si no, debemos leer la tabla apuntada por indirectos (getIndirectBlockTable) y buscar ahí la traducción
	 

	if (bl<=NDIRECTOS) {
		return node->blocks[bl];
	}
	else {
		IBlockStruct * ind = getIndirectBlockTable(node);
		int bf; // variable para guardar el LBA que vamos a devolver
		
		bf = ind->table[bl-2];

		free(ind);
		return bf;
	}
}

/**
* @brief Returns the LBA of the last logic block of the file
**/
DISK_LBA getBF_of_last_BL(NodeStruct *node) {
	return  getBF_from_BL(node,node->numBlocks - 1);
}

/**
* @brief Modifies the node to assign the BF (physical block) to the corresponding BL (maybe through the indirecto pointer)
**/ 
void assignBF_to_BL(NodeStruct *node, int bl, DISK_LBA bf) {

	// 1. Si se debe usar puntero directo, la traduccion es directa: node->blocks[bl]
	// 2 Si no, debemos leer la tabla apuntada por indirectos (getIndirectBlockTable) y buscar ahí la traducción, y actualizarla (setIndirectBlockTable)

	if (bl<=NDIRECTOS) {
		node->blocks[bl] = bf;
	}
	else {
		IBlockStruct * ind = getIndirectBlockTable(node);
	
		ind->table[bl-2]=bf;


		setIndirectBlockTable(node,ind);
		free(ind);

	}

}

