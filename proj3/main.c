#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

//max rats and rooms
#define MAXRATS 5
#define MAXROOMS 8

//used for verbose and debugging prints
#define VERBOSE 1
#define DEBUG 0

//entry in the visitor book
struct vbentry
{
	int iRat;		//rat identifier
	int tEntry;		//time of entry into room
	int tDep;		//time of departure from room
};

//Rooms
struct Room
{
	int number;		//room number
	int capacity;	//room capacity
	int timeDelay; 	//time delay for room
};

//state enumeration
//distinguishes inorder, distributed, nonblocking from each other
enum State
{
	inorder,
	distributed,
	nonblocking
};

//global variable
int rats;
char *type;
int startTime;
int numberOfRooms;
enum State state;

//global arrays
struct Room roomArray[MAXROOMS];
sem_t semArray[MAXROOMS];
sem_t semArray2[MAXROOMS];
int visitorCount[MAXROOMS] = {0};
struct vbentry roomVB [MAXROOMS][MAXRATS];

//function prototypes
void * rat(void * x);
void LeaveRoom(int iRat, int iRoom, int tEnter);
void EnterRoom(int iRat, int iRoom);

void * rat(void * x)
{
	if(state == inorder)
	{
		int i = 0;
		//start at room 0 and go inorder
		for(i = 0; i < numberOfRooms; i++)
		{
			//try to enter room
			sem_wait(&semArray[i]);
			int tEntry = (int)(time(NULL) - startTime);
			EnterRoom(*((int*)(&x)), i);
			
			//leave room -- leaves entry in visitor book
			LeaveRoom(*((int*)(&x)), i, tEntry);
			sem_post(&semArray[i]);
		}
	}
	else if(state == distributed)
	{
		int i;
		//start at rat numbered room
		for(i = *((int*)(&x)) % numberOfRooms; i < numberOfRooms; i++)
		{
			//try to enter room
			sem_wait(&semArray[i]);
			int tEntry = (int)(time(NULL) - startTime);
			EnterRoom(*((int*)(&x)), i);
			
			//leave room -- leaves entry in visitor book
			LeaveRoom(*((int*)(&x)), i, tEntry);
			sem_post(&semArray[i]);
		}
		//wrap to 0 and finish
		for(i = 0; i < *((int*)(&x)) % numberOfRooms; i++)
		{
			//try to enter room
			sem_wait(&semArray[i]);
			int tEntry = (int)(time(NULL) - startTime);
			EnterRoom(*((int*)(&x)), i);
			
			//leave room -- leaves entry in visitor book
			LeaveRoom(*((int*)(&x)), i, tEntry);
			sem_post(&semArray[i]);
		}
	}
	else
	{
		int roomsVisited[numberOfRooms];
		int numberRoomsVisited = 0;
		int i;
		for(i = 0; i < numberOfRooms; i++)
		{
			roomsVisited[i] = 0;
		}
		while(numberRoomsVisited < numberOfRooms)
		{
			for(i = 0; i < numberOfRooms; i++)
			{
				if(roomsVisited[i] == 0)
				{
					//sem_wait returns 0 if wait succeeds, otherwise does nothing to sem and comes back
					//other than that, same algorithm as the other two
					if(sem_trywait(&semArray[i]) == 0)
					{
						//enter
						numberRoomsVisited++;
						roomsVisited[i] = 1;
						int tEntry = (int)(time(NULL) - startTime);
						EnterRoom(*((int*)(&x)), i);
						
						//leave room -- leaves entry in visitor book
						LeaveRoom(*((int*)(&x)), i, tEntry);
						sem_post(&semArray[i]);
					}
				}
			}
		}
	}
}

//enters room and sleeps for the room's timeDelay
void EnterRoom(int iRat, int iRoom)
{
	if(VERBOSE)
		printf("Rat %d is in room number %d\n", iRat, roomArray[iRoom].number);

	//sleep for given timeDelay
	sleep(roomArray[iRoom].timeDelay);
}

//marks the visitor book
void LeaveRoom(int iRat, int iRoom, int tEnter)
{
	if(VERBOSE)
		printf("Rat %d is leaving room number %d\n", iRat, roomArray[iRoom].number);

	//update visitor values for the rat in this room
	struct vbentry *visitor = malloc(sizeof(struct vbentry));
	visitor->iRat = iRat;
	visitor->tEntry = tEnter;
	visitor->tDep = (int)(time(NULL) - startTime);

	//store entry in the book and increment total visitors to this room
	roomVB[iRoom][visitorCount[iRoom]] = *visitor;
	visitorCount[iRoom]++;
}


//================================================================================================================================
//========================================================main.c==================================================================
//================================================================================================================================

int main(int argc, char * argv[])
{
	//log time the program started for visitor book entries
	startTime = time(NULL);

	//check valid number of arguments
	if(argc != 3)
	{
		printf("Wrong number of command line arguments. Must provide 2 arguments for number of rats and rooms! Try again!\n");
		exit(1);
	}

	//parse command line arguments and print to user
	rats = atoi(argv[1]);
	type = argv[2];
	if(VERBOSE)
		printf("You entered %d rats.\n", rats);

	//check valid number of rats
	if(rats > MAXRATS || rats < 0)
	{
		printf("Invalid number of rats. Must be positive and less than %d. Try again!\n", MAXRATS);
		exit(1);
	}

	//checks for the second param, traversal type
	//sets enum to dictate state
	if(strcmp(type, "i") == 0)
	{
		state = inorder;
	}
	else if(strcmp(type, "d") == 0)
	{
		state = distributed;
	}
	else if(strcmp(type, "n") == 0)
	{
		state = nonblocking;
		//printf("Nonblocking is not yet implemented. Sorry, try again.\n");
		//exit(1);
	}
	else
	{
		printf("Invalid traversal type. Please enter i, d, or n for the second argument.\n");
		exit(1);
	}

	pthread_t ratArray[rats];

	//init rooms
	FILE *roomsFile;
	roomsFile = fopen("./rooms", "r");

	int capacity, timeDelay;

	//creates desired number of rooms and stores them in an array
	numberOfRooms = 0;
	while(fscanf(roomsFile, "%d %d", &capacity, &timeDelay) != EOF)
	{
		if(numberOfRooms >= MAXROOMS)
		{
			break;
		}

		struct Room room;
		room.number = numberOfRooms;
		room.capacity = capacity;
		room.timeDelay = timeDelay;

		if(VERBOSE)
			printf("Creating room %d with capacity of %d and a time delay of %d.\n", room.number, room.capacity, room.timeDelay);
		roomArray[numberOfRooms] = room;
		numberOfRooms++;
	}
	//user confirmation
	printf("You were able to import %d room(s) from the file. Good luck!\n", numberOfRooms);

	//done with the file
	fclose(roomsFile);

	//create semaphores and store them in an array
	int j;
	for(j = 0; j < numberOfRooms; j++)
	{
		sem_t semaphore;
		semArray[j] = semaphore;
	}

	//initialize the semaphores in the array with initial value matching the capacity of each room
	for(j = 0; j < numberOfRooms; j++)
	{
		if(VERBOSE)
			printf("Initializing a semaphore for room %d with value %d\n", j, roomArray[j].capacity);
		sem_init(&semArray[j], 0, roomArray[j].capacity);
	}

	//creates and stores rat pthread_ts in an array
	for(j = 0; j < rats; j++)
	{
		pthread_t rat;
		ratArray[j] = rat;
	}

	//create threads from the ratArray threads
	long i = 0;
	for(i = 0; i < sizeof(ratArray)/sizeof(ratArray[0]); i++)
	{
		if(pthread_create(&ratArray[i], NULL, rat, (void *)i) != 0)
		{
			printf("Error creating thread number %ld. Try again.\n", i);
			exit(1);
		}
		else
		{
			if(VERBOSE)
				printf("Creating a thread with rat number %ld.\n", i);
		}
	}

	//wait for all threads to finish
	for(i = 0; i < sizeof(ratArray)/sizeof(ratArray[0]); i++)
	{
		if(pthread_join(ratArray[i], NULL))
		{
			printf("Error joining thread number %ld. Try again.\n", i);
			exit(1);
		}
		else
		{
			if(VERBOSE)
				printf("Joining a thread with rat number %ld.\n", i);
		}
	}

	
	//statistic variables
	int totalTime = 0;
	int idealTime = 0;

	int a, b;
	//printf time that rats finished maze
	if(state == inorder)
	{
		for(a = 0; a < visitorCount[numberOfRooms - 1]; a++)
		{
			printf("Rat %d completed the maze in %d seconds.\n", a, roomVB[numberOfRooms - 1][a].tDep);
		}
	}
	else
	{
		//nonblocking
		for(a = 0; a < rats; a++)
		{
			int largestDep = 0;
			for(b = 0; b < numberOfRooms; b++)
			{
				int c;
				for(c = 0; c < visitorCount[b]; c++)
				{
					if(a == roomVB[b][c].iRat)
					{
						if(largestDep < roomVB[b][c].tDep)
						{
							largestDep = roomVB[b][c].tDep;
						}
					}
				}
			}
			totalTime += largestDep;
			printf("Rat %d completed the maze in %d seconds.\n", a, largestDep);
		}
	}

	//print visitor book entries
	for(a = 0; a < numberOfRooms; a++)
	{
		printf("Room %d [%d %d]: ", a, roomArray[a].capacity, roomArray[a].timeDelay);
		for(b = 0; b < visitorCount[a]; b++)
		{
			//calculates total traversal time for inorder
			if(state == inorder)
			{
				if(a == numberOfRooms - 1)
				{
					totalTime += roomVB[a][b].tDep;
				}
			}
			//calculate total traversal time for distributed
			else if(state == distributed)
			{
				//fix this shit
				/*
				//if we're at the max rooms, look for rat 0
				int c;
				if(a == numberOfRooms - 1)
				{
					if(roomVB[a][b].iRat == 0 && numberOfRooms > 2)
					{
						totalTime += roomVB[a][b].tDep;
					}
				}
				//otherwise look for the rat number thats one less than the current room
				else if(roomVB[a][b].iRat - 1 == a)
				{
					totalTime += roomVB[a][b].tDep;
				}
				*/
			}
			else
			{
				//nonblocking would go here
			}

			//prints entry
			printf("%d %d %d; ", roomVB[a][b].iRat, roomVB[a][b].tEntry, roomVB[a][b].tDep);
		}
		//readability
		printf("\n");

		//update ideal time
		idealTime += roomArray[a].timeDelay;
	}

	//user sees completion of the simulation with statistics.
	printf("Total traversal time: %d seconds, compared to ideal time: %d seconds.\n", totalTime, idealTime * rats);
}
