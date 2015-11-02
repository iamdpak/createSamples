/***************createSamples.cpp******************

createSamples for marking the objects to be detected  from positive samples and then creating the
description file for positive images.

compile this code and run with two arguments, first one the name of the descriptor file and the second one
the address of the directory in which the positive images are located

while running this code, each image in the given directory will open up. Now mark the edges of the object using the mouse buttons
  then press then press "SPACE" to save the selected region, or any other key to discard it. Then use "B" to move to next image. the program automatically
  quits at the end. press ESC at anytime to quit.

  *the key B was chosen  to move to the next image because it is closer to SPACE key and nothing else.....

author: achu_wilson@rediffmail.com
modified by: iamdpakgre@gmail.com
*/


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <cmath>

#include <stdio.h>

// for fileoutput
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace cv;

#define ASPECT_RATIO_X 1
#define ASPECT_RATIO_Y 1

//#define IMAGE

//int start_roi=0;
int pos_roi_x0=0;
int pos_roi_y0=0;
int pos_roi_x1=0;
int pos_roi_y1=0;
int neg_roi_x0 = 0;
int neg_roi_y0 = 0;
int neg_roi_x1 = 0;
int neg_roi_y1 = 0;

int numOfRec=0;
int numOfRec_pos=0; //initialize this with the count
int numOfRec_neg = 0;
int pos_startDraw = 0;
bool pos_chosen = false;
int neg_startDraw = 0;
//char* window_name="<SPACE>add <B>save and load next <ESC>exit";
cv::String window_name="car_samples";

string IntToString(int num)
{
    ostringstream myStream; //creates an ostringstream object
    myStream << num << flush;
    /*
    * outputs the number into the string stream and then flushes
    * the buffer (makes sure the output is put into the stream)
    */
    return(myStream.str()); //returns the string form of the stringstream object
};

void on_mouse(int event,int x,int y,int flag, void *param)
{
	Mat image, image_copy;
	image = * (Mat*) param;

	//drawing positive ROI
    if(event==CV_EVENT_LBUTTONDOWN)
    {
        if(!pos_startDraw)
        {
            pos_roi_x0=x;
            pos_roi_y0=y;
            pos_startDraw = 1;
        }
    }
    if(event == CV_EVENT_LBUTTONUP)
    {
        pos_startDraw = 0;
		pos_chosen = true;
    }

    if(event==CV_EVENT_MOUSEMOVE && pos_startDraw)
    {
		if (abs(pos_roi_x0 - x) > abs(pos_roi_y0 - y))
		{
			pos_roi_x1 = pos_roi_x0 + (x - pos_roi_x0) * ASPECT_RATIO_X;
			pos_roi_y1 = pos_roi_y0 + (x - pos_roi_x0) * ASPECT_RATIO_Y;
		}
		else
		{
			pos_roi_x1 = pos_roi_x0 + (y - pos_roi_y0) * ASPECT_RATIO_X;
			pos_roi_y1 = pos_roi_y0 + (y - pos_roi_y0) * ASPECT_RATIO_Y;
		}

    	image.copyTo(image_copy);
        //redraw ROI selection
		rectangle(image_copy, cvPoint(pos_roi_x0, pos_roi_y0), cvPoint(pos_roi_x1, pos_roi_y1), CV_RGB(255, 0, 255), 1);	
        imshow(window_name,image_copy);
    }

	if (event == CV_EVENT_RBUTTONDOWN)
	{
		if(!neg_startDraw)
		{
			neg_roi_x0=x;
			neg_roi_y0=y;
			neg_startDraw = 1;
		}
	}
	if(event == CV_EVENT_RBUTTONUP)
	{
		neg_startDraw = 0;
	}
	if (event == CV_EVENT_MOUSEMOVE && neg_startDraw && pos_chosen)
	{
		neg_roi_x1 = x;
		neg_roi_y1 = y;


		image.copyTo(image_copy);
		//redraw ROI selection
		rectangle(image_copy, cvPoint(pos_roi_x0, pos_roi_y0), cvPoint(pos_roi_x1, pos_roi_y1), CV_RGB(255, 0, 255), 1);
		rectangle(image_copy, cvPoint(neg_roi_x0, neg_roi_y0), cvPoint(x, y), CV_RGB(0, 255, 255), 1);	
		imshow(window_name,image_copy);
	}
}

#ifdef IMAGE
int main(int argc, char** argv)
{
    char iKey=0;
    string strPrefix;
    string strPostfix;
    string input_directory;
    string output_file;
	Mat inpImg;
	Mat outImg;
	string outFileName;

#ifdef RELEASE
    if(argc != 3) {
        fprintf(stderr, "%s output_info.txt raw/data/directory/\n", argv[0]);
        return -1;
    }
    input_directory = argv[2];
    output_file = argv[1];
#else
    //input_directory = "Data/"; //linux
	input_directory = "..\\Data\\cars"; //windows
	//input_directory = "C:\\Users\\iamdp_000\\Documents\\Visual Studio 2013\\Projects\\webSVNProjects\\headcount\\trunk\\headCount\\Data\\cars";
    output_file = "positive.txt";
#endif
    //getting ready to mark the image
    namedWindow(window_name);
    setMouseCallback(window_name,on_mouse, &inpImg);

    /* Get a file listing of all files with in the input directory */
	vector <cv::String> imgNames;
	//new opencv function to get the list of files present in a folder
	glob(input_directory, imgNames,false);

    if(imgNames.size() == 0) {
        cerr << format("No files in the directory or failed to open directory %s",input_directory.c_str()) << endl;
        return -1;
    }

    cout << "creating the output file..." << endl;
    //    init output of rectangles to the info file
    ofstream output(output_file.c_str(),std::ios::app);
    cout << "done" << endl;

	for (uint i = 0; i < imgNames.size(); ++i)
	{
		inpImg = imread(imgNames[i]);

		// Assign prefix info
		strPrefix = imgNames[i];
		
		numOfRec = 0;

		if (!inpImg.empty())
		{
            //    work on current image
            do
            {
				//initialize postfix info to provide only one entry per line
				strPostfix = "";
                imshow(window_name,inpImg);

                // used cvWaitKey returns:
                //    <B>=66        save added rectangles and show next image
                //    <ESC>=27        exit program
                //    <Space>=32        add rectangle to current image
                //  any other key clears rectangle drawing only
                iKey=waitKey(0);
                switch(iKey)
                {
                case 27:
                        cv::destroyWindow(window_name);
                        output.close();
                        return 0;
                case 32:

                        numOfRec++;
                        
                        cout << format("   %d. rect x=%d\ty=%d\tx2h=%d\ty2=%d\n",numOfRec,pos_roi_x0,pos_roi_y0,pos_roi_x1,pos_roi_y1) << endl;
                        //cout << format("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,pos_roi_x1,pos_roi_y1,pos_roi_x0-pos_roi_x1,pos_roi_y0-pos_roi_y1) << endl;
                        // currently two draw directions possible:
                        //        from top left to bottom right or vice versa
                        if(pos_roi_x0<pos_roi_x1 && pos_roi_y0<pos_roi_y1)
                        {
							numOfRec_pos++;
                            cout << format("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,pos_roi_x0,pos_roi_y0,pos_roi_x1-pos_roi_x0,pos_roi_y1-pos_roi_y0) << endl;
                            // append rectangle coord to previous line content
                            Rect ROI(pos_roi_x0,pos_roi_y0,(pos_roi_x1-pos_roi_x0),(pos_roi_y1-pos_roi_y0));
                            outImg = Mat(inpImg,ROI);
							//writing the cropped image to a file
#ifdef __linux__
							outFileName = ("posSamples/Sample-" + IntToString(numOfRec_pos) + ".jpg");
#elif _WIN32
							outFileName = ("..\\posSamples\\Sample-" + IntToString(numOfRec_pos) + ".jpg");
#endif
							float height = 50;
							float ratio = outImg.rows / height;
							float width = outImg.cols / ratio;
							resize(outImg, outImg, Size((int)ceil(width), (int)ceil(height)));
							cv::imwrite(outFileName, outImg);
							strPostfix += " " + IntToString(0) + " " + IntToString(0) + " " + IntToString((int)ceil(width)) + " " + IntToString((int)ceil(height));
							output << outFileName << " " << "1" << strPostfix << "\n";
                        }
						else if (pos_roi_x0>pos_roi_x1 && pos_roi_y0>pos_roi_y1)
                                                    //(pos_roi_x0>pos_roi_x1 && pos_roi_y0>pos_roi_y1)
                        {
							numOfRec_pos++;
                            printf("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,pos_roi_x1,pos_roi_y1,pos_roi_x0-pos_roi_x1,pos_roi_y0-pos_roi_y1);
                            // append rectangle coord to previous line content

                            Rect ROI(pos_roi_x1,pos_roi_y1,(pos_roi_x0-pos_roi_x1),(pos_roi_y0-pos_roi_y1));
                            outImg = Mat(inpImg,ROI);
							//writing the cropped image to a file
#ifdef __linux__
							outFileName = ("posSamples/Sample-"+ IntToString(numOfRec_pos) + ".jpg");
#elif _WIN32
							outFileName = ("..\\posSamples\\Sample-" + IntToString(numOfRec_pos) + ".jpg");
#endif
							//create a sample of height 100 X something  -- this number changes with the type of object to be trained
							float height = 50; 
							float ratio = outImg.rows / height;
							float width = outImg.cols / ratio;
							resize(outImg, outImg, Size((int)ceil(width), (int)ceil(height)));
							cv::imwrite(outFileName, outImg);
							strPostfix += " " + IntToString(0) + " " + IntToString(0) + " " + IntToString((int)ceil(width)) + " " + IntToString((int)ceil(height));
							output << outFileName << " " << "1" << strPostfix << "\n";
                        }

                        break;

                }
				pos_roi_x0 = pos_roi_x1 = pos_roi_y0 = pos_roi_y1 = 0;
            }
            while(iKey!=66);

		}
		else
		{
			cerr << format("skipping the file, %s", strPrefix.c_str()) << endl;
			continue;
		}
	}
    output.close();
    destroyWindow(window_name);
    return 0;
}
#else
int main(int argc, char** argv)
{
    char iKey=0;
    string strPrefix;
    string strPostfix;
    string input_video;
    string output_file;
	Mat inpImg;
	Mat outImg;
	string outFileName;

#ifdef RELEASE
    if(argc != 3) {
        fprintf(stderr, "%s output_info.txt raw/data/directory/\n", argv[0]);
        return -1;
    }
    input_directory = argv[2];
    output_file = argv[1];
#else
    //input_video = "Data/inpVideo.mp4"; //linux
	input_video = "..\\Data\\video2.mp4"; //windows
    output_file = "positive.txt";
#endif
    //getting ready to mark the image
    namedWindow(window_name);
    setMouseCallback(window_name,on_mouse, &inpImg);

    cout << "creating the output file..." << endl;
    //    init output of rectangles to the info file
    ofstream output(output_file.c_str());
    cout << "done" << endl;

    cv::VideoCapture capture;
    capture.open(input_video);
	if (!capture.isOpened())
		return -1;

	while(1)
	{
		capture >> inpImg;
		if (inpImg.empty())
			break;
		cv::resize(inpImg, inpImg, cv::Size(inpImg.cols / 3, inpImg.rows / 3));

		// Assign postfix/prefix info
		outFileName = "";// "Sample-" + IntToString(numOfRec_pos) + ".jpg";
		strPostfix="";
		numOfRec = 0;

		imshow(window_name,inpImg);

        // used cvWaitKey returns:
        //    <B>=66        save added rectangles and show next image
        //    <ESC>=27        exit program
        //    <Space>=32        add rectangle to current image
        //  any other key clears rectangle drawing only
        iKey=waitKey(0);
        switch(iKey)
        {
        	case 27:
                   cv::destroyWindow(window_name);
                   output.close();
                   return 0;
            case 32:
				   pos_chosen = false;
                   numOfRec++;
                   
                   cout << format("   %d. rect x=%d\ty=%d\tx2h=%d\ty2=%d\n",numOfRec,pos_roi_x0,pos_roi_y0,pos_roi_x1,pos_roi_y1) << endl;
                   //cout << format("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,pos_roi_x1,pos_roi_y1,pos_roi_x0-pos_roi_x1,pos_roi_y0-pos_roi_y1) << endl;
                   // currently two draw directions possible:
                   //        from top left to bottom right or vice versa
                   if(pos_roi_x0<pos_roi_x1 && pos_roi_y0<pos_roi_y1)
                   {
					   numOfRec_pos++;
                       cout << format("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,pos_roi_x0,pos_roi_y0,pos_roi_x1-pos_roi_x0,pos_roi_y1-pos_roi_y0) << endl;
                       // append rectangle coord to previous line content
                       //strPostfix+=" "+IntToString(0)+" "+IntToString(0)+" "+IntToString(pos_roi_x1-pos_roi_x0)+" "+IntToString(pos_roi_y1-pos_roi_y0);
                       Rect ROI(pos_roi_x0,pos_roi_y0,(pos_roi_x1-pos_roi_x0),(pos_roi_y1-pos_roi_y0));
                       outImg = Mat(inpImg,ROI);
					   //writing the cropped image to a file
#ifdef __linux__
					   outFileName = ("posSamples/Sample-" + IntToString(numOfRec_pos) + ".jpg");
#elif _WIN32
					   outFileName = ("..\\posSamples\\Sample-" + IntToString(numOfRec_pos) + ".jpg");
#endif
					   float height = 50;
					   float ratio = outImg.rows / height;
					   float width = outImg.cols / ratio;
					   resize(outImg, outImg, Size((int)ceil(width), (int)ceil(height)));
					   cv::imwrite(outFileName, outImg);
					   strPostfix += " " + IntToString(0) + " " + IntToString(0) + " " + IntToString((int)ceil(width)) + " " + IntToString((int)ceil(height));
					   output << outFileName << " " << "1" << strPostfix << "\n";

                   }
                   else if (pos_roi_x0>pos_roi_x1 && pos_roi_y0>pos_roi_y1)
                   {
					   numOfRec_pos++;
                	   printf("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,pos_roi_x1,pos_roi_y1,pos_roi_x0-pos_roi_x1,pos_roi_y0-pos_roi_y1);
                       // append rectangle coord to previous line content
                      // strPostfix+=" "+IntToString(0)+" "+IntToString(0)+" "+IntToString(pos_roi_x0-pos_roi_x1)+" "+IntToString      (pos_roi_y0-pos_roi_y1);
                       Rect ROI(pos_roi_x1,pos_roi_y1,(pos_roi_x0-pos_roi_x1),(pos_roi_y0-pos_roi_y1));
                       outImg = Mat(inpImg,ROI);
					   //writing the cropped image to a file
#ifdef __linux__
					   outFileName = ("posSamples/Sample-" + IntToString(numOfRec_pos) + ".jpg");
#elif _WIN32
					   outFileName = ("..\\posSamples\\Sample-"+ IntToString(numOfRec_pos) + ".jpg");
#endif
					   float height = 50;
					   float ratio = outImg.rows / height;
					   float width = outImg.cols / ratio;
					   resize(outImg, outImg, Size((int)ceil(width), (int)ceil(height)));
					   cv::imwrite(outFileName, outImg);
					   strPostfix += " " + IntToString(0) + " " + IntToString(0) + " " + IntToString((int)ceil(width)) + " " + IntToString((int)ceil(height));
					   output << outFileName << " " << "1" << strPostfix << "\n";

                   }
//--------------------------------------------------
				   if (neg_roi_x0<neg_roi_x1 && neg_roi_y0<neg_roi_y1)
				   {
					   numOfRec_neg++;
					   Rect ROI(neg_roi_x0, neg_roi_y0, (neg_roi_x1 - neg_roi_x0), (neg_roi_y1 - neg_roi_y0));
					   outImg = Mat(inpImg, ROI);
					   //writing the cropped image to a file
#ifdef __linux__
					   outFileName = ("negSamples/Sample-" + IntToString(numOfRec_neg) + ".jpg");
#elif _WIN32
					   outFileName = ("..\\negSamples\\Sample-" + IntToString(numOfRec_neg) + ".jpg");
#endif
					   float height = 80;
					   float ratio = outImg.rows / height;
					   float width = outImg.cols / ratio;
					   resize(outImg, outImg, Size((int)ceil(width), (int)ceil(height)));
					   cv::imwrite(outFileName, outImg);

				   }
				   else if (pos_roi_x0>pos_roi_x1 && pos_roi_y0>pos_roi_y1)
				   {
					   numOfRec_neg++;
					   // append rectangle coord to previous line content
					   Rect ROI(neg_roi_x1, neg_roi_y1, (neg_roi_x0 - neg_roi_x1), (neg_roi_y0 - neg_roi_y1));
					   outImg = Mat(inpImg, ROI);
					   //writing the cropped image to a file
#ifdef __linux__
					   outFileName = ("negSamples/Sample-" + IntToString(numOfRec_neg) + ".jpg");
#elif _WIN32
					   outFileName = ("..\\negSamples\\Sample-" + IntToString(numOfRec_neg) + ".jpg");
#endif
					   float height = 80;
					   float ratio = outImg.rows / height;
					   float width = outImg.cols / ratio;
					   resize(outImg, outImg, Size((int)ceil(width), (int)ceil(height)));
					   cv::imwrite(outFileName, outImg);

				   }

                   break;
        }
		pos_roi_x0 = pos_roi_x1 = pos_roi_y0 = pos_roi_y1 = 0;
		neg_roi_x0 = neg_roi_x1 = neg_roi_y0 = neg_roi_y1 = 0;
	}

    output.close();
    destroyWindow(window_name);

    return 0;
}
#endif
