
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "event.h"

#include <eq/base/idPool.h>

using namespace std;
using namespace eq::base;

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#else
#  include <strings.h>
#endif

namespace eq
{
namespace{
/** String representation of event types. */
static std::string _eventTypeNames[ Event::ALL ] =
{
    "expose",
    "window resize",
    "window close",
    "window show",
    "window hide",
    "pointer motion",
    "pointer button press",
    "pointer button release",
    "key press",
    "key release",
    "channel resize",
    "statistic",
    "view resize",
    "unknown",
    "user-specific"
};

/** String representation of statistic event types. */
static std::string _stateEventTypeNames[Statistic::TYPE_ALL] =
{
    "NO EVENT          ",
    "channel clear     ",
    "channel draw      ",
    "channel finishdraw",
    "channel assemble  ",
    "channel readback  ",
    "channel transmit  ",
    "channel transmit 1",
    "channel wait frame",
    "channel compress",
    "window finish",
    "window swap barrier",
    "window swap buffer",
    "window throttle framerate",
    "pipe idle",
    "node transmit"
    "node compress",
    "config start frame",
    "config finishframe",
    "config wait finish"
};
}

Event::Event()
        : type( UNKNOWN )
        , originator( EQ_ID_INVALID )
{
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Event& event )
{
    os << disableFlush << event.type << ':' << event.originator << ' ';
    switch( event.type )
    {
        case Event::EXPOSE:
        case Event::WINDOW_CLOSE:
            break;

        case Event::WINDOW_RESIZE:
        case Event::WINDOW_SHOW:
        case Event::WINDOW_HIDE:
        case Event::CHANNEL_RESIZE:
        case Event::VIEW_RESIZE:
            os << event.resize;
            break;

        case Event::POINTER_MOTION:
        case Event::POINTER_BUTTON_PRESS:
        case Event::POINTER_BUTTON_RELEASE:
            os << event.pointer;
            break;

        case Event::KEY_PRESS:
        case Event::KEY_RELEASE:
            os << event.key;
            break;

        case Event::STATISTIC:
            os << event.statistic;
        default:
            break;
    }
    
    os << /*", context " << event.context <<*/ enableFlush;
    return os;
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Event::Type& type)
{
    if( type >= Event::ALL )
        os << "unknown (" << static_cast<unsigned>( type ) << ')';
    else 
        os << _eventTypeNames[ type ];

    return os;
}

std::ostream& operator << ( std::ostream& os, const ResizeEvent& event )
{
    os << event.x << 'x' << event.y << '+' << event.w << '+' << event.h << ' ';
    return os;
}

std::ostream& operator << ( std::ostream& os, const PointerEvent& event )
{
    os << '[' << event.x << "], [" << event.y << "] d(" << event.dx << ", "
       << event.dy << ')' << " buttons ";

    if( event.buttons == PTR_BUTTON_NONE ) os << "none";
    if( event.buttons & PTR_BUTTON1 ) os << "1";
    if( event.buttons & PTR_BUTTON2 ) os << "2";
    if( event.buttons & PTR_BUTTON3 ) os << "3";
    if( event.buttons & PTR_BUTTON4 ) os << "4";
    if( event.buttons & PTR_BUTTON5 ) os << "5";

    os << " fired ";
    if( event.button == PTR_BUTTON_NONE ) os << "none";
    if( event.button & PTR_BUTTON1 ) os << "1";
    if( event.button & PTR_BUTTON2 ) os << "2";
    if( event.button & PTR_BUTTON3 ) os << "3";
    if( event.button & PTR_BUTTON4 ) os << "4";
    if( event.button & PTR_BUTTON5 ) os << "5";

    os << ' ';
    return os;
}

std::ostream& operator << ( std::ostream& os, const KeyEvent& event )
{
    os << "key " << event.key << ' ' << endl;
    return os;
}

std::ostream& operator << ( std::ostream& os, const Statistic& event )
{
    os << event.resourceName << ": " << _stateEventTypeNames[ event.type ]
       << ' ' << event.frameNumber << ' ' << event.startTime << " - " 
       << event.endTime;
    return os;
}
}
