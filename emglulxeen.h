// Main header for Glulxe

#ifndef EMGLULXEEN_H
#define EMGLULXEEN_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "glk.h"
#include "emglken.h"
#include "glulxe.h"

extern void emautorestore ( glui32 *ramStreamTag, glui32 *iosysmode, glui32 *iosysrock, glui32 *protectend, glui32 *protectstart, glui32 *stringtable );
extern void emautosave ( glui32 jsonStreamTag, glui32 ramStreamTag );
extern void emglulxeen ( glui32 gameStreamTag, glui32 profileStreamTag, glui32 profcalls );

#endif // EMGLULXEEN_H