#pragma once 

#include "OutputDevice.h"
#include "NearImageManager.h"
#include "ImageLoader.h"
#include "ImageCacheManager.h"

class OutputDeviceWindow : public OutputDevice {
public:
  OutputDeviceWindow();
  virtual ~OutputDeviceWindow();

  void output(cv::VideoCapture& in_video);

private:
  ImageCacheManager<cv::Mat, ImageLoader> icm;
  NearImageManager nim;
};
