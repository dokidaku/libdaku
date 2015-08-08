#include "core.h"
#include "types.h"
#include "utils/frame-pusher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

daku_matter *daku_matter_create()
{
    daku_matter *ret = (daku_matter *)malloc(sizeof(daku_matter));
    ret->content_start_x = ret->content_start_y = 0;
    ret->content_width = ret->content_height = ret->pict_width = ret->pict_height = 0;
    ret->anchor_x = ret->anchor_y = 0.5;
    ret->x = ret->y = ret->rotation = ret->skew_x = ret->skew_y = 0;
    ret->scale_x = ret->scale_y = 1;
    ret->z_order = 0;
    ret->opacity = 65535;
    ret->flipped_x = ret->flipped_y = ret->is_frozen = FALSE;
    ret->actions = daku_list_create(NULL);
    return ret;
}
void daku_matter_setlife(daku_matter *m, float life_time)
{
    m->life_time = life_time;
}
void daku_matter_setsize(daku_matter *m, float width, float height)
{
    m->content_width = m->pict_width = width;
    m->content_height = m->pict_height = height;
}
void daku_matter_setgrosssize(daku_matter *m, float nw, float nh, float gw, float gh)
{
    m->content_width = nw; m->pict_width = gw;
    m->content_height = nh; m->pict_height = gh;
}
void daku_matter_setanchor(daku_matter *m, float ax, float ay)
{
    m->anchor_x = ax;
    m->anchor_y = ay;
}
void daku_matter_setpos(daku_matter *m, float x, float y)
{
    m->x = x; m->y = y;
}
void daku_matter_setopacity(daku_matter *m, uint8_t opacity)
{
    m->opacity = (float)opacity / 255.0 * 65535.0;
}
void daku_matter_setscale(daku_matter *m, float scale_x, float scale_y)
{
    m->scale_x = scale_x;
    m->scale_y = scale_y;
}
void daku_matter_act(daku_matter *m, float start_time, daku_action *action)
{
    action->start_time = start_time;
    action->target = m;
    daku_list_push_back(m->actions, action);
}

void daku_matter_init(daku_matter *m)
{
    // All matters use RGBA format
    m->picture = (uint16_t *)malloc(m->pict_width * m->pict_height * 4 * sizeof(uint16_t));
    memset(m->picture, 0, m->pict_width * m->pict_height * 4 * sizeof(uint16_t));
}

daku_wave *daku_wave_create()
{
    daku_wave *ret = (daku_wave *)malloc(sizeof(daku_wave));
    ret->data_len = ret->sample_rate = 0;
    ret->instruments = daku_list_create(NULL);
    return ret;
}
void daku_wave_setlife(daku_wave *w, float life_time)
{
    w->life_time = life_time;
}
void daku_wave_play(daku_wave *w, float start_time, daku_instrument *instrument)
{
    instrument->start_time = start_time;
    instrument->target = w;
    daku_list_push_back(w->instruments, instrument);
}

daku_world *daku_world_create(int width, int height, float duration, int fps, int sample_rate)
{
    daku_world *ret = (daku_world *)malloc(sizeof(daku_world));
    ret->width = width; ret->height = height;
    ret->fps = fps > 0 ? fps : 30;
    ret->sample_rate = sample_rate > 0 ? sample_rate : 11025;
    ret->duration = duration;
    ret->population = daku_list_create(NULL);
    ret->clangs = daku_list_create(NULL);
    return ret;
}

void daku_world_populate(daku_world *world, daku_matter *resident, float start_time, int z_order)
{
    resident->start_time = start_time;
    resident->z_order = z_order;
    daku_list_push_back(world->population, resident);
}
void daku_world_clang(daku_world *world, daku_wave *duangduang, float start_time)
{
    duangduang->start_time = start_time;
    duangduang->sample_rate = world->sample_rate;
    daku_list_push_back(world->clangs, duangduang);
}

void __save_frame_ppm(const uint8_t *rgb_data, int width, int height, int linesize, const char *path)
{
    FILE *fp = fopen(path, "wb");
    //assert(fp);
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    int y;
    for (y = 0; y < height; ++y)
        fwrite(rgb_data + y * linesize, 1, width * 3, fp);
    fclose(fp);
}

typedef struct __daku_world_event {
    float time;
#define DAKU_WORLD_EVENT_ENTER ((unsigned char)0)
#define DAKU_WORLD_EVENT_EXIT  ((unsigned char)1)
    unsigned char type;
    void *target;
} _daku_world_event;

int _daku_world_event_cmp(const void *a, const void *b) {
    static float ta, tb;
    ta = ((_daku_world_event *)a)->time;
    tb = ((_daku_world_event *)b)->time;
    return ta < tb ? -1 : (ta == tb ? 0 : 1);
}


_daku_world_event *_daku_world_make_events(daku_world *world, int *evncnt) {
    *evncnt = (world->population->length - 1) * 2;
    int i = -1;
    _daku_world_event *ret = (_daku_world_event *)malloc(*evncnt * sizeof(_daku_world_event));
    // Generate events
    daku_matter *m;
    daku_list_foreach_t(world->population, daku_matter *, m) if (m) {
#ifdef __cplusplus
        ret[++i] = _daku_world_event{ m->start_time, DAKU_WORLD_EVENT_ENTER, m };
        ret[++i] = _daku_world_event{ m->start_time + m->life_time, DAKU_WORLD_EVENT_EXIT, m };
#else
        ret[++i] = (_daku_world_event){ m->start_time, DAKU_WORLD_EVENT_ENTER, m };
        ret[++i] = (_daku_world_event){ m->start_time + m->life_time, DAKU_WORLD_EVENT_EXIT, m };
#endif
    }
    // We have to ensure that two matters of same z-orders are sorted by arrival.
    // Therefore a stable sorting algorithm must be applied. Use merge sort.
    // XXX: Will quicksort work?
    qsort(ret, *evncnt, sizeof(_daku_world_event), &_daku_world_event_cmp);
    return ret;
}

#define MIN(__a, __b) ((__a) < (__b) ? (__a) : (__b))
#define MAX(__a, __b) ((__a) > (__b) ? (__a) : (__b))
void daku_world_write(daku_world *world, const char *path)
{
    clock_t start_time = clock();

    frame_pusher *pusher;
    if (frame_pusher_open(&pusher, path, world->sample_rate, world->fps, world->width, world->height, 800000) < 0) return;
    unsigned int line_size = world->width * 3 * sizeof(uint8_t);
    if (line_size % 64) line_size += (64 - line_size % 64); // Will this work...?
    unsigned int buf_size = line_size * world->height;
    uint8_t *pict = (uint8_t *)malloc(buf_size);
    uint16_t *ipict = (uint16_t *)malloc(buf_size * 2);     // The internal picture used for rendering
    uint8_t *pusher_pict[4] = { pict };
    // The buffer size needs to be multiplied by 3 because the format is RGB24
    int pusher_linesize[4] = { line_size };
    // The buffer for one second of sound.
    int waveform_len = world->sample_rate * world->duration;
    int16_t *waveform[2] = {
        (int16_t *)malloc(waveform_len * sizeof(int16_t)),
        (int16_t *)malloc(waveform_len * sizeof(int16_t))
    };
    memset(waveform[0], 0, waveform_len * sizeof(int16_t));
    memset(waveform[1], 0, waveform_len * sizeof(int16_t));

    daku_matter *m;
    daku_action *ac;
    daku_wave *wv;
    daku_instrument *inst;
    daku_list_foreach_t(world->population, daku_matter *, m) if (m) {
        daku_matter_init(m);
        printf("RESIDENT 0x%x\n", (int)m);
        printf("Start: %f, life: %f\n", m->start_time, m->life_time);
        printf("Net: %dx%d, gross: %dx%d\n",
            (int)m->content_width, (int)m->content_height,
            (int)m->pict_width, (int)m->pict_height);
        printf("(%f, %f), rotation %f\n\n", m->x, m->y, m->rotation);
        daku_list_foreach_t(m->actions, daku_action *, ac) if (ac) printf("Action 0x%x\n", ac);
    }
    daku_list/* struct __daku_matter */ *presenters = daku_list_create(NULL);
    int event_count, next_event_idx = 0;
    _daku_world_event *events = _daku_world_make_events(world, &event_count);

    unsigned int frame_num = 0;
    float cur_time;
    float action_progress;
    int x0, y0, x, y, w, h;
    float anchor_px_x, anchor_px_y;
    float x1, y1, x3[4], y3[4];
    float sin_rad, sin_negrad, cos_rad, cos_negrad;
    int x2, y2, min_x, max_x, min_y, max_y;
    unsigned char line_started;
    uint16_t alpha;
    float rotation_rad, tan_skew_x, tan_skew_y;
    int seconds;
    int frames_in_sec;
    // Initialize all waves and instruments first, for they don't need any extra data.
    daku_list_foreach_t(world->clangs, daku_wave *, wv) if (wv) {
        wv->data_len = world->sample_rate * wv->life_time;
        wv->waveform_data[0] = (int16_t *)malloc(wv->data_len * sizeof(int16_t));
        wv->waveform_data[1] = (int16_t *)malloc(wv->data_len * sizeof(int16_t));
        daku_list_foreach_t(wv->instruments, daku_instrument *, inst) if (inst) {
            inst->target_data[0] = wv->waveform_data[0] + (int)(inst->start_time * world->sample_rate);
            inst->target_data[1] = wv->waveform_data[1] + (int)(inst->start_time * world->sample_rate);
            inst->samples_affected = inst->duration * world->sample_rate;
            inst->init(inst);
        }
        y = wv->start_time * world->sample_rate;
        for (x = 0; x < MIN(wv->data_len, waveform_len - y - 1); ++x) {
            waveform[0][x + y] += wv->waveform_data[0][x];
            waveform[1][x + y] += wv->waveform_data[1][x];
        }
    }
    // Generate picture frames one by one.
    for (seconds = 0; seconds < world->duration; ++seconds) {
        printf("Second #%d, elapsed %.2f seconds\n", seconds + 1, (float)(clock() - start_time) / CLOCKS_PER_SEC);
        frames_in_sec = (world->duration - seconds >= 1) ? world->fps : (world->duration - seconds) * world->fps;
        for (frame_num = 0; frame_num < frames_in_sec; ++frame_num) {
            // Render one frame.
            memset(ipict, 0, buf_size * 2);
            cur_time = seconds + (float)frame_num / (float)world->fps;
            // First, check for events.
            if (next_event_idx < event_count) while (cur_time >= events[next_event_idx].time) {
                if (events[next_event_idx].type == DAKU_WORLD_EVENT_ENTER) {
                    // Add the matter to the presenters list.
                    daku_list_block *last = NULL;
                    daku_list_foreach_t(presenters, daku_matter *, m) {
                        if (!m || m->z_order > ((daku_matter *)events[next_event_idx].target)->z_order) {
                            daku_list_insert(presenters, last, events[next_event_idx].target); break;
                        }
                        last = presenters->itr;
                    }
                } else {
                    // Remove the matter from the list.
                    daku_list_block *last = NULL;
                    daku_list_foreach_t(presenters, daku_matter *, m) {
                        if (m == events[next_event_idx].target) {
                            daku_list_remove(presenters, last); break;
                        }
                        last = presenters->itr;
                    }
                }
                if (++next_event_idx >= event_count) break;
            }
            daku_list_foreach_t(presenters, daku_matter *, m) if (m) {
                daku_list_foreach_t(m->actions, daku_action *, ac)
                    if (ac && m->start_time + ac->start_time <= cur_time &&
                        (m->start_time + ac->start_time + ac->duration >= cur_time || !ac->finalized) &&
                        !(m->is_frozen && ac->is_clip))
                    {
                        if (!ac->initialized) {
                            ac->initialized = TRUE;
                            if (ac->init) ac->init(ac);
                        }
                        action_progress = (cur_time - m->start_time - ac->start_time) / ac->duration;
                        if (action_progress < 1) {
                            ac->update(ac, (cur_time - m->start_time - ac->start_time) / ac->duration);
                        } else {
                            ac->update(ac, 1);
                            ac->finalized = TRUE;
                        }
                    }
                // Image processing order:
                // > Scale
                // > Skew
                // > Rotate
                anchor_px_x = m->anchor_x * m->content_width;
                anchor_px_y = m->anchor_y * m->content_height;
                x0 = m->x - (anchor_px_x + m->content_start_x) * m->scale_x;
                y0 = m->y - (anchor_px_y + m->content_start_y) * m->scale_y;
                // Precalculate trigonometric values
                rotation_rad = m->rotation * M_PI / 180.0;
                if (fabs(rotation_rad) <= 1e-5) rotation_rad = 0;
                sin_rad = sin(rotation_rad); cos_rad = cos(rotation_rad);
                sin_negrad = -sin_rad; cos_negrad = cos_rad;
                tan_skew_x = tan(m->skew_x * M_PI / 180.0);
                tan_skew_y = tan(m->skew_y * M_PI / 180.0);
                // The image range after scaling & rotating.
                // > Calculate three vertices first: [0], [1], [3]
                // > [1] ----- [2]
                // >  /         /
                // >  /         /
                // > [0] ----- [3]
                x3[0] = x3[1] = y3[0] = y3[3] = 0;
                x3[3] = m->pict_width * m->scale_x;
                y3[1] = m->pict_height * m->scale_y;
                // > Skew. This code can be made more elegant.
                x2 = x3[3] + y3[3] * tan_skew_x;
                y2 = y3[3] + x3[3] * tan_skew_y;
                x3[3] = x2; y3[3] = y2;
                x2 = x3[1] + y3[1] * tan_skew_x;
                y2 = y3[1] + x3[1] * tan_skew_y;
                x3[1] = x2; y3[1] = y2;
                // > vector([0]-[1]) = vector([3]-[2])
                x3[2] = x3[3] + x3[1] - x3[0];
                y3[2] = y3[3] + y3[1] - y3[0];
                for (x = 0; x < 4; ++x) {
                    x3[x] += x0 - (anchor_px_y + m->content_start_y) * tan_skew_x * m->scale_y;
                    y3[x] += y0 - (anchor_px_x + m->content_start_x) * tan_skew_y * m->scale_x;
                }
                min_x = world->width; max_x = 0;
                min_y = world->height; max_y = 0;
                if (rotation_rad == 0) {
                    for (x = 0; x < 4; ++x) {
                        if (max_x < x3[x]) max_x = x3[x]; if (min_x > x3[x]) min_x = x3[x];
                        if (max_y < y3[x]) max_y = y3[x]; if (min_y > y3[x]) min_y = y3[x];
                    }
                } else {
            #define ROT(__nx, __ny, __x, __y, __cx, __cy, __sinrad, __cosrad) do { \
                __nx = (float)((__x) - (__cx)) * (__cosrad) - (float)((__y) - (__cy)) * (__sinrad) + (__cx); \
                __ny = (float)((__x) - (__cx)) * (__sinrad) + (float)((__y) - (__cy)) * (__cosrad) + (__cy); \
            } while (0)
                    for (x = 0; x < 4; ++x) {
                        ROT(x2, y2, x3[x], y3[x], m->x, m->y, sin_negrad, cos_negrad);
                        if (max_x < x2) max_x = x2; if (min_x > x2) min_x = x2;
                        if (max_y < y2) max_y = y2; if (min_y > y2) min_y = y2;
                        // Uncomment to get debug information about bounds.
                        // Marks the four vertices of the translated image.
                        // for (x1 = x2 - 10; x1 <= x2 + 10; ++x1)
                        //     for (y1 = y2 - 10; y1 <= y2 + 10; ++y1)
                        //         ipict[(int)((world->height - y1 - 1) * world->width + x1) * 3 + 0] = 65535;
                    }
                    // Some crops may occur due to... precision limits...?
                    max_x += 5; min_x -= 5; max_y += 5; min_y -= 5;
                }
                if (max_x > world->width) max_x = world->width; if (min_x < 0) min_x = 0;
                if (max_y > world->height) max_y = world->height; if (min_y < 0) min_y = 0;
            #define ALPHA_MIX(__orig, __new) \
                (__orig = (__orig * (65535 - alpha) + __new * alpha) / 65535)
            #define COPY_PICT(__fx, __fy) do { \
                    for (y = min_y; y < max_y; ++y) { \
                        y1 = anchor_px_y + m->content_start_y + (float)(y - m->y); \
                        line_started = 0; \
                        for (x = min_x; x < max_x; ++x) { \
                            /* Map the position (x, y) on the screen to (x1, y1) in the image. */ \
                            x1 = anchor_px_x + m->content_start_x + (float)(x - m->x); \
                            /* Rotate. */ \
                            if (rotation_rad == 0) { \
                                x2 = x1; y2 = y1; \
                            } else { \
                                ROT(x2, y2, x1, y1, anchor_px_x + m->content_start_x, anchor_px_y + m->content_start_y, sin_rad, cos_rad); \
                            } \
                            /* Skew. Reuse resource :D */ \
                            /* x3[0] + y3[0] * tan_skew_x = x1
                             * y3[0] + x3[0] * tan_skew_y = y1
                             *         x3[0] * tan_skew_y + y3[0] * tan_skew_x * tan_skew_y = x1 * tan_skew_y **/ \
                            x2 -= anchor_px_x + m->content_start_x; \
                            y2 -= anchor_px_y + m->content_start_y; \
                            x3[0] = (x2 - y2 * tan_skew_x) / (1 - tan_skew_x * tan_skew_y); \
                            y3[0] = (y2 - x2 * tan_skew_y) / (1 - tan_skew_x * tan_skew_y); \
                            x2 = x3[0] + anchor_px_x + m->content_start_x; \
                            y2 = y3[0] + anchor_px_y + m->content_start_y; \
                            x2 = anchor_px_x + m->content_start_x + (x2 - anchor_px_x - m->content_start_x) / m->scale_x; \
                            y2 = anchor_px_y + m->content_start_y + (y2 - anchor_px_y - m->content_start_y) / m->scale_y; \
                            if (x2 >= 0 && x2 < m->pict_width && y2 >= 0 && y2 < m->pict_height) { \
                                ++line_started; \
                                alpha = m->picture[((__fy) * (int)m->pict_width + (__fx)) * 4 + 3] * m->opacity / 65535; \
                                ALPHA_MIX(ipict[(int)((world->height - y - 1) * world->width + x) * 3 + 0], m->picture[((__fy) * m->pict_width + (__fx)) * 4 + 0]); \
                                ALPHA_MIX(ipict[(int)((world->height - y - 1) * world->width + x) * 3 + 1], m->picture[((__fy) * m->pict_width + (__fx)) * 4 + 1]); \
                                ALPHA_MIX(ipict[(int)((world->height - y - 1) * world->width + x) * 3 + 2], m->picture[((__fy) * m->pict_width + (__fx)) * 4 + 2]); \
                            } else if (line_started >= 10) break; \
                        } \
                    } \
                } while (0)
                    // XXX: Will the compiler detect unchanged values (__fx and__fy) in loops and optimize?
                    // We won't need these macros if so.
                    if (m->flipped_y) {
                        if (m->flipped_x) COPY_PICT(m->pict_width - x2 - 1, m->pict_height - y2 - 1);
                        else COPY_PICT(x2, m->pict_height - y2 - 1);
                    } else {
                        if (m->flipped_x) COPY_PICT(m->pict_width - x2 - 1, y2);
                        else COPY_PICT(x2, y2);
                    }
            #undef COPY_PICT
            #undef ALPHA_MIX
            #undef ROT
                }
            // Save.
            // TODO: Directly use RGB48 format in frame pushers.
            for (y = 0; y < world->height; ++y)
                for (x = 0; x < world->width; ++x) {
                    pict[y * line_size + x * 3 + 0] = ipict[(y * world->width + x) * 3 + 0] >> 8;
                    pict[y * line_size + x * 3 + 1] = ipict[(y * world->width + x) * 3 + 1] >> 8;
                    pict[y * line_size + x * 3 + 2] = ipict[(y * world->width + x) * 3 + 2] >> 8;
                }
            frame_pusher_write_video(pusher, pusher_pict, pusher_linesize, 1);
        }
        // Audio part
        x = MIN(world->sample_rate * (seconds + 1), waveform_len);
        for (frame_num = world->sample_rate * seconds; frame_num < x; ++frame_num) {
            frame_pusher_write_audio(pusher, waveform[0][frame_num], waveform[1][frame_num]);
        }
    }
    frame_pusher_close(pusher);
    printf("Video time: %f s\n", world->duration);
    printf("Execution time: %f s\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);
}
#undef MIN
#undef MAX
