#ifndef __FOLIC_TYPE__
#define __FOLIC_TYPE__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "utf8char.h"
#include "vector.h"
#include "string.h"

#ifndef u8first_byte
#define u8first_byte(x) (((uint8_t*)((x).data))[0])
#endif

#endif