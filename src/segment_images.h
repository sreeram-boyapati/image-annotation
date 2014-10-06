

#ifndef SEGMENT_IMAGES_H
#define SEGMENT_IMAGES_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "surffeatures.h"
#include "segment_images.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

cv::Mat NormalizeImage(cv::Mat source);

void run(mongo::DBClientConnection* c);

cv::Mat ProcessImage(const std::string image_url);

cv::Mat multi_channelSegmentation(cv::Mat src);

mongo::BSONObj StoreinDatabase(cv::Mat & image_des, int image_no, std::string image_folder);

void run(mongo::DBClientConnection* c);

int main(int argc, char *argv[]);

#endif // SEGMENT_IMAGES_H
