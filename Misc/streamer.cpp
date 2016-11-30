#include "streamer.hpp"


Streamer::Streamer()
  :is_inited(false)
{
  return;
}


Streamer::~Streamer(){
  if(!is_inited){
    close(sock);
  }

  return;
}


void Streamer::initialize(const char *ipaddr, const int port){
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ipaddr);

  is_inited = true;

  printf("Connection established: %s:%d\n", ipaddr, port);
  
  return;
}



//H264 nal unit parser
/*
  #nal_sidx will contains the start index of nalunit
  #nal-size will contains the size of indexed nalunit
*/
void Streamer::parse_nalunit(const uint8_t *inframe, const int insize, 
		    int *nal_sidx, int *nal_size, int *nal_num){
  int readindex = 0;
  int nalindex = 0;
  int i;

  for(readindex = 0; readindex < insize; readindex++){
    if(inframe[readindex] == 0x00){
      if(insize - readindex > 2){
	if((inframe[readindex + 1] == 0x00) 
	   && (inframe[readindex + 2] == 0x00) 
	   && (inframe[readindex + 3] == 0x01)){
	  //detect nal unit
	  nal_sidx[nalindex] = readindex;
	  if(nalindex != 0){
	    nal_size[nalindex - 1] = readindex - nal_sidx[nalindex - 1];
	  }
	  readindex += 3;
	  nalindex++;
	}
      }
    }
  }
  nal_size[nalindex-1] = insize - nal_sidx[nalindex-1];
  *nal_num = nalindex;

    
  // for(i = 0; i < nalindex; i++){
  //   if(nalp_smax < nal_size[i]) nalp_smax = nal_size[i];
  //   if(nalp_smin > nal_size[i]) nalp_smin = nal_size[i];
  //   nalp_ave = (nalp_ave * (double)nal_n + (double)nal_size[i]) / (double)(++nal_n);
  //   printf("nal %d idx: %d, size: %d smax: %d, smin: %d, ave: %f\n", 
  // 	   i, nal_sidx[i], nal_size[i], nalp_smax, nalp_smin, nalp_ave);
  // }

  return ;
}



void Streamer::send_split(const uint8_t *src, const int& size_src, const int& maxsize){

  int i= 0;
  const int MAX_N_O_NALUNIT = 20;
  int nal_sidx[MAX_N_O_NALUNIT];
  int nal_size[MAX_N_O_NALUNIT];
  int no_nalunit = 0;
  //std::vector<uint8_t>buffer;

  int start_idx = 0;

  int size_n = 0;

  //Parse AV packet
  parse_nalunit(src, size_src, nal_sidx, nal_size, &no_nalunit);
  
  //Send all of src if the size of src is less than maxsize
  if(size_src < maxsize){
    //Send data
    send_data(src, size_src);
    return;
  }

  //send data devided
  start_idx = 0;
  size_n = 0;
  for(i = 0; i < no_nalunit; i++){
    if(size_n + nal_size[i] > maxsize){
      //Send data SI: start_idx, EI: end_idx, SIZE: size_n
      send_data(&src[start_idx], size_n);
      
      //Reset size_n for next step
      size_n = nal_size[i];
      //Set start idx
      start_idx = nal_sidx[i];

    }else{
      //Add size_n
      size_n += nal_size[i];

    }
  }

  //Send last nalunit
  send_data(&src[start_idx], size_n);
  
  
  return;
}


//send data to another PC
/*
  Data structure
  
 | 0 | 1 | 2 | 3 | 4 | ....
 |   data size   |   raw h264 nal unit ....
*/
int Streamer::send_data(const uint8_t * src, const int size_src){
  uint8_t * buff = NULL;
  uint32_t size = (uint32_t)size_src;
  int res = 0;
  int packetsize = size_src + sizeof(uint32_t);

  //Allocate data for sender
  buff = (uint8_t *)malloc(sizeof(uint32_t) + size_src);

  //write size data
  memcpy((void *)&buff[0], &size, sizeof(uint32_t));

  //write main data
  memcpy((void *)&buff[sizeof(uint32_t)], src, size_src);

  //send data
  if(is_inited == false){
    printf("Socket have not initialized\n");
    return -4;
  }
  res = sendto(sock, (char *)buff, packetsize, 0, (sockaddr *)&addr, sizeof(struct sockaddr));
  
  //Free buffer space
  free(buff);

  if(res < 0){
    perror("Failed to send data ");
  }else{
    printf("Sent %d bytes\n",res);
  }
  
  return res;
}


















