
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_CHANNEL_H
#define EQFABRIC_CHANNEL_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/renderContext.h> // member
#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
namespace fabric
{
    class ChannelVisitor;
    class Window;
    template< typename T > class LeafVisitor;

    /**
     * A channel represents a two-dimensional viewport within a Window.
     *
     * The channel is the basic rendering entity. It represents a 2D rendering
     * area within a Window. It executes all rendering-relevant tasks, such as
     * clear, draw, assemble and readback. Each rendering task is using its own
     * RenderContext, which is computed by the server based on the rendering
     * description of the current configuration.
     */
    template< typename T, typename W > class Channel : public Object
    {
    public:    
        /** 
         * The drawable format defines the components used as an alternate
         * drawable for this cannel. If an alternate drawable is configured, the
         * channel uses the appropriate targets in place of the window's frame
         * buffer.
         * @version 1.0
         */
        enum Drawable
        {
            FB_WINDOW   = EQ_BIT_NONE, //!< Use the window's frame buffer
            FBO_COLOR   = EQ_BIT1,     //!< Use an FBO for color values
            FBO_DEPTH   = EQ_BIT2,     //!< Use an FBO for depth values
            FBO_STENCIL = EQ_BIT3      //!< Use an FBO for stencil values
        };
        
        /** @name Data Access */
        //@{
        /** @return the parent window. @version 1.0 */
        W* getWindow() { return _window; }

        /** @return the parent window. @version 1.0 */
        const W* getWindow() const { return _window; }

        /** 
         * Set the channel's pixel viewport wrt its parent window.
         * 
         * @param pvp the viewport in pixels.
         */
        void setPixelViewport( const PixelViewport& pvp );

        /** @return the native pixel viewport. @version 1.0 */
        const PixelViewport& getNativePixelViewport() const
            { return _nativeContext.pvp; }

        /** 
         * Set the channel's viewport wrt its parent window.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const Viewport& vp );

        /** @return true if a viewport was specified last. @version 1.0 */
        bool hasFixedViewport() const { return _fixedVP; }

        /** 
         * Return the set of tasks this channel might execute in the worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @return the tasks.
         * @warning Experimental - may not be supported in the future
         */
        uint32_t getTasks() const { return _tasks; }

        /** 
         * Set the near and far planes for this channel.
         * 
         * The given near and far planes update the current perspective and
         * orthographics frustum accordingly. Furthermore, they will be used in
         * the future by the server to compute the frusta.
         *
         * @param nearPlane the near plane.
         * @param farPlane the far plane.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setNearFar( const float nearPlane, 
                                         const float farPlane);

        /** @return a fixed unique color for this channel. @version 1.0 */
        const Vector3ub& getUniqueColor() const { return _color; }

        /**
         * @return the channel's framebuffer attachment configuration.
         * @version 1.0
         */
        uint32_t getDrawable() const { return _drawable; }

        /** 
         * Traverse this channel using a channel visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( LeafVisitor< T > & visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_EXPORT VisitorResult accept( LeafVisitor< T >& visitor ) const;

        /** @warning  Undocumented - may not be supported in the future */
        EQFABRIC_EXPORT void setMaxSize( const Vector2i& size );

        void setOverdraw( const Vector4i& overdraw ); //!< @internal

        /** @internal */
        const Vector2i& getMaxSize()  const { return _maxSize; }
        //@}

        /**
         * @name Context-specific data access.
         * 
         * The data returned by these methods depends on the context (callback)
         * they are called from, typically the data for the current rendering
         * task. If they are called outside of a frame task method, they
         * return the channel's native parameter, e.g., a placeholder value for
         * the task decomposition parameters.
         */
        //@{
        /** @return the current draw buffer for glDrawBuffer. @version 1.0 */
        uint32_t getDrawBuffer() const { return _context->buffer; }

        /** @return the current read buffer for glReadBuffer. @version 1.0 */
        uint32_t getReadBuffer() const { return _context->buffer; }

        /** @return the current color mask for glColorMask. @version 1.0 */
        const ColorMask& getDrawBufferMask() const
            { return _context->bufferMask; }

        /**
         * @return the current pixel viewport for glViewport and glScissor.
         * @version 1.0
         */
        const PixelViewport& getPixelViewport() const { return _context->pvp; }

        /**
         * @return the current perspective frustum for glFrustum.
         * @version 1.0
         */
        const Frustumf& getFrustum() const { return _context->frustum; }

        /**
         * @return the current orthographic frustum for glOrtho.
         * @version 1.0
         */
        const Frustumf& getOrtho() const { return _context->ortho; }

        /**
         * Return the view matrix.
         *
         * The view matrix is part of the GL_MODEL*VIEW* matrix, and is
         * typically applied first to the GL_MODELVIEW matrix.
         * 
         * @return the head transformation matrix
         * @version 1.0
         */
        const Matrix4f& getHeadTransform() const
            { return _context->headTransform; }

        /**
         * @return the fractional viewport wrt the destination view.
         * @version 1.0
         */
        const Viewport& getViewport() const { return _context->vp; }

        /**
         * @return the database range for the current rendering task.
         * @version 1.0
         */
        const Range& getRange() const { return _context->range; }

        /**
         * @return the pixel decomposition for the current rendering task.
         * @version 1.0
         */
        const Pixel& getPixel() const { return _context->pixel; }

        /**
         * @return the subpixel decomposition for the current rendering task.
         * @version 1.0
         */
        const SubPixel& getSubPixel() const { return _context->subpixel; }

        /**
         * @return the up/downscale zoom factor for the current rendering task.
         * @version 1.0
         */
        const Zoom& getZoom() const { return _context->zoom; }

        /**
         * @return the DPlex period for the current rendering task.
         * @version 1.0
         */
        uint32_t getPeriod() const { return _context->period; }

        /**
         * @return the DPlex phase for the current rendering task.
         * @version 1.0
         */
        uint32_t getPhase() const { return _context->phase; }

        /**
         * Get the channel's current position wrt the destination channel.
         *
         * Note that computing this value from the current viewport and pixel
         * viewport is inaccurate because it neglects rounding errors of the
         * pixel viewport done by the server.
         *
         * @return the channel's current position wrt the destination channel.
         * @version 1.0
         */
        const Vector2i& getPixelOffset() const { return _context->offset; }

        /** @return the currently rendered eye pass. @version 1.0 */
        Eye getEye() const { return _context->eye; }

        /** @internal  Undocumented - may not be supported in the future */
        const Vector4i& getOverdraw() const { return _context->overdraw; }

        /** @internal  Undocumented - may not be supported in the future */
        uint32_t getTaskID() const { return _context->taskID; }
        //@}

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in channel.cpp
        /** Integer attributes for a channel. @version 1.0 */
        enum IAttribute
        {
            /** Statistics gathering mode (OFF, FASTEST [ON], NICEST) */
            IATTR_HINT_STATISTICS,
            /** Use a send token for output frames (OFF, ON) */
            IATTR_HINT_SENDTOKEN,
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };
        
        /** @return the value of an integer attribute. @version 1.0 */
        EQFABRIC_EXPORT int32_t getIAttribute( const IAttribute attr ) const;
        /** @return the name of an integer attribute. @version 1.0 */
        EQFABRIC_EXPORT static const std::string& getIAttributeString(
                                                        const IAttribute attr );
        //@}

        /** @name Error Information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within configInit().
         *
         * @param message the error message.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setErrorMessage( const std::string& message );

        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}
        
    protected:
        /** Construct a new channel. @internal */
        EQFABRIC_EXPORT Channel( W* parent );

        /** Construct a new deep copy of a channel. @internal */
        Channel( const Channel& from, W* parent );

        /** Destruct the channel. @version 1.0 */
        EQFABRIC_EXPORT virtual ~Channel();

        enum DirtyBits
        {
            DIRTY_ATTRIBUTES = Object::DIRTY_CUSTOM << 0,
            DIRTY_VIEWPORT   = Object::DIRTY_CUSTOM << 1,
            DIRTY_MEMBER     = Object::DIRTY_CUSTOM << 2,
            DIRTY_ERROR      = Object::DIRTY_CUSTOM << 3,
            DIRTY_FRUSTUM    = Object::DIRTY_CUSTOM << 4,
            DIRTY_FILL1      = Object::DIRTY_CUSTOM << 5,
            DIRTY_FILL2      = Object::DIRTY_CUSTOM << 6,
            DIRTY_CUSTOM     = Object::DIRTY_CUSTOM << 7,
        };

        /** @internal */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        /** Set thetasks this channel might potentially execute. @internal */
        void setTasks( const uint32_t tasks );

        /** Update the native view identifier and version. @internal */
        void setViewVersion( const net::ObjectVersion& view );

        void setDrawable( const uint32_t drawable ); //!< @internal

        /** Notification that parameters influencing the vp/pvp have changed.*/
        virtual void notifyViewportChanged();

        /** @name Render context access @internal */
        //@{
        /** Override the channel's native render context. @internal */
        void overrideContext( RenderContext& context ) { _context = &context; }

        /** Re-set the channel's native render context. @internal */
        void resetContext() { _context = &_nativeContext; }

        /** @return the current render context. @internal */
        const RenderContext& getContext() const { return *_context; }

        /** @return the native render context. @internal */
        const RenderContext& getNativeContext() const { return _nativeContext; }
        //@}

        /** @name Attributes */
        //@{
        /** @internal */
        void setIAttribute( const IAttribute attr, const int32_t value )
            { _iAttributes[attr] = value; setDirty( DIRTY_ATTRIBUTES ); }
        //@}


    private:
        /** The parent window. */
        W* const _window;
        friend class Window;

        /** The native render context parameters of this channel. */
        RenderContext _nativeContext;

        /** The current rendering context. */
        RenderContext* _context;

        /** A unique color assigned by the server during config init. */
        Vector3ub _color;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** An alternate drawable config. */
        uint32_t _drawable;

        /** The reason for the last error. */
        std::string _error;

        /** Overdraw limiter */
        Vector2i    _maxSize;
        
        /** true if the vp is immutable, false if the pvp is immutable */
        bool _fixedVP;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQFABRIC_CHANNEL_H
