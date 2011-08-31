#include "Metadata.h"
#include "../Database.h"

#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

bool Metadata::loadData(fs::path path)
{
    std::string ext = fs::extension(path);
    for(unsigned int i = 0; i < ext.size(); ++i) {
        ext[i] = tolower(ext[i]);
    }

    if(ext == ".flac") {
        TagLib::FLAC::File flac(path.string().c_str(), true);
        if(!parseXiphComment(flac.xiphComment(false))) {
            if(!parseId3v2(flac.ID3v2Tag(false))) {
                parseId3v1(flac.ID3v1Tag(true));
            }
        }
        parseProperties(&flac);
    }
    else if(ext == ".ogg") {
        // TODO: Vorbis is assumed even though it might be FLAC/Speex. May or may not cause fatal problems.
        TagLib::Ogg::Vorbis::File *ogg = new TagLib::Ogg::Vorbis::File(path.string().c_str(), true);
        parseXiphComment(ogg->tag());
        parseProperties(ogg);
        delete ogg;
    }
    else if(ext == ".mp3") {
        TagLib::MPEG::File mp3(path.string().c_str(), true);
        //parseID3(mp3->tag());
        parseProperties(&mp3);
    }

    _loaded = true;

    return true;
}

bool Metadata::loaded()
{
    return _loaded;
}

bool Metadata::fetchData(int index)
{
    Database db;
    fs::path path(db.getResourcePath(index));
    loadData(path);

    return true;
}

std::map<const char*, std::string> Metadata::getFields()
{
    std::map<const char*, std::string> meta;

    if(!album.empty()) {
        meta["ALBUM"] = album;
    }
    if(!albumartist.empty()) {
        meta["ALBUMARTIST"] = albumartist;
    }
    if(!albumartist_sort.empty()) {
        meta["ALBUMARTISTSORT"] = albumartist_sort;
    }
    if(!artist.empty()) {
        meta["ARTIST"] = artist;
    }
    if(!artist_sort.empty()) {
        meta["ARTISTSORT"] = artist_sort;
    }
    if(!musicbrainz_albumartistid.empty()) {
        meta["MUSICBRAINZ_ALBUMARTISTID"] = musicbrainz_albumartistid;
    }
    if(!musicbrainz_albumid.empty()) {
        meta["MUSICBRAINZ_ALBUMID"] = musicbrainz_albumid;
    }
    if(!musicbrainz_artistid.empty()) {
        meta["MUSICBRAINZ_ARTISTID"] = musicbrainz_artistid;
    }
    if(!musicbrainz_trackid.empty()) {
        meta["MUSICBRAINZ_TRACKID"] = musicbrainz_trackid;
    }
    if(!title.empty()) {
        meta["TITLE"] = title;
    }
    if(!tracknumber.empty()) {
        meta["TRACKNUMBER"] = tracknumber;
    }
    if(!date.empty()) {
        meta["DATE"] = date;
    }

    return meta;
}

bool Metadata::parseXiphComment(TagLib::Ogg::XiphComment *tag)
{
    if(tag == NULL) return false;

    const TagLib::Ogg::FieldListMap map = tag->fieldListMap();
    if (!map["ALBUM"].isEmpty())
        album = map["ALBUM"].front().to8Bit(true);
    if (!map["ALBUMARTIST"].isEmpty())
        albumartist = map["ALBUMARTIST"].front().to8Bit(true);
    if (!map["ALBUMARTISTSORT"].isEmpty())
        albumartist_sort = map["ALBUMARTISTSORT"].front().to8Bit(true);
    if (!map["ARTIST"].isEmpty())
        artist = map["ARTIST"].front().to8Bit(true);
    if (!map["ARTISTSORT"].isEmpty())
        artist_sort = map["ARTISTSORT"].front().to8Bit(true);
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
        date = map["DATE"].front().to8Bit(true);

    return true;
}

bool Metadata::parseProperties(TagLib::File *file)
{
    TagLib::AudioProperties *prop = file->audioProperties();
    if(prop == NULL)
        return false;

    bitrate = prop->bitrate();
    length = prop->length();
    return true;
}

bool Metadata::parseId3v2(TagLib::ID3v2::Tag *tag)
{
    (void) tag;
    return false;
}

bool Metadata::parseId3v1(TagLib::ID3v1::Tag *tag)
{
    (void) tag;
    return false;
}
