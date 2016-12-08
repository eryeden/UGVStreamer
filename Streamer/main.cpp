//
// Created by 菊池和気 on 2016/12/08.
//


#include "streamer.hpp"
#include "v4l2cap.hpp"

int main() {

  v4l2cap cap;

  Streamer sm;

  sm.initialize("172.20.36.160", 12345);

//  cap.initialize("/dev/video0", v4l2cap::IMGSIZE_VGA, v4l2cap::IMGFMT_H264, v4l2cap::FRATE_30);
  cap.initialize("/dev/video0",
				 v4l2cap::IMGSIZE_HD,
				 v4l2cap::IMGFMT_H264,
				 v4l2cap::FRATE_30);

  cap.startcap();


//  for(int i = 0; i < 50; ++i){
  for (;;) {
	int ret = cap.readstreamb();

	printf("read : %d, %d\n", (int) cap.outimgsize, ret);

	sm.send_split(cap.outimg, cap.outimgsize, 60000);

  }

  cap.stopcap();
  cap.closecam();

  return 1;

}




