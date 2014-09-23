#include <iostream>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include<opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace cv;
using namespace std;

void run(mongo::DBClientConnection* c){
    c->connect("127.0.0.1:27017");
}

Mat NormalizeImage(Mat source){
    int channels = source.channels();
    Mat dst;
    switch(channels){
    case 1:
        normalize(source, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        normalize(source, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        source.copyTo(dst);
        break;
    }
    return dst;
}

Mat multi_channelSegmentation(Mat src){
    Mat binary_image;
    Mat dist;
    namedWindow("samples");
    cvtColor(src, binary_image, CV_BGR2GRAY);
    //Distance Transform
    imshow("samples", binary_image);
    waitKey(0);
    distanceTransform(binary_image, dist, CV_DIST_L2, 3);
    //Normalize to [0, 1]
    imshow("samples", dist);
    waitKey(0);
    normalize(dist, dist, 0, 1., NORM_MINMAX);
    //Take a threshold of 0.2
    threshold(dist, dist, .2, 1., CV_THRESH_BINARY);
    //Convert it into 8 Unsigned single channel immage
    imshow("samples", dist);
    waitKey(0);
    Mat dist_8u;
    dist.convertTo(dist_8u, CV_8U);
    //Find Markers
    vector<vector<Point> > contours;
    findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    //Total Objects
    int ncomp = contours.size();
    //Segmenting foreground Images
    Mat markers = Mat::zeros(dist.size(), CV_32SC1);
    imshow("samples", dist);
    waitKey(0);
    for (int i = 0; i < ncomp; i++)
        drawContours(markers, contours, i, cv::Scalar::all(i+1), -1);
    imshow("samples", dist);
    waitKey(0);
    circle(markers, cv::Point(5,5), 3, CV_RGB(255,255,255), -1);
    watershed(src, markers);
    cv::Mat dst = cv::Mat::zeros(markers.size(), CV_8UC3);
    std::vector<cv::Vec3b> colors;
    for (int i = 0; i < ncomp; i++)
    {
        int b = cv::theRNG().uniform(0, 255);
        int g = cv::theRNG().uniform(0, 255);
        int r = cv::theRNG().uniform(0, 255);

        colors.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
    }
    for (int i = 0; i < markers.rows; i++)
    {
        for (int j = 0; j < markers.cols; j++)
        {
            int index = markers.at<int>(i,j);
            if (index > 0 && index <= ncomp)
                dst.at<cv::Vec3b>(i,j) = src.at<cv::Vec3b>(i, j);
            else
                dst.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
        }
    }
    return dst;
}

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
    Mat descriptors;
    //Extract Descriptions of the Keypoint.
    Ptr<DescriptorExtractor> descriptionExtractor = DescriptorExtractor::create("SURF");
    descriptionExtractor->compute(temp, keypoints, descriptors);
    cout<<"Extract Features Rows: "<<descriptors.rows<< "Columns: "<<descriptors.cols<<endl;
    return descriptors;
}

void ProcessImage(const string image_url){
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
    //Mat outputImage = multi_channelSegmentation(sourceImage);
    Mat img_descriptors = SURF_Feature_Detector(sourceImage);
    namedWindow("surf");
    //imshow("surf", img_descriptors);
    waitKey(0);
}

int main(int argc, char *argv[]){
    mongo::client::initialize();
    mongo::DBClientConnection c;
    try{
        run(&c);
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
        ProcessImage(result.getStringField("image_url"));
        i++;
        if(i==1){
            break;
        }
    }
    return 0;
}
