#include "core.h"
#include "types.h"
#include "utils/frame-pusher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

daku_matter *daku_matter_create()
{
    daku_matter *ret = (daku_matter *)malloc(sizeof(daku_matter));
    ret->content_width = ret->content_height = ret->pict_width = ret->pict_height = 0;
    ret->anchor_x = ret->anchor_y = 0.5;
    ret->x = ret->y = ret->rotation = 0;
    ret->flipped_x = ret->flipped_y = 0;
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
}

daku_world *daku_world_create(int width, int height, float duration)
{
    daku_world *ret = (daku_world *)malloc(sizeof(daku_world));
    ret->width = width; ret->height = height;
    ret->fps = 30;
    ret->duration = duration;
    ret->population = daku_list_create(NULL);
    return ret;
}

void daku_world_populate(daku_world *world, daku_matter *resident, float start_time)
{
    resident->start_time = start_time;
    daku_list_push_back(world->population, resident);
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

#define MIN(__a, __b) ((__a) < (__b) ? (__a) : (__b))
void daku_world_write(daku_world *world, const char *path)
{
    clock_t start_time = clock();

    frame_pusher *pusher;
    if (frame_pusher_open(&pusher, path, 44100, world->fps, world->width, world->height, 800000) < 0) return;
    unsigned int line_size = world->width * 3 * sizeof(uint8_t);
    if (line_size % 64) line_size += (64 - line_size % 64); // Will this work...?
    unsigned int buf_size = line_size * world->height;
    uint8_t *pict = (uint8_t *)malloc(buf_size);
    uint16_t *ipict = (uint16_t *)malloc(buf_size * 2);     // The internal picture used for rendering
    uint8_t *pusher_pict[4] = { pict };
    // The buffer size needs to be multiplied by 3 because the format is RGB24
    int pusher_linesize[4] = { line_size };

    daku_matter *m;
    daku_action *ac;
    daku_list_foreach_t(world->population, daku_matter *, m) if (m) {
        daku_matter_init(m);
        printf("RESIDENT 0x%x\n", (int)m);
        printf("Start: %f, life: %f\n", m->start_time, m->life_time);
        printf("Net: %dx%d, gross: %dx%d\n",
            (int)m->content_width, (int)m->content_height,
            (int)m->pict_width, (int)m->pict_height);
        printf("(%f, %f), rotation %f\n\n", m->x, m->y, m->rotation);
    }
    unsigned int frame_num = 0;
    float cur_time;
    int x0, y0, x1, y1, x, y, w, h;
    uint16_t alpha;
    for (frame_num = 0; frame_num < world->duration * world->fps; ++frame_num) {
        // Render one frame.
        memset(ipict, 0, buf_size * 2);
        cur_time = (float)frame_num / (float)world->fps;
        daku_list_foreach_t(world->population, daku_matter *, m)
            if (m && m->start_time <= cur_time
                && m->start_time + m->life_time >= cur_time)
            {
                daku_list_foreach_t(m->actions, daku_action *, ac)
                    if (ac && m->start_time + ac->start_time <= cur_time
                        && m->start_time + ac->start_time + ac->duration >= cur_time)
                    {
                        ac->update(ac, (cur_time - m->start_time - ac->start_time) / ac->duration);
                    }
                x0 = m->x - m->anchor_x * m->pict_width;
                y0 = m->y - m->anchor_y * m->pict_height;
                w = MIN(m->pict_width, world->width - x0);
                h = MIN(m->pict_height, world->height - y0);
                x1 = x0 < 0 ? -x0 : 0;
                y1 = y0 < 0 ? -y0 : 0;
        #define ALPHA_MIX(__orig, __new) \
            (__orig = (__orig * (65535 - alpha) + __new * alpha) / 65535)
        #define COPY_PICT(__fx, __fy) do { \
                for (y = y1; y < h; ++y) \
                    for (x = x1; x < w; ++x) { \
                        alpha = m->picture[(int)(y * m->pict_width + x) * 4 + 3]; \
                        ALPHA_MIX(ipict[(int)((world->width - y - y0 - 1) * world->width + x + x0) * 3 + 0], m->picture[(int)((__fy) * m->pict_width + (__fx)) * 4 + 0]); \
                        ALPHA_MIX(ipict[(int)((world->width - y - y0 - 1) * world->width + x + x0) * 3 + 1], m->picture[(int)((__fy) * m->pict_width + (__fx)) * 4 + 1]); \
                        ALPHA_MIX(ipict[(int)((world->width - y - y0 - 1) * world->width + x + x0) * 3 + 2], m->picture[(int)((__fy) * m->pict_width + (__fx)) * 4 + 2]); \
                    } \
                } while (0)
                // XXX: Will the compiler detect unchanged values (__fx and__fy) in loops and optimize?
                // We won't need these macros if so.
                if (m->flipped_y) {
                    if (m->flipped_x) COPY_PICT(m->pict_width - x - 1, m->pict_height - y - 1);
                    else COPY_PICT(x, m->pict_height - y - 1);
                } else {
                    if (m->flipped_x) COPY_PICT(m->pict_width - x - 1, y);
                    else COPY_PICT(x, y);
                }
        #undef COPY_PICT
        #undef ALPHA_MIX
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
    frame_pusher_close(pusher);
    printf("Video time: %f s\n", world->duration);
    printf("Execution time: %f s\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);
}
#undef MIN
