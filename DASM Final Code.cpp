#include <iostream>
#include <fstream>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include <sstream>
#include <string.h>
#include <math.h>

using namespace std;
using namespace cv;
Mat img0,img1,roi;
vector<Point> points;
Point prevPt(-1, -1);
int z=1;
static void EraseConnection( int event, int x, int y, int flags, void* )
{

	int createTrackbar(Line_Thickness,2,0,20,0,0);
    if( x < 0 || x >= img1.cols || y < 0 || y >= img1.rows )
        return;
    if( event == EVENT_LBUTTONUP || !(flags & EVENT_FLAG_LBUTTON) )
        prevPt = Point(-1,-1);
    else if( event == EVENT_LBUTTONDOWN )
        prevPt = Point(x,y);
    else if( event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON) )
    {
        Point pt(x, y);
        if( prevPt.x < 0 )
            prevPt = pt;
        line( img1, prevPt, pt, Scalar::all(0), 20, 8, 0 );
        prevPt = pt;
        imshow("2", img1);
    }
}
// function to select rectangular region
void DrawRect(int evt, int x, int y, int flags, void* param) 
{
	if(evt == CV_EVENT_LBUTTONDOWN) 
    {	
        
        points.push_back(Point(x,y));
         if (points.size() > 1) //we have 2 points
        {
            Mat drawing = Mat::zeros( img0.size(), CV_8UC3 );
            Rect rec(points[0].x, points[0].y, points[1].x-points[0].x, points[1].y-points[0].y);
            rectangle(img0,rec,Scalar(0,0,0),1,8);
            roi = img0(rec);
            Rect rect(points[0].x, points[0].y, points[1].x-points[0].x, points[1].y-points[0].y);
            roi.copyTo(drawing(rect));
            imshow("Input Image", img0);
            stringstream ss1;
			ss1 << z;
            // image is saved 
            imwrite(ss1.str() + ".jpg", drawing);
            z++;
            cout << points[0].x << ' ' << points[0].y <<endl;
            cout << points[1].x << ' ' << points[1].y <<endl;
            points.clear();
            
        }    
    }
}
double distance(Point2f a, Point2f b)
{
	double D;
	D = sqrt(pow(a.x-b.x,2) + pow(a.y-b.y,2));
	return D;

}
void drawLine(Mat *imgy,Point2f p1, Point2f p2)
{
	int i = 40;
	Point2f p,q;
	if (p1.x != p2.x)
        {
                p.x = i;
                q.x = imgy->cols+i;
                // Slope equation (y1 - y2) / (x1 - x2)
                float m = (p1.y - p2.y) / (p1.x - p2.x);
                // Line equation:  y = mx + b
                float b = p1.y - (m * p1.x);
                p.y = m * p.x + b;
                q.y = m * q.x + b;
        }
        else
        {
                p.x = q.x = p2.x;
                p.y = 0;
                q.y = imgy->rows-100;
        }
    line(img0, p, q, Scalar(255,0,0), 4);

}

int main()
{
	float Length[1000];
	// read the image
	img0 = imread("1.png");
	imshow("Input Image",img0);

	// select rectangular region
	// each rectangular region that is selected must have a set of contours
	setMouseCallback("Input Image", DrawRect, (void*)&points);
	int X=0, Y=0;

	waitKey(0);
	destroyWindow("Input Image");
	// separate the contours 
	// saved image from the previous step is read for furthur processing
	int a;


	for(a=1;a<z;a++)
	{
		stringstream ss;
		ss << a;
		img1 = imread(ss.str() + ".jpg");
		imshow("2",img1);
	
		setMouseCallback("2", EraseConnection,0);
		for(;;)
		{
			char c = (char)waitKey(0);
			if(c==27)
				break;
			if(c=='s')
			{
				imwrite("1.jpg",img1);
			}
		} 	
		destroyWindow("2");
	
		Mat canny_output;
		vector<vector<Point>>contours;
		vector<Vec4i> hierarchy;
		Canny(img1, canny_output,50,200,3);
		findContours(canny_output,contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		

		 vector<Moments> mu(contours.size() );
	  	for( int i = 0; i < contours.size(); i++ )
	   	    { 
	   	    	mu[i] = moments( contours[i], false ); 
	   	    }
	
	    //  Get the mass centers:
	 	 vector<Point2f> mc( contours.size() );
	 	for( int i = 0; i < contours.size(); i++ )
	     	{ 
	     		mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); 
	     	}
	 	 Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
	  	for( int i = 0; i< contours.size(); i++ )
	     	{
		       
		       circle( drawing, mc[i], 4, Scalar(0,255,0), -1, 8, 0 );
		       circle( img0, mc[i], 4, Scalar(0,255,0), -1, 8, 0 );
	    	 }
	    Mat imgx(img1.rows, img1.cols, CV_8UC3, Scalar(0,0,0));
	 	for(int i=0;i<contours.size();i++)
	 	{
	 		circle(imgx,mc[i],1,Scalar(255),1,8);
	 	}
	 	int c1=0,c2=0;
	 	double maxDist =0.0;
	 	for(int i=0;i<contours.size();i++)
	 	{
	 		for(int j = i+1;j<contours.size();j++)
	 		{
	 			double dist = distance(mc[i],mc[j]);
	 			if(dist>maxDist)
	 			{
	 				c1=i;
	 				c2=j;
	 				maxDist = dist;
	 			}
	 		}
	 	}
	 	Point2f p1=mc[c1], p2=mc[c2];
	 	line(img0,p1,p2,Scalar(255,0,0), 4);
	 	cout<< maxDist << ' ' << p1<< ' '<< p2 <<' '<<endl;
	 	
	 	//cout<< contours.size()<<endl;
	 	Length[a] = maxDist/(contours.size() - 1);
		
		cout << "Average Length of the DENDRITE ARM is" << "   " << Length[a] << endl;
	}
	float sum = 0.0;
	for(a=1;a<z;a++)
	{
		sum = Length[a] + sum;
	}
	float length = sum/(z-1);
	cout<< "Average Length of the all the DENDRITE ARMS is" << "   " << length << endl;
	imshow("image",img0);
	waitKey(0);
    return 0;
}