#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <ctime>
#include <string>
#include <sstream>

int thresholdValue = 60, maxValue = 250;
int thresholdType = 3, maxType = 4;
int thresholdBinary = 255;
int edge = 52;
cv::Mat  frameMask, Roi;

std::string trackHand ()
{
    int count = 0;
    std::string fingers = "wait";
    cv::Mat blur, canny, thresh, dil;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::GaussianBlur(frameMask, blur, cv::Size(15,15), 3, 0);
    cv::threshold(blur, thresh, thresholdValue, thresholdBinary, thresholdType);
    cv::Canny(thresh, canny, edge, edge*2, 3);
    cv::Mat kernal = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7));
    cv::dilate(canny,dil, kernal);
    cv::findContours (dil, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    //cv::drawContours(Roi, contours, -1, cv::Scalar(0, 255, 0), 5);
    cv::Mat drawing = cv::Mat::zeros(canny.size(), CV_8UC3);

    if (contours.size() > 0)
    {
        size_t indexOfBiggestContour = -1;
        size_t sizeOfBiggestContour = 0;
        
        for (int i=0; i < contours.size(); i++)
        {
            if (contours[i].size() > sizeOfBiggestContour)
            {
                sizeOfBiggestContour = contours[i].size();
                indexOfBiggestContour = i;
            }
        }

        std::vector<std::vector<int>> hull (contours.size());
        std::vector<std::vector<cv::Point>> hullPoints (contours.size());
        std::vector<std::vector<cv::Vec4i>> defects (contours.size());
        std::vector<std::vector<cv::Point>> defectPoint (contours.size());
        std::vector<std::vector<cv::Point>> conPoly (contours.size());
        cv::Point2f rectPoint [4];
        std::vector<cv::RotatedRect> miniRect (contours.size());
        std::vector<cv::Rect> boundRect (contours.size());

        for(size_t i =0; i<contours.size(); i++)
        {
            int area = cv::contourArea(contours[i]);
            //std::cout << "Area found is " << area << std::endl;
            if (area > 15000)
            {
                cv::convexHull(contours[i], hull[i], true);
                cv::convexityDefects(contours[i], hull[i], defects[i]);
                //cv::drawContours(Roi, hull, i, cv::Scalar(255,0,255), 2);
                cv::drawContours(Roi, contours, i, cv::Scalar(0, 255, 0), 2);

                if (indexOfBiggestContour == i)
                {
                    miniRect[i] = cv::minAreaRect(contours[i]);
                    for(size_t k = 0; k < hull[i].size(); k++)
                    {
                        int ind = hull[i][k];
                        hullPoints[i].push_back(contours[i][ind]);
                    }
                    count = 0;
                    for (size_t k = 0; k < defects[i].size(); k++)
                    {
                        std::cout << "\nDefect = " << defects[i][k][3] << std::endl;
                        if (defects[i][k][3] > 20000) //13*256
                        {
                            int pStart = defects[i][k][0];
                            int pEnd = defects[i][k][1];
                            int pFar = defects[i][k][2];
                            defectPoint[i].push_back(contours[i][pFar]);
                            cv::circle(Roi, contours[i][pEnd], 5, cv::Scalar(0,255,0), 2);
                            count++;

                        }
                    }

                    if (count == 1){fingers = "1";}
                    else if(count == 2){fingers = "2";}
                    else if(count == 3){fingers = "3";}
                    else if(count == 4){fingers = "4";}
                    else if(count == 5){fingers = "5";}
                    else if(count == 6){fingers = "6";}
                    else if(count == 7){fingers = "7";}
                    
                    else {fingers = "Show Hand";}

                    cv::putText (Roi, fingers, cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 255), 1);

                    cv::drawContours(Roi, hullPoints, i, cv::Scalar(255,255,0), 2, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
                    cv::drawContours(drawing, hullPoints, i, cv::Scalar(255,255,0), 2, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
                    
                    

                }
            }
        }

        

    }

    std::string returnValue;

    if (count >= 2 && count <= 3)
    {
        returnValue = "start";
    }
    else if (count == 5)
    {
        returnValue = "stop";
    }
    else
    {
        returnValue = "none";
    }


    //cv::imshow ("Blur", blur);
    //cv::imshow ("Threshold", thresh);
    //cv::imshow ("Canny", canny);
    //cv::imshow ("Drawing", drawing);
    //cv::imshow ("Dilate", dil);



    return returnValue;
    //returnValue = "none";
    return returnValue;


}

std::string getFileName ()
{
    std::time_t currentTime = std::time(0);
	
	std::string timeString = std::ctime(&currentTime);
	
	std::cout << "Current Time is: " << timeString << std::endl;
	
	std::tm* localTime = std::localtime (&currentTime);
	
	int year = localTime->tm_year + 1900;
	int month = localTime->tm_mon + 1;
	int day = localTime->tm_mday;
	int hour = localTime->tm_hour;
	int minute = localTime->tm_min;
	int seconds = localTime->tm_sec;

    std::ostringstream oss;
    oss << "VID" << year << (month < 10? "0": "") << month 
        << (day < 10? "0" : "") << day 
        << (hour < 10? "0" : "") << hour 
        << (minute < 10? "0" : "") << minute 
        << (seconds < 10? "0" : "") << seconds 
        << ".mp4";

    return oss.str();

}


int main ()
{
    cv::Mat frameGray, frameBlur, frameCanny, frameDil;
    cv::VideoCapture cam(0);
    if (!cam.isOpened())
    {
        std::cout << "Error: Unable to open webcam!!!" << std::endl;
        return -1;
    }

    cv::namedWindow("Track Bars", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Threshold Value", "Track Bars", &thresholdValue, 250);
    cv::createTrackbar("Threshold Type", "Track Bars", &thresholdType, 4);
    cv::createTrackbar("Threshold Binary", "Track Bars", &thresholdBinary, 255);
    cv::createTrackbar("Canny Edge", "Track Bars", &edge, 100);

    cv::Ptr<cv::BackgroundSubtractor> MOG2 = cv::createBackgroundSubtractorMOG2();
    cv::Rect myRoi(50, 12, 450, 450);
    cv::Mat mirrowed;

    bool recording = false;
    cv::VideoWriter videoWriter;
    while (true)
    {
        cv::Mat frame;
        cam >> frame;
        cv::flip(frame, mirrowed, 1);
        Roi = mirrowed(myRoi);
        //cv::GaussianBlur(Roi, Roi, cv::Size(15,15), 3, 0);
        MOG2->apply(Roi, frameMask);
        //cv::rectangle(frameMask, myRoi, cv::Scalar(0,0,255));

        
        
        std::string& returnValue = trackHand();
        

        if (returnValue == "start")
        {
            if (!recording)
            {
                std::string fileName = getFileName();
                int frameWidth = cam.get(cv::CAP_PROP_FRAME_WIDTH);
                int frameHeight = cam.get(cv::CAP_PROP_FRAME_HEIGHT);
                videoWriter.open (fileName, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(frameWidth, frameHeight));
                if (!videoWriter.isOpened())
                    std::cout << "\n\n\nCould not open Video Writer!\n";
                else
                    recording = true;
            }
            
        }
        else if (returnValue == "stop")
        {
            
            videoWriter.release();
            recording = false;
        }
        else if (returnValue == "none")
        {
            
        }
        else{}

        if(recording)
        {
            videoWriter.write(mirrowed);
            cv::circle(mirrowed, cv::Point (20, 50), 10, cv::Scalar(0,0,255), cv::FILLED);
            cv::circle(mirrowed, cv::Point (20, 50), 15, cv::Scalar(0,0,255), 2);
            cv::putText (mirrowed, "Recording", cv::Point(40,50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,255), 1);
            
        }

        //cv::imshow ("Hand Gesture", frame);
        //cv::imshow ("Background Subtract", frameMask);
        cv::imshow ("Mirrowed", mirrowed);
        //cv::imshow ("Region of Interest", Roi);
        
        
        if (cv::waitKey(5)== 27)
            break;
        //cv::waitKey(10);
    }



    return 0;
}