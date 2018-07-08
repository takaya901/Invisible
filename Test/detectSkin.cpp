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
#include <random>

using namespace std;
using namespace cv;

Mat input_img;
Mat hsv_skin_img = Mat(Size(640,480),CV_8UC3);
Mat smooth_img;
Mat hsv_img;
Mat result_img;

int back_bgr[3] = {0};

//クリックした画素の8近傍の中央値を取得
void getBackValue( int event, int x, int y, int flags, void *param )
{
	if ( event == CV_EVENT_LBUTTONDOWN )
	{
		int p[9][3];
		int tmp, med[3];

		for(int i=0; i<3; i++)
		{
			p[0][i] = hsv_skin_img.data[3 * (x-1) + hsv_skin_img.step * (y-1) + i];
			p[1][i] = hsv_skin_img.data[3 * (x  ) + hsv_skin_img.step * (y-1) + i];
			p[2][i] = hsv_skin_img.data[3 * (x+1) + hsv_skin_img.step * (y-1) + i];
			p[3][i] = hsv_skin_img.data[3 * (x-1) + hsv_skin_img.step * (y  ) + i];
			p[4][i] = hsv_skin_img.data[3 * (x  ) + hsv_skin_img.step * (y  ) + i];
			p[5][i] = hsv_skin_img.data[3 * (x+1) + hsv_skin_img.step * (y  ) + i];
			p[6][i] = hsv_skin_img.data[3 * (x-1) + hsv_skin_img.step * (y+1) + i];
			p[7][i] = hsv_skin_img.data[3 * (x  ) + hsv_skin_img.step * (y+1) + i];
			p[8][i] = hsv_skin_img.data[3 * (x+1) + hsv_skin_img.step * (y+1) + i];

			//BGR値を並べ替え，中央値を求める
			for(int j=1; j<9; j++)
			{
				for(int k=0; k<8; k++){
					if( p[j][i] > p[j+1][i] )
					{
						tmp = p[j][i];
						p[j][i] = p[j+1][i];
						p[j+1][i] = p[j][i];
					}
				}
			}
			med[i] = p[4][i];
		}
		printf("(%4d, %4d): ( %4d, %4d, %4d )\n", x, y, p[4][0], p[4][1], p[4][2]);
		back_bgr[0] = med[0];
		back_bgr[1] = med[1];
		back_bgr[2] = med[2];
	}
}

int main(int argc, char *argv[])
{
	VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	if (!cap.isOpened()) {
		printf("Can't detect a camera.");
		return -1;
	}

	namedWindow("hsv_skin_img", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("hsv_skin_img", getBackValue);

	while(1)
	{
		random_device rnd;     // 非決定的な乱数生成器
		hsv_skin_img = Scalar(0,0,0);

		cap >> input_img;
		medianBlur(input_img,smooth_img,7);	//ノイズがあるので平滑化
		cvtColor(smooth_img,hsv_img,CV_BGR2HSV);	//HSVに変換

		for(int y=0; y<480;y++){
			for(int x=0; x<640; x++)
			{
				int a = hsv_img.step*y+(x*3);
				if( hsv_img.data[a] >=0 && hsv_img.data[a] <=15 &&hsv_img.data[a+1] >=50 && hsv_img.data[a+2] >= 50 ) //detect skin
				{
					hsv_skin_img.data[a] = back_bgr[0];
					hsv_skin_img.data[a+1] = back_bgr[1];
					hsv_skin_img.data[a+2] = back_bgr[2];
				}
				else if( hsv_img.data[a+1] <=20 ) //detect wall
				{
					hsv_skin_img.data[a] = back_bgr[0];
					hsv_skin_img.data[a+1] = back_bgr[1];
					hsv_skin_img.data[a+2] = back_bgr[2];
				}
				else {
					hsv_skin_img.data[a] = input_img.data[a];
					hsv_skin_img.data[a+1] = input_img.data[a+1];
					hsv_skin_img.data[a+2] = input_img.data[a+2];
				}
			}
		}
		medianBlur(hsv_skin_img, result_img, 11);  //erace face contours

		imshow("hsv_skin_img", result_img);
		if (waitKey(30) >= 0)
			break;
	}
}