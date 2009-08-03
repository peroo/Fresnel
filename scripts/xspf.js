var search = Slingshot.params[0];
var obj = Slingshot.params[1];
var log = Slingshot.Log;

var query;
if(search == 'artist') {
    log('artist');
    query =
    "SELECT R.id,T.title,I.name AS artist, A.title AS album,tracknumber, length, bitrate " +
    "FROM resource R " +
    "JOIN audio_track T USING (id) " +
    "JOIN audio_album A ON album=A.id " +
    "JOIN artist I ON T.artist=I.id " +
    "WHERE type=0 " +
    "AND I.name LIKE '%" + obj + "%' " +
    "ORDER BY A.date, I.name, A.title, tracknumber ASC";
}
else if(search == 'album') {
    query =
    "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length, bitrate " +
    "FROM resource R " +
    "JOIN audio_track T USING (id) " +
    "JOIN audio_album A ON album=A.id " +
    "JOIN artist I ON T.artist=I.id " +
    "WHERE type=0 " +
    "AND A.title LIKE '%" + obj + "%' " +
    "ORDER BY A.date, A.title, tracknumber ASC";
}
else if(search == 'random') {
    var count = parseInt(obj) || 10;
    query =
    "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length, bitrate " +
    "FROM resource R " +
    "JOIN audio_track T USING (id) " +
    "JOIN audio_album A ON album=A.id " +
    "JOIN artist I ON T.artist=I.id " +
    "WHERE type=0 " +
    "ORDER BY random() " +
    "LIMIT " + count;
}
else {
    return;
}

query = Slingshot.Query(query);

var Plain = ['<?xml version="1.0" encoding="UTF-8"?>', '<playlist version="1" xmlns="http://xspf.org/ns/0/">', '\t<trackList>'];
for(var track, i=0; track = query.rows[i]; ++i)
{
    Plain.push("\t\t<track>");
    Plain.push("\t\t\t<location>" + "http://129.241.122.50:9999/resource/" + track.id + "/" + String(track.artist).replace(/ /g, '_') + "_-_" + String(track.title).replace(/ /g, '_') + ".ogg</location>");
    Plain.push("\t\t\t<title>" + track.title + "</title>");
    Plain.push("\t\t\t<creator>" + track.artist + "</creator>");
    Plain.push("\t\t\t<album>" + track.album + "</album>");
    Plain.push("\t\t\t<trackNum>" + track.tracknumber + "</trackNum>");
    Plain.push("\t\t\t<duration>" + track.length * 1000 + "</duration>");
    Plain.push("\t\t</track>");
}
Plain.push("\t</trackList>");
Plain.push("</playlist>");

for(var i=0, str; str = Plain[i]; ++i) {
    Plain[i] = Plain[i].replace(/&/g, '&amp;');
}
Plain = Plain.join("\r\n");
Mimetype = "application/xspf+xml";
