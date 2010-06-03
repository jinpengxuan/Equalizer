
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#include "canvas.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "configVisitor.h"
#include "layout.h"
#include "log.h"
#include "node.h"
#include "pipe.h"
#include "segment.h"
#include "window.h"

#include <eq/fabric/paths.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>
#include <eq/base/stdExt.h>

namespace eq
{
namespace server
{
typedef fabric::Canvas< Config, Canvas, Segment, Layout > Super;

Canvas::Canvas( Config* parent )
        : Super( parent )
        , _state( STATE_STOPPED )
{}

Canvas::~Canvas()
{
}

Segment* Canvas::getSegment( const SegmentPath& path )
{
    const Segments& segments = getSegments();
    EQASSERTINFO( segments.size() > path.segmentIndex,
                  segments.size() << " <= " << path.segmentIndex );

    if( segments.size() <= path.segmentIndex )
        return 0;

    return segments[ path.segmentIndex ];
}

CanvasPath Canvas::getPath() const
{
    const Config* config = getConfig();
    EQASSERT( config );

    const Canvases& canvases = config->getCanvases();
    Canvases::const_iterator i = std::find( canvases.begin(), canvases.end(),
                                            this );
    EQASSERT( i != canvases.end( ));

    CanvasPath path;
    path.canvasIndex = std::distance( canvases.begin(), i );
    return path;
}

void Canvas::activateLayout( const uint32_t index )
{
    if( _state == STATE_RUNNING )
        _switchLayout( getActiveLayoutIndex(), index );
}

void Canvas::init()
{
    EQASSERT( _state == STATE_STOPPED );
    _switchLayout( EQ_ID_NONE, getActiveLayoutIndex( ));
    _state = STATE_RUNNING;
}

void Canvas::exit()
{
   EQASSERT( _state == STATE_RUNNING || _state == STATE_DELETE );
    _switchLayout( getActiveLayoutIndex(), EQ_ID_NONE );
    if( _state == STATE_RUNNING )
        _state = STATE_STOPPED;
}

namespace
{
class ActivateVisitor : public ConfigVisitor
{
public:
    ActivateVisitor( const Channels& channels ) : _channels( channels ) {}
    virtual ~ActivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( Channels::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->activate();
                compound->getConfig()->postNeedsFinish();
                break;
            }

            return TRAVERSE_PRUNE;
        }

private:
    Channels _channels;
};

class DeactivateVisitor : public ConfigVisitor
{
public:
    DeactivateVisitor( Channels& channels )
            : _channels( channels ) {}
    virtual ~DeactivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( Channels::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->deactivate();
                compound->getConfig()->postNeedsFinish();
                break;
            }

            return TRAVERSE_PRUNE;
        }

private:
    Channels& _channels;
};
}

void Canvas::_switchLayout( const uint32_t oldIndex, const uint32_t newIndex )
{
    Config* config = getConfig();
    if( oldIndex == newIndex )
        return;

    const Layouts& layouts = getLayouts();
    const size_t nLayouts = layouts.size();
    const Layout* oldLayout = (oldIndex >= nLayouts) ? 0 : layouts[oldIndex];
    const Layout* newLayout = (newIndex >= nLayouts) ? 0 : layouts[newIndex];

    const Segments& segments = getSegments();
    for( Segments::const_iterator i = segments.begin();
         i != segments.end(); ++i )
    {
        const Segment* segment = *i;        
        const Channels& destChannels = segment->getDestinationChannels();

        if( newLayout )
        {
            // activate channels used by new layout
            Channels usedChannels;
            for( Channels::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == newLayout )
                    usedChannels.push_back( channel );
            }
            
            ActivateVisitor activator( usedChannels );
            config->accept( activator );
        }

        if( oldLayout )
        {
            // de-activate channels used by old layout
            Channels usedChannels;

            for( Channels::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == oldLayout )
                    usedChannels.push_back( channel );
            }
            DeactivateVisitor deactivator( usedChannels );
            config->accept( deactivator );
        }
    }
}

void Canvas::deregister()
{
    net::Session* session = getSession();
    EQASSERT( session );

    const Segments& segments = getSegments();
    for( Segments::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        Segment* segment = *i;
        EQASSERT( segment->getID() != EQ_ID_INVALID );
        EQASSERT( segment->isMaster( ));
        
        session->deregisterObject( segment );
    }

    EQASSERT( getID() != EQ_ID_INVALID );
    EQASSERT( isMaster( ));
    session->deregisterObject( this );
}

void Canvas::postDelete()
{
    _state = STATE_DELETE;
    getConfig()->postNeedsFinish();
}

}
}

#include "nodeFactory.h"
#include "../lib/fabric/canvas.ipp"

template class eq::fabric::Canvas< eq::server::Config, eq::server::Canvas,
                                   eq::server::Segment, eq::server::Layout >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
    const eq::fabric::Canvas< eq::server::Config, eq::server::Canvas,
                              eq::server::Segment, eq::server::Layout >& );
/** @endcond */
