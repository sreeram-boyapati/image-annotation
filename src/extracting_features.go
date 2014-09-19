package main

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

func Panicif(err error) {
	if err != nil {
		panic(err)
	}
}

func Process_ontologyfile(folder_path string, ontology_path string) {
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
	present_directory, err := path.Dir(os.Getwd())
	Panicif(err)
	dataset_directory := filepath.Join(present_directory, "/Datasets/saiaprtc12ok/benchmark/saiapr_tc-12")
	files, err := filepath.Glob(dataset_directory + "/*")
	Panicif(err)

	for _, file := range files {
		fmt.Println(file)
		ontology_path := filepath.Join(file, "ontology_path.txt")
		Process_ontologyfile(file, ontology_path)
	}
	defer func() {
		if r := recover(); r != nil {
			fmt.Println("Recovered in f", r)
		}
	}()
}
