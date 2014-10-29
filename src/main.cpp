#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "surffeatures.h"
#include "segment_images.h"
#include "kmeans_wrapper.h"
#include "mongo_helper_funcs.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
using namespace cv;
using namespace std;

void DisplayImageLevelFeatures(mongo::DBClientConnection);
void BOW(mongo::DBClientConnection*);
void AddDescribtorstoDB(mongo::DBClientConnection *);

Mat ProcessImage(const string image_url, const int image_no){
    using namespace boost::algorithm;
    vector<string> tokens;
    //Preprocessing the strings.
    split(tokens, image_url, is_any_of("/"));
    string file_name = tokens[tokens.size() - 1];
    vector<string> filename_tokens;
    split(filename_tokens, file_name, is_any_of("."));
    string output_filename = filename_tokens[0] + "_pca" + ".jpg";
    tokens.pop_back();
    tokens.push_back(output_filename);
    string output_path = join(tokens, "/");
    cout<<"Output Path: "<<output_path<<endl;

    //Watershed Segmentation
    Mat sourceImage = imread(image_url);
    //imshow("Original Image", sourceImage);
    //Mat outputImage = multi_channelSegmentation(sourceImage);

    //Surf Feature Detection
    Mat img_descriptors = SURF_Feature_Detector(sourceImage);

    //Clustering descriptors
    //kmeans_wrapper(img_descriptors, 50, image_no);

    //imwrite("output.jpg", outputImage);
    //imshow("img", outputImage);
    //return outputImage;
    return img_descriptors;
}

int main(){
    initModule_nonfree();
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

    //To Add Image Descriptors to Mongodb
    //AddDescribtorstoDB(&c);

    //Generate Vocabulary
    BOW(&c);

    return 0;
}

void BOW(mongo::DBClientConnection *c){
    ofstream error_file;
    error_file.open("errors.txt");
    //Extract Image_Descriptors to add to BOW Trainer , 200 clusters
    BOWKMeansTrainer bowkmeans(200, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 100, 0.001), 1, KMEANS_PP_CENTERS);
    auto_ptr<mongo::DBClientCursor> cursor = c->query("image_annotation.image_descriptors", mongo::BSONObj());
    while(cursor->more()){
        mongo::BSONObj obj = cursor->next();
        int image_no = obj.getIntField("image_no");
        cout<<image_no<<endl;
        Mat descriptors = ExtractDescriptorMatrix(obj.getObjectField("descriptors"), image_no);
        if(!descriptors.empty()){
            bowkmeans.add(descriptors);
        }
        else{
            cout<<"Empty Descriptors: "<<image_no<<endl;
            error_file <<image_no<<"\n" ;
        }
    }
    error_file.close();
    Mat dictionary = bowkmeans.cluster();
    FileStorage fs("clusters_vocavulary.yml", FileStorage::WRITE);
    fs << "vocabulary" << dictionary;
    fs.release();
}

void DisplayImageLevelFeatures(mongo::DBClientConnection c, const int image_no){
    auto_ptr<mongo::DBClientCursor> result_cursor = c.query("image_annotation.imglevel_results", QUERY("image_no" << image_no));
    mongo::BSONObj image_result = result_cursor->next();
    vector<mongo::BSONElement> entities = image_result.getField("entities").Array();
    vector<string> result_entities;
    for(vector<mongo::BSONElement>::iterator it = entities.begin(); it != entities.end(); it++){
        string str(it->valuestrsafe());
        result_entities.push_back(str);
    }
}

void AddDescribtorstoDB(mongo::DBClientConnection* c){
    auto_ptr<mongo::DBClientCursor> cursor = c->query("image_annotation.images", mongo::BSONObj());

    while(cursor->more()){
        mongo::BSONObj image_object = cursor->next();
        int image_no = image_object.getIntField("image_no");
        string image_folder = image_object.getStringField("image_folder");
        string image_url = image_object.getStringField("image_url");
        //To Process the image
        Mat image_descriptors = ProcessImage(image_url, image_no);
        //To add image descriptors to mongodb database
        if(!image_descriptors.empty()){
            try{
                mongo::BSONObj bsonObject = MakeObj(image_descriptors, image_no, image_folder);
                c->insert("image_annotation.image_descriptors", bsonObject);
            }
            catch(Exception e){
                string str(e.msg);
                mongo::BSONObj bsonObject = MakeErrorObj(str, image_no, image_folder);
                c->insert("image_annotation.image_descriptors", bsonObject);
            }
        }
        //To display image level features
        // DisplayImageLevelFeatures(c);
    }
}

