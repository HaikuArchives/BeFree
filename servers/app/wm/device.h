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

#ifndef __PROPHECY__DEVICB_H__
#define __PROPHECY__DEVICB_H__

#include <directfb.h>

#include <fusion/reactor.h>

#include <core/coretypes.h>

#include "types.h"


typedef struct {
     int data_size;


     DFBResult (*Initialize)    ( UniqueDevice        *device,
                                  void                *data,
                                  void                *ctx );

     void      (*Shutdown)      ( UniqueDevice        *device,
                                  void                *data,
                                  void                *ctx );


     void (*Connected)   ( UniqueDevice           *device,
                           void                   *data,
                           void                   *ctx,
                           CoreInputDevice        *source );

     void (*Disconnected)( UniqueDevice           *device,
                           void                   *data,
                           void                   *ctx,
                           CoreInputDevice        *source );

     void (*ProcessEvent)( UniqueDevice           *device,
                           void                   *data,
                           void                   *ctx,
                           const DFBInputEvent    *event );


     bool (*FilterEvent) ( const UniqueInputEvent *event,
                           const UniqueInputEvent *filter );
} UniqueDeviceClass;

typedef unsigned int UniqueDeviceID;
typedef unsigned int UniqueDeviceClassID;

typedef enum {
     UDCI_POINTER,
     UDCI_WHEEL,
     UDCI_KEYBOARD,

     _UDCI_NUM
} UniqueDeviceClassIndex;


DFBResult prophecy_device_class_register  ( const UniqueDeviceClass *clazz,
                                          UniqueDeviceClassID     *ret_id );

DFBResult prophecy_device_class_unregister( UniqueDeviceClassID      id );


DFBResult prophecy_device_create       ( CoreDFB                *core,
                                       UniqueContext          *context,
                                       UniqueDeviceClassID     class_id,
                                       void                   *ctx,
                                       UniqueDevice          **ret_device );

DFBResult prophecy_device_destroy      ( UniqueDevice           *device );


DFBResult prophecy_device_connect      ( UniqueDevice           *device,
                                       CoreInputDevice        *source );

DFBResult prophecy_device_disconnect   ( UniqueDevice           *device,
                                       CoreInputDevice        *source );

DFBResult prophecy_device_attach       ( UniqueDevice           *device,
                                       ReactionFunc            func,
                                       void                   *ctx,
                                       Reaction               *reaction );

DFBResult prophecy_device_detach       ( UniqueDevice           *device,
                                       Reaction               *reaction );

DFBResult prophecy_device_attach_global( UniqueDevice           *device,
                                       int                     index,
                                       void                   *ctx,
                                       GlobalReaction         *reaction );

DFBResult prophecy_device_detach_global( UniqueDevice           *device,
                                       GlobalReaction         *reaction );

DFBResult prophecy_device_dispatch     ( UniqueDevice           *device,
                                       const UniqueInputEvent *event );

bool      prophecy_device_filter       ( UniqueDeviceClassID     class_id,
                                       const UniqueInputEvent *event,
                                       const UniqueInputEvent *filter );


/* global reactions */

typedef enum {
     PROPHECY_INPUT_SWITCH_DEVICB_LISTENER,
     PROPHECY_CURSOR_DEVICB_LISTENER
} PROPHECY_DEVICB_GLOBALS;

#endif

