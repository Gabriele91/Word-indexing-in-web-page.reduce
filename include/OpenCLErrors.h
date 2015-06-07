//
//  OpenCLErrors.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 17/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Config.h>
#include <OpenCLInfo.h>

//OpenCL Assert
#define ASSERTCL( error )                              \
if( error != CL_SUCCESS )                              \
{                                                      \
    MESSAGE( OpenCLInfo::get_error_string(error) );    \
    DEBUGCODE( assert( error == CL_SUCCESS ); )        \
}
#define ASSERTCL_MSG( error, msg )                     \
if( error != CL_SUCCESS )                              \
{                                                      \
    MESSAGE( msg );                                    \
    MESSAGE( OpenCLInfo::get_error_string(error) );    \
    DEBUGCODE( assert( error == CL_SUCCESS ); )        \
}