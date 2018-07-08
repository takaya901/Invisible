// リンカーの入力を設定
// デバッグとリリースで入力するファイルが異なるのでこのようにしています。
#ifdef _DEBUG
#define CV_EXT "d.lib"
#else
#define CV_EXT ".lib"
#endif
#pragma comment(lib, "opencv_core330" CV_EXT)
#pragma comment(lib, "opencv_highgui330" CV_EXT)
#pragma comment(lib, "opencv_imgproc330" CV_EXT)
#pragma comment(lib, "opencv_videoio330" CV_EXT)

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat input_img;
Mat back_img;
Mat hsv_skin_img = Mat(Size(640,480),CV_8UC3);
Mat smooth_img;
Mat hsv_img;
Mat result_img;

int main(int argc, char *argv[])
{
	VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	if (!cap.isOpened()) {
		printf("Can't detect a camera.");
		return -1;
	}

	cap >> back_img;
	namedWindow("invisible", CV_WINDOW_AUTOSIZE);
	namedWindow("back_img", CV_WINDOW_AUTOSIZE);

	while(1)
	{
		hsv_skin_img = Scalar(0,0,0);

		cap >> input_img;
		//medianBlur(input_img,smooth_img,7);	//ノイズがあるので平滑化
		cvtColor(input_img, hsv_img, CV_BGR2HSV);	//HSVに変換

		for(int y=0; y<480;y++){
			for(int x=0; x<640; x++)
			{
				int a = hsv_img.step*y+(x*3);
				int b = back_img.step*y+(x*3);
				if( hsv_img.data[a] >=0 && hsv_img.data[a] <=15 && hsv_img.data[a+1] >=50 && hsv_img.data[a+2] >= 50 || hsv_img.data[a+2] <= 50 ) //detect skin
				{
					hsv_skin_img.data[a] = back_img.data[b];
					hsv_skin_img.data[a+1] = back_img.data[b+1];
					hsv_skin_img.data[a+2] = back_img.data[b+2];
				}
				else {
					hsv_skin_img.data[a] = input_img.data[a];
					hsv_skin_img.data[a+1] = input_img.data[a+1];
					hsv_skin_img.data[a+2] = input_img.data[a+2];
				}
			}
		}

		imshow("invisible", hsv_skin_img);
		imshow("back_img", back_img);
		if (waitKey(30) >= 0)
			break;
	}
}