#ifndef MONGO_HELPER_FUNCS_H_INCLUDED__
#define MONGO_HELPER_FUNCS_H_INCLUDED__

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

mongo::BSONObj MakeMatObj(cv::Mat& matrix);

mongo::BSONArray MakeArrayObj(mongo::BSONObj entity, const std::string type);

mongo::BSONObj MakeObj(cv::Mat & image_des, int image_no, std::string image_folder);

mongo::BSONObj MakeErrorObj(std::string error, int image_no, std::string image_folder);

cv::Mat ExtractMatrixObj(mongo::BSONObj descriptor_object);

void Mongo_run(mongo::DBClientConnection* c);

#endif // MONGO_HELPER_FUNCS_H
