#include <obelisk.h>

u8 onLoad(ObeliskApp* app, u32 w, u32 h)
{
    MINIMAL_INFO("[Minimal] Version: %s", minimalGetVersionString());
    
    return MINIMAL_OK;
}

void onDestroy(ObeliskApp* app)
{

}

u8 onEvent(ObeliskApp* app, const MinimalEvent* e)
{
    if (minimalEventKeyPressed(e) == MINIMAL_KEY_ESCAPE)
        minimalClose(app->window);

    return MINIMAL_OK;
}

void onTick(ObeliskApp* app, f32 deltatime)
{

}

int main(void)
{
    ObeliskApp app = {
        .on_load =    onLoad,
        .on_destroy = onDestroy,
        .on_event =   onEvent,
        .on_tick =    onTick
    };

    if (obeliskLoad(&app, "Obelisk Test", 100, 100, 1280, 720))
        obeliskRun(&app);

    obeliskDestroy(&app);

    return 0;
}