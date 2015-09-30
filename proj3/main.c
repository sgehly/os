#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXRATS 5
#define MAXROOMS 8

#define VERBOSE 1
#define DEBUG 1

struct vbentry
{
	int iRat;		//rat identifier
	int tEntry;		//time of entry into room
	int tDep;		//time of departure from room
};

struct Room
{
	int number;		//room number
	int capacity;	//room capacity
	int timeDelay; 	//time delay for room
};

//array of room visitors books
struct vbentry roomVB [MAXROOMS][MAXRATS];

int rats;
int rooms;
int startTime;

struct Room roomArray[MAXROOMS];
sem_t semArray[MAXROOMS];
sem_t semArray2[MAXROOMS];
int visitorCount[MAXROOMS] = {0};

//function prototypes
void * rat(void * x);
void LeaveRoom(int iRat, int iRoom, int tEnter);
void EnterRoom(int iRat, int iRoom);

void * rat(void * x)
{
	//TO DO
	//check capacity using semaphores
	//use EnterRoom function

	int i = 0;
	for(i = 0; i < rooms; i++)
	{
		if(VERBOSE)
			printf("In room number %d\n", roomArray[i].number);
		
		int tEntry = (int)(time(NULL) - startTime);

		//try to enter room
		EnterRoom(*((int*)(&x)), i);
		
		//leave room -- leaves entry in visitor book
		LeaveRoom(*((int*)(&x)), i, tEntry);
	}
}

void EnterRoom(int iRat, int iRoom)
{
	if(DEBUG)
		printf("Decrementing semaphore %d for rat %d.\n", iRoom, iRat);

	sem_wait(&semArray[iRoom]);
	sem_post(&semArray2[iRoom]);

	if(DEBUG)
		printf("Sleeping in room %d for rat %d.\n", iRoom, iRat);

	sleep(roomArray[iRoom].timeDelay);

	if(DEBUG)
		printf("Incrementing semaphore %d for rat %d.\n", iRoom, iRat);
}

void LeaveRoom(int iRat, int iRoom, int tEnter)
{
	//update visitor values for the rat in this room
	struct vbentry *visitor = malloc(sizeof(struct vbentry));
	visitor->iRat = iRat;
	visitor->tEntry = tEnter;
	visitor->tDep = (int)(time(NULL) - startTime);

	//store entry in the book and increment total visitors to this room
	roomVB[iRoom][visitorCount[iRoom]] = *visitor;
	visitorCount[iRoom]++;

	sem_post(&semArray[iRoom]);
	sem_wait(&semArray2[iRoom]);
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
	rooms = atoi(argv[2]);
	if(VERBOSE)
		printf("You entered %d rats and %d rooms.\n", rats, rooms);

	//check valid number of rats
	if(rats > MAXRATS || rats < 0)
	{
		printf("Invalid number of rats. Must be positive and less than %d. Try again!\n", MAXRATS);
		exit(1);
	}

	//check valid number of rooms
	if(rooms > MAXROOMS || rooms < 0)
	{
		printf("Invalid number of rooms. Must be positive and less than %d. Try again!\n", MAXROOMS);
		exit(1);
	}


	pthread_t ratArray[rats];

	//init rooms
	FILE *roomsFile;
	roomsFile = fopen("./rooms", "r");

	int capacity, timeDelay;

	//creates desired number of rooms and stores them in an array
	int numberOfRooms = 0;
	while(fscanf(roomsFile, "%d %d", &capacity, &timeDelay) != EOF)
	{
		if(numberOfRooms >= rooms)
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

	//checking rooms file to make sure number of rooms is ok
	//exit if argument doesn't match number of imported rooms
	if(numberOfRooms == rooms)
	{
		printf("You were able to import %d room(s) from the file. Good luck!\n", numberOfRooms);
	}
	else
	{
		printf("You could only import %d room(s) from the file. Check that again.\n", numberOfRooms);
		exit(1);
	}

	//done with the file
	fclose(roomsFile);

	//create semaphores and store them in an array
	int j;
	for(j = 0; j < rooms; j++)
	{
		sem_t semaphore;
		sem_t semaphore2;
		semArray[j] = semaphore;
		semArray2[j] = semaphore2;
	}

	//initialize the semaphores in the array with initial value matching the capacity of each room
	for(j = 0; j < rooms; j++)
	{
		if(VERBOSE)
			printf("Initializing a semaphore for room %d with value %d\n", j, roomArray[j].capacity);
		sem_init(&semArray[j], roomArray[j].capacity, 0);
		sem_init(&semArray2[j], 0, 0);
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

	//print visitor book
	int a, b;
	for(a = 0; a < rooms; a++)
	{
		for(b = 0; b < visitorCount[a]; b++)
		{
			printf("Room %d [%d %d]: ", a, roomArray[a].capacity, roomArray[a].timeDelay);
			printf("%d %d %d; \n", roomVB[a][b].iRat, roomVB[a][b].tEntry, roomVB[a][b].tDep);
		}
	}

	//user sees completion of the simulation with statistics.
	printf("Simulation completed in %d seconds.\n", (int)(time(NULL) - startTime));

	//ADD OPTIMAL TIME HERE
}
