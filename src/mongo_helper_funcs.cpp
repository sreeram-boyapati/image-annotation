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

using namespace std;
using namespace cv;

void Mongo_run(mongo::DBClientConnection* c){
    c->connect("127.0.0.1:27017");
}

mongo::BSONObj MakeObj(cv::Mat & image_des, int image_no, string image_folder){
   mongo::BSONObjBuilder bob;
   bob.append("image_no", image_no);
   bob.append("image_folder", image_folder);
   mongo::BSONObjBuilder descriptors;
   for(int i=0; i<image_des.rows; i++){
       mongo::BSONArrayBuilder bab;
       for(int j=0; j<image_des.cols; j++){
            bab.append(image_des.at<int>(i, j));
       }
       ostringstream ss;
       ss<<i;
       descriptors.appendArray(ss.str(), bab.arr());
   }
   bob.append("descriptors", descriptors.obj());
   return bob.obj();
}

mongo::BSONObj MakeErrorObj(string error, int image_no, string image_folder){
    mongo::BSONObjBuilder bob;
    bob.append("image_no", image_no);
    bob.append("image_folder", image_folder);
    bob.append("error", error);

    return bob.obj();
}
