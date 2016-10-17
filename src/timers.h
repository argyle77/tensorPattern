// timers.h
// Joshua Krueger
// Timing for the machine?

#ifndef TIMERS_H_
  #define TIMERS_H_

  #define ERROR_SDLREGISTER ((Uint32) - 1)
    
  // Global shared
  extern Uint32 User_Timer;
  
  // Make sure timers_e and timerTable correspond in order.
  typedef enum timers_e {
    TI_FPS = 0, TI_GUI, TI_LIVE, TI_ALT, /* TI_BROADCAST, */
    TI_COUNT // Last
  } timers_e;
  
  // Make sure clocks_e and fpsClocks[] correspond.
  typedef enum clocks_e {
    CL_LIVE, CL_ALT, CL_GUI, CL_SLEEP, CL_MAIN,
    CL_COUNT // Last
  } clocks_e;
  
  // Public Prototypes
  bool_t InitTimers(void);
  void ResetFPSCounters(void);
  void ClockTick(clocks_e clock);
  int GetFPSClock(clocks_e clock);
  
#endif /* TIMERS_H_ */
