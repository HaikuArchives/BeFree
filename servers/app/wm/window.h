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

#ifndef __PROPHECY__WINDOW_H__
#define __PROPHECY__WINDOW_H__

#include <core/coretypes.h>
#include <core/wm.h>

#include "types.h"


typedef enum {
     UWF_NONE       = 0x00000000,

     UWF_VISIBLE    = 0x00000001,
     UWF_DECORATED  = 0x00000002,

     UWF_DESTROYED  = 0x00000010,

     UWF_ALL        = 0x00000013
} UniqueWindowFlags;


typedef enum {
     UWNF_NONE           = 0x00000000,

     UWNF_DESTROYED      = 0x00000001,

     UWNF_UPDATE         = 0x00000010,

     UWNF_ALL            = 0x00000011
} UniqueWindowNotificationFlags;

typedef struct {
     UniqueWindowNotificationFlags  flags;
     UniqueWindow                  *window;

     DFBRegion                      update;
} UniqueWindowNotification;



DFBResult prophecy_window_create       ( CoreDFB                 *core,
                                       CoreWindow              *window,
                                       UniqueContext           *context,
                                       DFBWindowCapabilities    caps,
                                       const CoreWindowConfig  *config,
                                       UniqueWindow           **ret_window );

DFBResult prophecy_window_close        ( UniqueWindow            *window );

DFBResult prophecy_window_destroy      ( UniqueWindow            *window );


DFBResult prophecy_window_notify       ( UniqueWindow                  *window,
                                       UniqueWindowNotificationFlags  flags );


DFBResult prophecy_window_update       ( UniqueWindow            *window,
                                       const DFBRegion         *region,
                                       DFBSurfaceFlipFlags      flags );

DFBResult prophecy_window_post_event   ( UniqueWindow            *window,
                                       DFBWindowEvent          *event );

DFBResult prophecy_window_set_config   ( UniqueWindow            *window,
                                       const CoreWindowConfig  *config,
                                       CoreWindowConfigFlags    flags );

DFBResult prophecy_window_get_config   ( UniqueWindow            *window,
                                       CoreWindowConfig        *config );


DFBResult prophecy_window_restack      ( UniqueWindow            *window,
                                       UniqueWindow            *relative,
                                       int                      relation );


DFBResult prophecy_window_grab         ( UniqueWindow            *window,
                                       const CoreWMGrab        *grab );

DFBResult prophecy_window_ungrab       ( UniqueWindow            *window,
                                       const CoreWMGrab        *grab );

DFBResult prophecy_window_request_focus( UniqueWindow            *window );




/*
 * Creates a pool of window objects.
 */
FusionObjectPool *prophecy_window_pool_create( const FusionWorld *world );

/*
 * Generates prophecy_window_ref(), prophecy_window_attach() etc.
 */
FUSION_OBJECT_METHODS( UniqueWindow, prophecy_window )


/* global reactions */

typedef enum {
     PROPHECY_WM_MODULB_WINDOW_LISTENER
} PROPHECY_WINDOW_GLOBALS;


#endif

