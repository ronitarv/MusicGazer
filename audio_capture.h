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

static void on_process(void *userdata);

static void on_stream_param_changed(void *_data, uint32_t id, const struct spa_pod *param);

extern const struct pw_stream_events stream_events;

void do_quit(void *userdata, int signal_number);