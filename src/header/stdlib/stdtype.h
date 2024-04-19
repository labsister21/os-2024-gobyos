#ifndef _STDTYPE
#define _STDTYPE

/**
 * Unsigned integer representing object size
 */
#ifdef external
typedef unsigned long int size_t;
#else
typedef unsigned int size_t;
#endif

/**
 * 32-bit unsigned integer
 */
typedef unsigned int uint32_t;

/**
 * 16-bit unsigned integer
 */
typedef unsigned short uint16_t;

/**
 * 8-bit unsigned integer
 */
typedef unsigned char uint8_t;

/**
 * 32-bit signed integer
 */
typedef signed int int32_t;

/**
 * 16-bit signed integer
 */
typedef signed short int16_t;

/**
 * 8-bit signed integer
 */
typedef signed char int8_t;

#endif