
includedir = ./include
srcdir = ./src
objdir = ./obj

dir_fileutils = ${srcdir}/fileUtils
dir_restree = ${srcdir}/restree
CXXFLAGS = -Wall -O3 -m64 -std=c++11 -pedantic

all: FileStreamer.o lz77.o ResTree.o

FileStreamer.o:
	g++ ${CXXFLAGS} -I ${includedir} -c ${dir_fileutils}/FileStreamer.cpp -o ${objdir}/FileStreamer.o
	
lz77.o:
	g++ ${CXXFLAGS} -I ${includedir} -c ${dir_fileutils}/lz77.cpp -o ${objdir}/lz77.o
	
ResTree.o:
	g++ ${CXXFLAGS} -I ${includedir} -c ${dir_restree}/ResTree.cpp -o ${objdir}/ResTree.o