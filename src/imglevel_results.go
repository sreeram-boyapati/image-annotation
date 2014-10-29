package main

import (
	"fmt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
)

type result struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int64         `bson:"image_no"`
	Image_seg_no int64         `bson:"image_seg_no"`
	Entities     []string      `bson:"entities"`
}

type Img_result struct {
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

func Panicif(err error) {
	if err != nil {
		panic(err)
	}
}

func ConnectoMongoDB(url string) (session *mgo.Session) {
	session, err := mgo.Dial(url)
	Panicif(err)
	return session
}

func queriesByNumbers() {

}

func main() {
	var results []result
	var img_results []Img_result
	session := ConnectoMongoDB("127.0.0.1:27017")
	defer session.Close()
	results_collection := session.DB("image_annotation").C("results")
	imglevel_results_collection := session.DB("image_annotation").C("imglevel_results")
	images_collection := session.DB("image_annotation").C("images")
	err := images_collection.Find(nil).All(&img_results)
	Panicif(err)
	for _, img_result := range img_results {
		fmt.Println(img_result.Image_no)
		err := results_collection.Find(bson.M{"image_no": img_result.Image_no}).All(&results)
		Panicif(err)
		distinct_entities := []string{}
		distinct_entities_map := map[string]string{}
		for _, result := range results {
			entities := result.Entities
			for _, entity := range entities {
				if _, ok := distinct_entities_map[entity]; !ok {
					if entity == "" {
						continue
					}
					distinct_entities = append(distinct_entities, entity)
					distinct_entities_map[entity] = entity
				}
			}
		}
		if img_result.Image_no == 17945 {
			err = imglevel_results_collection.Insert(&Imglevel_result{Image_no: img_result.Image_no, Image_folder: img_result.Image_folder, Entities: distinct_entities})
			Panicif(err)
			break
		}
	}

	defer func() {
		if r := recover(); r != nil {
			fmt.Println("Recovered in f", r)
		}
	}()
}
