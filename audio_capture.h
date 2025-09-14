#pragma once

extern "C" {
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
}

struct data {
        struct pw_main_loop *loop;
        struct pw_stream *stream;
        pw_loop* pwloop;
        struct spa_audio_info format;
        unsigned move:1;
};

extern float intensity;

data* init_pipewire(int argc, char* argv[], std::string source);

extern const struct pw_stream_events stream_events;

void do_quit(void *userdata, int signal_number);