#include <minimal.h>

u8 onLoad(MinimalApp* app, u32 w, u32 h)
{
    MINIMAL_INFO("[Minimal] Version: %s", minimalGetVersionString());
    
    return MINIMAL_OK;
}

void onDestroy(MinimalApp* app)
{

}

u8 onEvent(MinimalApp* app, const MinimalEvent* e)
{
    if (minimalEventKeyPressed(e) == MINIMAL_KEY_ESCAPE)
        minimalClose(app);

    return MINIMAL_OK;
}

void onTick(MinimalApp* app, f32 deltatime)
{

}

int main(void)
{
    MinimalApp app = {
        .on_load =    onLoad,
        .on_destroy = onDestroy,
        .on_event =   onEvent,
        .on_tick =    onTick
    };

    if (minimalLoad(&app, "Obelisk Test", 100, 100, 1280, 720))
        minimalRun(&app);

    minimalDestroy(&app);

    return 0;
}