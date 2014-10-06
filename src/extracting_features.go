package main

import (
	"bufio"
	"fmt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
)

func Panicif(err error) {
	if err != nil {
		panic(err)
	}
}

type document struct {
	ID           bson.ObjectId `bson:"_id,omitempty"`
	Image_no     int64         `bson:"image_no"`
	Image_seg_no int64         `bson:"image_seg_no"`
	Entities     []string      `bson:"entities"`
}

func ConnectoMongoDB(url string) (session *mgo.Session) {
	session, err := mgo.Dial(url)
	Panicif(err)
	return session
}

func AddtoMongoDB(string_to_be_written string, session *mgo.Session) {
	var i int = 2
	var entities []string
	splitted_strings := strings.Split(string_to_be_written, ";")
	image_no, err := strconv.ParseInt(splitted_strings[0], 0, 64)
	image_seg_no, err := strconv.ParseInt(splitted_strings[1], 0, 64)
	for i = 2; i < len(splitted_strings); i++ {
		entities = append(entities, splitted_strings[i])
	}
	collection_handler := session.DB("image_annotation").C("results")
	err = collection_handler.Insert(&document{Image_no: image_no, Image_seg_no: image_seg_no, Entities: entities})
	Panicif(err)
	defer func() {
		if r := recover(); r != nil {
			fmt.Println("Recovered in f", r)
		}
	}()
}

func Process_ontologyfile(folder_path string, ontology_path string, waitGroup *sync.WaitGroup) {
	defer waitGroup.Done()
	session := ConnectoMongoDB("127.0.0.1:27017")
	defer session.Close()
	ontology_file, err := os.Open(ontology_path)
	Panicif(err)
	ontology_csv, err := os.Create(folder_path + "/ontology_formatted.csv")
	Panicif(err)
	writer := bufio.NewWriter(ontology_csv)
	scanner := bufio.NewScanner(ontology_file)
	for scanner.Scan() {
		single_line := scanner.Text()
		splitted_strings := strings.Fields(single_line)
		image_no := splitted_strings[0]
		image_seg_no := splitted_strings[1]
		entities := splitted_strings[2]
		splitted_entities := strings.Split(entities, "->->")
		if len(splitted_entities) == 1 {
			string_to_be_written := image_no + ";" + image_seg_no + ";" + splitted_entities[0] + ";\n"
			writer.WriteString(string_to_be_written)
			writer.Flush()
			continue
		}
		splitted_distinctive_entities := strings.Split(splitted_entities[1], "->")
		string_to_be_written := image_no + ";" + image_seg_no + ";" + splitted_entities[0] + ";"
		for _, splitted_entity := range splitted_distinctive_entities {
			string_to_be_written += splitted_entity + ";"
		}
		AddtoMongoDB(string_to_be_written, session)
		string_to_be_written += "\n"
		writer.WriteString(string_to_be_written)
		writer.Flush()
	}
	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "reading standard input:", err)
	}

	defer ontology_csv.Close()
	defer ontology_file.Close()
	defer func() {
		if r := recover(); r != nil {
			fmt.Println("Recovered in f", r)
		}
	}()
}

func main() {
	present_directory, err := os.Getwd()
	parent_directory := path.Dir(present_directory)
	Panicif(err)
	dataset_directory := filepath.Join(parent_directory, "/Datasets/saiaprtc12ok/benchmark/saiapr_tc-12")
	files, err := filepath.Glob(dataset_directory + "/*")
	Panicif(err)
	var waitGroup sync.WaitGroup
	for _, file := range files {
		waitGroup.Add(1)
		fmt.Println(file)
		ontology_path := filepath.Join(file, "ontology_path.txt")
		go Process_ontologyfile(file, ontology_path, &waitGroup)
	}
	waitGroup.Wait()
	defer func() {
		if r := recover(); r != nil {
			fmt.Println("Recovered in f", r)
		}
	}()
}
