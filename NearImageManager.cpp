#include "NearImageManager.h"

#include <opencv/highgui.h>

NearImageManager::NearImageManager() : 
  select_mode(SELECT_MODE_USE_ONCE),
  max_use_count(1) {
}

NearImageManager::~NearImageManager() {
}

void NearImageManager::load(const std::string& image_dir_path) {
  namespace fs = boost::filesystem;

  //std::cout << "begin load" << std::endl;
  fs::path dir(image_dir_path);
  fs::directory_iterator end;
  for(fs::directory_iterator it(dir); it!=end; ++it) {
    color_t color;
      
    ImageInfo ii;
    ii.imagename = it->path().string();
    ii.color     = mat2avgcolor(cv::imread(it->path().string(), 1));
    ii.used      = 0;

    image_list_.push_back(ii);

    //std::cout << "color: " << ii.color.x << "," << ii.color.y << "," << ii.color.z << std::endl;
    int index = (int)(ii.color.x * color_step)
      + (int)(ii.color.y * color_step) * color_array_size
      + (int)(ii.color.z * color_step) * color_array_size_2;
    //std::cout << "index: " << index << std::endl;
    if(index >= 0){
      color_image_list_[index].push_back(ii);
    }
  }

  color_t search_color;
  for(int r=0; r<color_array_size; r++){
    for(int g=0; g<color_array_size; g++){
      for(int b=0; b<color_array_size; b++){
        int index = r + g * color_array_size + b * color_array_size_2;
        if(color_image_list_[index].size() <= 0) {
          // クラスタに画像が１つも無ければ一番色が近い画像を追加する
          search_color.x = (int)((r + 0.5) / color_step);
          search_color.y = (int)((g + 0.5) / color_step);
          search_color.z = (int)((b + 0.5) / color_step);
          color_image_list_[index].push_back(get_near_image_precise(search_color));
        }
      }
    }
  }
  //std::cout << "end load" << std::endl;
}

color_t NearImageManager::mat2avgcolor(const cv::Mat &image) {
  int es   = image.elemSize();
  int step = image.step;
  uint64_t r = 0;
  uint64_t g = 0;
  uint64_t b = 0;

  // sum image pix
  for(int y = 0 ; y < image.rows; y++){
    for(int x = 0 ; x < image.cols; x++){
      b += image.data[y * step + x * es + 0]; //b
      g += image.data[y * step + x * es + 1]; //g
      r += image.data[y * step + x * es + 2]; //r
    }
  }

  // image avg
  float pixsum = image.rows*image.cols;

  color_t color;
  color.x = b/pixsum;
  color.y = g/pixsum;
  color.z = r/pixsum;

  return color;
}

NearImageManager::ImageInfo& NearImageManager::get_near_unused_image(color_t cp) {
  // 一番色が近い未使用の画像を求める
  int min_len=std::numeric_limits<int>::max();
  ImageInfo& min_image = dummy_image;

  for(ImageInfo& i : image_list_) {
    if(i.used == 0){
      const color_t& p = i.color;
      int len = (p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y) + (p.z-cp.z)*(p.z-cp.z);
      if(len < min_len) {
        min_image     = i;
        min_len       = len;
      }
    }
  }

  return min_image;
}

NearImageManager::ImageInfo& NearImageManager::get_near_image(color_t cp) {
  // クラスタの中で一番色が近い画像を求める
  unsigned int min_used=max_use_count;
  int min_len=std::numeric_limits<int>::max();
  ImageInfo& min_image = dummy_image;
  int index = (int)(cp.x * color_step)
    + (int)(cp.y * color_step) * color_array_size
    + (int)(cp.z * color_step) * color_array_size_2;

  if(select_mode == SELECT_MODE_RECYCLE) {
    if(color_image_list_[index].size() == 1){
      min_image = color_image_list_[index].at(0);
    }else{
      for(ImageInfo& i : color_image_list_[index]) {
        const color_t& p = i.color;
        int len = (p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y) + (p.z-cp.z)*(p.z-cp.z);
        if(len < min_len) {
          min_image     = i;
          min_len       = len;
        }
      }
    }
  }else{
    for(ImageInfo& i : color_image_list_[index]) {
      const color_t& p = i.color;
      int len = (p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y) + (p.z-cp.z)*(p.z-cp.z);
      if(i.used < min_used || (i.used == min_used && len < min_len)) {
        min_image     = i;
        min_len       = len;
        min_used      = i.used;
      }
    }

    if(min_image == dummy_image) {
      min_image = get_near_unused_image(cp);
    }
    min_image.used++;
  }

  return min_image;
}

NearImageManager::ImageInfo& NearImageManager::get_near_image_precise(color_t cp) {
  // 一番色が近い画像を求める
  int min_len=std::numeric_limits<int>::max();
  ImageInfo& min_image = dummy_image;

  for(ImageInfo& i : image_list_) {
    const color_t& p = i.color;
    int len = (p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y) + (p.z-cp.z)*(p.z-cp.z);
    if(len < min_len) {
      min_image     = i;
      min_len       = len;
    }
  }

  return min_image;
}
