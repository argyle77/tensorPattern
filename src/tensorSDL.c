// A Tensor Pattern.
// For Kevin (FB) McCormick (w/o you, there may be nothing) <3
// Blau

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_image.h>
#include <time.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "drv-tensor.h"
#include "tensorDefs.h"
#include "my_font.h"
#include "pattern.h"




// Tensor output
#define USE_TENSOR
#define DEF_IMAGE "Fbyte-01.jpg"

// Preview window definitions
#define PREVIEW_PIXEL_SIZE 10
#define PREVIEW_BORDER_SIZE 10


// Initial Values
#define INITIAL_COLOR_MULTIPLIER 1.0
#define INITIAL_DIFF_COEF 0.002
#define INITIAL_FADEOUT_DEC 1
#define INITIAL_RAND_MOD 4
#define INITIAL_RAINBOW_INC 8
#define INITIAL_EXPAND 1
#define INITIAL_FLOAT_INC 0.1
#define INITIAL_ROTATION_DELTA 1.0
#define INITIAL_ROTATION_ANGLE 0
#define INITIAL_ROTATION_ANGLE2 10





// Gui object pointers
typedef struct {
  GtkWidget *window;
  GtkWidget *randomdotsMode;
} gui_pointers_t;


// Color Constants
const color_t red = COLOR_RED;
const color_t orange = COLOR_ORANGE;
const color_t yellow = COLOR_YELLOW;
const color_t green = COLOR_GREEN;
const color_t cyan = COLOR_CYAN;
const color_t blue = COLOR_BLUE;
const color_t magenta = COLOR_MAGENTA;
const color_t white = COLOR_WHITE;
const color_t light_gray = COLOR_LIGHT_GRAY;
const color_t gray = COLOR_GRAY;
const color_t dark_gray = COLOR_DARK_GRAY;
const color_t black = COLOR_BLACK;

// Some color selections
#define PREVIEW_BACKGROUND_COLOR black
#define PREVIEW_PANEL_COLOR light_gray

// Prototypes
void SetDims(void);
void DrawBox(int x, int y, int w, int h, color_t color);
void DrawPreviewBorder(void);
void InitPreview(void);
void DrawRectangle(int x, int y, int w, int h, color_t color);
void ColorAll(color_t color, unsigned char *fb);
void MomentToGui(moment_t *moment);
void SetPixel(int x, int y, color_t color, unsigned char *buffer);
void Update(float intensity_limit, moment_t *moment);
void UpdateTensor(unsigned char *buffer);
void UpdatePreview(unsigned char *buffer);
color_t GetPixel(int x, int y, unsigned char *buffer);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
G_MODULE_EXPORT void quit (GtkObject *object, gpointer user_data);
Uint32 FrameCounter(Uint32 interval, void *param);
Uint32 TriggerProcessing(Uint32 interval, void *param);
void *MainLoop(void *data);

// Globals
SDL_Surface *preview = NULL;
SDL_Surface *ttemp = NULL;
gui_pointers_t gui_pointers;
int randomMode = NO;  // Random mode is a global (for now).
float global_intensity_limit;
moment_t moments[MOMENT_COUNT];
int now = 0;
moment_t *currentMoment;
modes_t *currentMode;
parms_t *currentParms;
text_info_t *currentText;
unsigned char *currentFB;
int TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF;


// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  GtkBuilder *builder;
  int i;
  static pthread_t patternThread;
  // gint event_mask;

  // Unbuffer the console...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  setvbuf(stderr, (char *)NULL, _IONBF, 0);

  // Initialize GTK with thread awareness
  if (!g_thread_supported ()) {
    g_thread_init(NULL);
  }
  gdk_threads_init();
  gdk_threads_enter();

  // Initialize gtk.  This goes here to strip out any gtk commandline parms.
  gtk_init (&argc, &argv);

  // Load the gui
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "tensor_gui.xml", NULL);

  // Grab anything you might need from the gui.
  gui_pointers.window = GTK_WIDGET(gtk_builder_get_object (builder, "mainWindow"));
  gui_pointers.randomdotsMode = GTK_WIDGET(gtk_builder_get_object (builder, "randomDotCheck"));

  // Attach the widget signal handlers
  gtk_builder_connect_signals(builder, NULL);
  // Free up the memory from the gui xml.
  g_object_unref (G_OBJECT (builder));

  // Events
  //event_mask = GDK_BUTTON_PRESS_MASK & GDK_BUTTON_RELEASE_MASK;
  //gdk_window_set_events((GdkWindow *)gui_pointers.window, event_mask);

  // Show the gui.
  gtk_widget_show(gui_pointers.window);
  // Commandline parameter for limiting the intensity.
  if (argc == 2) {
    global_intensity_limit = atof(argv[1]);
    if (global_intensity_limit < -0.0001 || global_intensity_limit > 1.000001) {
      global_intensity_limit = 1.0;
    }
  } else {
    global_intensity_limit = 1.0;
  }

  // Init the display window.
  if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  } else {
    atexit(SDL_Quit);
  }


  // Set the widths / heights
  tensor_landscape_p = 0;  // Portrait mode.
  InitPreview();

  // Init tensor.
  #ifdef USE_TENSOR
    tensor_init();
  #endif

  // Set up moment 0.
  currentMoment = &moments[now];
  currentMode = &currentMoment->mode;
  currentParms = &currentMoment->coefs;
  currentText = &currentMoment->text;
  currentFB = currentMoment->fb;

  currentMode->randDots = NO;

  currentParms->foreground = RED_E;
  currentParms->randMod = INITIAL_RAND_MOD;

  snprintf(currentText->textBuffer, TEXT_BUFFER_SIZE, "Be the light you wish to see in the world. ");
  currentText->tindex = sizeof("Be the light you wish to see in the world. ") - 1;
  currentText->imaginaryIndex = -1;
  for (i = 1; i < MOMENT_COUNT; i++) {
    memcpy(&moments[i].mode, currentMode, sizeof(modes_t));
    memcpy(&moments[i].coefs, currentParms, sizeof(parms_t));
    memcpy(&moments[i].text, currentText, sizeof(text_info_t));
  }

  MomentToGui(currentMoment);

  // Bam!
  ColorAll(black, currentFB);

  Update(global_intensity_limit, currentMoment);

  // Create the drawing thread.
  pthread_create(&patternThread, NULL, MainLoop, NULL);

  gtk_main();

  gdk_threads_leave();


  // Cleanup. Technically, we never get here.
  exit(EXIT_SUCCESS);  // Is it?
}


// Set up the dimensions and the preview window.
void InitPreview(void) {
  // Vars
  int w, h;

  // Set the width / height.
  if (tensor_landscape_p) {
    TENSOR_WIDTH_EFF = TENSOR_HEIGHT;
    TENSOR_HEIGHT_EFF = TENSOR_WIDTH;
  } else {
    TENSOR_WIDTH_EFF = TENSOR_WIDTH;
    TENSOR_HEIGHT_EFF = TENSOR_HEIGHT;
  }

  // Destroy the old preview window if it exists.
  if (!preview) {
    SDL_FreeSurface(preview);
  }

  // Make a new preview window.
  w = (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER_SIZE * 2);
  h = (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER_SIZE * 2);
  preview = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE);
  if (preview == NULL) {
    fprintf(stderr, "Unable to set up preview window: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Fill it with the background color.
  DrawBox(0, 0, w, h, PREVIEW_BACKGROUND_COLOR);

  // Update the preview window to show what we drew.
  SDL_UpdateRect(preview, 0, 0, w , h);

  // Destroy the old temp surface if it exists.
  if (!ttemp) {
    SDL_FreeSurface(ttemp);
  }

  // Make a new temp surface.
  ttemp = SDL_CreateRGBSurface(SDL_SWSURFACE, TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF, 32, 0,0,0,0);
  if (ttemp == NULL) {
    fprintf(stderr, "Unable to allocate a temp buffer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}


void DrawBox(int x, int y, int w, int h, color_t color) {
  boxRGBA(preview, x, y, x + (w - 1), y + (h - 1),
                (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}

void DrawRectangle(int x, int y, int w, int h, color_t color) {
  rectangleRGBA(preview, x, y, x + (w - 1), y + (h - 1),
                (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}



// Copies parms and modes of current moment into the gui.
void MomentToGui(moment_t *moment) {
  modes_t *modes = &moment->mode;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gui_pointers.randomdotsMode), modes->randDots == YES ? TRUE : FALSE);
}



// Send out the frame buffer to tensor and/or the display window.
// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...

void Update(float intensity_limit, moment_t *moment) {
  //modes_t *mode = &moment->mode;
  //parms_t *parms = &moment->coefs;
  unsigned char *buffer = moment->fb;
  unsigned char fba[TENSOR_BYTES];
  int i;

  // Output rotate.
  //if (mode->rotozoom == YES) {
  //  parms->rotation = parms->rotation + parms->rotationDelta;
  //  Rotate(parms->rotation, parms->expand, mode->alias, &fba[0], &buffer[0]);

  //  UpdatePreview(fba);

    // Output diminish - no reason to do this to the preview.
  //  for (i = 0; i < TENSOR_BYTES_EFF; i++) {
  //    fba[i] = (unsigned char)((float) fba[i] * intensity_limit);
  //  }

  //} else {
    UpdatePreview(buffer);
    // Output diminish
   // for (i = 0; i < TENSOR_BYTES; i++) {
   //   fba[i] = (unsigned char)((float) buffer[i] * intensity_limit);
   // }
  //}

  #ifdef USE_TENSOR
    UpdateTensor(fba);
  #endif
  //usleep(50000);
  usleep(25000);
  return;
}

void UpdateTensor(unsigned char *buffer) {
  tensor_send(buffer);
}
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);



// Frame buffer to surface
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB) {
  color_t pixel;
  int x, y;

  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {

      // Get pixel color.
      pixel = GetPixel(x, y, FB);

      // Write it to the output surface as a 10x10 square. (PSIZE_X x PSIZE_Y).
      pixelRGBA(surface, x, y, pixel.r, pixel.g, pixel.b, 255);
    }
  }

  return surface;
}

// Send frame buffer to display window
void UpdatePreview(unsigned char *buffer) {
  int i,w,h, w2, h2;
  SDL_Rect offset;
  SDL_Surface *tttemp;
  color_t lineColor = PREVIEW_BACKGROUND_COLOR;

  FBToSurface(ttemp, &buffer[0]);
  tttemp = rotozoomSurface (ttemp, 0, PREVIEW_PIXEL_SIZE, 0);

  offset.x = 0 - (tttemp->w - preview->w) / 2;
  offset.y = 0 - (tttemp->h - preview->h) / 2;
  SDL_BlitSurface(tttemp, NULL, preview, &offset);

  SDL_FreeSurface(tttemp);

  for (i = 1; i < (TENSOR_WIDTH_EFF + 1); i++) {
    lineRGBA(preview, (i * PREVIEW_PIXEL_SIZE) + PREVIEW_BORDER_SIZE, PREVIEW_BORDER_SIZE,
                      (i * PREVIEW_PIXEL_SIZE) + PREVIEW_BORDER_SIZE,
                      (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_SIZE) + PREVIEW_BORDER_SIZE,
                          lineColor.r, lineColor.g, lineColor.b, 255);
  }

  for (i = 1; i < (TENSOR_HEIGHT_EFF + 1); i++) {
    lineRGBA(preview, PREVIEW_BORDER_SIZE, (i * PREVIEW_PIXEL_SIZE) + PREVIEW_BORDER_SIZE,
                      (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_SIZE) + PREVIEW_BORDER_SIZE,
                      (i * PREVIEW_PIXEL_SIZE) + PREVIEW_BORDER_SIZE,
                          lineColor.r, lineColor.g, lineColor.b, 255);
  }

  // Draw the panel indicators.
   w2 = (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_SIZE) + 2;
   h2 = (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_SIZE) + 2;
   if (tensor_landscape_p) {
     // Landscape - Horizontal indicators.
     DrawRectangle(PREVIEW_BORDER_SIZE - 1, 0 * (h2 / 3) + PREVIEW_BORDER_SIZE,
                   w2, (h2 / 3) + 1, PREVIEW_PANEL_COLOR);
     DrawRectangle(PREVIEW_BORDER_SIZE - 1, 1 * (h2 / 3) + PREVIEW_BORDER_SIZE,
                   w2, (h2 / 3) + 1, PREVIEW_PANEL_COLOR);
     DrawRectangle(PREVIEW_BORDER_SIZE - 1, 2 * (h2 / 3) + PREVIEW_BORDER_SIZE,
                   w2, (h2 / 3) + 1, PREVIEW_PANEL_COLOR);
   } else {
     // Portrait - Vertical indicators
     DrawRectangle(0 * (w2 / 3) + PREVIEW_BORDER_SIZE, PREVIEW_BORDER_SIZE - 1,
                   (w2 / 3) + 1, h2, PREVIEW_PANEL_COLOR);
     DrawRectangle(1 * (w2 / 3) + PREVIEW_BORDER_SIZE, PREVIEW_BORDER_SIZE - 1,
                   (w2 / 3) + 1, h2, PREVIEW_PANEL_COLOR);
     DrawRectangle(2 * (w2 / 3) + PREVIEW_BORDER_SIZE, PREVIEW_BORDER_SIZE - 1,
                   (w2 / 3) + 1, h2, PREVIEW_PANEL_COLOR);
   }

  // Tell the surface to update.
  w = (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER_SIZE * 2);
  h = (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER_SIZE * 2);
  SDL_UpdateRect(preview, 0, 0, w, h);
}




int frames, fps;

// Program loop...
void *MainLoop(void *data) {
  SDL_Event event;
  unsigned char doprocess = NO;

  // Init the processing timer.
    if (SDL_AddTimer(55, TriggerProcessing, NULL) == NULL) {
      printf("Process Timer problem!\n");
      exit(0);
    }

  if (SDL_AddTimer(1000, FrameCounter, NULL) == NULL) {
     printf("Frame Count Timer problem!\n");
     exit(0);
  }

  FOREVER {
    // Act on queued events.
       while (SDL_PollEvent(&event)) {
         switch(event.type) {
           case SDL_USEREVENT:
             switch (event.user.code) {
               case 0:
                 doprocess = YES;
                 break;
               case 1:
                 fps = frames;
                 printf("FPS: %i\n", fps);
                 frames = 0;
                 break;
               default:
                 break;
             }
             break;

           default:
             break;
         }
       }

    if (doprocess == YES) {
          doprocess = NO;
          frames++;
          // Update the buffer.
           PatternEngine();

           // Update the preview and tensor.
           Update(global_intensity_limit, currentMoment);
    }



  } // End FOREVER
}  // End program loop

Uint32 TriggerProcessing(Uint32 interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);

  return(interval);
}


Uint32 FrameCounter(Uint32 interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 1;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);

  return(interval);
}




G_MODULE_EXPORT void quit (GtkObject *object, gpointer user_data) {
  gtk_main_quit();
}

G_MODULE_EXPORT gboolean on_randomDotCheck_toggled(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    currentMode->randDots = YES;
  } else {
    currentMode->randDots = NO;
  }
  return FALSE;
}

