//
//  GCPointer.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Config.h>

template < class T >
class GCPointer
{
public:
    
    typedef std::shared_ptr< T > ptr;
    typedef std::weak_ptr< T >   w_ptr;
    typedef std::unique_ptr< T > u_ptr;
    
    ptr shared() const
    {
        return ptr((T*)this);
    }
    w_ptr weak() const
    {
        return w_ptr((T*)this);
    }
    u_ptr unique() const
    {
        return u_ptr((T*)this);
    }
    
    template< typename...Args >
    static ptr shared_new( Args && ... args)
    {
        return ptr(new T(args...));
    }
    
};