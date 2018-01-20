CC=clang++
CFLAGS= -std=c++11 -lpthread 
SRCS=main.cpp
OUT=main
OPENCV=`pkg-config opencv --cflags --libs`
LIBS=$(OPENCV)
$(OUT):$(SRCS)
	$(CC) $(CFLAGS) -o $(OUT) $(SRCS) $(LIBS)
