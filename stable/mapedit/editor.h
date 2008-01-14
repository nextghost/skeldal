#ifndef ___EDITOR__H___
#define ___EDITOR__H___

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_FORCESAVE (WM_APP+1)
#define MSG_FINDANDPOPUP (WM_APP+2)
#define MSG_CLOSEEDITOR (WM_APP+3)

void EditSkeldalFile(const char *filename);





#ifdef __cplusplus
  }
#endif

#endif