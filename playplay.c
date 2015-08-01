#include "daku.h"
#include <stdio.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <output>\n", argv[0]);
        printf("A sample for libdaku.\n\n");
        return 888;
    }
    av_register_all();
    daku_world *world = daku_world_create(600, 600, 15);
    world->sample_rate = 44100;
    daku_matter *m;
    daku_wave *w;

    m = daku_matter_create();
    daku_matter_setlife(m, 7);
    daku_matter_setsize(m, 200, 400);
    daku_matter_setanchor(m, 0, 0);
    daku_matter_setpos(m, 233, -40);
    daku_matter_act(m, 0, daku_matter_shape(7, DAKU_SHAPE_RECT, 0x66ccff, 255));
    daku_matter_act(m, 2, daku_fx_moveby(3, 233, 90));
    daku_matter_act(m, 6, daku_fx_moveby(0.5, 0, 350));
    daku_matter_act(m, 6.5, daku_fx_moveto(0.5, 0, 0));
    m->opacity = 64;
    daku_matter_act(m, 0, daku_fx_fadein(2));
    daku_world_populate(world, m, 5);

    m = daku_matter_create();
    daku_matter_setlife(m, 9);
    daku_matter_setsize(m, 300, 300);
    daku_matter_setanchor(m, 1, 0);
    daku_matter_setpos(m, 600, 0);
    daku_matter_act(m, 0, daku_matter_shape(9, DAKU_SHAPE_RECT, 0x99ffff, 128));
    daku_world_populate(world, m, 4);

    m = daku_matter_create();
    daku_matter_setlife(m, 5);
    daku_matter_setsize(m, 300, 200);
    daku_matter_setpos(m, 300, 300);
    daku_matter_act(m, 0, daku_video_clip("xx.mp4", 3, 5));
    daku_matter_act(m, 1, daku_fx_fadeto(4, 128));
    m->flipped_y = 1;
    m->flipped_x = 1;
    daku_world_populate(world, m, 0);

    w = daku_wave_create();
    daku_wave_setlife(w, 15);
    daku_wave_play(w, 0, daku_audio_clip("xx.mp3", 208, 15));
    daku_wave_play(w, 0, daku_audio_reverse(15));
    daku_world_clang(world, w, 0);

    m = daku_matter_create();
    daku_matter_setlife(m, 5);
    daku_matter_setsize(m, 100, 100);
    daku_matter_setanchor(m, 1, 0);
    daku_matter_setpos(m, 500, 100);
    daku_matter_act(m, 0, daku_video_clip("xx.gif", 3, 5));
    daku_matter_act(m, 1, daku_fx_fadeto(4, 128));
    daku_world_populate(world, m, 0);

    m = daku_matter_create();
    daku_matter_setlife(m, 5);
    daku_matter_setsize(m, 150, 150);
    daku_matter_setanchor(m, 0, 1);
    daku_matter_setpos(m, 100, 500);
    daku_matter_act(m, 0, daku_image_clip(5, "xx.png"));
    m->flipped_x = 1;
    daku_world_populate(world, m, 1);

    m = daku_matter_create();
    daku_matter_setlife(m, 4);
    daku_matter_setsize(m, 600, 272);
    daku_matter_setanchor(m, 0, 0);
    daku_matter_setpos(m, 0, 0);
    daku_matter_act(m, 0, daku_text(4,
        "Proudly powered by DOKIDAKU\nCC BY-NC-SA International 4.0\n大家好我是中文，我是中文", "xx.ttc",
        24, 0, DAKU_HALIGN_LEFT, 0xc0ffee));
    daku_matter_act(m, 0, daku_fx_fadein(0.5));
    daku_matter_act(m, 3.5, daku_fx_fadeout(0.5));
    daku_matter_act(m, 0, daku_fx_moveby(4, 0, 40));
    m->opacity = 0;
    daku_world_populate(world, m, 0);

    m = daku_matter_create();
    daku_matter_setlife(m, 4);
    daku_matter_setsize(m, 600, 120);
    daku_matter_setanchor(m, 1, 1);
    daku_matter_setpos(m, 600, 600);
    daku_matter_act(m, 0, daku_text(4,
        "by Pisces000221\n1786762946@qq.com\n@Pisces000221 on GitHub", "xx.ttf",
        12, 0, DAKU_HALIGN_RIGHT, 0xffffff));
    daku_matter_act(m, 0, daku_fx_fadein(0.5));
    daku_matter_act(m, 3.5, daku_fx_fadeout(0.5));
    daku_matter_act(m, 0, daku_fx_moveby(4, -30, 0));
    m->opacity = 0;
    daku_world_populate(world, m, 0);

    daku_world_write(world, argv[1]);

    return 0;
}
