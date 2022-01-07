#ifndef __ONEGUI_MEMORY__
#define __ONEGUI_MEMORY__

unsigned int memorySize();
unsigned int memoryGrow(unsigned int amount);
unsigned int memoryCopy(unsigned int to, unsigned int from, unsigned int size);
unsigned int memoryFill(unsigned int at, unsigned int amount);

#endif