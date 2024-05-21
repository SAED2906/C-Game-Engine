#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#include "constants.h"

#define ASSERT(_e, ...) if (!(_e))  {fprintf(stderr, __VA_ARGS__); exit(1); }

typedef int bool;
#define true 1
#define false 0

const int SCREEN_HEIGHT = 720;
const int SCREEN_WIDTH = 1280;

SDL_Window* window = NULL;

SDL_Surface* screen_surface = NULL;
SDL_Surface* screen_surface_hello_world = NULL;

bool init();
bool load_media();
void close_window();

SDL_Surface* load_surface (char* path);
SDL_Texture* load_texture(char* path);


SDL_Surface* key_press_surfaces[ KEY_PRESS_SURFACE_TOTAL ];
SDL_Surface* current_surface = NULL;

SDL_Renderer* renderer = NULL;

SDL_Texture* texture = NULL;

struct {
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_Renderer* renderer;
    bool quit;
} state;

struct {
    float x;
    float y;
    float z;
    float theta; // rotation around y-axis
    float phi; // elevation angle
} camera;

struct {
    float x;
    float y;
    float z;
    float theta;
    float phi;
    float width;
    float height;
} cube;

int main(int argc, char* args[]) {

    // Start up SDL and create Window
    ASSERT(
        !SDL_Init(SDL_INIT_VIDEO),
        "SDL failed to initialize: %s\n",
        SDL_GetError());

    state.window =
        SDL_CreateWindow(
            "Object Renderer v0.01",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1280,
            720,
            SDL_WINDOW_SHOWN
            //SDL_WINDOW_ALLOW_HIGHDPI
            );

    ASSERT(
        state.window,
        "Failed to create SDL window: %s\n",
        SDL_GetError());

    state.renderer =
        SDL_CreateRenderer(
            state.window,
            -1,
            SDL_RENDERER_ACCELERATED
            //SDL_RENDERER_PRESENTVSYNC
            );

    ASSERT(
        state.renderer,
        "Failed to create SDL renderer: %s\n", SDL_GetError()
        );

    state.texture =
        SDL_CreateTexture(
            state.renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);

    camera.x = -10.0f;
    camera.y = 0.0f;
    camera.z = 0.0f;
    camera.theta = 0.0f;
    camera.phi = 0.0f;

    cube.x = 10.0f;
    cube.y = 0.0f;
    cube.z = 0.0f;
    cube.theta = 0.0f;
    cube.phi = 0.0f;
    cube.width = 100.0f;
    cube.height = 100.0f;

    SDL_Event e;
    bool quit = false;
    while( !quit )
    {
        while( SDL_PollEvent( &e ) )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
            else if ( e.type == SDL_KEYDOWN )
            {
                // Switch surfaces based on key press
                switch ( e.key.keysym.sym )
                {
                    case SDLK_UP:
                        current_surface =
                            key_press_surfaces[ KEY_PRESS_SURFACE_UP ];
                            camera.y-=2.5f;
                        break;
                    case SDLK_DOWN:
                        current_surface =
                            key_press_surfaces[ KEY_PRESS_SURFACE_DOWN ];
                            camera.y+=2.5f;
                        break;
                    default:
                        current_surface =
                            key_press_surfaces[ KEY_PRESS_SURFACE_DEFAULT ];
                        break;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor( state.renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear( state.renderer );

        // Render texture
        //SDL_RenderCopy( renderer, texture, NULL, NULL );

        //Distance between camera and object
        float distance = sqrtf( (camera.x-cube.x)*(camera.x-cube.x) +
                                (camera.y-cube.y)*(camera.y-cube.y) +
                                (camera.z-cube.z)*(camera.z-cube.z)
                               );
        printf( "Distance to cube: %fm\n", distance );


        SDL_Vertex verts[] = {
            { {SCREEN_WIDTH/2-cube.width/2, SCREEN_HEIGHT/2-cube.y-cube.height/2 + camera.y}, {255,255,255,255}, {0.0f,0.0f} },
            { {SCREEN_WIDTH/2+cube.width/2, SCREEN_HEIGHT/2-cube.y-cube.height/2 + camera.y}, {255,255,255,255}, {1.0f,0.0f} },
            { {SCREEN_WIDTH/2-cube.width/2, SCREEN_HEIGHT/2-cube.y+cube.height/2 + camera.y}, {255,255,255,255}, {0.0f, 1.0f} },
            { {SCREEN_WIDTH/2+cube.width/2, SCREEN_HEIGHT/2-cube.y+cube.height/2 + camera.y}, {0,255,255,255}, {1.0f,1.0f} }
        };

        int indices[] = { 0, 1, 2, 1, 2, 3};
        SDL_RenderGeometry(state.renderer, NULL, verts, 4, &indices, 6);
        // Renderer blue filled quad
        SDL_Rect fill_rect =
            {   SCREEN_WIDTH / 4, SCREEN_HEIGHT/2-cube.y-cube.height/2 + camera.y,
                SCREEN_WIDTH / 2, cube.height
            };
        SDL_SetRenderDrawColor( state.renderer, 0x00, 0x00, 0xFF, 0xFF );
        //SDL_RenderFillRect( state.renderer, &fill_rect );

        //Render green outlined quad
        SDL_Rect outline_rect =
        {   SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6,
            SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3
        };
        SDL_SetRenderDrawColor( state.renderer, 0x00, 0xFF, 0x00, 0xFF );
        SDL_RenderDrawRect( state.renderer, &outline_rect );

        //Draw blue horizontal line
        SDL_SetRenderDrawColor( state.renderer, 0x00, 0x00, 0xFF, 0xFF );
        SDL_RenderDrawLine( state.renderer, 0, SCREEN_HEIGHT / 2,
                            SCREEN_WIDTH, SCREEN_HEIGHT / 2
                            );

        //Draw vertical line of yellow dots
        SDL_SetRenderDrawColor( state.renderer, 0xFF, 0xFF, 0x00, 0xFF );

        SDL_RenderPresent( state.renderer );
    }

    // Free resources
    close_window();

    return 0;
}

bool init()
{
    bool success = true;

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        // Create Window
        window = SDL_CreateWindow( "SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );

        if ( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = true;
        }
        else
        {
            // initialize renderer
            renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
            if ( renderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                // initialize renderer colour
                SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );

                // initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if ( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                else
                {
                    // Get window surface
                    screen_surface = SDL_GetWindowSurface( window );
                }
            }
        }
    }
    return success;
}

bool load_media()
{
    // Loading success flag
    bool success = true;

    /*
    // Load default surface
    key_press_surfaces[ KEY_PRESS_SURFACE_DEFAULT ] = load_surface( "textures/test.bmp" );
    if ( key_press_surfaces[ KEY_PRESS_SURFACE_DEFAULT ] == NULL )
    {
        printf( "Failed to load up image!\n" );
        success = false;
    }

    // Load down surface
    key_press_surfaces[ KEY_PRESS_SURFACE_DOWN ] = load_surface( "textures/test.bmp" );
    if ( key_press_surfaces[ KEY_PRESS_SURFACE_DOWN ] == NULL )
    {
        printf( "Failed to load up image!\n" );
        success = false;
    }

    // Load up surface
    key_press_surfaces[ KEY_PRESS_SURFACE_UP ] = load_surface( "textures/test2.bmp" );
    if ( key_press_surfaces[ KEY_PRESS_SURFACE_UP ] == NULL )
    {
        printf( "Failed to load up image!\n" );
        success = false;
    }
    */
    // Loading PNG Texture
    texture = load_texture( "textures/test.png" );
    if ( texture == NULL )
    {
        printf( "Failed to load texture image!\n" );
        success = false;
    }
    return success;
}

void close_window()
{
    // Free loaded image
    SDL_DestroyTexture( texture );
    texture = NULL;

    // Destroy Window
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );

    window = NULL;
    renderer = NULL;

    // Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

SDL_Surface* load_surface( char* path )
{
    // The final optimized image
    SDL_Surface* optimized_surface = NULL;

    // Load image at specified path
    SDL_Surface* loaded_surface = IMG_Load( path );
    if ( loaded_surface == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", path, SDL_GetError() );
    }
    else
    {
        // Convert to screen format
        optimized_surface = SDL_ConvertSurface( loaded_surface, screen_surface->format, 0 );
        if ( optimized_surface == NULL )
        {
            printf( "Unable to optimize image %s! SDL Error: %s\n", path , SDL_GetError() );
        }

        SDL_FreeSurface( loaded_surface );
    }

    return optimized_surface;
}

SDL_Texture* load_texture(char* path)
{
    // The final texture
    SDL_Texture* new_texture = NULL;

    // Load image at specified path
    SDL_Surface* loaded_surface = IMG_Load( path );
    if ( loaded_surface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError() );
    }
    else
    {
        // Create texture from surface pixels
        new_texture = SDL_CreateTextureFromSurface( renderer, loaded_surface );
        if ( new_texture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
        }

        // Get rid of old loaded surface
        SDL_FreeSurface( loaded_surface );
    }

    return new_texture;
}
/**
 *SDL_Rect stretchRect;
                stretchRect.x = 0;
                stretchRect.y = 0;
                stretchRect.w = SCREEN_WIDTH;
                stretchRect.h = SCREEN_HEIGHT;
                SDL_BlitScaled( gStretchedSurface, NULL, gScreenSurface, &stretchRect );
 *
 */
