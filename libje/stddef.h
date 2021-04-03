#if !defined(_STDDEF_H)

typedef long ptrdiff_t;

typedef unsigned long size_t;

typedef int wchar_t;
typedef unsigned int wint_t;


#define NULL ((void *)0)

#define offsetof(type, member) __builtin_offsetof (type, member)

#endif /* _STDDEF_H */

