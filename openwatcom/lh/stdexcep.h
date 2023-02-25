///////////////////////////////////////////////////////////////////////////
// FILE: stdexcept.h/stdexcept (Standard exception classes)
//
// =========================================================================
//
//                          Open Watcom Project
//
// Copyright (c) 2004-2021 The Open Watcom Contributors. All Rights Reserved.
// Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
//
//    This file is automatically generated. Do not edit directly.
//
// =========================================================================
//
// Description: This header is part of the C++ standard library. It
//              defines various exception classes based on exception.
///////////////////////////////////////////////////////////////////////////
#ifndef _STDEXCEPT_H_INCLUDED
#define _STDEXCEPT_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef _STDEXCEPT_INCLUDED
 #include <stdexcept>
#endif
using std::logic_error;
using std::domain_error;
using std::invalid_argument;
using std::length_error;
using std::out_of_range;
using std::runtime_error;
using std::range_error;
using std::overflow_error;
using std::underflow_error;

// All included names should also be in the global namespace.
#ifndef _EXCEPTION_H_INCLUDED
 #include <exception.h>
#endif

#endif
