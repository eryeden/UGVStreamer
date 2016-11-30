
#ifndef V4L2CAP
#define V4L2CAP

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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/types.h>

#include <linux/videodev2.h>

#include <pthread.h>

#include <sys/types.h>

/*
  About V4L2cap class
  
  #Open camera device
   Req:device name
  #Set setting
   Req:
   IMAGE SIZE
   IMAGE FORMAT
   FRAMERATE
    
   
  #Change setting
  #Reset camera device

 */

//#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
  void *                  start;
  size_t                  length;
};


class v4l2cap{
public:

  
  //Image size
  static const int IMGSIZE_QVGA = 1;
  static const int IMGSIZE_VGA  = 2;
  static const int IMGSIZE_SVGA = 3;
  static const int IMGSIZE_XGA  = 4;
  static const int IMGSIZE_HD   = 5;
  static const int IMGSIZE_SXGA = 6;
  static const int IMGSIZE_HDP  = 7;
  static const int IMGSIZE_FHD  = 8;

  //Image resolutions
  /*  static const int IMGSIZE_SET[][] = {
    {320, 240},
    {640, 480},
    {800, 600},
    {1024, 768},
    {1280, 720},
    {1280, 1024},
    {1600, 900},
    {1920, 1080}
    };*/
  static const int IMGSIZE_SET[9][2];

  //Image formats
  static const int IMGFMT_YUYV  = 1;
  static const int IMGFMT_MJPEG = 2;
  static const int IMGFMT_H264  = 3;
  
  //Image formats array
  /*  static const int IMGFMT_SET = {
    V4L2_PIX_FMT_YUYV,
    V4L2_PIX_FMT_MPEG,
    V4L2_PIX_FMT_H264
    };*/
  static const int IMGFMT_SET[4];

  //Frame rates
  static const int FRATE_5   = 1;
  static const int FRATE_10  = 2;
  static const int FRATE_15  = 3;
  static const int FRATE_20  = 4;
  static const int FRATE_24  = 5;
  static const int FRATE_30  = 6;
  static const int FRATE_60  = 7;
  static const int FRATE_120 = 8;
  static const int FRATE_240 = 9;

  /*static const int FRATE_SET[9] = {
    5,
    10, 
    15,
    20,
    24,
    30,
    60,
    120,
    240
    };*/
  static const int FRATE_SET[10];

  //Camera settings
  int imagesize;
  int imageformat;
  int framerate;
  int8_t cam_isopened; //Is camera opened

  int fd; // File discriptor for camera device
  struct buffer * buffers;
  unsigned int n_buffers;
  char dev_name[256];

  //出力イメージサイズ
  int outimgsize;
  //出力イメージ
  uint8_t *outimg;

  //Flow
  /*
    #1 opencam
    #2 setcam
    #3 startcap
    #4 readstream
    
    If chage the setting of capturing camera, do the following
    ##1 stopcap
    ##2 closecam
    ##3 opencam
    ##4 setcam
    ##5 startcap
    ##6 readstream

    OR,
    ##1 changesettings

    If you want to stop capturing, do the following
    ##1 stopcap
    If you want to restart capturing, do the following,
    ##1 startcap
    ##2 readstream

   */


  //Constructor
  v4l2cap();
  ~v4l2cap();
  
  //Oepn cam
  int8_t opencam(char * devname); //open v4l2 video device
  //Reset cam settings
  int8_t resetcam(); //reset opened video device, free allocated video memeory space
  //Configuration of camera
  int8_t setcam(int imgsize, int imgfmt, int frate); //

  //二回目の設定に使う
  int8_t settingcam(int imgsize, int imgfmt, int frate);

  //Close cam
  int8_t closecam();
  //Start capturing
  int8_t startcap();
  //Stop capturing
  int8_t stopcap();
  //Read streaming data from camera
  int8_t readstream();
  //Select used reader
  int8_t readstreamb();

  //カメラを起動する
  int8_t initialize(char * devname, int imgsize, int imgfmt, int frate);
  //動いてるカメラの設定変更に使用する
  int8_t changesettings(int imgsize, int imgfmt, int frate);

private:
  //For inclass use
  static int xioctl(int _fd, int request, void * arg);
  static void errno_exit(const char *s);

  int8_t init_mmap();
  int8_t deinit_mmap();

  //デバグ用出力関数
  static void debugout(char *str);

  //テンプレートのテスト #define のかわり
  template <typename T>
  inline void CLEAR(T x);

};



#endif














