// machine.h
// Joshua Krueger
// 2016_10_12
// Contains the 

#ifndef MACHINE_H_
  #define MACHINE_H_

  // Include
  #include "useful.h"
  #include "draw.h"
  
  // Prototypes
  void InitMachine(int fbSize, point_t live, point_t alt, int *l, int *a);
  void DrawNextFrame(int set, bool_t isPrimary);
  void ProcessModes(int set);
  void PostProcessModes(int set, bool_t isPrimary);
  void UpdateDisplay(bool_t isPrimary, bool_t sendToTensor, float intensity_limit);
  void UpdatePreview(point_t xyOffset, unsigned char *buffer);
  point_t GetAltLoc(void );
  point_t GetLiveLoc(void);
  int GetLiveSet(void);
  int GetAltSet(void);
  void SetLiveSet(int set);
  void SetAltSet(int set);
  void SetSets(int lSet, int aSet);
  void SetMachineWH(int w, int h);
  
#endif /* MACHINE_H_ */
