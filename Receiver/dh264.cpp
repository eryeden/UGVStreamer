#include "dh264.hpp"




dH264::dH264()
  :ccount(0),
   codec(NULL),
   codec_context(NULL),
   parser(NULL),
   sws_context(NULL),
   frame_timeout(0),
   frame_delay(0)
{
  //Register all codecs
  avcodec_register_all();
}


dH264::~dH264(){
  if(parser){
    av_parser_close(parser);
    parser = NULL;
  }

  if(codec_context){
    avcodec_close(codec_context);
    av_free(codec_context);
    codec_context = NULL;
  }

  if(picture){
    av_free(picture);
    picture = NULL;
  }

  if(outyuv){
    av_free(outyuv);
    outyuv = NULL;
  }  
}

bool dH264::initialize_decoder(){

  //Search specified decoder
  codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if(!codec){
    printf("INIT: cannot find the h264 codec.\n");
    return false;
  }

  //Get decoder context
  codec_context = avcodec_alloc_context3(codec);

  if(codec->capabilities & CODEC_CAP_TRUNCATED){
    codec_context->flags |= CODEC_FLAG_TRUNCATED;
  }

  //Oepn codec
  if(avcodec_open2(codec_context, codec, NULL) < 0){
    printf("INIT: could not open codec.\n");
    return false;
  }

  //Allocate memory space for decoded frame
  picture = av_frame_alloc();
  outyuv = av_frame_alloc();
  outrgb = av_frame_alloc();

  buffyuv = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,
													 Constants::WIDTH,
													 Constants::HEIGHT));
  avpicture_fill((AVPicture *) outyuv,
				 buffyuv,
				 AV_PIX_FMT_YUV420P,
				 Constants::WIDTH,
				 Constants::HEIGHT);

  buffrgb = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24,
													 Constants::WIDTH,
													 Constants::HEIGHT));
  avpicture_fill((AVPicture *) outrgb,
				 buffrgb,
				 AV_PIX_FMT_RGB24,
				 Constants::WIDTH,
				 Constants::HEIGHT);

  
  //Initialize H264 parser
  parser = av_parser_init(AV_CODEC_ID_H264);

  if(!parser){
    printf("INIT: cannot create H264 parser.\n");
    return false;
  }

  printf("INIT: decodeer initialized\n");
  return true;
}


//retruns ture if the parser collected all of nalunits of one frame, and decoded frame will be outptued
//returns false if the parser dose not finished colllecting all naulunits
bool dH264::collectFrame(uint8_t *src, int srcsize){
  //bool iscollected = false;
  
  if((srcsize <= 0) || (src == NULL)){
    return false;
  }

  //Collect nal unit into buffer
  std::copy(src, src + srcsize, std::back_inserter(buffer));

  uint8_t * data = NULL;
  int size = 0;
  int len = av_parser_parse2(parser,
			     codec_context,
			     &data, &size,
			     &buffer[0], buffer.size(),
			     0, 0, AV_NOPTS_VALUE);

  //he have not collected all of nalunits.
  if(size == 0 && len >= 0){
    return false;
  }

  //he have collected all of nalunits.
  if(len){

    //call decoder
    decodeFrame(&buffer[0], size);

    //Reset decoder to avoid delay
    if(ccount == 500){
      avcodec_flush_buffers(codec_context);
      ccount = 0;
    }
    ccount++;

    //Erase stored nalunit to prepaer next frame.
    buffer.erase(buffer.begin(), buffer.begin() + len);
    return true;
  }
  
  return false;
}


bool dH264::decodeFrame(uint8_t *src, int srcsize){

  AVPacket pkt;
  int got_picture = 0;
  int len = 0;

  av_init_packet(&pkt);

  pkt.data = src;
  pkt.size = srcsize;


  len = avcodec_decode_video2(codec_context, picture, &got_picture, &pkt);
  if(len < 0){
    printf("DECODE: Error while decoding a frame\n");
  }

  if(got_picture == 0){
    printf("DECODE: have not got picture yet\n");
    return false;
  }

  // //rgb settings
  // sws_context = sws_getContext(codec_context->width, codec_context->height,
  // 			       codec_context->pix_fmt,
  // 			       codec_context->width, codec_context->height,
  // 			       PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
 
  
  // //RGB output
  // sws_scale(sws_context, (uint8_t const * const *)picture->data,
  // 	    picture->linesize, 0, codec_context->height,
  // 	    outrgb->data, outrgb->linesize);

  
  //YUV420P settings
  sws_context = sws_getContext(codec_context->width, codec_context->height,
							   codec_context->pix_fmt,
							   codec_context->width, codec_context->height,
							   AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
 
  
  //YUV420P output
  sws_scale(sws_context, (uint8_t const * const *)picture->data,
  	    picture->linesize, 0, codec_context->height,
  	    outyuv->data, outyuv->linesize);

  
  //printf("DECODE: decoded ss:%d, os:%d\n", srcsize, len);
  //printf("DECODE: w:%d, h:%d, ls:%d\n", outrgb->width, outrgb->height, outrgb->linesize[0]);

  return true;
}




void dH264::saveframergb(char *fname){
  FILE *pFile;
  int y;

  if(outrgb == NULL){
    printf("Outputs have not initialized yet\n");
    return;
  }

  //OpenFile
  pFile = fopen(fname, "wb");
  if(pFile == NULL){
    printf("Could not open file\n");
    return;
  }

  //Write header
  fprintf(pFile, "P6\n%d %d\n255\n", codec_context->width, codec_context->height);

  //Write picel data
  for(y = 0; y < codec_context->height; y++){
    fwrite(outrgb->data[0] + y * outrgb->linesize[0],
	   1, codec_context->width * 3, pFile);
  }

  //Close file
  fclose(pFile);
  
  return;
}

void dH264::saveframeyuv(char *fname){
  FILE *pFile;
  int y;
  
  if(outyuv == NULL){
    printf("Outputs have not initialized yet\n");
    return;
  }

  //OpenFile
  pFile = fopen(fname, "wb");
  if(pFile == NULL){
    printf("Could not open file\n");
    return;
  }

  //Write header
  //fprintf(pFile, "P6\n%d %d\n255\n", codec_context->width, codec_context->height);

  //Write picel data
  // for(y = 0; y < codec_context->height; y++){
  //   fwrite(outyuv->data[0] + y * outyuv->linesize[0],
  // 	   1, codec_context->width * 3, pFile);
  // }

  //Write Y
  fwrite(outyuv->data[0], 1,
	 outyuv->linesize[0] * sizeof(uint8_t) * codec_context->height, pFile);
  //Write Y
  fwrite(outyuv->data[1], 1,
	 outyuv->linesize[1] * sizeof(uint8_t)* codec_context->height, pFile);
  //Write V
  fwrite(outyuv->data[2], 1,
	 outyuv->linesize[2] * sizeof(uint8_t)* codec_context->height, pFile);

  //Close file
  fclose(pFile);
  
  return;
}


void dH264::copyyuvframe(uint8_t *out){

  //Copy Y data
  memcpy((void *)out, outyuv->data[0],
	 outyuv->linesize[0] * codec_context->height);

  //Copy U data
  memcpy((void *)(out + outyuv->linesize[0] * codec_context->height),
	 outyuv->data[1],
	 outyuv->linesize[1] * codec_context->height);

  //Copy V data
  memcpy((void *)(out
		  + outyuv->linesize[0] * codec_context->height
		  + outyuv->linesize[1] * codec_context->height),
	 outyuv->data[2],
	 outyuv->linesize[2] * codec_context->height);
  
  return;
}

void dH264::copyrgbframe(uint8_t *out){

  //Copy Y data
  memcpy((void *)out, outrgb->data[0],
	 outrgb->linesize[0] * codec_context->height);
  
  return;
}

















