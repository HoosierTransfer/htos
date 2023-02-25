/*
 *  conio.h     Console and Port I/O functions
 *
 * =========================================================================
 *
 *                          Open Watcom Project
 *
 * Copyright (c) 2004-2021 The Open Watcom Contributors. All Rights Reserved.
 * Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
 *
 *    This file is automatically generated. Do not edit directly.
 *
 * =========================================================================
 */
#ifndef _CONIO_H_INCLUDED
#define _CONIO_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ___VA_LIST_DEFINED
#define ___VA_LIST_DEFINED
 #ifdef __PPC__
   typedef struct {
       char  __gpr;
       char  __fpr;
       char  __reserved[2];
       char  *__input_arg_area;
       char  *__reg_save_area;
   } __va_list;
 #elif defined(__AXP__)
  typedef struct {
      char  *__base;
      int   __offset;
  } __va_list;
 #elif defined(__MIPS__)
  typedef struct {
      char  *__base;
      int   __offset;
  } __va_list;
 #elif defined(_M_IX86)
  #if defined(__SW_ZU)
   typedef char _WCFAR *__va_list[1];
  #else
   typedef char    *__va_list[1];
  #endif
 #endif
#endif

_WCRTLINK extern char *cgets(char *__buf);
_WCRTLINK extern int cputs(const char *__buf);
_WCRTLINK extern int cprintf(const char *__fmt,...);
_WCRTLINK extern int cscanf(const char *__fmt,...);
_WCRTLINK extern int getch(void);
_WCRTLINK extern int _getch(void);
_WCRTLINK extern int getche(void);
_WCRTLINK extern int _getche(void);
_WCRTLINK extern int kbhit(void);
_WCRTLINK extern int _kbhit(void);
#ifdef _M_IX86
 _WCIRTLINK extern unsigned inp(unsigned __port);
 _WCIRTLINK extern unsigned inpw(unsigned __port);
 _WCIRTLINK extern unsigned outp(unsigned __port, unsigned __value);
 _WCIRTLINK extern unsigned outpw(unsigned __port,unsigned __value);
  _WCIRTLINK extern unsigned inpd(unsigned __port);
  _WCIRTLINK extern unsigned outpd(unsigned __port, unsigned __value);
#endif
_WCRTLINK extern int putch(int __c);
_WCRTLINK extern int ungetch(int __c);
_WCRTLINK extern int vcprintf( const char *__format, __va_list __arg );
_WCRTLINK extern int vcscanf( const char *__format, __va_list __arg );

#if defined(__INLINE_FUNCTIONS__) && defined(_M_IX86)
 #pragma intrinsic(inp,inpw,outp,outpw)
  #pragma intrinsic(inpd,outpd)
#endif

#ifdef _M_IX86
 #define _inp   inp
 #define _inpw  inpw
 #define _inpd  inpd
 #define _outp  outp
 #define _outpw outpw
 #define _outpd outpd
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
