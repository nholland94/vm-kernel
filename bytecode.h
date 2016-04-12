#include <stddef.h>

typedef opcode uint16_t

#define OP_CALL  0x0001
#define OP_RET   0X0002
#define OP_JMP   0x0003
#define OP_LPUSH 0x0010
#define OP_LCOPY 0x0011
#define OP_LADD  0x0012
#define OP_LSUB  0x0013
#define OP_LJE   0x0014
#define OP_LJNE  0x0015
#define OP_LJL   0x0016
#define OP_LJG   0x0017
#define OP_LJLE  0x0018
#define OP_LJGE  0x0019
#define OP_RPUSH 0x0020
#define OP_RCOPY 0x0021
#define OP_RADD  0x0022
#define OP_RSUB  0x0023
#define OP_RJE   0x0024
#define OP_RJNE  0x0025
#define OP_RJL   0x0026
#define OP_RJG   0x0027
#define OP_RJLE  0x0028
#define OP_RJGE  0x0029
