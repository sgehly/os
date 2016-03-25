-------------------------------------------------------------------------------
CS3013 Operating Systems A 2015 Project 3
-------------------------------------------------------------------------------
Date: 09/30/2015
-------------------------------------------------------------------------------
Project state:

Fully functioning inorder and distributed traversals of the maze.
I have a nonblocking implementation but it currently does not produce 
traversal times that are below that of the distributed and inorder traversals.
-------------------------------------------------------------------------------
Credits
	Jacob Hackett (jrhackett@wpi.edu)
-------------------------------------------------------------------------------
Project description

This project introduces thread coordination through building a maze for "rat" 
threads to traverse. The maze consists of rooms with different capacity limits
that take different amounts of time to traverse. This project is written in C.

This program handles a max of 5 rats and 8 rooms. Rooms are taken in through
a file called "rooms".
-------------------------------------------------------------------------------
Dependencies:

Linux
-------------------------------------------------------------------------------
Documentation

Files are self documented
-------------------------------------------------------------------------------
Installation instructions

make
-------------------------------------------------------------------------------
Execution instructions

./maze (number of rats) (i for inorder, d for distributed, n for nonblocking)
-------------------------------------------------------------------------------
Additional Notes

Currently implemented version of nonblocking does not produce good results
in terms of traversal times. Nonblocking is typically the same or a couple
seconds longer than distributed.