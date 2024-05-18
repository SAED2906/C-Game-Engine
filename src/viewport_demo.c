#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#include "constants.h"

typedef int bool;
#define true 1
#define false 0

#define PI 3.1415926535f
#define CELL_SIZE 100.0f
#define degree 0.0174533f

typedef struct {
    int width;
    int height;
    int data[6][6];
    int total;
} Map;

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

void print_map(Map*);
void initialize_map(Map*);
void initialize_verts(SDL_Vertex**, int*, const Map*);

void raycast(Map*, SDL_Renderer*);

SDL_Surface* key_press_surfaces[ KEY_PRESS_SURFACE_TOTAL ];
SDL_Surface* current_surface = NULL;

SDL_Renderer* renderer = NULL;

SDL_Texture* texture = NULL;



float px, py, pdx, pdy, pa;


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
            int counter = 0;
            Uint32 start_time = SDL_GetTicks();
            py = SCREEN_HEIGHT/2.0f;
            px = SCREEN_WIDTH/2.0f;
            pa = 0.01f;
            pdx = cos(pa) * 5;
            pdy = sin(pa) * 5;

            // Load Map
            Map* current_map = (Map*) malloc(sizeof(Map));
            initialize_map(current_map);
            print_map(current_map);

            SDL_Vertex** verts = (SDL_Vertex**) malloc(current_map->total*sizeof(SDL_Vertex));
            int total_verts = 0;
            initialize_verts(verts, &total_verts, current_map);

            while( !quit )
            {
                while( SDL_PollEvent( &e ) )
                {
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                        free(current_map);
                        //free(verts);
                    }
                    else if ( e.type == SDL_KEYDOWN )
                    {
                        // Switch surfaces based on key press
                        switch ( e.key.keysym.sym )
                        {
                            case SDLK_a:
                                pa-=0.1f;
                                if ( pa < 0 ){ pa += 2 * PI;}
                                pdx = cos(pa) * 5;
                                pdy = sin(pa) * 5;
                                break;
                            case SDLK_d:
                                pa+=0.1f;
                                if ( pa > 2 * PI ){ pa -= 2 * PI;}
                                pdx = cos(pa)*5;
                                pdy = sin(pa)*5;
                                break;
                            case SDLK_w:
                                px += pdx;
                                py += pdy;
                                break;
                            case SDLK_s:
                                px -= pdx;
                                py -= pdy;
                                break;
                            case SDLK_UP:
                                //current_surface = key_press_surfaces[
                                    //KEY_PRESS_SURFACE_UP ];
                                py -= 10.0f;
                                break;
                            case SDLK_DOWN:
                                //current_surface = key_press_surfaces[
                                    //KEY_PRESS_SURFACE_DOWN ];
                                py += 10.0f;
                                break;
                            case SDLK_LEFT:
                                px -= 10.0f;
                                break;
                            case SDLK_RIGHT:
                                px += 10.0f;
                                break;
                            default:
                                current_surface = key_press_surfaces[
                                    KEY_PRESS_SURFACE_DEFAULT ];
                                quit = true;
                                break;
                        }
                    }
                }
                //if (posy < 0) posy += SCREEN_HEIGHT;
                //if (posy > SCREEN_HEIGHT) posy -= SCREEN_HEIGHT;

                Uint32 current_time = SDL_GetTicks();
                // Clear screen
                SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear( renderer );

                SDL_Vertex verts2[] = {
                    { { px-25.0f, py-25.0f }, { 255, 255, 255, 255 }, { 0.0f, 0.0f } },  // Top-left vertex
                    { { 25.0f + px, py - 25.0f}, { 255, 255, 255, 255 }, { 1.0f, 0.0f } },  // Top-right vertex
                    { { px-25.05, 25.0f + py}, { 255, 255, 255, 255 }, { 0.0f, 1.0f } },  // Bottom-right vertex
                    { { 25.0f + px, 25.0f + py}, { 255, 255, 255, 255 }, { 1.0f, 1.0f } }   // Bottom-left vertex
                };

                // Indices for the two triangles that make up the quad
                int indices[] = { 0, 1, 2, 1, 2, 3 };
                for (int v = 0; v < total_verts; v++) {
                    SDL_RenderGeometry(renderer, texture, verts[v], 4, indices, 6);
                }
                SDL_RenderGeometry(renderer, texture, verts2, 4, indices, 6);
                //SDL_RenderGeometry(renderer, texture, verts, 4, indices, 6);
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                //SDL_RenderDrawLine(renderer, 75.0f + px, 75.0f+py, px + 75.0f + pdx*15, py + 75.0f + pdy*15);

                raycast(current_map, renderer);

                // Top left
                //SDL_Rect top_left_viewport;
                //top_left_viewport.x = 0;
                //top_left_viewport.y = 0 + posy;
                //top_left_viewport.w = SCREEN_WIDTH / 2;
                //top_left_viewport.h = SCREEN_HEIGHT / 2;

                //SDL_RenderSetViewport( renderer, &top_left_viewport );
                //SDL_RenderCopy( renderer , texture, NULL, NULL );

                // Top right
                //SDL_Rect top_right_viewport;
                //top_right_viewport.x = SCREEN_WIDTH / 2;
                //top_right_viewport.y = 0 + posy;
                //top_right_viewport.w = SCREEN_WIDTH / 2;
                //top_right_viewport.h = SCREEN_HEIGHT / 2;

                //SDL_RenderSetViewport( renderer, &top_right_viewport);
                //SDL_RenderCopy( renderer, texture, NULL, NULL );


                // Update screen
                SDL_RenderPresent( renderer );
                float avg_fps = counter / ((SDL_GetTicks() - start_time)/1000.f);
                //float fps =  1000.f / (SDL_GetTicks() - current_time);
                printf( " Ave FPS %f\n", avg_fps );
                //printf( "FPS: %f\n", fps );

                counter++;
            }
            free(verts);
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

void print_map(Map* map) {
    for (int i = 0; i < map->height; ++i) {
        for (int j = 0; j < map->width; ++j) {
            printf("%d ", map->data[i][j]);
        }
        printf("\n");
    }
}

void initialize_map(Map* map) {
    int initialData[6][6] = {
        { 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 1, 1 },
        { 1, 0, 1, 0, 1, 1 },
        { 1, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1 }
    };

    map->width = 6;
    map->height = 6;
    map->total = 25;

    // Copy initialData into the map's data array
    for (int i = 0; i < map->height; ++i) {
        for (int j = 0; j < map->width; ++j) {
            map->data[i][j] = initialData[i][j];
        }
    }
}

void initialize_verts(SDL_Vertex** verts, int* vertCount, const Map* map)
{
    int count = 0;
    for (int i = 0; i < map->height; ++i)
    {
        for (int j = 0; j < map->width; ++j)
        {
            if (map->data[i][j] == 1)
            {
                SDL_Vertex* quad = (SDL_Vertex*)malloc(4 * sizeof(SDL_Vertex));
                if (quad == NULL)
                {
                    printf("Failed to allocate memory for quad\n");
                    exit(1);
                }
                quad[0] = (SDL_Vertex){{ j * 100.0f, i * 100.0f }, { 255, 0, 0, 255 }, { 0.0f, 0.0f }};
                quad[1] = (SDL_Vertex){{ j * 100.0f, (i + 1.0f) * 100.0f }, { 0, 255, 0, 255 }, { 1.0f, 0.0f }};
                quad[2] = (SDL_Vertex){{ (j + 1.0f) * 100.0f, i * 100.0f }, { 0, 0, 255, 255 }, { 0.0f, 1.0f }};
                quad[3] = (SDL_Vertex){{ (j + 1.0f) * 100.0f, (i + 1.0f) * 100.0f }, { 255, 255, 255, 255 }, { 1.0f, 1.0f }};
                verts[count++] = quad;
            }
        }
    }
    *vertCount = count;
}

float dist(float x1, float y1, float x2, float y2)
{
    return (sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)));
}

void raycast(Map* map, SDL_Renderer* renderer)
{
    int r, mx, my, mp, dof; float rx, ry, ra, xo, yo;
    ra = pa;
    ra -= PI/6;
    for (r = 0; r < 60; r++)
    {
        // Check horizontal
        dof = 0;
        float h_dist = 1000000, hx = px, hy = py;
        float atan = -1/tanf(ra);
        if (sinf(ra) < -0.001) { ry = CELL_SIZE*floorf(py/CELL_SIZE) - 0.0001; rx=(py-ry)*atan+px; yo=-CELL_SIZE; xo=-yo*atan;} // Looking up
        if (sinf(ra) > 0.001) { ry = CELL_SIZE*ceilf(py/CELL_SIZE); rx=(py-ry)*atan+px; yo=CELL_SIZE; xo=-yo*atan;} // Looking down
        if (ra == 0 || ra == PI) {rx = px; ry = py; dof=8;} // Looking left or right
        while (dof < 8)
        {
            mx = (int) rx/CELL_SIZE; my = (int) (ry/CELL_SIZE);

            if ((mx < 6) && (my < 6) && mx >= 0 && my >= 0)
            {
                if (map->data[mx][my] == 1) {dof = 8;} // Hit a wall
                else { rx += xo; ry+=yo; dof++;}
            }
            else { rx += xo; ry+=yo; dof++;} // Next point
        }
        hx = rx;
        hy = ry;
        //SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        //SDL_RenderDrawLine(renderer, px, py, rx, ry);
        // Check Vertical
        float vx = px;
        float vy = py;
        dof = 0;
        float ntan = -tanf(ra);
        if (cosf(ra) < -0.001) { rx = CELL_SIZE*floorf(px/CELL_SIZE) - 0.0001; ry=(px-rx)*ntan+py; xo=-CELL_SIZE; yo=-xo*ntan;} // Looking up
        if (cosf(ra) > 0.001) { rx = CELL_SIZE*ceilf(px/CELL_SIZE); ry=(px-rx)*ntan+py; xo=CELL_SIZE; yo=-xo*ntan;} // Looking down
        if (ra == PI/2 || ra == 3*PI/4) {rx = px; ry = py; dof=8;} // Looking left or right
        while (dof < 8)
        {
            mx = (int) rx/CELL_SIZE; my = (int) ry/CELL_SIZE;

            if ((mx < 6) && (my < 6) && mx >= 0 && my >= 0)
            {
                if (map->data[mx][my] == 1) { dof = 8;} // Hit a wall
                else { rx += xo; ry+=yo; dof++;}
            }
            else { rx += xo; ry+=yo; dof++;} // Next point
        }

        float dist1 = 0;
        float dist2 = 0;
        dist1 = dist(rx, ry, px, py);
        dist2 = dist(hx, hy, px, py);
        int colour = 0;
        if (dist1 > dist2) {rx = hx; ry = hy; dist1 = dist2; colour = 1;}
        //} else {
            //if (dist(px, py, rx, ry) > dist(px, py, hx, hy)) {rx = hx; ry = hy;}
        //}
        //printf("Pink: %f\tCyan: %f\n", dist(px, py, hx, hy), dist(px, py, rx, ry));
	
	if (colour)
        {
        	SDL_SetRenderDrawColor(renderer, 127, 127, 167, 255);
        }
        else
        {
        	SDL_SetRenderDrawColor(renderer, 197, 167, 197, 255);
        }
        SDL_RenderDrawLine(renderer, px, py, rx, ry);


	SDL_Rect right;
        right.x = SCREEN_WIDTH/2;
        right.y = 0;
        right.w = SCREEN_WIDTH/2;
        right.h = SCREEN_HEIGHT;

        SDL_RenderSetViewport(renderer, &right );
        
        float ca = pa-ra; if (ca < 0) { ca += 2*PI;} if (ca > 2*PI) { ca -= 2*PI;}
        float line_heigth = (36*SCREEN_HEIGHT)/(dist1*cosf(ca)); if (line_heigth > SCREEN_HEIGHT) {line_heigth = SCREEN_HEIGHT;}
        SDL_RenderSetScale(renderer,15.0f, 1.0f);
        float line = SCREEN_HEIGHT/2 - line_heigth/2;
        SDL_RenderDrawLine(renderer, r, line, r, line_heigth+line);
        
        
        
        SDL_Rect reset;
        reset.y = 0;
        reset.x = 0;
        reset.w = SCREEN_WIDTH;
        reset.h = SCREEN_HEIGHT;

        SDL_RenderSetViewport(renderer, &reset);
        SDL_RenderSetScale(renderer, 1.0f, 1.0f);
        

        ra+=degree;
    }

    //ra = pa;
    //for (r = 0; r < 1; r++)
    //{
    //    dof = 0;
    //    float atan = -1/tan(ra);
    //    if(ra > PI)
    //   {
    //       ry = py*roundf(py/CELL_SIZE); rx = (py-ry)*atan + px
    //    }
    //}
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
