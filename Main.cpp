#include "Main.h"
#include "JavaScript.h"
#include "JSDatabase.h"
#include "HttpRequest.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <v8.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <cstdio>
#include <stdint.h>
#include <microhttpd.h>
#include <iostream>

extern "C" {
#include <avcodec.h>
}

namespace fs = boost::filesystem;
using namespace std;


int requestCurrier(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, unsigned int *upload_data_size, void **con_cls)
{
    HttpRequest *req = new HttpRequest(connection, url, method);
    return req->Process();
}

bool Main::initSQLite(const char* filename) {
    return sqlite3_open(filename, &db) == SQLITE_OK;
}

/*
bool openJPEG()
{
    AVCodec *codec;
    codec = avcodec_find_decoder(CODEC_ID_LJPEG);
    AVCodecContext *c = avcodec_alloc_context();

    if(avcodec_open(c, codec) < 0) {
        av_free(c);
        cout << "codec open phail" << endl;
        return false;
    }

    FILE *f = fopen("test.jpeg", "rb");
    FILE *outfile = fopen("out.jpg", "wb");

    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *inbuf_ptr = inbuf;
    uint8_t *outbuf = new uint8_t[AVCODEC_MAX_AUDIO_FRAME_SIZE];

    for(;;) {
        int size = fread(inbuf, 1, INBUF_SIZE, f);
        if (size == 0)
            break;

        inbuf_ptr = inbuf;
        while (size > 0) {
            int out_size;
            int len = avcodec_decode_audio2(c, (short *)outbuf, &out_size,
                    inbuf_ptr, size);
            int len = avcodec_decode_video(c, )


2811 int avcodec_decode_video(AVCodecContext *avctx, AVFrame *picture,
2812                          int *got_picture_ptr,
2813                          const uint8_t *buf, int buf_size);


            if (len < 0) {
                fprintf(stderr, "Error while decoding\n");
                exit(1);
            }
            if (out_size > 0) {
                /* if a frame has been decoded, output it */
                fwrite(outbuf, 1, out_size, outfile);
            }
            size -= len;
            inbuf_ptr += len;
        }
    }

    fclose(outfile);
    fclose(f);
    delete [] outbuf;

    avcodec_close(c);
    av_free(c);
}
*/

bool Main::Initialize() {
	// Setup working dir
	string home = getenv("HOME");
	Main::base = home + "/.tessst";
	if(!fs::exists(Main::base)) {
		if(!create_directory(Main::base))
			return false;
	}
	chdir(Main::base.directory_string().c_str());

	// Init SQLite
	if(!initSQLite("database.db")) {
        cout << "SQL initialization failed." << endl;
        return false;
	}

	JSDatabase::setDB(db);

	// Init MHD
	server = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, PORT, false, false, requestCurrier, false, MHD_OPTION_END);
	if (!server)
        return false;

	cout << "Init successful." << endl;
}

void Main::StopServer()
{
    MHD_stop_daemon(server);
}

int main(void)
{
    Main *instance = new Main();

    instance->Initialize();

    sleep (600);

    instance->StopServer();

    return 0;
}
