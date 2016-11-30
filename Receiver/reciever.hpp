#ifndef _RECIEVER_H_
#define _RECIEVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
//#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//#include <asm/types.h>

#include <pthread.h>

#include <vector>


class Reciever{
public:
  Reciever();
  ~Reciever();

  void initailize(const int port);
  void recieve_b();

  void recieve_idx();
  
private:
  int sock;
  struct sockaddr_in addr;

  bool is_allocated;
  bool is_inited;

public:  
  uint8_t *outbuff;
  int size_outbuff;
  
};



#endif /* _RECIEVER_H_ */
