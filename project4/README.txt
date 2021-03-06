Author: Mengwen Li (mli2)

Platform: Visual Studio 2015 is used with platform toolset set to Visual Studio 2013 (v120). It can also run on Visual Studio 2013.

To compile and run: 
1. Put the source code and the solution file in "vs-2013" directory (the source codes part).
2. Put the sprites in the "sprites" directory shown in the directory structure.
3. Double click on the "game.sln" file to open the project.
4. To run as host: Right click on the project in "Solution Explorer", Select "Property->Configuration Properties->Debugging" and set the "Command Arguments" as 
   "c <port number> <host ip address>" and then click "Apply".
   After setting the command line argument, click "Local Windows Debugger" with solution configuration set as "Debug" and solution platforms set as "Win32" to start client.
5. To run as host: Right click on the project in "Solution Explorer", Select "Property->Configuration Properties->Debugging" and set the "Command Arguments" as 
   "h <port number>" and then click "Apply".
   After setting the command line argument, click "Local Windows Debugger" with solution configuration set as "Debug" and solution platforms set as "Win32" to start host.

The "Debug" directory will appear after running "Buile->Build Solution"

The port number and host ip address are mandatory command line arguments.

The directory structure looks like the following: 
Documents
└── Visual Studio 2015
    └── Projects
        ├── dragonfly
	├── SFML-2.3
	└── game0
            └── vs-2013
                ├── source codes
		└── Debug
		    ├── sounds
	    	    ├── sprites
	            ├── df-font.ttf
	            ├── df-config.txt
                    └── other files and .exe file

The game will start immediately after client is connected. 
The game will end when one of the hero dies or a player press "Q". If this is the case, the end game splash screen will appear.
The game will also end when one of the players close the window. 

For extra points, the end game splash screen was added and the nuke operation on both host side and client side was added.
The end game splash screen was included in "GameOver.h" and "GameOver.cpp" files.
The nuke operation was included in "EventNuke.h" and "EventNuke.cpp". Handling nuke operation and nuke count update was included in "Host.cpp".





