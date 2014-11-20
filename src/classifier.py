import pymongo
import numpy as np
from sklearn.neighbors import KNeighborsClassifier
from sklearn.multiclass import OneVsRestClassifier
from sklearn.svm import SVC
from sklearn.preprocessing import MultiLabelBinarizer
import pickle
import os
from os.path import dirname, abspath, join
indexes = {}

def getBuildDir():
    parent_dir = dirname(os.getcwd())
    build_dir = join(parent_dir, 'pkg')
    return build_dir

def fit_images():
    client = pymongo.MongoClient('localhost', 27017)
    db = client['image_annotation']
    responses = db['mapped_responses'].find()
    no_labels = db['labels_binary'].find()
    numbers = []
    for i in no_labels:
        numbers.append(set([int(i["number"])]))
    train_data = []
    labels = []
    i=0
    mlb = MultiLabelBinarizer()
    mlb.fit(numbers)
    for index, instance in enumerate(responses):
        t_data =  instance['hist']['0']
        indexes[index] = instance['image_no']
        train_data.append(t_data)
        label = instance['binary_results']
        new_labels = []
        for key, value in enumerate(label):
            value1 = int(value)
            new_labels.append(set([value1]))
        new_labels = mlb.transform(new_labels)
        labels.append(label)
    classifier = KNeighborsClassifier(n_neighbors = 5, weights='uniform')
    classifier.fit(train_data, labels)
    build_dir = getBuildDir()
    pickle.dump(classifier, open(join(build_dir, 'model.data'),'w'),protocol=1)
    client.close()

def assign_labels(predictions):
    labels = predictions[1][0]
    true_index = []
    for image in labels:
        true_index.append(indexes[image])
    print true_index
    client = pymongo.MongoClient('localhost', 27017)
    responses = []
    db = client['image_annotation']
    for index in true_index:
        response = db.mapped_responses.find({'image_no':index})
        labels = response[0]['classes']
        responses.append(labels)
    labels_dict = {}
    for each_image in responses:
        for label in each_image:
            if label in labels_dict:
                labels_dict[label] = labels_dict[label] + 1
            else:
                labels_dict[label] = 1
    assigned_labels = []
    for key in labels_dict:
        if labels_dict[key]  > 1:
            assigned_labels.append(key)
    client.close()
    return assigned_labels

def predict_annotations():
    import ipdb
    build_dir = getBuildDir()
    classifier = pickle.load(open(join(build_dir, "model.data"), "rb"))
    client = pymongo.MongoClient('localhost', 27017)
    db = client["image_annotation"]
    test_hists = db['testing_hists'].find()
    i = 0
    for test in test_hists:
        t_data = test['hist']['0']
        predictions =classifier.kneighbors(t_data)
        assigned_labels = assign_labels(predictions)
        if i==2:
            print assigned_labels
        i=i+1
    client.close()

if __name__ == '__main__':
    fit_images()
    predict_annotations()








