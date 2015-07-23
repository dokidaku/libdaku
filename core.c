#include "core.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

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
    ret->duration = duration;
    ret->population = daku_list_create(NULL);
    return ret;
}

void daku_world_populate(daku_world *world, daku_matter *resident, float start_time)
{
    resident->start_time = start_time;
    daku_list_push_back(world->population, resident);
}

void daku_world_write(daku_world *world, const char *path)
{
    daku_matter *m;
    daku_list_foreach_t(world->population, daku_matter *, m) if (m) {
        printf("RESIDENT 0x%x\n", (int)m);
        printf("Start: %f, life: %f\n", m->start_time, m->life_time);
        printf("Net: %dx%d, gross: %dx%d\n",
            (int)m->content_width, (int)m->content_height,
            (int)m->pict_width, (int)m->pict_height);
        printf("(%f, %f), rotation %f\n\n", m->x, m->y, m->rotation);
    }
}
