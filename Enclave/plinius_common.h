

/*
 * Created on Fri Nov 26 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 */

#ifndef PLINIUS_COMMON_H
#define PLINIUS_COMMON_H

#define _PLINIUS_CODELINE_ __FILE__, __LINE__, __FUNCTION__ ///< for debugging

#define PLINIUS_DEBUG_INFO()                               \
    do                                                     \
    {                                                      \
        printf("Plinius: %s:%s:%s\n", _PLINIUS_CODELINE_); \
    } while (0)

#define PLINIUS_INFO(x) \
    do                  \
    {                   \
        printf x;      \
    } while (0)
#endif /* PLINIUS_COMMON_H */
