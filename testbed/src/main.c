#include <obelsik.h>

#include <platform/platform.h>

b8 onLoad(ObeliskApp* app, u32 w, u32 h)
{
    return TRUE;
}

void onDestroy(ObeliskApp* app)
{

}

void onTick(ObeliskApp* app, f32 deltatime)
{

}

int main(void)
{
    ObeliskApp app = {
        .on_load =    onLoad,
        .on_destroy = onDestroy,
        //.on_event =   onEvent,
        .on_tick =  onTick
    };

    if (obeliskLoad(&app, "Obelisk Test", 100, 100, 1280, 720))
        obeliskRun(&app);

    obeliskDestroy(&app);

    return 0;
}