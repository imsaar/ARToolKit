/*
 *	gsub_lite.h
 *
 *	Graphics Subroutines (Lite) for ARToolKit.
 *
 *	Copyright (c) 2003-2004 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *	2.6.5	????-??-??	MB/HK	Original from ARToolKit-2.65DS gsub.c
 *  2.7.0   2003-08-13  PRL     Lipo'ed and whipped into shape.
 *  2.7.1   2004-03-03  PRL		Avoid defining BOOL if already defined
 *	2.7.1	2004-03-03	PRL		Don't enable lighting if it was not enabled.
 *	2.7.2	2004-04-27	PRL		Added headerdoc markup. See http://developer.apple.com/darwin/projects/headerdoc/
 *	2.7.3	2004-07-02	PRL		Much more object-orientated through use of ARGL_CONTEXT_SETTINGS type.
 *	2.7.4	2004-07-14	PRL		Added gluCheckExtension hack for GLU versions pre-1.3.
 *	2.7.5	2004-07-15	PRL		Added arglDispImageStateful(); removed extraneous glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,...) calls.
 *	2.7.6	2005-02-18	PRL		Go back to using int rather than BOOL, to avoid conflict with Objective-C.
 *
 */
/*
 * 
 * This file is part of ARToolKit.
 * 
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

/*!
	@header gsub_lite
	@abstract A collection of useful OpenGL routines for ARToolKit.
	@discussion
		Sample code for example usage of gsub_lite is included with
		ARToolKit, in the directory &lt;AR/examples/simpleLite&gt;.
		
		gsub_lite v2.7 was designed as a replacement for the original gsub from
		ARToolKit, by Mark Billinghurst (MB) & Hirokazu Kato (HK), with the
		following functionality removed: <br>
			- GLUT event handling. <br>
			- Sub-window ("MINIWIN") and half-size drawing. <br>
			- HMD support for stereo via stencil. <br>
		and the following functionality added or improved: <br>
			+ Support for true stereo and multiple displays through removal
				of most dependencies on global variables. <br>
			+ Prepared library for thread-safety by removing global variables. <br>
			+ Optimised texturing, particularly for Mac OS X platform. <br>
			+ Added arglCameraFrustum to replace argDraw3dCamera() function. <br>
			+ Renamed argConvGlpara() to arglCameraView() to more accurately
				represent its functionality. <br>
			+ Correctly handle textures with ARGB and ABGR byte ordering. <br>
			+ Library setup and cleanup functions. <br>
			+ Version numbering. <br>
			
		This file is part of ARToolKit.
		
		ARToolKit is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.
		
		ARToolKit is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.
		
		You should have received a copy of the GNU General Public License
		along with ARToolKit; if not, write to the Free Software
		Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
	@copyright 2003-2004 Philip Lamb
	@updated 2004-07-02
 */

#ifndef __gsub_lite_h__
#define __gsub_lite_h__

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef _WIN32
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif
#include <AR/config.h>
#include <AR/ar.h>		// ARUint8
#include <AR/param.h>	// ARParam, arParamDecompMat(), arParamObserv2Ideal()

// ============================================================================
//	Public types and definitions.
// ============================================================================

// Keep code nicely typed.
#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

/*!
    @typedef ARGL_CONTEXT_SETTINGS_REF
    @abstract Opaque type to hold ARGL settings for a given OpenGL context.
    @discussion
		An OpenGL context is an implementation-defined structure which
		keeps track of OpenGL state, including textures and display lists.
		Typically, individual OpenGL windows will have distinct OpenGL
		contexts assigned to them by the host operating system.
 
		As gsub_lite uses textures and display lists, it must be able to
		track which OpenGL context a given texture or display list it is using
		belongs to. This is especially important when gsub_lite is being used to
		draw into more than one window (and therefore more than one context.)
 
		Basically, functions which depend on OpenGL state, will require an
		ARGL_CONTEXT_SETTINGS_REF to be passed to them. An ARGL_CONTEXT_SETTINGS_REF
		is generated by setting the current OpenGL context (e.g. if using GLUT,
		you might call glutSetWindow()) and then calling arglSetupForCurrentContext().
		When you have finished using ARGL in a given context, you should call
		arglCleanup(), passing in an ARGL_CONTEXT_SETTINGS_REF, to free the
		memory used by the settings structure.
	@availability First appeared in ARToolKit 2.68.
 */
typedef struct _ARGL_CONTEXT_SETTINGS *ARGL_CONTEXT_SETTINGS_REF;

// ============================================================================
//	Public globals.
// ============================================================================

// It'd be nicer if these were wrapped in accessor functions.

/*!
    @var arglDrawMode
	@abstract Determines display method by which arglDispImage() transfers pixels. 
    @discussion
		The value of this variable determines the method by which
		arglDispImage transfers pixels of an image to the display. Setting this
		variable to a value of AR_DRAW_BY_GL_DRAW_PIXELS specifies the use of OpenGL
		DrawPixels to do the transfer. Setting this variable to a value of
		AR_DRAW_BY_TEXTURE_MAPPING specifies the use of OpenGL TexImage2D to do the
		transfer. The DrawPixels method is guaranteed to be available on all
		implementations, but arglDispImage does not correct the image
		for camera lens distortion under this method. In contrast, TexImage2D is only
		available on some implementations, but allows arglDispImage() to apply a correction
		for camera lens distortion, and additionally offers greater potential for
		accelerated drawing on some implementations.
 
		The initial value is defined to the value of the symbolic constan DEFAULT_DRAW_MODE
		(defined in &lt;AR/config.h&gt;).
	@availability First appeared in ARToolKit 2.68.
 */
extern int arglDrawMode;

/*!
    @var arglTexmapMode
	@abstract Determines use of full or half-resolution TexImage2D pixel-transfer in arglDispImage().
	@discussion
		When arglDrawMode is set to AR_DRAW_BY_TEXTURE_MAPPING, the value of this variable
		determines whether full or half-resolution data is transferred to the
		texture. A value of AR_DRAW_TEXTURE_FULL_IMAGE uses all available pixels in the
		source image data. A value of AR_DRAW_TEXTURE_HALF_IMAGE discards every second pixel
		in the source image data, defining a half-width texture which is then drawn stretched
		horizontally to double its width. The latter method offers some advantages to
		certain implentations where texture transfer is slow or costly, at the expense of
		image detail.
	@availability First appeared in ARToolKit 2.68.
 */
extern int arglTexmapMode;

/*!
    @var arglTexRectangle
	@abstract Determines use of rectangular TexImage2D pixel-transfer in arglDispImage().
	@discussion
		On implementations which support the OpenGL extension for rectangular textures (of
		non power-of-two size), and when arglDrawMode is set to AR_DRAW_BY_TEXTURE_MAPPING,
		the value of this variable determines whether rectangular textures or ordinary
		(power-of-two) textures are used by arglDispImage(). A value of TRUE specifies the
		use of rectangluar textures. A value of FALSE specifies the use of ordinary textures.
		
		If gsub_lite was not built without support for rectangular textures, changing the
		value of this variable will have no effect, and ordinary textures will always be
		used. Support for rectangular textures is only available when gsub_lite is built
		with AR_OPENGL_TEXTURE_RECTANGLE defined in &lt;AR/config.h&gt and with either
		GL_EXT_texture_rectangle or GL_NV_texture_rectangle defined in &lt;GL/glext.h&gt;
		or &lt;GL/gl.h&gt;
	@availability First appeared in ARToolKit 2.68.
 */
extern int arglTexRectangle;

#if defined(__APPLE__) && defined(APPLE_TEXTURE_FAST_TRANSFER)
extern int arglAppleClientStorage;
extern int arglAppleTextureRange;
extern GLuint arglAppleTextureRangeStorageHint;
#endif // __APPLE__ && APPLE_TEXTURE_FAST_TRANSFER

// ============================================================================
//	Public functions.
// ============================================================================

/*!
    @function arglSetupForCurrentContext
    @abstract Initialise the gsub_lite library for the current OpenGL context.
    @discussion
		This function performs required setup of the gsub_lite library
		for the current OpenGL context and must be called before any other argl*()
		functions are called for this context.
 
		An OpenGL context holds all of the state of the OpenGL machine, including
		textures and display lists etc. There will usually be one OpenGL context
		for each window displaying OpenGL content.
 
		Other argl*() functions whose operation depends on OpenGL state will
		require an ARGL_CONTEXT_SETTINGS_REF. This is just so that
		they can keep track of per-context variables.
 
		You should call arglCleanup() passing in the ARGL_CONTEXT_SETTINGS_REF
		when you have finished with the library for this context.
    @result An ARGL_CONTEXT_SETTINGS_REF. See the documentation for this type for more info.
	@availability First appeared in ARToolKit 2.68.
*/
ARGL_CONTEXT_SETTINGS_REF arglSetupForCurrentContext(void);

/*!
    @function arglCleanup
    @abstract Free memory used by gsub_lite associated with the specified context.
    @discussion
		Should be called after no more argl* functions are needed, in order
		to prevent memory leaks etc.
 
		The library can be setup again for the context at a later time by calling
		arglSetupForCurrentContext() again.
	@param contextSettings A reference to ARGL's settings for an OpenGL
		context, as returned by arglSetupForCurrentContext().
	@availability First appeared in ARToolKit 2.68.
*/
void arglCleanup(ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
    @function arglCameraFrustum
    @abstract Create an OpenGL perspective projection matrix.
    @discussion
		Use this function to create a matrix suitable for passing to OpenGL
		to set the viewing projection.
    @param cparam Pointer to a set of ARToolKit camera parameters for the
		current video source.
	@param focalmax The maximum distance at which geometry will be rendered.
		Any geometry further away from the camera than this distance will be clipped
		and will not be appear in a rendered frame. Thus, this value should be
		set high enough to avoid clipping of any geometry you care about. However,
		the precision of the depth buffer is correlated with the ratio of focalmin
		to focalmax, thus you should not set focalmax any higher than it needs to be.
		This value should be specified in the same units as your OpenGL drawing.
	@param focalmin The minimum distance at which geometry will be rendered.
		Any geometry closer to the camera than this distance will be clipped
		and will not be appear in a rendered frame. Thus, this value should be
		set low enough to avoid clipping of any geometry you care about. However,
		the precision of the depth buffer is correlated with the ratio of focalmin
		to focalmax, thus you should not set focalmin any lower than it needs to be.
		Additionally, geometry viewed in a stereo projections that is too close to
		camera is difficult and tiring to view, so if you are rendering stereo
		perspectives you should set this value no lower than the near-point of
		the eyes. The near point in humans varies, but usually lies between 0.1 m
		0.3 m. This value should be specified in the same units as your OpenGL drawing.
	@param m_projection Pointer to a array of 16 GLdoubles, which will be filled
		out with a projection matrix suitable for passing to OpenGL. The matrix
		is specified in column major order.
	@availability First appeared in ARToolKit 2.68.
*/
void arglCameraFrustum(const ARParam *cparam, const double focalmin, const double focalmax, GLdouble m_projection[16]);

/*!
    @function arglCameraView
    @abstract Create an OpenGL viewing transformation matrix.
	@discussion
		Use this function to create a matrix suitable for passing to OpenGL
		to set the viewing transformation of the virtual camera.
	@param para Pointer to 3x4 matrix array of doubles which specify the
		position of an ARToolKit marker, as returned by arGetTransMat().
	@param m_modelview Pointer to a array of 16 GLdoubles, which will be filled
		out with a modelview matrix suitable for passing to OpenGL. The matrix
		is specified in column major order.
	@param scale Specifies a scaling between ARToolKit's
		units (usually millimeters) and OpenGL's coordinate system units.
	@availability First appeared in ARToolKit 2.68.
*/
void arglCameraView(double para[3][4], GLdouble m_modelview[16], double scale);

/*!
    @function arglDispImage
    @abstract Display an ARVideo image, by drawing it using OpenGL.
    @discussion
		This function draws an image from an ARVideo source to the current
		OpenGL context. This operation is most useful in video see-through
		augmented reality applications for drawing the camera view as a
		background image, but can also be used in other ways.
 
		An undistorted image is drawn with the lower-left corner of the
		bottom-left-most pixel at OpenGL screen coordinates (0,0), and the
		upper-right corner of the top-right-most pixel at OpenGL screen
		coodinates (x * zoom, y * zoom), where x and y are the values of the
		fields cparam->xsize and cparam->ysize (see below) and zoom is the
		value of the parameter zoom (also see below). If cparam->dist_factor
		indicates that an un-warping correction should be applied, the actual
		coordinates will differ from the values specified here. 
 
		OpenGL state: Drawing is performed with depth testing and lighting
		disabled, and thus leaves the the depth buffer (if any) unmodified. If
		pixel transfer is by texturing (see documentation for arglDrawMode),
		the drawing is done in replacement texture environment mode.
		The depth test enable and lighting enable state and the texture
		environment mode are restored before the function returns.
	@param image Pointer to the tightly-packed image data (as returned by
		arVideoGetImage()). The horizontal and vertical dimensions of the image
		data must exactly match the values specified in the fields cparam->xsize
		and cparam->ysize (see below).
 
		The first byte of image data corresponds to the first component of the
		top-left-most pixel in the image. The data continues with the remaining
		pixels of the first row, followed immediately by the pixels of the second
		row, and so on to the last byte of the image data, which corresponds to
		the last component of the bottom-right-most pixel.
		
		In the current version of gsub_lite, the format of the pixels (i.e. the
		arrangement of components within each pixel) is fixed at the time the
		library is built, and cannot be changed at runtime. (It is determined by
		which of the possible AR_PIXEL_FORMAT_xxxx symbols was defined at the
		time the library was built.) Usually, image data is passed in directly
		from images generated by ARVideo, and so you should ensure that ARVideo
		is generating pixels of the same format.
	@param cparam Pointer to a set of ARToolKit camera parameters for the
		current video source. The size of the source image is taken from the
		fields xsize and ysize of the ARParam structure pointed to. Also, when
		the draw mode is AR_DRAW_BY_TEXTURE_MAPPING (see the documentation for
		the global variable arglDrawMode) the field dist_factor of the ARParam
		structure pointed to will be taken as the amount to un-warp the supplied
		image.		
	@param zoom The amount to scale the video image up or down. To draw the video
		image double size, use a zoom value of 2.0. To draw the video image
		half size use a zoom value of 0.5.
	@param contextSettings A reference to ARGL's settings for the current OpenGL
		context, as returned by arglSetupForCurrentContext() for this context. It
		is the callers responsibility to make sure that the current context at the
		time arglDisplayImage() is called matches that under which contextSettings
		was created.
	@availability First appeared in ARToolKit 2.68.
*/
void arglDispImage(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
	@function arglDispImageStateful
    @abstract Display an ARVideo image, by drawing it using OpenGL, using and modifying current OpenGL state.
    @discussion
		This function is identical to arglDispImage except that whereas
		arglDispImage sets an orthographic 2D projection and the OpenGL state
		prior to drawing, this function does not. It also does not restore any
		changes made to OpenGL state.
 
		This allows you to do effects with your image, other than just drawing it
		2D and with the lower-left corner of the bottom-left-most pixel attached
		to the bottom-left (0,0) of the window. For example, you might use a
		perspective projection instead of an orthographic projection with a
		glLoadIdentity() / glTranslate() on the modelview matrix to place the
		lower-left corner of the bottom-left-most pixel somewhere other than 0,0
		and leave depth-testing enabled.
 
		See the documentation for arglDispImage() for more information.
	@availability First appeared in ARToolKit 2.68.2.
 */
void arglDispImageStateful(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings);

#ifdef __cplusplus
}
#endif

#endif /* !__gsub_lite_h__ */
