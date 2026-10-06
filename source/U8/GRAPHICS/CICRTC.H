#ifndef CICRTC_H
#define CICRTC_H

void InitializeCrtc(void);

extern unsigned CrtcStatusPort;
extern unsigned CrtcAddressRegister;
extern unsigned CrtcDataRegister;
extern unsigned BiosSegment;

#endif
