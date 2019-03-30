#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <iostream>
#include <time.h>

using namespace cv;
using namespace std;
using namespace zbar;

/*
 *
 * EC535 Spring 2017
 * Team OpenSaysPi
 * Modified by Santanu Bobhate and Michael Z. Webster
 * Handles QR capture and decode using OpenCV and ZBar
 *
 */

#define NUM_PASSWORDS 1
#define LOGNAME_FORMAT "auth%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 20

static string passwords[NUM_PASSWORDS] = { "OpenSaysPi" };
static unsigned int locked = 1;

int main(int argc, char* argv[])
{
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cout << "Cannot open the video cam" << endl;
		return -1;
	}
	
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	
    //To be used when box needed around QR code
        //double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        //double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        //namedWindow("MyVideo", CV_WINDOW_AUTOSIZE);
	
    //Run continuously - manage by Cron for on boot and keep-alive
	while(1)
	{
		Mat frame;
		cap.read(frame);
		Mat gray;
		cvtColor(frame, gray, CV_BGR2GRAY);
		int width = frame.cols; int height = frame.rows;
		uchar *raw = (uchar *) gray.data;
		
		Image image(width , height, "Y800", raw, width *height);
		
		int n = scanner.scan(image);
		
		for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
		{
			vector<Point> vp;
			cout << "decoded " << symbol->get_type_name() << " symbol " << symbol->get_data() << endl;
			
			string input = symbol->get_data();

            //Check if input matches wuth with passwords
			// Authenticate :-
			int i;
			for (i = 0; i < NUM_PASSWORDS; i++)
			{
				if (passwords[i].compare(input) == 0)
				{
					locked = 0;
					cout << "Unlock" << endl;
					system("echo open > /dev/mylockcontrol");
                    
                    //Filename with time code embedded save to current folder
                    //Only done at successful QR scan
                    static char name[LOGNAME_SIZE];
                    time_t now = time(0);
                    strftime(name, sizeof(name), LOGNAME_FORMAT, localtime(&now));
                    string tmp = string(name);
                    string outname = tmp + ".jpg";
					imwrite(outname, frame);
					break;
				}
			}

            //To be used when box needed around QR code
                //int n = symbol->get_location_size();
                //for (int i = 0; i < n; i++)
                //{
                //	vp.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
                //}
                
                //RotatedRect r = minAreaRect(vp);
                //Point2f pts[4];
                //r.points(pts);
                //for (int i = 0; i < 4; i++)
                //{
                //	line(frame, pts[i], pts[(i+1) % 4], Scalar(255, 0, 0), 3);
                //}
			
		}
        
        //View live stream when directly connected to pi
		//imshow("MyVideo", frame);
        
        //Set key for break in testing cases
		//if (waitKey(30) == 27) break;
	}
	
	return 0;
}
