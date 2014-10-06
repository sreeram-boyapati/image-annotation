#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "surffeatures.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/nonfree.hpp>
using namespace cv;
using namespace std;

Mat SURF_Feature_Detector(Mat src){
    //Hessian Threshold
    int minHession = 400;
    Mat temp;
    src.copyTo(temp);
    SurfFeatureDetector detector (minHession);
    vector<KeyPoint> keypoints;
    //Extract KeyPoints
    detector.detect(temp, keypoints);
    //Draw KeyPoints
    Mat img_keypoints;
    cout<<"Extract Keypoints: "<<keypoints.size()<<endl;
    drawKeypoints(temp, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    imshow("img", img_keypoints);
    waitKey(0);
    Mat descriptors;
    //Extract Descriptions of the Keypoint.
    Ptr<DescriptorExtractor> descriptionExtractor = DescriptorExtractor::create("SURF");
    descriptionExtractor->compute(temp, keypoints, descriptors);
    cout<<"Extract Features Rows: "<<descriptors.rows<< "Columns: "<<descriptors.cols<<endl;
    return img_keypoints;
}


bool flann_matcher(Mat img1_descriptors, Mat img2_descriptors){
    FlannBasedMatcher f_matcher;
    vector<DMatch> matches;
    f_matcher.match(img1_descriptors, img2_descriptors, matches);
    double max_dist = 0; double min_dist = 100;
    for(int i=0; i<img1_descriptors.rows; i++){
        double dist= matches[i].distance;
        if( dist < min_dist) min_dist = dist;
        if( dist > min_dist) max_dist = dist;
    }
    vector<DMatch> good_matches;
    for(int i=0; i< img1_descriptors.rows; i++){
        if(matches[i].distance <= max(2*min_dist, 0.02)){
            good_matches.push_back(matches[i]);
        }
    }
    //Algorithm to determine if they are similar or not.
    // If good_matches >= total_matches/3;
    if(good_matches.size() >= matches.size()/4){
        return true;
    }

    return false;
}
