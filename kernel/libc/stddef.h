#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void*)0)

typedef unsigned int size_t;
typedef int ptrdiff_t;

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif