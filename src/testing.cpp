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

using namespace std;
using namespace cv;

int main(int argc, char *argv[]){
    if(argc==1){
        cout<<"No file number has been given"<<endl;
        return 0;
    }
    string file_path(argv[1]);
    cout<<file_path<<endl;
    Mat image = imread(file_path);

    mongo::DBClientConnection conn;
    Mongo_run(&conn);

    FileStorage fs("clusters_vocabulary.yml", FileStorage::READ);
    Mat vocabulary;
    fs["vocabulary"] >> vocabulary;

    Ptr<FeatureDetector> detector(new SurfFeatureDetector());
    Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher());
    Ptr<DescriptorExtractor> extractor = DescriptorExtractor::create("SURF");
    Ptr<BOWImgDescriptorExtractor> bowide(new BOWImgDescriptorExtractor(extractor,matcher));
    bowide->setVocabulary(vocabulary);

    Mat response_hist;
    vector<KeyPoint> keypoints;
    detector->detect(image,keypoints);
    bowide->compute(image, keypoints, response_hist);
    mongo::BSONObjBuilder bob;
    bob.append("hist", MakeMatObj(response_hist));
    conn.insert("image_annotation.testing_hists", bob.obj());
    return 0;
}
