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
using namespace cv;
using namespace std;

void Mongo_run(mongo::DBClientConnection* c){
    c->connect("127.0.0.1:27017");
}

Mat ProcessImage(const string image_url){
    using namespace boost::algorithm;
    vector<string> tokens;
    split(tokens, image_url, is_any_of("/"));
    string file_name = tokens[tokens.size() - 1];
    vector<string> filename_tokens;
    split(filename_tokens, file_name, is_any_of("."));
    string output_filename = filename_tokens[0] + "_pca" + ".jpg";
    tokens.pop_back();
    tokens.push_back(output_filename);
    string output_path = join(tokens, "/");
    cout<<"Output Path: "<<output_path<<endl;
    Mat sourceImage = imread(image_url);
    //imshow("Original Image", sourceImage);
    //Mat outputImage = multi_channelSegmentation(sourceImage);
    Mat img_descriptors = SURF_Feature_Detector(sourceImage);
    //imwrite("output.jpg", outputImage);
    //imshow("img", outputImage);
    //return outputImage;
    return img_descriptors;
}

mongo::BSONObj MakeObj(Mat & image_des, int image_no, string image_folder){
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

int main(int argc, char *argv[]){
    mongo::client::initialize();
    mongo::DBClientConnection c;
    try{
        Mongo_run(&c);
        cout<<"Running"<<endl;
    }
    catch(Exception e){
        cout<<e.msg<<endl;
    }
    //Get 20000 Images and segment them.
    cout<<"Count of Images: "<<c.count("image_annotation.images")<<endl;
    auto_ptr<mongo::DBClientCursor> cursor = c.query("image_annotation.images", mongo::BSONObj());
    int i=0;
    while(cursor->more()){
        mongo::BSONObj result = cursor->next();
        Mat image_descriptors = ProcessImage(result.getStringField("image_url"));
        int image_no = result.getIntField("image_no");
        string image_folder = result.getStringField("image_folder");
        try{
            mongo::BSONObj bsonObject = MakeObj(image_descriptors, image_no, image_folder);
            c.insert("image_annotation.image_descriptors", bsonObject);
        }
        catch(Exception e){
            string str(e.msg);
            mongo::BSONObj bsonObject = MakeErrorObj(str, image_no, image_folder);
            c.insert("image_annotation.image_descriptors", bsonObject);
        }
    }
    return 0;
}
