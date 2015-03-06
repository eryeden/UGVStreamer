#ifndef _DH264_H_
#define _DH264_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}


class dH264{
public:
  dH264();
  ~dH264();

  bool initialize_decoder();
  bool deinitialize_decoder();

  bool collectFrame(uint8_t *src, int srcsize);

  bool decodeFrame(uint8_t *src, int srcsize);

  void saveframergb(char * fname);

  void saveframeyuv(char * fname);

  void copyrgbframe(uint8_t *out);
  void copyyuvframe(uint8_t *out);
  
private:
  uint8_t ccount;
  
  AVCodec* codec; /* the AVCodec* which represents the H264 decoder */

  AVCodecContext* codec_context; /* the context; keeps generic state */

  AVCodecParserContext* parser;
  //parser that is used to decode the h264 bitstream

  struct SwsContext *sws_context;
  

  //AVPicture yuvout;
  
  uint8_t *inbuf; // will contain a encoded picture
  int16_t inbuffsize; //size of encoded picture

  void* cb_user;
  /* the void* with user data that is passed into the set callback */

  uint64_t frame_timeout; /* timeout when we need to parse a new frame */

  uint64_t frame_delay;/* delay between frames (in ns) */

  std::vector<uint8_t> buffer;
  /* buffer we use to keep track of read/unused bitstream data */

public:
  AVFrame* picture; //will contain a decoded picture
  int outbuffsize; //size of decoded picture

  AVFrame* outyuv;
  int outyuvsize;
  uint8_t * buffyuv;

  AVFrame* outrgb;
  int outrgbsize;
  uint8_t * buffrgb;
  
};






#endif /* _DH264_H_ */










