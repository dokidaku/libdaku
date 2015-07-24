#include "core.h"
#include "types.h"
#include "utils/frame-pusher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

daku_matter *daku_matter_create()
{
    daku_matter *ret = (daku_matter *)malloc(sizeof(daku_matter));
    ret->content_width = ret->content_height = ret->pict_width = ret->pict_height = 0;
    ret->anchor_x = ret->anchor_y = 0.5;
    ret->x = ret->y = ret->rotation = 0;
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
    m->picture = (uint8_t *)malloc(m->pict_width * m->pict_height * 3 * sizeof(uint8_t));
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
    frame_pusher *pusher;
    if (frame_pusher_open(&pusher, path, 44100, world->fps, world->width, world->height, 800000) < 0) return;
    unsigned int line_size = world->width * 3 * sizeof(uint8_t);
    if (line_size % 64) line_size += (64 - line_size % 64); // Will this work...?
    unsigned int buf_size = line_size * world->height;
    uint8_t *pict = (uint8_t *)malloc(buf_size);
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
    int x0, y0, x, y, w, h;
    for (frame_num = 0; frame_num < world->duration * world->fps; ++frame_num) {
        // Render one frame.
        memset(pict, 0, buf_size);
        cur_time = (float)frame_num / (float)world->fps;
        daku_list_foreach_t(world->population, daku_matter *, m)
            if (m && m->start_time <= cur_time
                && m->start_time + m->life_time >= cur_time)
            {
                daku_list_foreach(m->actions, ac)
                    if (ac && m->start_time + ac->start_time <= cur_time
                        && m->start_time + ac->start_time + ac->duration >= cur_time)
                    {
                        ac->update(ac, (cur_time - m->start_time - ac->start_time) / ac->duration);
                    }
                x0 = m->x - m->anchor_x * m->pict_width;
                y0 = m->y - m->anchor_y * m->pict_height;
                w = MIN(m->pict_width, world->width - x0);
                h = MIN(m->pict_height, world->height - y0);
                for (y = 0; y < h; ++y)
                    for (x = 0; x < w; ++x) {
                        pict[(int)(y + y0) * line_size + (int)(x + x0) * 3] = m->picture[(int)(y * m->pict_width * 3) + x * 3];
                        pict[(int)(y + y0) * line_size + (int)(x + x0) * 3 + 1] = m->picture[(int)(y * m->pict_width * 3) + x * 3 + 1];
                        pict[(int)(y + y0) * line_size + (int)(x + x0) * 3 + 2] = m->picture[(int)(y * m->pict_width * 3) + x * 3 + 2];
                    }
            }
        // Save.
        frame_pusher_write_video(pusher, pusher_pict, pusher_linesize, 1);
    }
    frame_pusher_close(pusher);
}
#undef MIN
