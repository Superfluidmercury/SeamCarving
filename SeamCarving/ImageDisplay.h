#pragma once
#include "stdafx.h"

class ImageDisplay
{
private:
	cv::Mat openImage;
	std::string name;
public:
	void showImage( cv::Mat& );
	void refresh() const;
	ImageDisplay(std::string name);
	~ImageDisplay();
};

