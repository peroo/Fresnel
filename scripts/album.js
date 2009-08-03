var obj = Slingshot.params[0];
var log = Slingshot.Log;

var query;
if(!obj) {
    log('Fetching all albums');
    query =
    "SELECT A.id, title, date, name, sortname " +
    "FROM audio_album A " +
    "JOIN artist T ON A.artist=T.id " +
    "ORDER BY sortname, title, date";
}
else if(obj){
    log('Fetching album ' + obj);
    query =
    "SELECT R.id,title,tracknumber, length, bitrate, path_id, name " +
    "FROM resource R " +
    "JOIN audio_track A USING (id) " +
    "JOIN artist T ON artist=T.id " +
    "WHERE type=0 " +
    "AND A.album = " + obj + " " +
    "ORDER BY tracknumber ASC";
}
else {
    return;
}


query = Slingshot.Query(query);

var Output = [];
for(var track, i=0; track = query.rows[i]; ++i)
{
    Output.push(track);
}
