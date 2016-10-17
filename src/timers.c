// timers.c
// Joshua Krueger

// Includes
#include <SDL.h> // Uint32, SDL_RegisterEvents, SDL_AddTimer, etc
#include "useful.h"
#include "timers.h"
#include "machine.h"
#include "log.h"

// Defines
#define GUIREFRESH_MS 25
#define FPSREFRESH_MS 1000

// Structure
typedef struct fpsClock_t {
  const char *name;
  int counter, fps;
} fpsClock_t;

// Internal prototypes
Uint32 TimerExpired(Uint32 interval, void *param);
int GetFPS(void);
int GetGui(void);
int GetLive(void);
int GetAlt(void);

// Global shared
Uint32 User_Timer;

// Global private
// Make sure timers_e and timerTable correspond in order.
int (*timerTable[])(void) = {GetFPS, GetGui, GetLive, GetAlt};
fpsClock_t fpsClocks[] = {
  { "Live", 0, 0 }, // Preview A FPS
  { "Alt", 0, 0 }, // Preview B FPS
  { "Gui", 0, 0 }, // Gui refresh
  { "Sleep", 0, 0 }, // Sleeps
  { "Main", 0, 0 }  // Main loops
};
const int clockCount = sizeof(fpsClocks) / sizeof(fpsClock_t);
 
// Getters
int GetFPS(void) { return FPSREFRESH_MS; }
int GetGui(void) { return GUIREFRESH_MS; }
int GetLive(void) { return *GetLiveRefresh(); }
int GetAlt(void) { return *GetAltRefresh(); }

bool_t InitTimers(void) {
  
  // Initialize the user events
  User_Timer = SDL_RegisterEvents(1);
  if (User_Timer == ERROR_SDLREGISTER) {
    fprintf(stderr, "Couldn't allocate a user defined event for the timers!\n");
    return FAIL;
  }

  // GUI update timer. The gui is updated whenever the previews update, but
  // previews update at the (adjustable) frame update rate, which doesn't give
  // us predictable timing.  To ensure that the gui stays responsive, there is
  // a timer for this purpose.  Current settings (GUIREFRESH_MS = 25) places
  // this timer at 40Hz.
  if (!SDL_AddTimer((*timerTable[TI_GUI])(), TimerExpired, (void *)TI_GUI)) {
    fprintf(stderr, "Can't initialize the gui update timer! %s\n", SDL_GetError());
    return FAIL;
  }
  
  // FPS counter (once per second).
  if (!SDL_AddTimer((*timerTable[TI_FPS])(), TimerExpired, (void *)TI_FPS)) {
    fprintf(stderr, "Can't initialize the frame counter timer! %s\n", SDL_GetError());
    return FAIL;
  }

  // Live machine timer.
  if (!SDL_AddTimer((*timerTable[TI_LIVE])(), TimerExpired, (void *) TI_LIVE)) {
    fprintf(stderr, "Can't initialize the live preview timer! %s\n", SDL_GetError());
    return FAIL;
  }

  // Alternate machine timer.
  if (!SDL_AddTimer((*timerTable[TI_ALT])(), TimerExpired, (void *) TI_ALT)) {
    fprintf(stderr, "Can't initialize the alternate preview timer! %s\n", SDL_GetError());
    return FAIL;
  }

  return PASS;  
}

// Handle timer expiration.  This is run on a separate thread, so we don't want
// to do much of anything here.
Uint32 TimerExpired (Uint32 interval, void *param) {
  
  // Identify which timer expired via param.
  timers_e timer = (timers_e) param;
  
  // We're gonna throw an SDL event and handle the consequences in the mainline.
  SDL_Event event;
  SDL_zero(event);
  event.type = User_Timer;
  event.user.code = (Sint32) timer;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);
  
  // Returning the interval starts the timer again for the same period.
  return((*timerTable[timer])());
}

// One of our timers is a second clock.  This should be called every second
// when it fires.
void ResetFPSCounters(void) {
  int i;
  
  // The fps timer expired.  Set fps and reset the frame count.
  for (i = 0; i < clockCount; i++) {
    fpsClocks[i].fps = fpsClocks[i].counter;
    fpsClocks[i].counter = 0;
    if (i != clockCount - 1) {
      FLog("%s: %i FPS, ", fpsClocks[i].name, fpsClocks[i].fps);
    } else {
      Log("%s: %i FPS", fpsClocks[i].name, fpsClocks[i].fps);
    }
  }
}

void ClockTick(clocks_e clock) {
  if (clock < 0 || clock >= CL_COUNT) return;
  fpsClocks[clock].counter++;
}

int GetFPSClock(clocks_e clock) {
  if (clock < 0 || clock >= CL_COUNT) return INVALID;
  return fpsClocks[clock].fps;
}
