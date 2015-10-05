#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int copiedChars = 0;
    char c;
    while (copiedChars < nBytes) {
    	c= getc(origin); //gets the next char from the origin file
        putc(c, destination); //puts that char in the destination file
        copiedChars++;
    }
    if (copiedChars != nBytes){
    	return (-1);
    }else
    return (copiedChars);
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * buf: parameter to return the read string. Buf is a
 * string passed by reference. 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly "built" in memory, return the starting 
 * address of the string (pointer returned by malloc()) in the buf parameter
 * (*buf)=<address>;
 * 
 * Returns: 0 if success, -1 if error
 */

int loadstr(FILE *file, char **buf){
    char charBuf;
    char* name;
    //We alocate enough space for 50 chars
    name = (char*) malloc (sizeof (char) * 50);
    fread(&charBuf, sizeof(char), 1, file);
    //We keep reading until we find the \0 flag
    while(charBuf!='\0'){
        strncat(name, &charBuf,1);
        fread(&charBuf, sizeof(char), 1, file);
    }
    **buf = *name;
    return 0;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * header: output parameter. It is used to return the starting memory address
 * of an array that contains the (name,size) pairs read from the tar file
 * nFiles: output parameter. Used to return the number of files stored in
 * the tarball archive (first 4 bytes of the header)
 *
 * On success it returns EXIT_SUCCESS. Upon failure, EXIT_FAILURE is returned.
 * (both macros are defined in stdlib.h).
 */
int
readHeader(FILE * tarFile, stHeaderEntry ** header, int *nFiles)
{
	int i;
    char **buf;
    stHeaderEntry* p;
    fread(&nFiles,sizeof(int),1,tarFile);

    //Memory reservation for the header entry.
    //Total size is nFiles times stHeaderEntry type size.
    if(!(p = malloc(sizeof (stHeaderEntry) * (*nFiles)))){
        fclose(tarFile);
        return(EXIT_FAILURE);
    }
	
    for (i = 0; i< *nFiles; i++){
        loadstr(tarFile, **buf);
        p[i].name=**buf;
        fread(&p[i].size,sizeof(unsigned int),1,tarFile);
    }

    return (EXIT_SUCCESS);
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE *tarFile, *inFile;
    int headerSize = sizeof(int);
    stHeaderEntry *header;

    if (nFiles <=0){ //No files, no tar.
	   return (EXIT_FAILURE);
    }

    if (!(tarFile = fopen(tarName, "w"))){
        return (EXIT_FAILURE);
    }

    if (!(header = malloc(sizeof(stHeaderEntry)*nFiles))){
        //If we can't allocate memory we fail.
        fclose(tarFile);
        remove(tarName);
        return(EXIT_FAILURE);

    int i = 0;
    //We calculate how much memory we need for the header
    for(; i<nFiles; i++){
        headerSize += sizeof(unsigned int);
        headerSize += sizeof(char)*((strlen(fileNames[i]))+1);
    }

    fseek(tarFile, headerSize, SEEK_SET); //Open the tarFile after the header
    
    i=0;
    for (; i<nFiles; i++){
        if ((inFile = fopen(fileNames[i], "r"))==NULL){
            fclose(tarFile);
            remove(tarName);
            return(EXIT_FAILURE);
        }
        header[i].name = fileNames[i];
        fseek(inFile, 0L, SEEK_SET);
        header[i].size = ftell(inFile);
        fseek(inFile, 0L, SEEK_SET);
        copynFile(inFile,tarFile,header[i].size);
    }

    fseek(tarFile, 0L, SEEK_SET);
    fwrite(&nFiles, sizeof(int), 1, tarFile);

    i=0;
    for(; i<nFiles; i++){
        fwrite(header[i].name, strlen(fileNames[i])+1, 1, tarFile);
        fwrite(&header[i].size, sizeof(unsigned int), 1, tarFile);
    }
    flcose(tarFile);
    free(header);
    return(EXIT_SUCCESS);
    }
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	// Complete the function
	return EXIT_FAILURE;
}
