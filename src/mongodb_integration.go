package main

import (
	"fmt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
)

func Panicif(err error) {
	if err != nil {
		panic(err)
	}
}

func StartMongoSession(url string) (session *mgo.Session) {
	session, err := mgo.Dial(url)
	Panicif(err)
	return session
}

type document struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int
	Image_folder string
	Image_url    string
}

func getfilename(path string) string {
	path_breadcrumbs := strings.Split(path, "/")
	path_file := path_breadcrumbs[len(path_breadcrumbs)-1]
	return path_file
}

func main() {
	present_dir, err := os.Getwd()
	parent_directory := path.Dir(present_dir)
	images_directory := filepath.Join(parent_directory, "/Datasets/saiaprtc12ok/benchmark/saiapr_tc-12")
	files, err := filepath.Glob(images_directory + "/*")
	Panicif(err)
	url := "127.0.0.1:27017"
	session := StartMongoSession(url)
	defer session.Close()
	collection_handler := session.DB("image_annotation").C("images")
	index := mgo.Index{
		Key:        []string{"image_url", "image_no"},
		Unique:     true,
		DropDups:   true,
		Background: true,
		Sparse:     true,
	}
	err = collection_handler.EnsureIndex(index)
	Panicif(err)
	for _, image_dir := range files {
		image_folder := getfilename(image_dir)
		abs_image_dir, err := filepath.Abs(filepath.Join(image_dir + "/images"))
		images_path, err := filepath.Glob(abs_image_dir + "/*")
		Panicif(err)
		for _, image := range images_path {
			image_no, err := strconv.Atoi(strings.Split(getfilename(image), ".")[0])
			Panicif(err)
			err = collection_handler.Insert(&document{Image_no: image_no, Image_folder: image_folder, Image_url: image})
			Panicif(err)
		}
	}
	defer func() {
		if r := recover(); r != nil {
			fmt.Println("Recovered in f", r)
		}
	}()
}
