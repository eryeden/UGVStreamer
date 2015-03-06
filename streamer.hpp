#ifndef _STREAMER_H_
#define _STREAMER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <pthread.h>

#include <vector>


class Streamer{
public:

  Streamer();
  ~Streamer();
  
  void initialize(const char * ipaddr, const int port);
  
  void send_split(const uint8_t *src, const int& size_src, const int& maxsize);

private:
  void parse_nalunit(const uint8_t *inframe, const int insize, int *nal_sidx, int *nal_size, int *nal_num);

  int send_data(const uint8_t * src, const int size_src);

  int sock;
  struct sockaddr_in addr;

  bool is_inited;


  
};


#endif /* _STREAMER_H_ */






