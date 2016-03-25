/*
	Project 4 CS3013 A15
	Jacob Hackett
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/resource.h>

#define BUFSIZE 1024
int MAX_THREADS = 15;

//stat variables are global
int numBadFiles = 0;
int numDir = 0;
int numRegFiles = 0;
int numSpecFiles = 0;
long totalRegBytes = 0;
int numRegText = 0;
long totalTextBytes = 0;
int totalNumFiles = 0;

//thread routine
void * thisFile(void * x)
{
	//stats struct
	struct stat *stats = malloc(sizeof(struct stat));

	//if stats is -1, its a bad file
	if(stat((char *)x, stats) == -1)
	{
		numBadFiles++;
	}
	else
	{
		totalNumFiles++;

		//check if directory
		numDir += S_ISDIR(stats->st_mode);

		//regular file
		if(S_ISREG(stats->st_mode) == 1)
		{
			numRegFiles++;
			totalRegBytes += stats->st_size;

			//must read here to find out if it's a text file
			int fdIn;
			if((fdIn = open((char *)x, O_RDONLY)) < 0)
			{
				//not a text file, can't open
				printf("Could not open regular file: %s\n",  (char*)x);
			}
			else
			{
				char buffer[BUFSIZE];
				int isText = 1;
				int cnt;
				while((cnt = read(fdIn, buffer, BUFSIZE)) > 0)
				{
					//check if printable or space
					isText = 1;
					int i;
					for(i = 0; i < cnt; i++)
					{
						if(isprint(buffer[i]) == 0 && isspace(buffer[i]) == 0)
						{
							//not text
							isText = 0;
							break;
						}
					}
				}
				if(isText == 1)
				{
					//text file
					numRegText++;
					totalTextBytes += stats->st_size;
				}
			}
		}
		//increment number of special files if appropriate
		numSpecFiles += S_ISCHR(stats->st_mode);
		numSpecFiles += S_ISBLK(stats->st_mode);
		numSpecFiles += S_ISFIFO(stats->st_mode);
	}
}

int main(int argc, char * argv[])
{
	//timing stuff
	struct rusage usage;
	struct timeval startS, endS, startU, endU, start, end;
	getrusage(RUSAGE_SELF, &usage);
	gettimeofday(&start, NULL);

	//variable initialization
	int numOfFiles = 0;
	int numberOfCurrentThreads = 0;
	char fileName[BUFSIZE];
	pthread_t threadArray[BUFSIZE];

	//while there is stuff to read, read it
	//also make sure we don't go over our limit of 1024
	while(fgets(fileName, sizeof(fileName), stdin) != NULL && numOfFiles < BUFSIZE)
	{
		//keep within limit of threads
		if(numberOfCurrentThreads < MAX_THREADS)
		{
			//make and store thread
			pthread_t file;
			threadArray[numOfFiles] = file;

			//strip the stray \n at the end of the word
			fileName[strcspn(fileName, "\n")] = 0;

			//create thread with param as the fileName
			if(pthread_create(&file, NULL, thisFile, (void *)fileName) != 0)
			{
				printf("Error creating thread. Try again.\n");
				exit(1);
			}
			else
			{
				//increment number of threads
				numberOfCurrentThreads++;
			}
			//increment number of files
			numOfFiles++;
		}
		else
		{
			//join all current threads
			int a;
			for(a = numOfFiles - MAX_THREADS + 1; a < numOfFiles; a++)
			{
				if(pthread_join(threadArray[a], NULL) != 0)
				{
					printf("Error joining thread number %d. Try again.\n", a);
					exit(1);
				}
				else
				{
					numberOfCurrentThreads--;
				}
			}
		}
	}
	//more timing stuff
	getrusage(RUSAGE_SELF, &usage);
	endS = usage.ru_stime;
	endU = usage.ru_utime;
	gettimeofday(&end, NULL);
	
	//print stats
	printf("Number of files: %d\n\n", totalNumFiles);

	printf("Number of bad files: %d\n", numBadFiles);
	printf("Number of directories: %d\n", numDir);
	printf("Number of regular files: %d\n", numRegFiles);
	printf("Number of special files: %d\n", numSpecFiles);
	printf("Number of bytes for regular files: %lu\n", totalRegBytes);
	printf("Number of regular files with text: %d\n", numRegText);
	printf("Number of bytes for regular text files: %lu\n", totalTextBytes);

	printf("\nWall clock time: %ldmcs\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
	printf("System CPU time:%ldmcs\n",((endS.tv_sec * 1000000 + endS.tv_usec) - (startS.tv_sec * 1000000 + startS.tv_usec)));
	printf("User CPU time: %ldmcs\n",((endU.tv_sec * 1000000 + endU.tv_usec) - (startU.tv_sec * 1000000 + startU.tv_usec)));

	return 0;
}