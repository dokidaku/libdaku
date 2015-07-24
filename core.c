#include "core.h"
#include "types.h"
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

daku_world *daku_world_create(float width, float height, float duration)
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

void daku_world_write(daku_world *world, const char *path)
{
    unsigned int line_size = world->width * 3 * sizeof(uint8_t);
    unsigned int buf_size = line_size * world->height;
    uint8_t *pict = (uint8_t *)malloc(buf_size);
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
    int x, y;
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
                for (x = 0; x < m->pict_height; ++x)
                    for (y = 0; y < m->pict_width; ++y) {
                        pict[x * line_size + y * 3] = m->picture[(int)(x * m->pict_width * 3) + y * 3];
                        pict[x * line_size + y * 3 + 1] = m->picture[(int)(x * m->pict_width * 3) + y * 3 + 1];
                        pict[x * line_size + y * 3 + 2] = m->picture[(int)(x * m->pict_width * 3) + y * 3 + 2];
                    }
            }
        // Save. No, no, I'll replace this with frame pushers (/_<)
        if (frame_num % 45 == 0) {
            char s[15];
            sprintf(s, "%d.ppm", frame_num);
            __save_frame_ppm(pict, world->width, world->height, line_size, s);
        }
    }
}
