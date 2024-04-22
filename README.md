This app records video on the PC through the webcam using difeerent hand gestures it detects. 
I utilises the features of OpenCV to achieve this.
First i captured frames from the webcam, processed it by running the background substractor algorithm, finding the contours, then processing the largest contours. 
Fromn the largest contours, i processed the convex hull and convexity defect. i counted the defects to know how many fingers the is showing. 
When you show two or three fingers, the app creates the video writer object and starts recording the video on the PC. When it sees five fingers, the video recording stops.
