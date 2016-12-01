.PHONY = all stidiff sticopy

all: stidiff sticopy

stidiff: 
	g++ -o sti sti.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc 

sticopy:
	g++ -o copy copy.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc 
