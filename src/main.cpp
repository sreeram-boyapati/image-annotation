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
void WeightedVectorGenerator(mongo:: DBClientConnection *);

Mat ProcessImage(const string image_url){
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
    //string output_path = join(tokens, "/");
    //cout<<"Output Path: "<<output_path<<endl;

    Mat sourceImage = imread(image_url);
    //imshow("Original Image", sourceImage);

    //Watershed Segmentation
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

    //Generate Histograms
    //WeightedVectorGenerator(&c);

    return 0;
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
        Mat image_descriptors = ProcessImage(image_url);
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

void BOW(mongo::DBClientConnection *c){
    ofstream error_file;
    ofstream images_used;
    images_used.open("images_used.txt", ios::out);
    error_file.open("errors.txt", ios::out);
    //Extract Image_Descriptors to add to BOW Trainer , 200 clusters
    BOWKMeansTrainer bowkmeans(1000, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 100, 0.001), 1, KMEANS_PP_CENTERS);
    auto_ptr<mongo::DBClientCursor> cursor = c->query("image_annotation.image_descriptors", mongo::BSONObj());
    int i=0;
    while(cursor->more() && i < 7000){
        mongo::BSONObj obj = cursor->next();
        int image_no = obj.getIntField("image_no");
        cout<<i << " + " << image_no << endl;
        Mat descriptors = ExtractMatrixObj(obj.getObjectField("descriptors"));
        if(!descriptors.empty()){
            bowkmeans.add(descriptors);
            images_used << image_no;
            images_used << "\n";
            i++;
        }
        else{
            cout<<"Empty Descriptors: "<<image_no<<endl;
            error_file<<image_no;
            error_file<<"\n";
        }
    }
    error_file.close();
    images_used.close();
    Mat dictionary = bowkmeans.cluster();
    FileStorage fs("clusters_vocabulary.yml", FileStorage::WRITE);
    fs << "vocabulary" << dictionary;
    fs.release();
}

void WeightedVectorGenerator(mongo::DBClientConnection *c){
    Ptr<FeatureDetector> detector(new SurfFeatureDetector());
    Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher());
    Ptr<DescriptorExtractor> extractor = DescriptorExtractor::create("SURF");
    Ptr<BOWImgDescriptorExtractor> bowide(new BOWImgDescriptorExtractor(extractor,matcher));
    FileStorage fs("clusters_vocabulary.yml", FileStorage::READ);
    Mat vocabulary;
    fs["vocabulary"] >> vocabulary;
    bowide->setVocabulary(vocabulary);
    auto_ptr<mongo::DBClientCursor> cursor = c->query("image_annotation.sample_images", mongo::BSONObj());
    while(cursor->more()){
        mongo::BSONObj image_obj = cursor->next();
        string image_path = image_obj.getStringField("image_url");
        int image_no = image_obj.getIntField("image_no");
        auto_ptr<mongo::DBClientCursor> result_cursor =
                c->query("image_annotation.sample_results", QUERY("image_no" << image_no));
        mongo::BSONObj entities = result_cursor->next().getObjectField("entities");
        Mat image = imread(image_path);
        Mat response_hist;
        vector<KeyPoint> keypoints;
        detector->detect(image,keypoints);
        bowide->compute(image, keypoints, response_hist);
        mongo::BSONObjBuilder bob;
        bob.append("image_no", image_no);
        bob.append("hist", MakeMatObj(response_hist));
        bob.append("classes", MakeArrayObj(entities, "string"));
        c->insert("image_annotation.mapped1_responses", bob.obj());
    }
}
