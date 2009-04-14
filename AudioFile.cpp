#include "AudioFile.h"

#include <boost/filesystem/convenience.hpp>
#include <taglib/xiphcomment.h>

namespace fs = boost::filesystem;

bool AudioFile::readMeta()
{
    if(fs::extension(file) == ".flac") {

        TagLib::FLAC::File *flac = new TagLib::FLAC::File(file.string().c_str(), false);
        readXiphComment(flac->xiphComment(true));
        delete flac;
    } else {
        return false;
    }

    return true;
}

void AudioFile::readXiphComment(const TagLib::Ogg::XiphComment *tag)
{
    const TagLib::Ogg::FieldListMap map = tag->fieldListMap();
    if (!map["ALBUM"].isEmpty())
        album = map["ALBUM"].front().to8Bit(true);
    if (!map["ALBUMARTIST"].isEmpty())
        albumartist = map["ALBUMARTIST"].front().to8Bit(true);
    if (!map["ALBUMARTISTSORT"].isEmpty())
        albumartistsort = map["ALBUMARTISTSORT"].front().to8Bit(true);
    if (!map["ARTIST"].isEmpty())
        artist = map["ARTIST"].front().to8Bit(true);
    if (!map["ARTISTSORT"].isEmpty())
        artistsort = map["ARTISTSORT"].front().to8Bit(true);
    if (!map["MUSICBRAINZ_ALBUMARTISTID"].isEmpty())
        musicbrainz_albumartistid = map["MUSICBRAINZ_ALBUMARTISTID"].front().to8Bit(true);
    if (!map["MUSICBRAINZ_ALBUMID"].isEmpty())
        musicbrainz_albumid = map["MUSICBRAINZ_ALBUMID"].front().to8Bit(true);
    if (!map["MUSICBRAINZ_ARTISTID"].isEmpty())
        musicbrainz_artistid = map["MUSICBRAINZ_ARTISTID"].front().to8Bit(true);
    if (!map["MUSICBRAINZ_TRACKID"].isEmpty())
        musicbrainz_trackid = map["MUSICBRAINZ_TRACKID"].front().to8Bit(true);
    if (!map["TITLE"].isEmpty())
        title = map["TITLE"].front().to8Bit(true);
    if (!map["TRACKNUMBER"].isEmpty())
        tracknumber = map["TRACKNUMBER"].front().to8Bit(true);
    if (!map["DATE"].isEmpty())
        released = map["DATE"].front().to8Bit(true);
    if (!map["GENRE"].isEmpty())
        genre = map["GENRE"].front().to8Bit(true);
}
