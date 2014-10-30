package main

import (
	"bufio"
	"fmt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"os"
	"strconv"
)

func Panicif(err error) {
	if err != nil {
		panic(err)
	}
}

type Result struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int64         `bson:"image_no"`
	Image_seg_no int64         `bson:"image_seg_no"`
	Entities     []string      `bson:"entities"`
}

type Image struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int           `bson:"image_no"`
	Image_folder string        `bson:"image_folder"`
	Image_url    string        `bson:"image_url"`
}

type Imglevel_result struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int           `bson:"image_no"`
	Image_folder string        `bson:"image_folder"`
	Entities     []string      `bson:"entities"`
}

func StartMongoSession(url string) (session *mgo.Session) {
	session, err := mgo.Dial(url)
	Panicif(err)
	return session
}

func main() {
	session := StartMongoSession("127.0.0.1:27017")
	images_used, err := os.Open("images_used.txt")
	Panicif(err)
	scanner := bufio.NewScanner(images_used)
	images_collection := session.DB("image_annotation").C("images")
	results_collection := session.DB("image_annotation").C("sample_images")
	/*var images []Image
	err = images_collection.Find(nil).All(&images)
	for _, image := range images {
		fmt.Println(image)
	}*/
	for scanner.Scan() {
		var image_one []Image
		image_no, err := strconv.Atoi(scanner.Text())
		Panicif(err)
		fmt.Println(image_no)
		err = images_collection.Find(bson.M{"image_no": image_no}).All(&image_one)
		Panicif(err)
		for _, image := range image_one {
			results_collection.Insert(image)
		}

	}
}
