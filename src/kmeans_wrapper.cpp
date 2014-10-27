#include "kmeans_wrapper.h"
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

/*
 * Kmeans is used to compact the surf descriptors.
 */
void kmeans_wrapper(Mat descriptor_matrix, int clusters, int index){
    Mat result;
    Mat centers;
    kmeans(descriptor_matrix, clusters, result, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 3, KMEANS_RANDOM_CENTERS, centers);
    cout<<"Columns of Center: "<<centers.cols<<endl;
    cout<<"Rows of Center: "<<centers.rows<<endl;
}
