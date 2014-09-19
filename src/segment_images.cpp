#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
using namespace cv;
using namespace std;

void run(mongo::DBClientConnection* c){
    c ->connect("127.0.0.1:27017");
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
    cout<<"Count of Images: "<<c.count("image_annotation.images")<<endl;
    return 0;
}
