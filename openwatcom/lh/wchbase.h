//
//  wchbase.h    Definitions for the base classes used by
//               the WATCOM Container Hash Classes
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
#ifndef _WCHBASE_H_INCLUDED
#define _WCHBASE_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef __cplusplus
 #error This header file requires C++
#endif

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifndef _WCDEFS_H_INCLUDED
 #include <wcdefs.h>
#endif

#if defined( new ) && defined( _WNEW_OPERATOR )
 #undef new
#endif
#if defined( delete ) && defined( _WDELETE_OPERATOR )
 #undef delete
#endif

//
// The default number of buckets in a hash table.  If a second parameter is
// passed to the hash table constructor, this value will not be used
//

const unsigned WC_DEFAULT_HASH_SIZE = 101;

//
// Hash implementation object:
// A singly linked list element for storing the values in the hash table
//

template <class Type>
class WCHashLink : public WCSLink {
public:
    Type data;

    // used by WCHashNew to be able to call a constructor on user allocator
    // allocated memory
    inline void * operator new( size_t, void * ptr ){
        return( ptr );
    }
    inline WCHashLink( const Type & datum ) : data( datum ) {};
    inline ~WCHashLink() {};
};



//
// Hash Dictionary implementation object:
// Combines the Key and Value into one object
//

template <class Key, class Value>
class WCHashDictKeyVal{
public:
    Key key;
    Value value;

    inline WCHashDictKeyVal( const WCHashDictKeyVal &orig )
            : key( orig.key ), value( orig.value ) {};
    inline WCHashDictKeyVal( const Key &new_key, const Value &new_value )
            : key( new_key ), value( new_value ) {};
    inline WCHashDictKeyVal( const Key &new_key ) : key( new_key ) {};
    inline WCHashDictKeyVal() {};
    inline ~WCHashDictKeyVal() {};

    // this operator is NEVER used, but necessary for compilation
    // (needed by WCValHashSet, inherited by WCValHashDict)
    inline int operator==( const WCHashDictKeyVal & ) const {
        return( 0 );
    };
};




//
// Provide base functionality for WATCOM container hash tables, sets and
// dictionaries.
//
// WCExcept provides exception handling.
//
// This class is an abstract class so that objects of this type cannot be
// created.
//


class WCHashBase : public WCExcept {
protected:
    WCIsvSList<WCSLink> *hash_array;
    unsigned num_buckets;
    unsigned num_entries;

    // link base non-templated class
    typedef WCSLink BaseHashLink;
    // pointer to element of templated type
    typedef const void *TTypePtr;

    enum find_type {            // enumerations for base_find
        FIND_FIRST,             // find first matching element
        NEXT_MULT_FIND };       // find next matching element in bucket

    // copy constructor base
    _WPRTLINK void base_construct( const WCHashBase * orig );

    // for copy constructor
    virtual BaseHashLink *base_copy_link( const BaseHashLink *orig ) = 0;

    virtual int base_equivalent( BaseHashLink *elem1
                               , TTypePtr elem2 ) const = 0;

    virtual unsigned base_get_bucket( TTypePtr elem ) const = 0;

    // for the resize member function
    virtual unsigned base_get_bucket_for_link( BaseHashLink *link ) const = 0;

    _WPRTLINK BaseHashLink *base_find( TTypePtr elem, unsigned *bucket
                           , unsigned *index, find_type type ) const;

    // common initialization code for the constructors
    _WPRTLINK void base_init( unsigned buckets );

    _WPRTLINK BaseHashLink *base_remove_but_not_delete( TTypePtr elem );

    // the insert function for WCValHashSet and WCPtrHashSet
    _WPRTLINK BaseHashLink *base_set_insert( TTypePtr elem );

    virtual BaseHashLink * WCHashNew( TTypePtr elem ) = 0;

    virtual void WCHashDelete( BaseHashLink *old_link ) = 0;


    inline WCHashBase() : hash_array( 0 ), num_buckets( 0 )
                        , num_entries( 0 ) {};

    inline WCHashBase( unsigned buckets ) {
        base_init( buckets );
    };

    virtual ~WCHashBase() = 0;

    _WPRTLINK void clear ();

    _WPRTLINK WCbool insert( TTypePtr elem );

    _WPRTLINK unsigned occurrencesOf( TTypePtr elem ) const;

    _WPRTLINK unsigned removeAll( TTypePtr elem );

    _WPRTLINK void resize( unsigned buckets );

public:
    _WPRTLINK static unsigned bitHash( const void * data, size_t size );

    friend class WCHashIterBase;
};

#ifdef _WNEW_OPERATOR
 #define new _WNEW_OPERATOR
#endif
#ifdef _WDELETE_OPERATOR
 #define delete _WDELETE_OPERATOR
#endif

#endif
