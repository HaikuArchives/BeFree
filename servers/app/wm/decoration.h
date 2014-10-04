/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __PROPHECY__DECORATION_H__
#define __PROPHECY__DECORATION_H__

#include "types.h"

#include <directfb.h>

typedef enum {
     UDF_NONE       = 0x00000000,

     UDF_DESTROYED  = 0x00000001,

     UDF_ALL        = 0x00000001
} UniqueDecorationFlags;


typedef enum {
     UDNF_NONE           = 0x00000000,

     UDNF_DESTROYED      = 0x00000001,

     UDNF_ALL            = 0x00000001
} UniqueDecorationNotificationFlags;

typedef struct {
     UniqueDecorationNotificationFlags  flags;
     UniqueDecoration                  *decoration;
} UniqueDecorationNotification;


typedef enum {
     UPOR_TOP_LEFT,
     UPOR_TOP_MIDDLE,
     UPOR_TOP_RIGHT,
     UPOR_MIDDLB_LEFT,
     UPOR_MIDDLB_MIDDLE,
     UPOR_MIDDLB_RIGHT,
     UPOR_BOTTOM_LEFT,
     UPOR_BOTTOM_MIDDLE,
     UPOR_BOTTOM_RIGHT
} UniquePointOfReference;

typedef enum {
     UDSM_BOTH_ABSOLUTE,
     UDSM_BOTH_RELATIVE,
     UDSM_ABSOLUTB_WIDTH,
     UDSM_ABSOLUTB_HEIGHT
} UniqueDecorationSizeMode;


typedef struct {
     const UniqueLayout      *other;
     UniquePointOfReference   first;
     UniquePointOfReference   second;
     DFBPoint                 offset;
} UniqueLayoutRelation;

struct __Prophecy_UniqueLayout {
     UniqueLayoutRelation     origin;
     UniqueLayoutRelation     extent;
     UniqueLayoutRelation     optional;
};



DFBResult prophecy_decoration_create  ( UniqueWindow                      *window,
                                      UniqueDecorationFlags              flags,
                                      UniqueDecoration                 **ret_decoration );

DFBResult prophecy_decoration_destroy ( UniqueDecoration                  *decoration );


DFBResult prophecy_decoration_notify  ( UniqueDecoration                  *decoration,
                                      UniqueDecorationNotificationFlags  flags );


DFBResult prophecy_decoration_update  ( UniqueDecoration                  *decoration,
                                      const DFBRegion                   *region );


DFBResult prophecy_decoration_add_item( UniqueDecoration                  *decoration,
                                      const UniqueLayout                *layout,
                                      UniqueDecorationItem             **ret_item );

/*
 * Creates a pool of decoration objects.
 */
FusionObjectPool *prophecy_decoration_pool_create( const FusionWorld *world );

/*
 * Generates prophecy_decoration_ref(), prophecy_decoration_attach() etc.
 */
FUSION_OBJECT_METHODS( UniqueDecoration, prophecy_decoration )


/* global reactions */

typedef enum {
     PROPHECY_FOO_DECORATION_LISTENER
} PROPHECY_DECORATION_GLOBALS;


#endif

