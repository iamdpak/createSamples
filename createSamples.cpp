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

#include <stdio.h>

// for fileoutput
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace cv;

//int start_roi=0;
int roi_x0=0;
int roi_y0=0;
int roi_x1=0;
int roi_y1=0;
int numOfRec=0;
int startDraw = 0;
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
    if(event==CV_EVENT_LBUTTONDOWN)
    {
        if(!startDraw)
        {
            roi_x0=x;
            roi_y0=y;
            startDraw = 1;
        }
    }
    if(event == CV_EVENT_LBUTTONUP)
    {
    	roi_x1=x;
    	roi_y1=y;
        startDraw = 0;
    }

    if(event==CV_EVENT_MOUSEMOVE && startDraw)
    {
    	image.copyTo(image_copy);
        //redraw ROI selection
        rectangle(image_copy,cvPoint(roi_x0,roi_y0),cvPoint(x,y),CV_RGB(255,0,255),1);
        imshow(window_name,image_copy);
    }

}

int main(int argc, char** argv)
{
    char iKey=0;
    string strPrefix;
    string strPostfix;
    string input_directory;
    string output_file;
	Mat inpImg;

#ifdef RELEASE
    if(argc != 3) {
        fprintf(stderr, "%s output_info.txt raw/data/directory/\n", argv[0]);
        return -1;
    }
    input_directory = argv[2];
    output_file = argv[1];
#else
    input_directory = "Data/"; //linux
	//input_directory = "..\\Data"; //windows
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
    ofstream output(output_file.c_str());
    cout << "done" << endl;

	for (uint i = 0; i < imgNames.size(); ++i)
	{
		inpImg = imread(imgNames[i]);

		// Assign postfix/prefix info
		strPrefix = imgNames[i];
		strPostfix="";
		numOfRec = 0;

		if (!inpImg.empty())
		{
            //    work on current image
            do
            {
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
                        cout << format("   %d. rect x=%d\ty=%d\tx2h=%d\ty2=%d\n",numOfRec,roi_x0,roi_y0,roi_x1,roi_y1) << endl;
                        //cout << format("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,roi_x1,roi_y1,roi_x0-roi_x1,roi_y0-roi_y1) << endl;
                        // currently two draw directions possible:
                        //        from top left to bottom right or vice versa
                        if(roi_x0<roi_x1 && roi_y0<roi_y1)
                        {
                            cout << format("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,roi_x0,roi_y0,roi_x1-roi_x0,roi_y1-roi_y0) << endl;
                            // append rectangle coord to previous line content
                            strPostfix+=" "+IntToString(roi_x0)+" "+IntToString(roi_y0)+" "+IntToString(roi_x1-roi_x0)+" "+IntToString(roi_y1-roi_y0);

                        }
                        else
                                                    //(roi_x0>roi_x1 && roi_y0>roi_y1)
                        {
                            printf("   %d. rect x=%d\ty=%d\twidth=%d\theight=%d\n",numOfRec,roi_x1,roi_y1,roi_x0-roi_x1,roi_y0-roi_y1);
                            // append rectangle coord to previous line content
                            strPostfix+=" "+IntToString(roi_x1)+" "+IntToString(roi_y1)+" "+IntToString(roi_x0-roi_x1)+" "+IntToString      (roi_y0-roi_y1);
                        }

                        break;
                }
            }
            while(iKey!=66);


            // save to info file as later used for HaarTraining:
            //    <rel_path>\bmp_file.name numOfRec x0 y0 width0 height0 x1 y1 width1 height1...
            if(numOfRec>0 && iKey==66)
            {
                //append line
                /* TODO: Store output information. */
                output << strPrefix << " "<< numOfRec << strPostfix <<"\n";

            }
            else
            {
            	cerr << format("Failed to load image, %s\n", strPrefix.c_str()) << endl;
            }

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
