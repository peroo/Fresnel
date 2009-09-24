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
    "ORDER BY A.title, I.name, tracknumber ASC";
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
    "ORDER BY A.title, tracknumber ASC";
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

var Plain = ["#EXTM3U", ""];
for(var track, i=0; track = query.rows[i]; ++i)
{
    if(track.artist)
        Plain.push("#EXTINF:" + track.length + ", " + track.artist + " - " + track.title);
        Plain.push("http://129.241.122.50:9999/resource/" + track.id + "/" + track.tracknumber + '_' + String(track.artist).replace(/ /g, '_') + "_-_" + String(track.title).replace(/ /g, '_') + '.ogg');
        Plain.push("");
}
Plain = Plain.join("\r\n");
