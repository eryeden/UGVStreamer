CC = gcc
CXX = g++
#libswscale
CFLAGS+=`pkg-config --cflags libavcodec`
CFLAGS+=`pkg-config --cflags libswscale`
CFLAGS+=`sdl2-config --cflags`

#CFLAGS+=-I/usr/local/include -I/usr/local/include/SDL2 -D_REENTRANT

CXXFLAGS+=$(CFLAGS)

LDFLAGS+=`pkg-config --libs libavcodec` 
LDFLAGS+=`pkg-config --libs libswscale` 
LDFLAGS+=`sdl2-config --libs`

#LDFLAGS+=-pthread -L/usr/local/lib -lavcodec -lx264 -lavresample -lavutil -lm -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL2 -lpthread

INCLUDES+=

TARGET = test_rcvdec.bin
SRCS = test_rcvdec dh264 v4l2cap reciever
OBJS = $(addsuffix .o,$(SRCS))

#リンクの依存関係か今のところわからない　そのため、Whole-archiveが必要になる
$(TARGET):  $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
#-Wl,--no-whole-archive
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -g -c $<

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)

.PHONY: deploy
deploy:
	./deploy $(TARGET)

