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

#include "v4l2cap.hpp"

//Constants initialization
const int v4l2cap::IMGSIZE_SET[][2] = {
  {0, 0},
  {320, 240},
  {640, 480},
  {800, 600},
  {1024, 768},
  {1280, 720},
  {1280, 1024},
  {1600, 900},
  {1920, 1080}
  
};

const int v4l2cap::IMGFMT_SET[] = {
  -1,
  V4L2_PIX_FMT_YUYV,
  V4L2_PIX_FMT_MPEG,
  V4L2_PIX_FMT_H264
};

const int v4l2cap::FRATE_SET[] = {
  0,
  5,
  10, 
  15,
  20,
  24,
  30,
  60,
  120,
  240
};


 //Flow
  /*
    #1 opencam
    #2 setcam
    #3 startcap
    #4 readstream
    
    IF chage the setting of capturing camera, do the following
    ##1 stopcap
    ##2 closecam
    ##3 setcam
    ##4 startcap
    ##5 readstream

    IF you want to stop capturing, do the following
    ##1 stopcap
    ##2 resetcap

    IF you want to restart capturing, do the following,
    ##1 setcam
    ##2 startcap
    ##3 readstream

   */



//初期化　各変数の初期化をする
v4l2cap::v4l2cap(){
  imagesize = 0;
  imageformat = 0;
  framerate = 0;
  cam_isopened = 0;
  fd = -1;
  buffers = NULL;
  n_buffers = 0;
  outimgsize = 0;
  outimg = NULL;
  
  return;
}


v4l2cap::~v4l2cap(){
  
}

//デバグ用出力関数
#define V4L2CAPDEBUG
void v4l2cap::debugout(char *str){
  #ifdef V4L2CAPDEBUG
  printf("%s\n", str);
  #endif
  return;
}

template <typename T>
inline void v4l2cap::CLEAR(T x){
  memset((void *)&x, 0, sizeof(x));
}

int v4l2cap::xioctl(int _fd, int request, void *arg){
    int r;
  do{ r = ioctl(_fd, request, arg); }
  while(-1 == r && EINTR == errno);
  return r;
}

void v4l2cap::errno_exit(const char *s){
  fprintf(stderr, "%s error %d, %s\n",s, errno, strerror(errno));  
  exit(EXIT_FAILURE);
}

//メモリーマッピング初期化
int8_t v4l2cap::init_mmap(){
  struct v4l2_requestbuffers req;
  
  CLEAR(req);
  
  req.count               = 8;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_MMAP;
  
  if(-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
      if(EINVAL == errno)
	{
	  fprintf(stderr, "%s does not support "
		  "memory mapping\n", dev_name);
	  printf("%s does not support "
		 "memory mapping\n", dev_name);
	  return -1;
	  //exit(EXIT_FAILURE);
	}
      else
	{
	  printf("VIDIOC_REQBUFS\n");
	  return -1;
	  //errno_exit("VIDIOC_REQBUFS");
	}
    }
  if(req.count < 2)
    {
      fprintf(stderr, "Insufficient buffer memory on %s\n",
	      dev_name);
      printf("Insufficient buffer memory on %s\n",
	     dev_name);
      return -1;
      //exit(EXIT_FAILURE);
    }
  buffers = (buffer *)calloc(req.count, sizeof(*buffers));
  
  if(!buffers)
    {
      fprintf(stderr, "Out of memory\n");
      printf("Out of memory\n");
      return -1;
      //exit(EXIT_FAILURE);
    }
  
  for(n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
      struct v4l2_buffer buf;
      CLEAR(buf);
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = n_buffers;
      
      if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)){
	printf("VIDIOC_QUERYBUF\n");
	return -1;
	//errno_exit("VIDIOC_QUERYBUF");
      }
    
      buffers[n_buffers].length = buf.length;
      buffers[n_buffers].start =
	mmap(NULL /* start anywhere */,
	     buf.length,
	     PROT_READ | PROT_WRITE /* required */,
	     MAP_SHARED /* recommended */,
	     fd, buf.m.offset);
      
      if(MAP_FAILED == buffers[n_buffers].start){
	printf("mmap\n");
	return -1;
	//errno_exit("mmap");
      }
    }


  return 1;
}

//メモリーマッピング開放
int8_t v4l2cap::deinit_mmap(){
  unsigned int i = 0;
  
  printf("free Mapped memory\n");
  for(i = 0; i < n_buffers; i++){
    munmap(buffers[i].start, buffers[i].length);
  }

  free(buffers);
  return 1;
}


/*
  オープン成功で１、失敗で−１をリターン
*/
int8_t v4l2cap::opencam(char * devname){
  struct stat st;
  if(-1 == stat(devname, &st))
    {
      fprintf(stderr, "Cannot identify '%s': %d, %s\n",
	      devname, errno, strerror(errno));
      return -1;
    }
  
  if(!S_ISCHR(st.st_mode))
    {
      fprintf(stderr, "%s is no device\n", devname);
      return -1;
    }
  fd = open(devname, O_RDWR /* required */ | O_NONBLOCK, 0);
  if(-1 == fd)
    {
      fprintf(stderr, "Cannot open '%s': %d, %s\n",
	      devname, errno, strerror(errno));
      return -1;
    }

  cam_isopened = 1;

  strcpy(dev_name, devname);
  return 1;
}

int8_t v4l2cap::setcam(int imgsize, int imgfmt, int frate){
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  struct v4l2_streamparm strp;

  if(-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
      if(EINVAL == errno)
	{
	  fprintf(stderr, "%s is no V4L2 device\n",dev_name);
	  printf("%s is no V4L2 device\n",dev_name);
	  return -1;
	  //exit(EXIT_FAILURE);
	}
      else
	{
	  printf("VIDIOC_QUERYCAP\n");
	  return -1;
	  //errno_exit("VIDIOC_QUERYCAP");
	}
    }
  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
      fprintf(stderr, "%s is no video capture device\n",dev_name);
      printf("%s is no video capture device\n",dev_name);
      //exit(EXIT_FAILURE);
      return -1;
    }
  if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
      fprintf(stderr, "%s does not support streaming i/o\n",
	      dev_name);
      printf("%s does not support streaming i/o\n", dev_name);
      return -1;
      //exit(EXIT_FAILURE);
    }

  /* Select video input, video standard and tune here. */
  CLEAR(cropcap);
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if(0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
      crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      crop.c = cropcap.defrect; /* reset to default */
      
      if(-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
	{
	  switch(errno)
	    {
	    case EINVAL:
	      /* Cropping not supported. */
	      break;
	    default:
	      /* Errors ignored. */
	      break;
	    }
	}
    }
  else
    {	
      /* Errors ignored. */
    }

  //Set Image format
  CLEAR(fmt);

  imagesize = imgsize;
  imageformat = imgfmt;
  framerate = frate;

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = IMGSIZE_SET[imgsize][0];
  fmt.fmt.pix.height      = IMGSIZE_SET[imgsize][1];
  fmt.fmt.pix.pixelformat = IMGFMT_SET[imgfmt];
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  
  if(-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)){ 
    printf("VIDIOC_S_FMT\n");
    return -1;
    //errno_exit("VIDIOC_S_FMT");
  }

  printf("SET:W:%d,H:%d,FMT:%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height, 
	 fmt.fmt.pix.pixelformat);

  memset(&strp, 0, sizeof(struct v4l2_streamparm));
  strp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  strp.parm.capture.timeperframe.numerator = 1;
  strp.parm.capture.timeperframe.denominator = FRATE_SET[frate];
  if(-1 == xioctl(fd, VIDIOC_S_PARM, &strp)){
    printf("VIDIOC_S_PARAM:fr\n");
    return -1;
    //errno_exit("VIDIOC_S_PARAM:fr");
  }


  //Initialization of mmap
  init_mmap();

  return 1;
}

//二回目の設定に使う
int8_t v4l2cap::settingcam(int imgsize, int imgfmt, int frate){
  struct v4l2_format fmt;
  struct v4l2_streamparm strp;

  imagesize = imgsize;
  imageformat = imgfmt;
  framerate = frate;

  //Set Image format
  CLEAR(fmt);

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = IMGSIZE_SET[imgsize][0];
  fmt.fmt.pix.height      = IMGSIZE_SET[imgsize][1];
  fmt.fmt.pix.pixelformat = IMGFMT_SET[imgfmt];
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  
  if(-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)){ 
    printf("VIDIOC_S_FMT\n");
    return -1;
    //errno_exit("VIDIOC_S_FMT");
  }

  printf("SET:W:%d,H:%d,FMT:%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height, 
	 fmt.fmt.pix.pixelformat);

  memset(&strp, 0, sizeof(struct v4l2_streamparm));
  strp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  strp.parm.capture.timeperframe.numerator = 1;
  strp.parm.capture.timeperframe.denominator = FRATE_SET[frate];
  if(-1 == xioctl(fd, VIDIOC_S_PARM, &strp)){
    printf("VIDIOC_S_PARAM:fr\n");
    return -1;
    //errno_exit("VIDIOC_S_PARAM:fr");
  }

  //Initialization of mmap
  init_mmap();  
  return 1;
}


//カメラのクローズ
int8_t v4l2cap::closecam(){
  if(cam_isopened == 1){
    cam_isopened = 0;
    deinit_mmap();
    if(-1 == close(fd)){
      printf("close\n");
      return -1;
    }
    fd = -1;
  }
  return 1;

}

//カメラのリセット　マップしたメモリの開放もする
//カメラの設定変更は何もしていない
int8_t v4l2cap::resetcam(){
  //stopcap();
  //これだけでいいかわからない
  printf("resetcam\n");
  deinit_mmap();
  return 1;
}


//キャプチャ開始
//セレクト使用でキャプチャ可能まで待機
int8_t v4l2cap::startcap(){
  unsigned int i;
  enum v4l2_buf_type type;
  fd_set fds;
  struct timeval tv;
  int r;

  
  for(i = 0; i < n_buffers; ++i)
    {
      struct v4l2_buffer buf;
	CLEAR(buf);
	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = i;
	
	if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
	  printf("VIDIOC_QBUF\n");
	  return -1;
	  //errno_exit("VIDIOC_QBUF");
	}
    }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(-1 == xioctl(fd, VIDIOC_STREAMON, &type)){
    printf("VIDIOC_STREAMON\n");
    return -1;
    //errno_exit("VIDIOC_STREAMON");
  }

  //使用可能になるまで待機
  for(;;){
    //clear fd_set
    FD_ZERO(&fds);
    //set the fd th bit
    FD_SET(fd, &fds);
    /* Timeout. */
    //Wait for the finish of webcam I/O settings
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    r = select(fd + 1, &fds, NULL, NULL, &tv);
    if(-1 == r){
      if(EINTR == errno)
	continue;
      printf("select\n");
      return -1;
      //errno_exit("select");
    }
    if(0 == r){
      //timeout webcam is estimated not to finish I/O seting
      fprintf(stderr, "select timeout\n");
      printf("select timesout\n");
      return -1;
      //exit(EXIT_FAILURE);
    }
    break;
    /* EAGAIN - continue select loop. */
  }
  //出力領域確保
  outimg = (uint8_t *)malloc(buffers[0].length);

  return 1;
}

//キャプチャ停止
int8_t v4l2cap::stopcap(){
  
  enum v4l2_buf_type type;
  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)){
    printf("VIDIOC_STREAMOFF\n");
    return -1;
    //errno_exit("VIDIOC_STREAMOFF");
  }

  return 1;
}

//ストリームから読み、保存
int8_t v4l2cap::readstream(){
  struct v4l2_buffer buf;

  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  
  // if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
  //   printf("VIDIOC_QBUF\n");
  //   //return -1;
  //   //errno_exit("VIDIOC_QBUF");
  //   //printf("failed to QBUF\n");
  //   //return -1;
  // }
  
  if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf)){
    switch(errno)
      {
      case EAGAIN:
	return -1;
      case EIO:
	/* Could ignore EIO, see spec. */
	/* fall through */
	return -1;
      default:
	printf("VIDIOC_DQBUF\n");
	return -1;
	//errno_exit("VIDIOC_DQBUF");
      }
  }
  assert(buf.index < n_buffers);

  //出力バッファをコピー
  outimgsize = buf.bytesused;
  //outimgsize = buffers[0].length;
  printf("bsize %d\n", buf.bytesused);
  memcpy((void *)outimg, (void *)buffers[buf.index].start, buf.bytesused);

  if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
    printf("VIDIOC_QBUF\n");
    //return -1;
    //errno_exit("VIDIOC_QBUF");
    //printf("failed to QBUF\n");
    //return -1;
  }

  
  return 1;
}

//ブロックするストリームリーダー
int8_t v4l2cap::readstreamb(){
  fd_set fds;
  struct timeval tv;
  int r;
  int ret = 0;
  
  //使用可能になるまで待機
  for(;;){
    //clear fd_set
    FD_ZERO(&fds);
    //set the fd th bit
    FD_SET(fd, &fds);
    /* Timeout. */
    //Wait for the finish of webcam I/O settings
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    r = select(fd + 1, &fds, NULL, NULL, &tv);
    if(-1 == r){
      if(EINTR == errno)
	continue;
      printf("select\n");
      return -1;
    }
    if(0 == r){
      //timeout webcam is estimated not to finish I/O seting
      fprintf(stderr, "select timeout\n");
      printf("select timesout\n");
      return -1;
    }
    //読み込み
    ret = readstream();
    break;
    /* EAGAIN - continue select loop. */
  }

  return ret;
}

//カメラ起動
int8_t v4l2cap::initialize(char *devname, int imgsize, int imgfmt, int frate){
  if(-1 == opencam(devname)){
    printf("Failed to open camera device\n");
    return -1;
  }

  if(-1 == setcam(imgsize, imgfmt, frate)){
    printf("Failed to apply camera device settings\n");
    return -1;
  }
  return 1;
}

//動いてるカメラの設定変更
int8_t v4l2cap::changesettings(int imgsize, int imgfmt, int frate){
  int ret = 1;
  if(cam_isopened == 1){
    ret = stopcap();
    ret = closecam();
    ret = opencam(dev_name);
    ret = setcam(imgsize, imgfmt, frate);
    ret = startcap();
  }else{
    printf("Camera is not opened\n");
    return -2;
  }
  return ret;
}
















