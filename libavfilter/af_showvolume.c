/*
 * Copyright (c) 2014 Lars Kiesow <lkiesow@uos.de>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FFmpeg; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/channel_layout.h"
#include "avfilter.h"
#include "internal.h"

static int query_formats(AVFilterContext *ctx)
{
    static const enum AVSampleFormat sample_fmts[] = {
        AV_SAMPLE_FMT_S16,
        AV_SAMPLE_FMT_S16P,
        AV_SAMPLE_FMT_NONE
    };
    AVFilterFormats *formats;

    if (!(formats = ff_make_format_list(sample_fmts)))
        return AVERROR(ENOMEM);
    ff_set_common_formats(ctx, formats);

    return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *samples)
{
    AVFilterContext *ctx = inlink->dst;
    uint64_t *n = ctx->priv;
    int64_t layout  = samples->channel_layout;
    int nb_samples  = samples->nb_samples;
    int nb_channels = av_get_channel_layout_nb_channels(layout);
    int nb_planes   = nb_channels;
    int is_planar = av_sample_fmt_is_planar(samples->format);
    int plane, i;
    int16_t *pcm;

    /* Interleaved audio data means there is only one large array */
    if (!is_planar) {
        nb_samples *= nb_channels;
        nb_planes = 1;
    }
    for (plane = 0; plane < nb_planes; plane++) {
        pcm = (int16_t *)samples->extended_data[plane];
        for (i = 0; i < nb_samples; i++) {
            av_log(ctx, AV_LOG_INFO, "n: %lu, channel: %i, volume: %i\n", (*n)++,
                  is_planar ? plane : i % nb_channels, pcm[i]);
        }
    }

    return ff_filter_frame(inlink->dst->outputs[0], samples);
}

static av_cold void uninit(AVFilterContext *ctx)
{
    /* print_stats(ctx); */
}

static const AVFilterPad showvolume_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_AUDIO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad showvolume_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_AUDIO,
    },
    { NULL }
};

AVFilter ff_af_showvolume = {
    .name          = "showvolume",
    .description   = NULL_IF_CONFIG_SMALL("Show audio volume information."),
    .priv_size     = sizeof(uint64_t),
    .query_formats = query_formats,
    .uninit        = uninit,
    .inputs        = showvolume_inputs,
    .outputs       = showvolume_outputs,
};
