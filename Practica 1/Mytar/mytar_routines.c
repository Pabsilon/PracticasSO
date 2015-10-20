#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
 
int copynFile(FILE * origin, FILE * destination, int nBytes)
{
    int numberCopied = 0;
    char c;
    while (numberCopied < nBytes){
        c = getc(origin);
        putc(c, destination);
        numberCopied++;
    }
    if (numberCopied != nBytes){
        /**
         * I figured that if you don't transfer the bytes asked
         * then there is an error. We don't need to identify it
         * so by doing this the function is greatly optimised.
         */
        return (-1);
    }
    return numberCopied;
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
	int size = 50, readChars = 0;	
    size_t numberChars; 
	//We alocate enough space 
    name = (char*) malloc (sizeof (char) * size);
    numberChars = fread(&charBuf, sizeof(char), 1, file);
	if(numberChars != 1)
	{
		return 1;
	}	
    //We keep reading until we find the \0 flag
    while(charBuf!='\0'){
        strncat(name, &charBuf,1);
		readChars++;
		// If there's not enough space
		if(readChars >= size)
		{
			// We increase the buffer in 50 chars and we reallocate memory
			size += 50;
			name = (char*) realloc (name, size);
		}
        numberChars = fread(&charBuf, sizeof(char), 1, file);
		if(numberChars != 1)
		{
			return 1;
		}
    }
	strncat(name, &charBuf,1);
    *buf = name;
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
    char *buf; //This will be 'initialized' in loadstr
    stHeaderEntry* p;
    fread(nFiles,sizeof(int),1,tarFile);

    //Memory reservation for the header entry.
    //Total size is nFiles times stHeaderEntry type size.
    if(!(p = malloc(sizeof (stHeaderEntry) * (*nFiles)))){
        fclose(tarFile);
        return(EXIT_FAILURE);
    }

    for (i = 0; i< *nFiles; i++){
        /**
         * buf returns the pointer to the allocated memory
         * where the name is stored
         */
        loadstr(tarFile, &buf);
        p[i].name = (char*) malloc(sizeof (char) * strlen(buf)+1);
        p[i].name=buf;
        fread(&p[i].size,sizeof(unsigned int),1,tarFile);
    }

    *header = p;

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

    if (nFiles <=0){
        //This shouldn't happen, but we never know.
	   return (EXIT_FAILURE);
    }

    if (!(tarFile = fopen(tarName, "w"))){
        //If we try to execute from a folder with read only permissions.
        return (EXIT_FAILURE);
    }

    if (!(header = malloc(sizeof(stHeaderEntry)*nFiles))){
        //If we can't allocate memory we fail.
        fclose(tarFile);
        remove(tarName);
        return(EXIT_FAILURE);
    }

    int i = 0;
    //We calculate how much memory we need for the header
    for(; i<nFiles; i++){
        headerSize += sizeof(unsigned int);
        headerSize += sizeof(char)*((strlen(fileNames[i]))+1);
    }

    fseek(tarFile, headerSize, SEEK_SET); //Open the tarFile after the header
    
    i=0;
    for (; i<nFiles; i++){
        /**
         * We dump every file's name, size (calculated with the
         * reading pointer) into our allocated header, and the 
         * content from the the original file into the .tar.
         */
        if ((inFile = fopen(fileNames[i], "r+"))==NULL){
        	fclose(tarFile);
            remove(tarName);
            return(EXIT_FAILURE);
        }
        header[i].name = fileNames[i];
        fseek(inFile, 0L, SEEK_END);
        header[i].size = ftell(inFile);
        fseek(inFile, 0L, SEEK_SET);
        copynFile(inFile,tarFile,header[i].size);
    }

    //The number of files is written at the start of the file
    fseek(tarFile, 0L, SEEK_SET);
    fwrite(&nFiles, sizeof(int), 1, tarFile);

    i=0;
    for(; i<nFiles; i++){
        /**
         * Lastly, the headear is copied into the file, following
         * the number of files, and before the actual files.
         */
        fwrite(header[i].name, strlen(fileNames[i])+1, 1, tarFile);
        fwrite(&header[i].size, sizeof(unsigned int), 1, tarFile);
    }
    fclose(tarFile);
    free(header);
    return(EXIT_SUCCESS);
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
	FILE *tarFile, *outFile;
    int numFiles,i = 0;
    stHeaderEntry *header;

    if (!(tarFile = fopen(tarName, "r"))){
        //If we don't have read permission on the .tar file
        return (EXIT_FAILURE);
    }

    //Extracts the name, size and number of files from the .tar header.
    readHeader(tarFile, &header, &numFiles);

    for (; i<numFiles; i++){
        if (!(outFile = fopen(header[i].name, "w"))){
            //If we don't have write permission in the 'extracting' folder.
            return (EXIT_FAILURE);
        }
        copynFile(tarFile, outFile, header[i].size);
        fclose(outFile);

    }
    fclose(tarFile);
    free(header);
return (EXIT_SUCCESS);
}
