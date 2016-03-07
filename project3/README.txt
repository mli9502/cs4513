Author: Mengwen Li (mli2)

The client and server functionalities are in file "nutella.c"

To compile the project, use the following command:
make
To clean the make result, use the following command:
make clean

The client and server functionalities are implemented in two processes.

The movie file needs to end with ".txt".

To run the program, use the following command:
./nutella -d <directory containing movie files> -p <port number used by unicast>
The following line will then pomp out:
"Enter a movie (-r for repeatly play): "
A movie name can be entered here. Use " -r" at the end of movie name for repeatly play functionality.
For example:
"Enter a movie (-r for repeatly play): matrix -r" 
To terminate a repeatly play, use "ctrl + c" on the server functionality process (the process that currently sending data).
If " -r" is not entered, it plays once and user can enter a new movie.

If the movie can't be found, it will time out after 5 seconds and "Enter a movie (-r for repeatly play): " will be displayed again so another movie can be entered.

The frame rate is defined at the start of the "nutella.c" file using "#define FREQSEC 0" and  "#define FREQNSEC 100000000".


