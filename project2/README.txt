Author: Mengwen Li (mli2)



To compile the project, use command "make".

To clean the project, use command "make all".

To run the server, use command "./server".

To run client, use command "./client <host ip address or host name> <username> <password> <command and corresponding options>".

The hard coded user names and passwords are:

<username> - <password>
	     
a - a
	     
b - b
	     
c - c
	     
d - d
	     
e - e


Assume the server runs on CCCWORK2, a sample client input might be: "./client CCCWORK2 a a ls -la".

The output for this command will be as following:


total 245
drwxrws--x 2 mli2 mli2   151 Feb  2 23:25 .
drwxrws--x 4 mli2 mli2    52 Jan 31 23:33 ..
-rwxrwx--x 1 mli2 mli2 11563 Feb  2 23:23 client
-rw-r--r-- 1 mli2 mli2  4629 Feb  2 23:20 client.c
-rw-r--r-- 1 mli2 mli2   153 Jan 31 11:35 Makefile
-rwxrwx--x 1 mli2 mli2     0 Feb  2 23:33 out.txt
-rwxrwx--x 1 mli2 mli2 13452 Feb  2 23:23 server
-rw-r--r-- 1 mli2 mli2  6050 Feb  2 23:20 server.c


If the command has no return string, such as "sleep", the client will not print out anything and will terminate after the server finish executing the command.


The createFile.sh, removeFile.sh and experiment.sh was used for doing experiments.
