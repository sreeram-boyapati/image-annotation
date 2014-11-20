package main

import (
	"fmt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
)

func Panicif(err error) {
	if err != nil {
		panic(err)
	}
}

type Imglevel_result struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int           `bson:"image_no"`
	Image_folder string        `bson:"image_folder"`
	Entities     []string      `bson:"entities"`
}

type Imglevel_binary_result struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int           `bson:"image_no"`
	Image_folder string        `bson:"image_folder"`
	Entities     []int         `bson:"entities"`
}

type Label_binary struct {
	ID     bson.ObjectId `bson:"_id,omitempty"`
	Label  string        `bson:"label"`
	Number int           `bson:"number"`
}

type Mapped_Response struct {
	ID          bson.ObjectId   `bson:"_id,omitempty"`
    Hist        map[int][]int   `bson:"hist"`
    Image_no    int             `bson:"image_no"`
    Classes     []int           `bson:"classes"`
}

type Mapped_Binary_Response struct{
    ID              bson.ObjectId   `bson:"_id,omitempty"`
    Hist            map[int][]int   `bson:"hist"`
    Image_no        int             `bson:"image_no"`
    Binary_Classes  []int           `bson:"binary_classes"`
}

var (
	labels      map[string]int
	counter     int = 1
	img_results []Imglevel_result
	ok          bool
    m_responses     []Mapped_Response
)

func label_binarizer(entities []string) {
	for _, entity := range entities {
		if _, ok = labels[entity]; !ok {
			fmt.Println(entity)
			labels[entity] = counter
			counter += 1
		}
	}
}

func main() {
	session, err := mgo.Dial("127.0.0.1:27017")
	Panicif(err)
	defer session.Close()

	labels = make(map[string]int)

	image_c := session.DB("image_annotation").C("imglevel_results")
	err = image_c.Find(nil).All(&img_results)
	Panicif(err)

	for _, image := range img_results {
		entities := image.Entities
		label_binarizer(entities)
	}

	/*labels_c := session.DB("image_annotation").C("labels_binary")
	for key, value := range labels {
		err = labels_c.Insert(&Label_binary{Label: key, Number: value})
		Panicif(err)
	}*/

	images_b_c := session.DB("image_annotation").C("imglevel_binary_results")
	for _, image := range img_results {
		entities := image.Entities
		var b_entities []int
		for _, entity := range entities {
			b_entities = append(b_entities, labels[entity])
		}
		images_b_c.Insert(&Imglevel_binary_result{Image_no: image.Image_no, Image_folder: image.Image_folder, Entities: b_entities})
	}

	err := session.DB("images_annotation").C("mapped_responses").Find(nil).All(&m_responses)
    PanicIf(err)
    mb_responses := session.DB("images_annotation").C("mapped_binary_responses")

    for _, response := range m_responses {
        image_no := response.Image_no
        var image_b Imglevel_binary_result
        images_b_c.Find(bson.M{"image_no":image_no}).One(&image_b)
        mb_responses.Insert(&
    }
}
