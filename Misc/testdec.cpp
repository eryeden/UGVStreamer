#include "v4l2cap.hpp"
#include "dh264.hpp"
#include "reciever.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

extern "C"{
#include <SDL.h>
}

int main(){
  int i = 0;
  v4l2cap vflc;
  dH264 dec;
  char fname[256];

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *fTexture;
  SDL_Rect displayrect;
  SDL_Event event;

  SDL_Color * color;
  uint8_t *src;
  uint32_t *dst;
  int row, col;
  void * pixels;
  int pitch;

  int ret = 0;
  
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL initialization failed: %s", SDL_GetError());
    return 0;
  }
  
  
  window = SDL_CreateWindow("Moose Hello",
  			    SDL_WINDOWPOS_UNDEFINED,
  			    SDL_WINDOWPOS_UNDEFINED,
  			    640, 480,
  			    SDL_WINDOW_SHOWN);
  if (!window) {
    printf("Could not create window: %s\n", SDL_GetError());
    return 0;
  }
  

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 0;
  }

  fTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
  			       SDL_TEXTUREACCESS_STREAMING, 640, 480);
  if (!fTexture) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return 0;
  }



  dec.initialize_decoder();

  vflc.initialize("/dev/video1", v4l2cap::IMGSIZE_VGA,
		  v4l2cap::IMGFMT_H264, v4l2cap::FRATE_30);
  vflc.startcap();


  for(i = 0; i < 200; i++){
    if(vflc.readstreamb() == 1){
      printf("omsize: %d\n", vflc.outimgsize);
      if(dec.collectFrame(vflc.outimg, vflc.outimgsize) == true){

	ret =  SDL_UpdateYUVTexture(fTexture, NULL,
				    dec.outyuv->data[0],
				    dec.outyuv->linesize[0] * sizeof(uint8_t),
				    dec.outyuv->data[1],
				    dec.outyuv->linesize[1] * sizeof(uint8_t),
				    dec.outyuv->data[2],
				    dec.outyuv->linesize[2] * sizeof(uint8_t));
	
	if(ret != 0){
	  printf("Failed to update texture: %s", SDL_GetError());
	}
	
	ret = SDL_RenderClear(renderer);
	if(ret != 0){
	  printf("Failed to clear render: %s", SDL_GetError());
	}
	
	ret = SDL_RenderCopy(renderer, fTexture, NULL, NULL);
	if(ret != 0){
	  printf("Failed to copy render: %s", SDL_GetError());
	}
	
	SDL_RenderPresent(renderer);
      }
      
    }
  }

  SDL_DestroyRenderer(renderer);
  
  vflc.stopcap();
  vflc.closecam();
  
  
  return 1;
}

















