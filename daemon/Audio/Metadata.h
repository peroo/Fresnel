#ifndef AUDIOMETADATA_H
#define AUDIOMETADATA_H

#include <taglib/vorbisfile.h>
#include <taglib/mpegfile.h>
#include <taglib/xiphcomment.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/flacfile.h>

#include <string>
#include <map>

class Metadata {
    public:
        Metadata() : _loaded(false) {}
        ~Metadata() {}
        bool loadData(std::string path);
        bool fetchData(int index);
        std::map<const char*, std::string> getFields();
        bool loaded();

        std::string album;
        std::string artist;
        std::string artist_sort;
        std::string albumartist;
        std::string albumartist_sort;
        std::string musicbrainz_albumid;
        std::string musicbrainz_artistid;
        std::string musicbrainz_albumartistid;
        std::string musicbrainz_trackid;
        std::string title;
        std::string tracknumber;
        std::string date;
        int         length;
        int         bitrate;

    private:
        bool parseProperties(TagLib::File *file);
        bool parseXiphComment(TagLib::Ogg::XiphComment *tag);
        bool parseId3v1(TagLib::ID3v1::Tag *tag);
        bool parseId3v2(TagLib::ID3v2::Tag *tag);

        bool _loaded;
};

#endif
