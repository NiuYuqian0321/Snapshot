#include <iostream>
#include <sys/time.h>
using namespace std;

#include<opencv2/opencv.hpp>
using namespace cv;

#include <fcntl.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#define PRE(s) ("$ " s)

#define SHOW_USED_TIME false
#define SHOW_CRNT_FPS  false

double Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}

bool SetExposure(bool bAuto, int i)
{
	char szCameraVideo[16];
	sprintf(szCameraVideo, "/dev/video%d", 0); // Device ID is 0
	
	int iExposure = i; // Exposure
	
	int fdDevice = v4l2_open(szCameraVideo, O_RDWR | O_NONBLOCK);
	if (fdDevice == -1)
	{
		cout << PRE("Camera V4L2 device open failed.") << endl;
	}
	
	struct v4l2_control tV4L2;
	tV4L2.id = V4L2_CID_EXPOSURE_AUTO;
	tV4L2.value = bAuto ? 3 : 1; // Here set the M/A, 1 is manual, 3 is auto
	
	if (v4l2_ioctl(fdDevice, VIDIOC_S_CTRL, &tV4L2) != 0)
	{
		 cout << PRE("Failed to set... ") << strerror(errno) << endl;
		 v4l2_close(fdDevice);
		 return false;
	}
	
	if (!bAuto)
	{
		cout << PRE("Set exposure: ") << iExposure << endl;
		v4l2_set_control(fdDevice, V4L2_CID_EXPOSURE_ABSOLUTE, iExposure);
	}

	v4l2_close(fdDevice);
	
	return true;
}

int main()
{
	static int iExposure = 150;

    VideoCapture cCaptrue(0);

    if (!cCaptrue.isOpened())
    {
		cout << PRE("Fail to open camera capture!") << endl;
		exit(1);
    }

	cCaptrue.set(CV_CAP_PROP_FRAME_WIDTH,  640);
	cCaptrue.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	VideoWriter cRaw("./imgsave/Raw.avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, Size(640, 480), true);

    Mat mFrame;

    while (true)
    {
		double dTimeStart = Timer();

		if (! (cCaptrue.read(mFrame)))
		{
			cout << PRE("Fail to read camera data!") << endl;
			exit(2);
		}

		char szExposure[32] = "Exposure:";		
		sprintf(szExposure, "%s%d", szExposure, iExposure);
		putText(mFrame, szExposure, Point(0, 40), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
			
		cRaw << mFrame;

		imshow("Frame", mFrame);

		static int iLog = 1;

		char cKey = waitKey(1);
		switch (cKey)
		{
		case 'q':
		case 'Q':
			goto FINI;

		case 's':
		case 'S':
			char szFileName[16];
			sprintf(szFileName, "./imgsave/camgrab%d.bmp", iLog);
			imwrite(szFileName, mFrame);
			cout << "Save cam pic: " << szFileName << endl;
			iLog++;
			goto LOOP;
			
		case 'u':
		case 'U':
			iExposure += 100;
			if (!SetExposure(false, iExposure))
			{
				cout << PRE("Set exposure error!") << endl;
				iExposure -= 100;
			}
			goto LOOP;

		case 'i':
		case 'I':
			iExposure += 10;
			if (!SetExposure(false, iExposure))
			{
				cout << PRE("Set exposure error!") << endl;
				iExposure -= 10;
			}
			goto LOOP;

		case 'o':
		case 'O':
			iExposure += 1;
			if (!SetExposure(false, iExposure))
			{
				cout << PRE("Set exposure error!") << endl;
				iExposure -= 1;
			}
			goto LOOP;

		case 'j':
		case 'J':
			iExposure -= 100;
			if (!SetExposure(false, iExposure))
			{
				cout << PRE("Set exposure error!") << endl;
				iExposure += 100;
			}
			goto LOOP;

		case 'k':
		case 'K':
			iExposure -= 10;
			if (!SetExposure(false, iExposure))
			{
				cout << PRE("Set exposure error!") << endl;
				iExposure += 10;
			}
			goto LOOP;

		case 'l':
		case 'L':
			iExposure -= 1;
			if (!SetExposure(false, iExposure))
			{
				cout << PRE("Set exposure error!") << endl;
				iExposure += 1;
			}
			goto LOOP;

		default:
			goto LOOP;
		}

		LOOP:
		; // Must have a ";" because the mark "LOOP"

		#if SHOW_USED_TIME
		cout << PRE("Used time: ") << Timer() - dTimeStart << endl;
		#endif
		
		#if SHOW_CRNT_FPS
		cout << PRE("Current FPS: ") << 1000 / (Timer() - dTimeStart) << endl;
		#endif
    }

	FINI:

	cCaptrue.release();
	
	return 0;
}

