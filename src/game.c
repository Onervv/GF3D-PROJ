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
#include "gf3d_particle.h"

#include "menu.h"
#include "entity.h"
#include "player.h"
#include "world.h"
#include "camera_entity.h"
#include "gfc_audio.h"
#include "hud.h"


extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;
static float deltaTime = 0.0f;

void parse_arguments(int argc,char *argv[]);
float game_frame_delay();

void exitGame()
{
    _done = 1;
}


int main(int argc,char *argv[])
{
    //local variables
    GFC_Vector3D cam = { 0,-45,15 };
    GFC_Vector3D lightPos = { -10, 0, 25 };

    Mesh* skybox;
    GFC_Matrix4 skyboxID;
    Texture* skyTexture;

    World* testworld;
    GFC_Matrix4 testworldID;
    CameraEntity* ce;
    Mix_Music *background_music = NULL;
    HUD *hud = NULL;
    Entity *playerEntity = NULL;
    Menu* menu = NULL;
    
    // Rain effect timer (ADD THIS)
    static float rainSpawnTimer = 0.0f;
    
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
    // particles init 
    gf3d_particle_system_init(1000);  // Max 1000 particles
    //audio init
    gfc_audio_init(256,1,1);
    //hud init
    hud = hud_init();
    //game init
    srand(SDL_GetTicks());
    slog_sync();
    
    gf2d_mouse_load("actors/mouse.actor");
    gf3d_camera_look_at(gfc_vector3d(0, 0, 0), &cam);
    
    menu = menu_init();
    playerEntity = player_spawn(gfc_vector3d(0, 0, 20), GFC_COLOR_WHITE);
    ce = camera_entity_new();

    skybox = gf3d_mesh_load("models/sky/sky.obj");
    skyTexture = gf3d_texture_load("models/sky/k0rILCL.png");
    gfc_matrix4_identity(skyboxID);

    testworld = world_load("defs/terrain/terrain1.def");
    gfc_matrix4_identity(testworldID);

    background_music = gfc_sound_load_music("music/arcade-beat-323176.mp3");
    if (background_music) {
        Mix_PlayMusic(background_music, -1);
        Mix_VolumeMusic(64);
    }
    
    // main game loop    
    // main game loop    
while(!_done)
{
    gfc_input_update();
    SDL_GetKeyboardState(NULL);
    gf2d_mouse_update();
    gf2d_font_update();
    
    // Update menu
    if (menu) {
        menu_update(menu, deltaTime);
        
        // Check if game should start
        if (menu_get_state(menu) == MENU_STATE_GAME && !playerEntity) {
            slog("Starting game...");
            playerEntity = player_spawn(gfc_vector3d(0, 0, 20), GFC_COLOR_WHITE);
            ce = camera_entity_new();
        }
    }
    
    // Only update game if player exists (game has started)
    if (playerEntity) {
        hud_update(hud, playerEntity);
        
        // UPDATE PARTICLES
        gf3d_particle_update(deltaTime);
        
        // UPDATE RAIN EFFECT
        Entity* player = get_the_player();
        if (player) {
            gf3d_particle_update_rain(deltaTime, player->position);
        }
        
        // Manual explosion burst for testing
        if (gfc_input_command_pressed("spawn_particles")) {
            Entity* player = get_the_player();
            if (player) {
                gf3d_particle_spawn_explosion(player->position, 50);
                slog("Spawned explosion burst!");
            }
        }
    }
    
    gf3d_camera_update_view();
    gf3d_vgraphics_render_start();
    
    // Draw menu OR game
    if (menu && menu_get_state(menu) == MENU_STATE_MAIN) {
        menu_draw(menu);
        
    } else {
        // IN GAME - draw game world
        entity_think_all(deltaTime);
        entity_update_all(deltaTime);
        if (ce) camera_think(ce);
        
        gf3d_mesh_sky_draw(skybox, skyboxID, GFC_COLOR_WHITE, skyTexture);
        world_draw(testworld);
        entity_draw_all(lightPos, GFC_COLOR_WHITE);
        
        // DRAW PARTICLES (after entities, before HUD)
        gf3d_particle_draw();
        
        if (playerEntity) {
            hud_draw(hud);
        }
    }
    
    // Always draw mouse and UI text on top
    gf2d_font_draw_line_tag("ctrl q", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(10, 10));
    gf2d_mouse_draw();
    
    gf3d_vgraphics_render_end();
    
    if (gfc_input_command_down("exit")) _done = 1;
    deltaTime = game_frame_delay();
}

vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());

// Cleanup
if (background_music) {
    Mix_FreeMusic(background_music);
}
hud_free(hud);
menu_free(menu);
gf3d_particle_system_close();

slog("gf3d program end");
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

float game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    float delta;
    
    then = now;
    slog_sync();
    now = SDL_GetTicks();
    diff = (now - then);
    
    delta = diff / 1000.0f;  // Convert milliseconds to seconds
    
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then, 0.001);
    
    return delta;  // Return delta time in seconds
}
/*eol@eof*/