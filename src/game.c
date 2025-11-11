#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_camera.h"
#include "gf3d_mesh.h"

#include "entity.h"
#include "player.h"
#include "world.h"
#include "camera_entity.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}


int main(int argc,char *argv[])
{
    //local variables
    //Sprite *bg;
    // float theta = 0;
    GFC_Vector3D cam = { 0,-45,15 };
    GFC_Vector3D lightPos = { -10, 0, 25 };

    Mesh* skybox;
    GFC_Matrix4 skyboxID;
    Texture* skyTexture;

    World* testworld;
    GFC_Matrix4 testworldID;
    CameraEntity* ce;
    
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    entity_system_init(100);
    //game init
    srand(SDL_GetTicks());
    slog_sync();
    //bg = gf2d_sprite_load_image("images/bg_flat.png");
    gf2d_mouse_load("actors/mouse.actor");
    gf3d_camera_look_at(gfc_vector3d(0, 0, 0), &cam);
    // Spawn player with vertical offset to avoid ground clipping
    player_spawn(gfc_vector3d(0, 0, 20), GFC_COLOR_WHITE);
    ce = camera_entity_new(); // Create camera entity to follow player

    skybox = gf3d_mesh_load("models/sky/sky.obj");
    skyTexture = gf3d_texture_load("models/sky/k0rILCL.png");
    gfc_matrix4_identity(skyboxID);

    // Make Terrain and add file here
    testworld = world_load("defs/terrain/terrain1.def");
    gfc_matrix4_identity(testworldID);
    // main game loop    
    while(!_done)
    {
        gfc_input_update();
        SDL_GetKeyboardState(NULL);
        gf2d_mouse_update();
        gf2d_font_update();
        //camera updaes
        gf3d_camera_update_view();
        gf3d_vgraphics_render_start();
                //3d draws
                entity_think_all();
                entity_update_all();
                camera_think(ce);
                gf3d_mesh_sky_draw(skybox, skyboxID, GFC_COLOR_WHITE, skyTexture);
                world_draw(testworld);
                // Enetities get drawn here
                entity_draw_all(lightPos, GFC_COLOR_WHITE);
                //2D draws
                //gf2d_sprite_draw_image(bg,gfc_vector2d(0,0));
                gf2d_font_draw_line_tag("ctrl q",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                gf2d_mouse_draw();
        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/