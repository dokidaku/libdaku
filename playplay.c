#include "daku.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    /*if (argc < 2) {
        printf("Usage: %s <output>\n", argv[0]);
        printf("A sample for libdaku.\n\n");
        return 888;
    }*/
    daku_world *world = daku_world_create(600, 600, 15);
    daku_matter *m;

    m = daku_matter_create(); //daku_shape_create(DAKU_SHAPE_RECT);
    daku_matter_setlife(m, 7);
    daku_matter_setsize(m, 200, 400);
    daku_matter_setpos(m, 100, 233);
    //daku_matter_act(m, 2, daku_fx_moveby(3, 150, 180));
    daku_world_populate(world, m, 5);

    daku_world_write(world, argv[1]);

    return 0;
}
