var obj = Slingshot.params[0];
var id = Slingshot.params[1];
var log = Slingshot.Log;

var query;
if(obj == "path") {
    log('Fetching images in' + id);
    query =
    "SELECT id, filename " +
    "FROM resource " +
    "WHERE type=1 " +
    "AND path_id=" + id;
}
else {
    return;
}


query = Slingshot.Query(query);

var Output = [];
for(var image, i=0; image = query.rows[i]; ++i)
{
    Output.push(image);
}
