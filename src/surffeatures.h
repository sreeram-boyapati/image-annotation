
#ifndef SURFFEATURES_H_INCLUDED__
#define SURFFEATURES_H_INCLUDED__

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/nonfree.hpp>

cv::Mat SURF_Feature_Detector(cv::Mat src);

bool flann_matcher(cv::Mat img1_descriptors, cv::Mat img2_descriptors);

#endif
