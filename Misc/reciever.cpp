#include "reciever.hpp"

Reciever::Reciever()
  :is_allocated(false),
   is_inited(false),
   outbuff(NULL),
   size_outbuff(0)
{
  ;
}

Reciever::~Reciever(){
  if(is_inited){
    close(sock);
  }

  if(is_allocated){
    free(outbuff);
  }
}



void Reciever::initailize(const int port){

  sock = socket(AF_INET, SOCK_DGRAM, 0);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
    
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));

  is_inited = true;
  
  return;
}

void Reciever::recieve_b(){
  uint32_t rcvsize = 0;
  int ret;
  uint8_t *tmpbuff = NULL;
  
  if(!is_inited){
    printf("Sockets have not initialized\n");
    return;
  }

  //MGS_PEEKではじめの４バイトだけ読む
  if(recv(sock, &rcvsize, sizeof(uint32_t), MSG_PEEK)){

    if(is_allocated){
      free(outbuff);
    }

    size_outbuff = (int)rcvsize;

    tmpbuff = (uint8_t *)malloc((int)rcvsize
				+ sizeof(uint32_t));
    
    outbuff = (uint8_t *)malloc((int)rcvsize);
    is_allocated = true;
    
    printf("RECV: %d bytes allcated\n", (int)rcvsize);
    
    ret = recv(sock, 
	       tmpbuff, 
	       sizeof(uint32_t) + (int)rcvsize, 
	       0);
    if(ret < 0){
      perror("Failed to recieve");
    }else{
      //get body
      memcpy(outbuff,
	     &(tmpbuff[sizeof(uint32_t)]),
	     size_outbuff);

      
      
      printf("RECV: %d bytes recieved\n", ret);
    }

    free(tmpbuff);
    
  }else{
    perror("MSG_PEEK");
  }
    
}

void Reciever::recieve_idx(){
  uint32_t rcvsize = 0;
  int ret;
  uint8_t idx = 0;
  uint8_t *tmpbuff = NULL;
  
  if(!is_inited){
    printf("Sockets have not initialized\n");
    return;
  }

  //MGS_PEEKではじめの４バイトだけ読む
  if(recv(sock, &rcvsize, sizeof(uint32_t), MSG_PEEK)){

    if(is_allocated){
      free(outbuff);
    }

    size_outbuff = (int)rcvsize;
    
    tmpbuff = (uint8_t *)malloc((int)rcvsize
				+ sizeof(uint32_t) + sizeof(uint8_t));

    outbuff = (uint8_t *)malloc((int)rcvsize);
    is_allocated = true;
    
    printf("RECV: %d bytes allcated\n", (int)rcvsize);
    
    ret = recv(sock, 
	       tmpbuff, 
	       sizeof(uint32_t) + sizeof(uint8_t) + (int)rcvsize, 
	       0);
    if(ret < 0){
      perror("Failed to recieve");
    }else{
      //get idx
      memcpy(&idx, &(tmpbuff[sizeof(uint32_t)]), sizeof(uint8_t));

      //get body
      memcpy(outbuff,
	     &(tmpbuff[sizeof(uint32_t) + sizeof(uint8_t)]),
	     size_outbuff);
      
      printf("RECV: %d bytes recieved, idx:%d\n", ret, (int)idx);
    }

    free(tmpbuff);
  }else{
    perror("MSG_PEEK");
  }

  
    
}















