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

#ifndef __PROPHECY__TYPES_H__
#define __PROPHECY__TYPES_H__


typedef struct __Prophecy_UniqueContext        UniqueContext;
typedef struct __Prophecy_UniqueDecoration     UniqueDecoration;
typedef struct __Prophecy_UniqueDecorationItem UniqueDecorationItem;
typedef struct __Prophecy_UniqueDevice         UniqueDevice;
typedef struct __Prophecy_UniqueLayout         UniqueLayout;
typedef struct __Prophecy_UniqueWindow         UniqueWindow;

typedef struct __Prophecy_UniqueInputChannel   UniqueInputChannel;
typedef union  __Prophecy_UniqueInputEvent     UniqueInputEvent;
typedef struct __Prophecy_UniqueInputFilter    UniqueInputFilter;
typedef struct __Prophecy_UniqueInputSwitch    UniqueInputSwitch;


typedef struct __Prophecy_StretRegion          StretRegion;


typedef struct __Prophecy_WMData               WMData;
typedef struct __Prophecy_WMShared             WMShared;

#endif

