#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <boost/filesystem/path.hpp>
#include <taglib/flacfile.h>

class AudioFile {
    public: 
        explicit AudioFile(const boost::filesystem::path *f) : file(f) {}
        bool readMeta();

        std::string album;
        std::string albumartist;
        std::string albumartistsort;
        std::string artist;
        std::string artistsort;
        std::string filename;
        std::string genre;
        std::string musicbrainz_albumartistid;
        std::string musicbrainz_albumid;
        std::string musicbrainz_artistid;
        std::string musicbrainz_trackid;
        std::string released;
        std::string title;
        std::string tracknumber;
    private:
        const boost::filesystem::path *file;

        void readXiphComment(const TagLib::Ogg::XiphComment *tag);
};
#endif
