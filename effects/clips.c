#include "clips.h"
#include <libavformat/avformat.h>
#include "../utils/frame-puller.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <ftbitmap.h>

struct __daku_video_clip {
    daku_action base;
    const char *path;
    frame_puller *puller;
    float start_time;
    double time_base;
    int vid_width, vid_height;
};

void _daku_video_clip_update(daku_action *action, float progress)
{
    struct __daku_video_clip *duang = (struct __daku_video_clip *)action;
    int x, y;
    while (duang->puller->packet.pts < (duang->start_time + progress * action->duration) * duang->time_base) {
        frame_puller_next_frame(duang->puller, NULL);
    }
    AVFrame *f = duang->puller->frame;
    uint16_t *frame_pict = (uint16_t *)f->data[0];
    int subscript_inc = f->linesize[0] / 2;
    for (y = 0; y < duang->vid_height; ++y)
        for (x = 0; x < duang->vid_width; ++x) {
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 0] = frame_pict[y * subscript_inc + x * 3 + 0];
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 1] = frame_pict[y * subscript_inc + x * 3 + 1];
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 2] = frame_pict[y * subscript_inc + x * 3 + 2];
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 3] = 65535;
        }
}
int _daku_video_clip_init(daku_action *action)
{
    struct __daku_video_clip *ret = (struct __daku_video_clip *)action;
    int width = action->target->pict_width, height = action->target->pict_height;
    if (frame_puller_open_video(&ret->puller, ret->path, width, height, 1) < 0) return -4;
    if (frame_puller_seek(ret->puller, ret->start_time) < 0) return -6;
    AVRational *tb = &ret->puller->fmt_ctx->streams[ret->puller->target_stream_idx]->time_base;
    ret->time_base = (double)tb->den / (double)tb->num;
    ret->vid_width = ret->puller->output_width;
    ret->vid_height = ret->puller->output_height;
    return 0;
}
daku_action *daku_video_clip(const char *path, float start_time, float duration)
{
    struct __daku_video_clip *ret =
        (struct __daku_video_clip *)malloc(sizeof(struct __daku_video_clip));
    ret->base.duration = duration;
    ret->base.initialized = 0;
    ret->base.init = &_daku_video_clip_init;
    ret->base.update = &_daku_video_clip_update;
    ret->path = path;
    ret->start_time = start_time;
    return (daku_action *)ret;
}

struct __daku_text_clip {
    daku_action base;
    int text_len;
    const char *text;
    int line_height;
    enum daku_text_h_align h_align;
    FT_Library ft_lib;
    FT_Face ft_face;
    unsigned char *bmp_buffer;
    unsigned char *line_buffer;
};
void _daku_text_clip_update(daku_action *action, float progress)
{
    struct __daku_text_clip *duang = (struct __daku_text_clip *)action;
    int i, pen_x = 0, pen_y = 0, x, y, xx, yy, line_w = 0, line_h = duang->line_height * 2;
    int max_line_w = 0;
    int w = action->target->pict_width, h = action->target->pict_height;
    FT_Bitmap bitmap;
    bitmap.buffer = duang->bmp_buffer;
    memset(duang->line_buffer, 0, w * h * 2);

    pen_y = -duang->line_height;    // Padding to prevent the last line from being cut
    for (i = 0; i < duang->text_len; ++i)
        if (duang->text[i] == '\n') pen_y -= duang->line_height;
    int content_w = 0, content_h = -pen_y;

#define COPY_LINE_TO_PICT_X(__x) do \
    for (y = h - line_h; y < h; ++y) { \
        yy = y + pen_y + duang->line_height;    /* Bottom padding of the line is duang->line_height */ \
        if (yy < 0 || yy >= h) continue; \
        for (x = 0; x < line_w; ++x) { \
            action->target->picture[((h - yy - 1) * w + (__x)) * 4 + 0] = action->target->picture[((h - yy - 1) * w + (__x)) * 4 + 1] = \
               action->target->picture[((h - yy - 1) * w + (__x)) * 4 + 2] = 65535; \
            action->target->picture[((h - yy - 1) * w + (__x)) * 4 + 3] |= (duang->line_buffer[y * w + x] << 8); \
        } \
    } while (0)
#define COPY_LINE_TO_PICT do \
    switch (duang->h_align) { \
        case DAKU_HALIGN_LEFT: COPY_LINE_TO_PICT_X(x); break; \
        case DAKU_HALIGN_RIGHT: COPY_LINE_TO_PICT_X(w - 1 - line_w + x); break; \
        case DAKU_HALIGN_CENTRE: COPY_LINE_TO_PICT_X((w - 1 - line_w) / 2 + x); break; \
    } while (0)

    for (i = 0; i < duang->text_len; ++i) {
        if (duang->text[i] == '\n') {
            COPY_LINE_TO_PICT;
            // Update pen
            pen_x = 0;
            pen_y += duang->line_height;
            line_w = 0;
            memset(duang->line_buffer, 0, w * h * 2);
            continue;
        }
        if (FT_Load_Char(duang->ft_face, duang->text[i], FT_LOAD_RENDER) != 0) continue;
        if (FT_Bitmap_Convert(duang->ft_lib, &duang->ft_face->glyph->bitmap, &bitmap, 1) != 0) continue;
        // Draw the character
        for (y = 0; y < bitmap.rows; ++y) {
            yy = y + h - duang->ft_face->glyph->bitmap_top - duang->line_height;    // Bottom padding of the line is duang->line_height
            if (yy < 0 || yy >= h) continue;
            for (x = 0; x < bitmap.width; ++x) {
                xx = x + pen_x + duang->ft_face->glyph->bitmap_left;
                if (xx < 0 || xx >= w) continue;
                duang->line_buffer[yy * w + xx] = bitmap.buffer[y * bitmap.pitch + x];
            }
        }
        pen_x += duang->ft_face->glyph->advance.x / 64;
        pen_y += duang->ft_face->glyph->advance.y / 64;
        line_w = pen_x;
        if (max_line_w < line_w) max_line_w = line_w;
        if (content_w < pen_x) content_w = pen_x;
    }
    COPY_LINE_TO_PICT;
    FT_Bitmap_Done(duang->ft_lib, &bitmap);
    action->target->content_width = content_w;
    action->target->content_height = content_h;
    action->target->content_start_y = duang->line_height;
    switch (duang->h_align) {
        case DAKU_HALIGN_LEFT:
            action->target->content_start_x = 0; break;
        case DAKU_HALIGN_RIGHT:
            action->target->content_start_x = w - max_line_w - 1; break;
        case DAKU_HALIGN_CENTRE:
            action->target->content_start_x = (w - 1 - max_line_w) / 2; break;
    }
#undef COPY_LINE_TO_PICT
#undef COPY_LINE_TO_PICT_X
}
int _daku_text_clip_init(daku_action *action)
{
    // Conservatively allocate twice as much as needed.
    ((struct __daku_text_clip *)action)->bmp_buffer =
        (unsigned char *)malloc(action->target->pict_width * action->target->pict_height * 2);
    ((struct __daku_text_clip *)action)->line_buffer =
        (unsigned char *)malloc(action->target->pict_width * action->target->pict_height * 2);
    return 0;
}
daku_action *daku_text(float duration, const char *text,
    const char *path, int size, int line_height, enum daku_text_h_align h_align)
{
    struct __daku_text_clip *ret =
        (struct __daku_text_clip *)malloc(sizeof(struct __daku_text_clip));
    ret->base.duration = duration;
    ret->base.initialized = 0;
    ret->base.init = &_daku_text_clip_init;
    ret->base.update = &_daku_text_clip_update;
    ret->text_len = strlen(text);
    ret->text = text;
    ret->line_height = line_height > 0 ? line_height : size;
    ret->h_align = h_align;
    int ft_err_code;
    if (FT_Init_FreeType(&ret->ft_lib) != 0) return NULL;
    if ((ft_err_code = FT_New_Face(ret->ft_lib, path, 0, &ret->ft_face)) != 0) {
        if (ft_err_code == FT_Err_Unknown_File_Format) {
            av_log(NULL, AV_LOG_ERROR, "Probably unsupported font format (/_<)\n");
            return NULL;
        } else {
            av_log(NULL, AV_LOG_ERROR, "Cannot open the font file\n");
            return NULL;
        }
    }
    if (FT_Set_Pixel_Sizes(ret->ft_face, size, size)) {
        av_log(NULL, AV_LOG_ERROR, "Invalid font size, perhaps?\n");
        return NULL;
    }
    ret->bmp_buffer = NULL;
    return (daku_action *)ret;
}
