#ifndef __CPS_LZO_H__
#define __CPS_LZO_H__

#include "../../compressedfs.h"
#include "minilzo.h"

#define LZO_NAME "LZO"

void minilzo_init(struct compression_operations* opt);

#endif
