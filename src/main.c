#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "constants.h"

typedef int bool;
#define true 1
#define false 0

const int SCREEN_HEIGHT = 1080;
const int SCREEN_WIDTH = 1920;

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

int main(int argc, char* args[]) {

    // Start up SDL and create Window

    if ( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        // Load media
        if ( !load_media() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
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
                                current_surface = key_press_surfaces[ KEY_PRESS_SURFACE_UP ];
                                break;
                            case SDLK_DOWN:
                                current_surface = key_press_surfaces[ KEY_PRESS_SURFACE_DOWN ];
                                break;
                            default:
                                current_surface = key_press_surfaces[ KEY_PRESS_SURFACE_DEFAULT ];
                                break;
                        }
                    }
                }

                // Clear screen
                SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear( renderer );

                // Render texture
                //SDL_RenderCopy( renderer, texture, NULL, NULL );

                // Renderer blue filled quad
                SDL_Rect fill_rect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0xFF, 0xFF );
                SDL_RenderFillRect( renderer, &fill_rect );

                //Render green outlined quad
                SDL_Rect outline_rect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
                SDL_SetRenderDrawColor( renderer, 0x00, 0xFF, 0x00, 0xFF );
                SDL_RenderDrawRect( renderer, &outline_rect );

                //Draw blue horizontal line
                SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0xFF, 0xFF );
                SDL_RenderDrawLine( renderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2 );

                //Draw vertical line of yellow dots
                SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0x00, 0xFF );
                for( int i = 0; i < SCREEN_HEIGHT; i += 4 )
                {
                    SDL_RenderDrawPoint( renderer, SCREEN_WIDTH / 2, i );
                }

                // Update screen
                SDL_RenderPresent( renderer );
            }
        }
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
