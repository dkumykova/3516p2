To run the program:

cd into the folder containing project2.c, project2.h, student2.c and the makefile
run "make"
run ./p2 [number of messages] [corruption probability] [out of order probability] [packet loss probability]
    [average time between messages] [tracing level] [randomization or not] [bi-directional or not] 
	into command line, or fill in prompts as they pop up


Notes:

This program uses the crc32 function from the zlib library in order to generate checksum values based on
the message data, sequence number and ack number for each packet.

To properly implement the full program with error detection and a 1 packet window, a message queue needs
to be created in order to keep track of messages that have been sent to layer 4 but are waiting for the 
window that allows packets to be transmitted between entities a and b. Using this method only 1 packet would
be in transit at a time.