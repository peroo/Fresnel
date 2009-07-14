#include "Metadata.h"

#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

bool Metadata::loadData(fs::path path)
{
    std::string ext = fs::extension(path);
    for(int i = 0; i < ext.size(); ++i) {
        ext[i] = tolower(ext[i]);
    }

    if(ext == ".flac") {
        TagLib::FLAC::File *flac = new TagLib::FLAC::File(path.string().c_str(), true);
        if(!parseXiphComment(flac->xiphComment(false))) {
            if(!parseId3v2(flac->ID3v2Tag(false))) {
                parseId3v1(flac->ID3v1Tag(true));
            }
        }
        parseProperties(flac);
        delete flac;
    }
    else if(ext == ".ogg") {
        // TODO: Vorbis is assumed even though it might be FLAC/Speex. May or may not cause fatal problems.
        TagLib::Ogg::Vorbis::File *ogg = new TagLib::Ogg::Vorbis::File(path.string().c_str(), true);
        parseXiphComment(ogg->tag());
        parseProperties(ogg);
        delete ogg;
    }

    return true;
}

bool Metadata::fetchData(int index)
{
    return false;
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
    bitrate = prop->bitrate();
    length = prop->length();
}

bool Metadata::parseId3v2(TagLib::ID3v2::Tag *tag)
{
    return false;
}

bool Metadata::parseId3v1(TagLib::ID3v1::Tag *tag)
{
    return false;
}
