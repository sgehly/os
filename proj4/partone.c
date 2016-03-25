/*
	Serial Architecture
	Project 4 CS3013 A15
	Jacob Hackett
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFSIZE 1024

/*problems: 
 *need the rest of the path for the file...
 *home directories aren't working?
 */

int main(int argc, char * argv[])
{
	int numBadFiles = 0;
	int numDir = 0;
	int numRegFiles = 0;
	int numSpecFiles = 0;
	long totalRegBytes = 0;
	int numRegText = 0;
	long totalTextBytes = 0;
	int totalNumFiles = 0;

	char fileName[BUFSIZE];

	while(fgets(fileName, sizeof(fileName), stdin) != NULL)
	{
		fileName[strcspn(fileName, "\n")] = 0;
		struct stat *stats = malloc(sizeof(struct stat));

		if(stat(fileName, stats) == -1)
		{
			numBadFiles++;
		}
		else
		{
			totalNumFiles++;
			numDir += S_ISDIR(stats->st_mode);

			//regular file
			if(S_ISREG(stats->st_mode) == 1)
			{
				numRegFiles++;
				totalRegBytes += stats->st_size;

				//must read here to find out if it's a text file
				int fdIn;
				if((fdIn = open(fileName, O_RDONLY)) < 0)
				{
					//not a text file, can't open
					//printf("Could not open regular file: %s\n", fileName);
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

	printf("Number of files: %d\n\n", totalNumFiles);

	printf("Number of bad files: %d\n", numBadFiles);
	printf("Number of directories: %d\n", numDir);
	printf("Number of regular files: %d\n", numRegFiles);
	printf("Number of special files: %d\n", numSpecFiles);
	printf("Number of bytes for regular files: %lu\n", totalRegBytes);
	printf("Number of regular files with text: %d\n", numRegText);
	printf("Number of bytes for regular text files: %lu\n", totalTextBytes);

	return 0;
}