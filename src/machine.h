// machine.h
// Joshua Krueger
// 2016_10_12
// Contains the 

#ifndef MACHINE_H_
  #define MACHINE_H_

  // Include
  #include "useful.h"
  #include "drawSupport.h"
  
  // Ploop
  #define LIVE 1
  #define ALT 0
  
  // Public? Prototypes
  bool_t InitMachine(int fbSize, point_t liveLoc, point_t altLoc, int *liveSetPtr, int *altSetPtr);
  void ProcessNextStep(int set, bool_t isPrimary);  
  void ProcessModes(int set);
  void PostProcessModes(int set, bool_t isPrimary);
  void UpdatePreview(bool_t isPrimary);
  void UpdateTensor(void);
  point_t GetAltLoc(void );
  point_t GetLiveLoc(void);
  int GetLiveSet(void);
  int GetAltSet(void);
  void SetLiveSet(int set);
  void SetAltSet(int set);
  void SetSets(int lSet, int aSet);
  void SetMachineWH(int w, int h);
  int *GetLiveRefresh(void);
  int *GetAltRefresh(void);
  
#endif /* MACHINE_H_ */
