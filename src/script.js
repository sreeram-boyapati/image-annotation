var db  = connect("127.0.0.1:27017/image_annotation");
var mr_cursor = db.mapped_responses.find({});
while(mr_cursor.hasNext()){
    mr = mr_cursor.next();
    img_br = db.imglevel_binary_results.findOne({image_no:mr["image_no"]});
    db.mapped_responses.update({image_no:mr["image_no"]}, {$set:{binary_results: img_br["entities"]}});
}