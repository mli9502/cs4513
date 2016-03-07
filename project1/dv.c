// Author: Mengwen Li (mli2)

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <utime.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

// Print the usage of this function.
void usage(void);
// Get the path of the source file using the given file name.
void getSrcFilePath(char* file, char* dumpPath, char** srcPath);
// Remove a folder.
void removeFolder(char* currPath, char* file, int isSamePtn);
// Concatinate two strings.
char* concat(char* s1, char* s2);
// Copy a file to target across partition.
void copyToTar(char* srcPath, char* tarPath, struct stat fileStat);
// // Print the stat of the file.
// void printFileStat(char* file);

int main(int argc, char** argv)
{
	int c, i;
	int hFlg = 0;
	int errFlg = 0;
	extern int optind, opterr;
	opterr = 1;
	while((c = getopt(argc, argv, "h")) != -1)
	{
		switch(c)
		{
			case 'h':
			{
				if(hFlg)
				{
					errFlg ++;
					break;
				}
				hFlg ++;
				break;
			}
			default:
			{
				errFlg ++;
			}
		}
	}
	if(errFlg)
	{
		usage();
		exit(-1);
	}
	if(hFlg)
	{
		usage();
	}
	// Get file names.
	int fileCnt = argc - optind;
	if(!fileCnt)
	{
		printf("dv: missing file name\n");
		exit(-1);
	}
	char* files[fileCnt];
	for(i = 0; i < fileCnt; i ++)
	{
		files[i] = argv[i + optind];
	}

	// Get DUMPSTER environment variable.
	char* dumpPath = NULL;
	dumpPath = getenv("DUMPSTER");
	if(!dumpPath)
	{
		printf("No match for DUMPSTER in environment\n");
		exit(-1);
	}
	// Get stat for dumpster.
	struct stat dumpStat;
	int sRtn = stat(dumpPath, &dumpStat);
	if(sRtn)
	{
		perror("stat() call failed");
		exit(sRtn);
	}
	// Get information for current directory.
	char currDir[1024];
	char* gRtn = getcwd(currDir, 1024);
	if(!gRtn)
	{
		perror("getcwd() call failed");
		exit(-1);
	}
	// printf("Current working directory is %s\n", currDir);
	// Get stat for current working directory.
	struct stat currDirStat;
	sRtn = stat(currDir, &currDirStat);
	if(sRtn)
	{
		perror("stat() call failed");
		exit(sRtn);
	}
	// Move file from dumpster to current directory.
	// Check for partition.
	// printf("On same partition\n");
	for(i = 0; i < fileCnt; i++)
	{
		char* file = files[i];
		// Check for file existance.
		int aRtn = access(file, F_OK);
		if(aRtn == 0)
		{
			printf("File or folder %s already exists in current directory!\n", file);
			exit(-1);
		}
		// Free this!
		// Get the file in dumpster.
		char* srcPath;
		getSrcFilePath(file, dumpPath, &srcPath);
		// TODO: May have to report error when input is "/..."
		// Get the file name of the actual file.
		char* dupFile = strdup(file);
		char* tarFile;
		char* token;
		while((token = strsep(&dupFile, "/"))){
			// printf("%s\n", token);
			tarFile = strdup(token);
		}
		// printf("Final token is %s\n", tarFile);
		// int k = 0;
		// for(k = 0; k < j; k ++){
		// 	printf("%s\n", tokens[j]);
		// }
		// Use access to check whether fiel exists.
		if(access(srcPath, F_OK) == -1){
			printf("File or folder %s does not exits\n", srcPath);
			continue;
		}
		struct stat srcFileStat;
		int sRtn = stat(srcPath, &srcFileStat);
		if(sRtn)
		{
			perror("stat() call failed");
			exit(sRtn);
		}
		// Check partition here!
		// Check file or folder.
		// If same partition.
		if(dumpStat.st_dev == currDirStat.st_dev)
		{
			// If it ia s file.
			if(S_ISREG(srcFileStat.st_mode))
			{
				// printf("It is a file!\n");
				int rRtn = rename(srcPath, tarFile);
                if(rRtn)
                {
                    perror("rename() call failed");
                    exit(rRtn);
                }
                int cRtn = chmod(tarFile, srcFileStat.st_mode);
                // printf("cRtn is %d\n", cRtn);
                if(cRtn)
                {
                    perror("chmod() call failed");
                    exit(cRtn);
                }
			}
			else if(S_ISDIR(srcFileStat.st_mode))
			{
				// Recursively add new folder back to current directory.
				removeFolder(srcPath, tarFile, 1);
				int rRtn = rmdir(srcPath);
                if(rRtn)
                {
                    perror("rmdir() call failed");
                    exit(rRtn);
                }
			}
		}
		else
		{
			if(S_ISREG(srcFileStat.st_mode))
			{
				copyToTar(srcPath, tarFile, srcFileStat);
				int uRtn = unlink(srcPath);
				if(uRtn)
                {
                    perror("unlink() call failed");
                    exit(uRtn);
                }
			}
			else if(S_ISDIR(srcFileStat.st_mode))
			{
				removeFolder(srcPath, tarFile, 0);
				int rRtn = rmdir(srcPath);
				if(rRtn)
				{
					perror("rmdir() call failed");
					exit(rRtn);
				}
			}
		}
		
	}

}
// fileStat: file stat from src file.
void copyToTar(char* srcPath, char* tarPath, struct stat fileStat)
{
	FILE* src;
	FILE* tar;
	size_t bytes;
	char* buf[1024];
	src = fopen(srcPath, "r");
	if(src == NULL)
	{
		printf("Error opening file: %s\n", srcPath);
		exit(-1);
	}
	tar = fopen(tarPath, "w");
	if(tar == NULL)
	{
		printf("Error opening file: %s\n", tarPath);
        exit(-1);
	}
	//TODO: Add error checking!
	while(bytes = fread(buf, 1, 1024, src))
	{
		fwrite(buf, 1, bytes, tar);
	}
    if(ferror(src) || ferror(tar))
    {
        printf("Error reading and writing file");
        exit(-1);
    }
	fclose(src);
	fclose(tar);
	// printf("src mode is %d\n", fileStat.st_mode);
	// printf("tar path is %s\n", tarPath);
	// printFileStat(tarPath);
	int cRtn = chmod(tarPath, fileStat.st_mode);
	// printf("cRtn is %d\n", cRtn);
	if(cRtn)
	{
		perror("chmod() call failed");
		exit(cRtn);
	}
	const struct utimbuf srcTim = {fileStat.st_atime, fileStat.st_mtime};
	// printf("tarPath is %s\n", tarPath);
	int uRtn = utime(tarPath, &srcTim);
	if(uRtn)
	{
		perror("utime() call failed");
		exit(uRtn);
	}
	// printFileStat(srcPath);
	// printFileStat(tarPath);
	return;
}

// currPath should be the file path in dumpster.
// destPath should be current working directory.
// file is the file name being transfered.
void removeFolder(char* currPath, char* file, int isSamePtn)
{
	DIR* dp;
	struct dirent* d;
	// printf("Directory is %s\n", currPath);
	char* tarPath = file;
	// printf("target path is %s\n", tarPath);
	struct stat srcFolderStat;
    int sRtn = stat(currPath, &srcFolderStat);
    // If dRtn is not 0, stat() failed
    if(sRtn)
    {
        perror("stat() call failed");
        exit(sRtn);
    }
    // Create a new folder in curretn working directory.
    int mRtn = mkdir(tarPath, srcFolderStat.st_mode);
    if(mRtn)
    {
        perror("mkdir() call failed");
        exit(mRtn);
    }
    // Open the directory in dumpster.
    dp = opendir(currPath);
    if(dp == NULL)
    {
        perror("open() call failed");
        exit(-1);
    }
    d = readdir(dp);
    while(d)
    {
    	if((strcmp(d->d_name, "..") == 0) ||
    		(strcmp(d->d_name, ".") == 0))
    	{
    		// printf("Handling .. and . folder\n");
    		d = readdir(dp);
    		continue;
    	}
    	struct stat currStat;
    	char* currFile = concat(currPath, "/");
    	currFile = concat(currFile, d->d_name);
    	// printf("Current processing file is: %s\n", currFile);
    	int sRtn = stat(currFile, &currStat);
    	if(sRtn)
    	{
    		perror("stat() call failed");
    		exit(sRtn);
    	}
    	char* newTarPath = concat(tarPath, "/");
    	newTarPath = concat(newTarPath, d->d_name);
    	if(S_ISREG(currStat.st_mode))
    	{
    		if(isSamePtn)
    		{
    			// printf("newTarPath is %s\n", newTarPath);
    			int rRtn = rename(currFile, newTarPath);
    			if(rRtn)
    			{
    				perror("rename() call failed");
    				exit(rRtn);
    			}
                int cRtn = chmod(newTarPath, currStat.st_mode);
                // printf("cRtn is %d\n", cRtn);
                if(cRtn)
                {
                    perror("chmod() call failed");
                    exit(cRtn);
                }
    		}
    		else
    		{
    			// Copy file to newTarPath.
    			copyToTar(currFile, newTarPath, currStat);
    			int uRtn = unlink(currFile);
				if(uRtn)
                {
                    perror("unlink() call failed");
                    exit(uRtn);
                }
    		}
    	}
    	else if(S_ISDIR(currStat.st_mode))
    	{
    		removeFolder(currFile, newTarPath, isSamePtn);
    		int rRtn = rmdir(currFile);
            if(rRtn)
            {
                perror("rmdir() call failed");
                exit(rRtn);
            }
    	}
    	free(currFile);
    	free(newTarPath);
        d = readdir(dp); 
    }
    closedir(dp);
    int cRtn = chmod(tarPath, srcFolderStat.st_mode);
    if(cRtn)
    {
        perror("chmod() call failed");
        exit(cRtn);
    }

}

// Get the path for the file that is already in dumpster.
// srcPath is the path for the file in dumpster.
// TODO: Is .1, .2 ... condition need to be handled?
void getSrcFilePath(char* file, char* dumpPath, char** srcPath)
{
	*srcPath = concat(dumpPath, "/");
	*srcPath = concat(*srcPath, file);
	// printf("src path is %s\n", *srcPath);
	return;
}

/*Print the usage of the command*/
void usage(void)
{
    fprintf(stderr, "dv - retrive file or directory from dumpster\n");
    fprintf(stderr, "usage: dv [-h] file [file ...]\n");
    fprintf(stderr, "\t-h\tdisplay basic usage message\n");
}

// Reference: http://stackoverflow.com/questions/8465006/how-to-concatenate-2-strings-in-c
char* concat(char* s1, char* s2)
{
    char* result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// // Reference: Sample code stat.c.
// void printFileStat(char* file)
// {
//     struct stat fileStat;
//     int sRtn = stat(file, &fileStat);
//     if(sRtn)
//     {
//         perror("stat() call failed");
//         exit(sRtn);
//     }
//     printf("stat of: %s\n", file);
//     printf("\tdev: %d\n",  fileStat.st_dev);
//     printf("\tinode: %d\n", fileStat.st_ino);
//     printf("\tmode: %d\n", fileStat.st_mode);
//     printf("\tnlink: %d\n", fileStat.st_nlink);
//     printf("\tuid: %d\n", fileStat.st_uid);
//     printf("\tgid: %d\n", fileStat.st_gid);
//     printf("\trdev: %d\n", fileStat.st_rdev);
//     printf("\tsize: %d\n", fileStat.st_size);
//     printf("\tblocksize: %d\n", fileStat.st_blksize);
//     printf("\tblocks: %d\n", fileStat.st_blocks);
//     printf("\tctime: %d\n", fileStat.st_ctime);
//     printf("\tatime: %d\n", fileStat.st_atime);
//     printf("\tmtime: %d\n", fileStat.st_mtime);
//     return;
// }
