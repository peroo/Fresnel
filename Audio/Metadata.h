#ifndef AUDIOMETADATA_H
#define AUDIOMETADATA_H

/*#include <taglib/vorbisfile.h>
#include <taglib/xiphcomment.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/flacfile.h>*/
#include <boost/filesystem/path.hpp>

#include <string>

class Metadata {
    public:
        bool loadData(boost::filesystem::path path);
        bool fetchData(int index);

        std::string album;
        std::string artist;
        std::string albumartist;
        std::string musicbrainz_albumid;
        std::string musicbrainz_artistid;
        std::string musicbrainz_albumartistid;
        std::string musicbrainz_trackid;
        std::string title;
        std::string tracknumber;
        std::string date;

    private:
/*        bool parseXiphComment(TagLib::Ogg::XiphComment *tag);
        bool parseId3v1(TagLib::ID3v1::Tag *tag);
        bool parseId3v2(TagLib::ID3v2::Tag *tag);*/
};

#endif
