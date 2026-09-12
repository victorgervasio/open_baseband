/**
 * OSAL behavior definition file.
 */

#include "OSAL.hh"

//OPERATION(CHECK_NODE)
//TRIGGER
//
//float msg[19];
//float out[19];
//
//float min1 = 0.0f;
//float min2 = 0.0f;
//float parity = 1.0f;
//
//unsigned min1Idx = 0;
//
///*
// * Read the 19 input operands.
// */
//for (unsigned j = 0; j < 19; ++j)
//{
//    msg[j] = FLT(1 + j);
//}
//
///*
// * Find the minimum and second minimum absolute values.
// */
//min1 = 3.402823466e+38F;
//min2 = 3.402823466e+38F;
//
//for (unsigned j = 0; j < 19; ++j)
//{
//    float absMsg = (msg[j] >= 0.0f) ? msg[j] : -msg[j];
//
//    if (absMsg < min1)
//    {
//        min2 = min1;
//        min1 = absMsg;
//        min1Idx = j;
//    }
//    else if (absMsg < min2)
//    {
//        min2 = absMsg;
//    }
//}
//
///*
// * Offset.
// */
//min1 = (min1 > 0.5f) ? min1 - 0.5f : 0.0f;
//min2 = (min2 > 0.5f) ? min2 - 0.5f : 0.0f;
//
///*
// * Calculate sign parity.
// */
//parity = 1.0f;
//
//for (unsigned j = 0; j < 19; ++j)
//{
//    float sign = (msg[j] >= 0.0f) ? 1.0f : -1.0f;
//    parity *= sign;
//}
//
///*
// * Generate the 19 output messages.
// */
//for (unsigned j = 0; j < 19; ++j)
//{
//    float sign = (msg[j] >= 0.0f) ? 1.0f : -1.0f;
//    float magnitude = (j == min1Idx) ? min2 : min1;
//
//    out[j] = magnitude * parity * sign;
//}
//
///*
// * Write the 19 output operands.
// */
//for (unsigned j = 0; j < 19; ++j)
//{
//    IO(20 + j) = out[j];
//}
//
//return true;
//
////for (unsigned j = 0; j < 19; ++j)
////{
////    IO(20 + j) = 1.5f;
////}
////
////return true;
//
//END_TRIGGER;
//END_OPERATION(CHECK_NODE)

OPERATION(CN_CORE_3)
TRIGGER
unsigned DEGREE = 3;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_3)

OPERATION(CN_CORE_4)
TRIGGER
unsigned DEGREE = 4;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_4)

OPERATION(CN_CORE_5)
TRIGGER
unsigned DEGREE = 5;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_5)

OPERATION(CN_CORE_6)
TRIGGER
unsigned DEGREE = 6;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_6)

OPERATION(CN_CORE_7)
TRIGGER
unsigned DEGREE = 7;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_7)

OPERATION(CN_CORE_8)
TRIGGER
unsigned DEGREE = 8;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_8)

OPERATION(CN_CORE_9)
TRIGGER
unsigned DEGREE = 9;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_9)

OPERATION(CN_CORE_10)
TRIGGER
unsigned DEGREE = 10;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_10)

OPERATION(CN_CORE_19)
TRIGGER
unsigned DEGREE = 19;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min;
IO(DEGREE + 1) = parity;

return true;

END_TRIGGER;
END_OPERATION(CN_CORE_19)

OPERATION(CN_PROC_4)
TRIGGER
unsigned DEGREE = 4;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min * parity;

return true;

END_TRIGGER;
END_OPERATION(CN_PROC_4)

OPERATION(CN_PROC_5)
TRIGGER
unsigned DEGREE = 5;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min * parity;

return true;

END_TRIGGER;
END_OPERATION(CN_PROC_5)

OPERATION(CN_PROC_6)
TRIGGER
unsigned DEGREE = 6;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min * parity;

return true;

END_TRIGGER;
END_OPERATION(CN_PROC_6)

OPERATION(CN_PROC_7)
TRIGGER
unsigned DEGREE = 7;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min * parity;

return true;

END_TRIGGER;
END_OPERATION(CN_PROC_7)

OPERATION(CN_PROC_19)
TRIGGER
unsigned DEGREE = 19;

float in_llr[DEGREE - 1];

float min = 3.402823466e+38F;
float parity = 1.0f;

/*
 * Read the (DEGREE - 1) input operands.
 */
for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    in_llr[j] = FLT(1 + j);
}


for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float absMsg = (in_llr[j] >= 0.0f) ? in_llr[j] : -in_llr[j];
    if (absMsg < min)
    {
        min = absMsg;
    }
}

/*
 * Calculate sign parity.
 */
parity = 1.0f;

for (unsigned j = 0; j < (DEGREE - 1); ++j)
{
    float sign = (in_llr[j] >= 0.0f) ? 1.0f : -1.0f;
    parity *= sign;
}

IO(DEGREE) = min * parity;

return true;

END_TRIGGER;
END_OPERATION(CN_PROC_19)
