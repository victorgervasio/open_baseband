/**
 * OSAL behavior definition file.
 */

#include "OSAL.hh"

OPERATION(CHECK_NODE)
TRIGGER

float msg[19];
float out[19];

float min1 = 0.0f;
float min2 = 0.0f;
float parity = 1.0f;

unsigned min1Idx = 0;

/*
 * Read the 19 input operands.
 */
for (unsigned j = 0; j < 19; ++j)
{
    msg[j] = FLT(1 + j);
}

/*
 * Find the minimum and second minimum absolute values.
 */
min1 = 3.402823466e+38F;
min2 = 3.402823466e+38F;

for (unsigned j = 0; j < 19; ++j)
{
    float absMsg = (msg[j] >= 0.0f) ? msg[j] : -msg[j];

    if (absMsg < min1)
    {
        min2 = min1;
        min1 = absMsg;
        min1Idx = j;
    }
    else if (absMsg < min2)
    {
        min2 = absMsg;
    }
}

/*
 * Offset.
 */
min1 = (min1 > 0.5f) ? min1 - 0.5f : 0.0f;
min2 = (min2 > 0.5f) ? min2 - 0.5f : 0.0f;

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < 19; ++j)
{
    float sign = (msg[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

/*
 * Generate the 19 output messages.
 */
for (unsigned j = 0; j < 19; ++j)
{
    float sign = (msg[j] >= 0.0f) ? 1.0f : -1.0f;
    float magnitude = (j == min1Idx) ? min2 : min1;

    out[j] = magnitude * parity * sign;
}

/*
 * Write the 19 output operands.
 */
for (unsigned j = 0; j < 19; ++j)
{
    IO(20 + j) = out[j];
}

return true;

//for (unsigned j = 0; j < 19; ++j)
//{
//    IO(20 + j) = 1.5f;
//}
//
//return true;

END_TRIGGER;
END_OPERATION(CHECK_NODE)
