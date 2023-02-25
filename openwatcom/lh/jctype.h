/*
 *  jctype.h    Japanese character test macros
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
#ifndef _JCTYPE_H_INCLUDED
#define _JCTYPE_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifndef _CTYPE_H_INCLUDED
 #include <ctype.h>
#endif

/*
 *       iskana(c)       カナ・コード
 *       iskpun(c)       カナ句切文字
 *       iskmoji(c)      カナ文字
 *       isalkana(c)     英文字またはカナ文字
 *       ispnkana(c)     英句切文字またはカナ句切文字
 *       isalnmkana(c)   英数字またはカナ文字
 *       isprkana(c)     表示可能文字（空白を含む）
 *       isgrkana(c)     表示可能文字（空白を除く）
 *
 *       iskanji(c)      漢字第１バイト
 *       iskanji2(c)     漢字第２バイト
 *
 */

#define _K      0x01    /* Kana moji      */
#define _KP     0x02    /* Kana punct.    */
#define _J1     0x04    /* Kanji 1st byte */
#define _J2     0x08    /* Kanji 2nd byte */

#ifdef __cplusplus
extern "C" {
#endif

_WCRTLINK extern int    iskana(int);
_WCRTLINK extern int    iskpun(int);
_WCRTLINK extern int    iskmoji(int);
_WCRTLINK extern int    isalkana(int);
_WCRTLINK extern int    ispnkana(int);
_WCRTLINK extern int    isalnmkana(int);
_WCRTLINK extern int    isprkana(int);
_WCRTLINK extern int    isgrkana(int);
_WCRTLINK extern int    iskanji(int);
_WCRTLINK extern int    iskanji2(int);

/* kana, kanji type table */
#if defined(__SW_BR) || defined(_RTDLL)
 #define _IsKTable _IsKTable_br
#endif
_WCRTDATA extern const char _WCDATA _IsKTable[257];

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifndef __FUNCTION_DATA_ACCESS

#define iskana(__c)     (_IsKTable[(unsigned char)(__c)+1] & (_K|_KP))
#define iskpun(__c)     (_IsKTable[(unsigned char)(__c)+1] & _KP)
#define iskmoji(__c)    (_IsKTable[(unsigned char)(__c)+1] & _K)
#define isalkana(__c)   (isalpha(__c) || iskmoji(__c))
#define ispnkana(__c)   (ispunct(__c) || iskpun(__c))
#define isalnmkana(__c) (isalnum(__c) || iskmoji(__c))
#define isprkana(__c)   (isprint(__c) || iskana(__c))
#define isgrkana(__c)   (isgraph(__c) || iskana(__c))

#define iskanji(__c)    (_IsKTable[(unsigned char)(__c)+1] & _J1)
#define iskanji2(__c)   (_IsKTable[(unsigned char)(__c)+1] & _J2)

#endif /* __FUNCTION_DATA_ACCESS not defined */

#endif
