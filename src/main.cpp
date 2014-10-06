#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "surffeatures.h"
#include "segment_images.h"
#include "mongo_helper_funcs.h"
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
        mongo::BSONObj image_object = cursor->next();
        //Mat image_descriptors = ProcessImage(image_object.getStringField("image_url"));
        int image_no = image_object.getIntField("image_no");
        auto_ptr<mongo::DBClientCursor> result_cursor = c.query("image_annotation.imglevel_results", QUERY("image_no" << image_no));
        mongo::BSONObj image_result = result_cursor->next();
        vector<mongo::BSONElement> entities = image_result.getField("entities").Array();
        vector<string> result_entities;
        for(vector<mongo::BSONElement>::iterator it = entities.begin(); it != entities.end(); it++){
            string str(it->valuestrsafe());
            result_entities.push_back(str);
        }
        for(vector<string>::iterator it = result_entities.begin(); it != result_entities.end(); it++){
            cout<<*it<<endl;
        }
        string image_folder = image_object.getStringField("image_folder");
        /*try{
            mongo::BSONObj bsonObject = MakeObj(image_descriptors, image_no, image_folder);
            c.insert("image_annotation.image_descriptors", bsonObject);
        }
        catch(Exception e){  mongo::BSONObj::iterator it = entities.begin();
            string str(e.msg);
            mongo::BSONObj bsonObject = MakeErrorObj(str, image_no, image_folder);
            c.insert("image_annotation.image_descriptors", bsonObject);
        }*/
        break;
    }
    return 0;
}
