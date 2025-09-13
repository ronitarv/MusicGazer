extern "C" {
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
}
#include <string>
#include "audio_capture.h"


float intensity = 0.0f;

data* init_pipewire(int argc, char* argv[], std::string source) {
    struct data* data = new struct data();
    const struct spa_pod *params[1];
    uint8_t buffer[1024];
    struct pw_properties *props;
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    pw_init(&argc, &argv);

    data->loop = pw_main_loop_new(NULL);
    pw_loop* pwloop = pw_main_loop_get_loop(data->loop);

    pw_loop_add_signal(pw_main_loop_get_loop(data->loop), SIGINT, do_quit, data);
    pw_loop_add_signal(pw_main_loop_get_loop(data->loop), SIGTERM, do_quit, data);
    
    if (source == "") {
        props = pw_properties_new(
            PW_KEY_MEDIA_CLASS, "Stream/Input/Audio",
            "stream.capture.sink", "true",
            PW_KEY_MEDIA_ROLE, "Music",
            NULL
        );
    } else {
        props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                        PW_KEY_MEDIA_CATEGORY, "Capture",
                        PW_KEY_MEDIA_ROLE, "Music",
                        NULL);
        if (argc > 1) {
                pw_properties_set(props, PW_KEY_TARGET_OBJECT, source.c_str());
        }
    } 


    data->stream = pw_stream_new_simple(
                    pw_main_loop_get_loop(data->loop),
                    "audio-capture",
                    props,
                    &stream_events,
                    data);

    struct spa_audio_info_raw raw_info = SPA_AUDIO_INFO_RAW_INIT(
     .format = SPA_AUDIO_FORMAT_F32
    );
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,&raw_info);
    data->pwloop = pwloop;
    pw_stream_connect(data->stream,
                        PW_DIRECTION_INPUT,
                        PW_ID_ANY,
                        static_cast<pw_stream_flags>(
                        PW_STREAM_FLAG_AUTOCONNECT |
                        PW_STREAM_FLAG_MAP_BUFFERS |
                        PW_STREAM_FLAG_RT_PROCESS
                        ),
                        params, 1);
    
    return data;
}

static void on_process(void *userdata)
{
        struct data *data = static_cast<struct data*>(userdata);
        struct pw_buffer *b;
        struct spa_buffer *buf;
        float *samples, max;
        uint32_t c, n, n_channels, n_samples, peak;

        if ((b = pw_stream_dequeue_buffer(data->stream)) == NULL) {
                pw_log_warn("out of buffers: %m");
                return;
        }

        buf = b->buffer;
        if ((samples = static_cast<float*>(buf->datas[0].data)) == NULL)
                return;

        n_channels = data->format.info.raw.channels;
        n_samples = buf->datas[0].chunk->size / sizeof(float);

        for (c = 0; c < data->format.info.raw.channels; c++) {
                max = 0.0f;
                for (n = c; n < n_samples; n += n_channels)
                        max = fmaxf(max, fabsf(samples[n]));

                peak = SPA_CLAMPF(max * 30, 0.f, 39.f);
                intensity = peak/15.0f;
        }
        data->move = true;
        fflush(stdout);

        pw_stream_queue_buffer(data->stream, b);
}

static void
on_stream_param_changed(void *_data, uint32_t id, const struct spa_pod *param)
{
        struct data *data = static_cast<struct data*>(_data);

        if (param == NULL || id != SPA_PARAM_Format)
                return;

        if (spa_format_parse(param, &data->format.media_type, &data->format.media_subtype) < 0)
                return;

        if (data->format.media_type != SPA_MEDIA_TYPE_audio ||
            data->format.media_subtype != SPA_MEDIA_SUBTYPE_raw)
                return;

        spa_format_audio_raw_parse(param, &data->format.info.raw);

}

const struct pw_stream_events stream_events = {
        PW_VERSION_STREAM_EVENTS,
        .param_changed = on_stream_param_changed,
        .process = on_process,
};

void do_quit(void *userdata, int signal_number)
{
        struct data *data = static_cast<struct data*>(userdata);
        pw_main_loop_quit(data->loop);
}