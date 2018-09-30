
#pragma once

// Class used to handle the processing of images being recieved by server:
// the image processing will be done within each of the threads currently active in 
// active node thread vector/map managed by main server class:
// the threads will then store these processed images into thier own queue
// the main server will iterate through list of rem_dev objects contianing threads
// and empty the respective threads queue of processed images into tcp connection
// to golang web backend:
class MotionDetect
{
public:
	MotionDetect();
	~MotionDetect();
};

