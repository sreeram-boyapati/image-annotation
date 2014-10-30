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

mongo::BSONObj MakeMatObj(Mat& matrix){
    mongo::BSONObjBuilder matrix_obj;
    for(int i=0; i<matrix.rows; i++){
        mongo::BSONArrayBuilder bab;
        for(int j=0; j<matrix.cols; j++){
             bab.append(matrix.at<int>(i, j));
        }
        ostringstream ss;
        ss<<i;
        matrix_obj.appendArray(ss.str(), bab.arr());
    }
    return matrix_obj.obj();
}

mongo::BSONObj MakeObj(cv::Mat & image_des, int image_no, string image_folder){
   mongo::BSONObjBuilder bob;
   bob.append("image_no", image_no);
   bob.append("image_folder", image_folder);
   bob.append("descriptors", MakeMatObj(image_des));
   return bob.obj();
}

mongo::BSONObj MakeErrorObj(string error, int image_no, string image_folder){
    mongo::BSONObjBuilder bob;
    bob.append("image_no", image_no);
    bob.append("image_folder", image_folder);
    bob.append("error", error);

    return bob.obj();
}

Mat ExtractMatrixObj(mongo::BSONObj descriptor_object){
    Mat descriptors = Mat::zeros(descriptor_object.nFields(), 64, CV_32F);
    mongo::BSONObjIterator it =  descriptor_object.begin();
    while(it.more()){
        mongo::BSONElement elem= it.next();
        vector<mongo::BSONElement> array =  elem.Array();
        Mat column = Mat::zeros(1, 64, CV_32F);
        for(int i=0; i!= array.size(); i++){
            mongo::BSONElement elem = array[i];
            column.at<int>(0, i) += elem.Int();
            //cout<<"Added Element: "<< column.at<int>(0, i) <<" Element: "<<elem.Int()<<endl;
        }
        descriptors.push_back(column);
    }
    return descriptors;
}

mongo::BSONArray MakeArrayObj(mongo::BSONObj entity, const string type){
    vector<mongo::BSONElement> entities;
    entity.elems(entities);
    mongo::BSONArrayBuilder bab;
    for(int i=0; i<entities.size(); i++){
        if(type.compare("string") == 0){
            bab.append(entities[i].String());
        }
        else if(type.compare("int") == 0){
            bab.append(entities[i].Int());
        }
    }
    return bab.arr();
}
