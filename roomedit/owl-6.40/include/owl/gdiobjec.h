//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of abstract GDI object class and derived classes
//----------------------------------------------------------------------------

#if !defined(OWL_GDIOBJEC_H)
#define OWL_GDIOBJEC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gdibase.h>


namespace owl {

/// \addtogroup gdi
/// @{

class _OWLCLASS TDC;
class _OWLCLASS TFile;
class _OWLCLASS TBrush;
class _OWLCLASS TBitmap;
class _OWLCLASS TDib;
class _OWLCLASS TPalette;
class _OWLCLASS TClipboard;
class _OWLCLASS TMetaFilePict;
class _OWLCLASS TXGdi;
class _OWLCLASS TRiffFile;
class _OWLCLASS TPoint;
class _OWLCLASS TSize;
class _OWLCLASS TRect;
class _OWLCLASS TResId;
//
// Define to not share handles in copy ctors, otherwise these classes may act
// as envelopes and treat the handles as ref counted letters
//
//#define NO_GDI_SHARE_HANDLES

//
/// GDI Orphan control element
//
struct TObjInfo;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TGdiObject
// ~~~~~ ~~~~~~~~~~
/// GdiObject is the root, pseudo-abstract base class for ObjectWindows' GDI
/// (Graphics Device Interface) wrappers. The TGdiOject-based classes let you work
/// with a GDI handle and construct a C++ object with an aliased handle. Some GDI
/// objects are also based on TGdiObject for handle management. Generally, the
/// TGdiObject-based class hierarchy handles all GDI objects apart from the DC
/// (Device Context) objects handled by the TDC-based tree.
/// 
/// The five DC selectable classes (TPen, TBrush, TFont, TPalette, and TBitmap), and
/// the TIcon, TCursor, TDib, and TRegion classes, are all derived directly from
/// TGdiObject.
/// 
/// TGdiObject maintains the GDI handle and a ShouldDelete flag that determines if
/// and when the handle and object should be destroyed. Protected constructors are
/// provided for use by the derived classes: one for borrowed handles, and one for
/// normal use.
/// An optional orphan control mechanism is provided. By default, orphan control is
/// active, but you can turn it off by defining the NO_GDI_ORPHAN_CONTROL
/// identifier:
/// \code
/// #define NO_GDI_ORPHAN_CONTROL
/// \endcode
/// With orphan control active, the following static member functions are available:
/// RefAdd, RefCount, RefDec, RefFind, RefInc, and RefRemove.
/// 
/// These maintain object reference counts and allow safe orphan recovery and
/// deletion. Macros, such as OBJ_REF_ADD, let you deactivate or activate your
/// orphan control code by simply defining or undefining NO_GDI_ORPHAN_CONTROL. When
/// NO_GDI_ORPHAN_CONTROL is undefined, for example, OBJ_REF_ADD(handle, type)
/// expands to TGdiObject::RefAdd((handle),(type)), but when NO_GDI_ORPHAN_CONTROL
/// is defined, the macro expands to handle.
//
class _OWLCLASS TGdiObject : private TGdiBase {
public:
	// Class scoped types
	//
	typedef HGDIOBJ THandle;  ///< TGdiObject encapsulates an HGDIOBJ

	// Destructor
	//
	~TGdiObject();  // GDI classes are not polymorphic, & dtor is not virtual

	HGDIOBJ     GetGdiHandle() const;
	operator    HGDIOBJ() const;

	int         GetObject(int count, void * object) const;
	bool        operator ==(const TGdiObject& other) const;
	
	/// not implemented just to make happy STL
	bool        operator <(const TGdiObject& other) const;

	bool        IsGDIObject() const;
	uint32      GetObjectType() const;

	// GDI handle management & orphan control
	//
	
/// This enumeration is used to store the object type in the struct TObjInfo. This
/// internal structure is used to track object reference counts during debugging
/// sessions.
	enum TType {
		None, Pen, Brush, Font, Palette, Bitmap, TextBrush
	};

	static TObjInfo* RefFind(HANDLE object);
	static void      RefAdd(HANDLE handle, TType type);
	static void      RefRemove(HANDLE handle);
	static void      RefInc(HANDLE handle);
	static void      RefDec(HANDLE handle, bool wantDelete);
	static int       RefCount(HANDLE handle);

protected:
	using TGdiBase::CheckValid;  // make this function available to derivatives
	using TGdiBase::_CheckValid;  // make this function available to derivatives
	using TGdiBase::Handle;      // and these members too
	using TGdiBase::ShouldDelete;

	// Constructors for use by derived classes only
	//
	TGdiObject();
	TGdiObject(HANDLE handle, TAutoDelete autoDelete = NoAutoDelete);

private:
	TGdiObject(const TGdiObject&); // Protect against copying of GDI objects
	TGdiObject& operator =(const TGdiObject&);

};

//
/// \class TPen
// ~~~~~ ~~~~
/// TPen is derived from TGdiObject. It encapsulates the GDI pen tool. Pens can be
/// constructed from explicit information or indirectly. TPen relies on the base
/// class's destructor, ~TGdiObject.
//
class _OWLCLASS TPen : public TGdiObject {
public:
	// Class scoped types
	//
	typedef HPEN THandle;  // TPen encapsulates an HPEN

	// Constructors
	//
	TPen(HPEN handle, TAutoDelete autoDelete = NoAutoDelete);
	TPen(const TColor& color, int width=1, int style=PS_SOLID);
	TPen(const LOGPEN& logPen);
	TPen(const TPen& src);
	TPen(uint32 penStyle, uint32 width, const TBrush& brush, uint32 styleCount,
		const uint32* style);
	TPen(uint32 penStyle, uint32 width, const LOGBRUSH& logBrush,
		uint32 styleCount, const uint32* style);

	// Type Accessor & Conversion Operator
	//
	HPEN     GetHandle() const;
	operator HPEN() const;

  LOGPEN GetObject() const;

#if defined(OWL5_COMPAT)

	TPen(const LOGPEN* logPen);
	bool GetObject(LOGPEN & logPen) const;

#endif

private:
	TPen& operator =(const TPen&);
};

//
/// \class TBrush
// ~~~~~ ~~~~~~
/// The GDI Brush class is derived from TGdiObject. TBrush provides constructors for
/// creating solid, styled, or patterned brushes from explicit information. It can
/// also create a brush indirectly from a borrowed handle.
//
class _OWLCLASS TBrush : public TGdiObject {
public:
	// Class scoped types
	//
	typedef HBRUSH THandle;  /// TBrush encapsulates an HBRUSH

	// Constructors
	//
	TBrush(HBRUSH handle, TAutoDelete autoDelete = NoAutoDelete);
	TBrush(const TColor& color, bool useCache = true);
	TBrush(const TColor& color, int style);
	TBrush(const LOGBRUSH& logBrush);
	TBrush(const TBitmap& pattern);
	TBrush(const TDib& pattern);
	TBrush(const TBrush& src);
	~TBrush();

	// Type Accessor & Conversion Operator
	//
	HBRUSH   GetHandle() const;
	operator HBRUSH() const;

	LOGBRUSH GetObject() const;

#if defined(OWL5_COMPAT)

	TBrush(const LOGBRUSH* logBrush);
	bool GetObject(LOGBRUSH & logBrush) const;

#endif

private:
	TBrush& operator =(const TBrush&);
};

//
/// \class THatch8x8Brush
// ~~~~~ ~~~~~~~~~~~~~~
/// Derived from TBrush, THatch8x8Brush defines a small, 8x8, monochrome,
/// configurable hatch brush. Because the hatch brush is a logical brush created
/// from device-independent bitmaps (DIBs), it can be passed to any device context
/// (DC), which then renders the brush into the appropriate form for the device.
/// 
/// Although the default brush color is a white foreground and a black background,
/// you can vary the colors of the hatched brush. The colors can be any one of the
/// TColor object encapsulated colors, namely the standard RGB values.
/// 
/// THatch8x8Brush contains static arrays that define common hatched brush patterns.
/// The hatched brush patterns you can select are
/// 
/// Standard
/// \image html bm156.png
/// 
/// Forward diagonal 1
/// \image html bm161.png 
///
/// Forward diagonal 2
/// \image html bm171.png
/// 
/// Backward diagonal 1
/// \image html bm172.png
///
/// Backward diagonal 2
/// \image html bm181.png
/// 
/// 
/// You can use THatch8x8Brush  to design a variety of hatched brush border patterns
/// around a simple rectangle or an OLE container. You can also use THatch8x8Brush
/// in conjunction with TUIHandle.
//
class _OWLCLASS THatch8x8Brush : public TBrush {
public:
	THatch8x8Brush(const uint8 hatch[], const TColor& fgColor=TColor::White,
		const TColor& bgColor=TColor::Black);
	void     Reconstruct(const uint8 hatch[], const TColor& fgColor, const TColor& bgColor);

/// The static array Hatch22F1[8] holds a monochrome hatched brush pattern of two
/// pixels on and two off in forward diagonal hatch marks, offset one per row as the
/// following pattern illustrates:
/// \image html bm203.png
	static const uint8 Hatch22F1[8];  

/// The static array Hatch13F1[8] holds a hatched brush pattern of one pixel on and
/// three pixels off in forward diagonal hatch marks, offset one per row as the
/// following pattern illustrates:
/// \image html bm192.png
	static const uint8 Hatch13F1[8];  

/// The static array Hatch11F1[8] holds the logical hatched brush pattern of one
/// pixel on and one pixel off in monochrome, offset one per row as the following
/// pattern illustrates:
/// \image html bm182.png
	static const uint8 Hatch11F1[8];

/// The static array Hatch22B1[8] holds a hatched brush pattern of two pixels on and
/// two off in backward diagonal hatch marks, offset one per row as the following
/// pattern illustrates:
/// \image html bm198.png
	static const uint8 Hatch22B1[8];  

/// The static array Hatch13B1[8] holds a hatched brush pattern of one pixel on and
/// three pixels off in backward diagonal hatch marks, offset one per row as the
/// following pattern illustrates:
/// \image html bm187.png
	static const uint8 Hatch13B1[8];  

private:
	static HBRUSH Create(const uint8 hatch[], const TColor& fgColor, const TColor& bgColor);
};

//
/// \class TFont
// ~~~~~ ~~~~~
/// TFont derived from TGdiObject provides constructors for creating font objects
/// from explicit information or indirectly.
//
class _OWLCLASS TFont : public TGdiObject {
public:
	// Class scoped types
	//
	typedef HFONT THandle;  /// TFont encapsulates an HFONT

	// Constructors
	//
	TFont(HFONT handle, TAutoDelete autoDelete = NoAutoDelete);

	// Convenient font ctor
	//
	TFont(const tstring& facename = tstring(),
		int height=0, int width=0, int escapement=0, int orientation=0,
		int weight=FW_NORMAL,
		uint8 pitchAndFamily=DEFAULT_PITCH|FF_DONTCARE,
		uint8 italic=false, uint8 underline=false, uint8 strikeout=false,
		uint8 charSet=1,  // DEFAULT_CHARSET or UNICODE_CHARSET
		uint8 outputPrecision=OUT_DEFAULT_PRECIS,
		uint8 clipPrecision=CLIP_DEFAULT_PRECIS,
		uint8 quality=DEFAULT_QUALITY);

	// CreateFont() matching font ctor
	//
	TFont(int height, int width, int escapement=0, int orientation=0,
		int weight=FW_NORMAL,
		uint8 italic=false, uint8 underline=false, uint8 strikeout=false,
		uint8 charSet=1,  // DEFAULT_CHARSET or UNICODE_CHARSET
		uint8 outputPrecision=OUT_DEFAULT_PRECIS,
		uint8 clipPrecision=CLIP_DEFAULT_PRECIS,
		uint8 quality=DEFAULT_QUALITY,
		uint8 pitchAndFamily=DEFAULT_PITCH|FF_DONTCARE,
		const tstring& facename = tstring());

  TFont(const LOGFONT& logFont);
	TFont(const TFont& src);

	// Type Accessor & Conversion Operator
	//
	HFONT       GetHandle() const;
	operator    HFONT() const;
  operator    LOGFONT() const {return GetObject();}

	// Retrieves info about this font when selected in the specified dc
	//
	TEXTMETRIC  GetTextMetrics(TDC& dc) const;
	void        GetTextMetrics(TEXTMETRIC& tm, TDC& dc) const;

	// Retrieves info about this font when selected in a screen DC
	//
	TEXTMETRIC  GetTextMetrics() const;
	void        GetTextMetrics(TEXTMETRIC& tm) const;

	// Retrieves specific attribute of font
	//
	int         GetHeight() const;
	int         GetHeight(TDC& dc) const;
	int         GetAveWidth() const;
	int         GetAveWidth(TDC& dc) const;
	int         GetMaxWidth() const;
	int         GetMaxWidth(TDC& dc) const;
	TSize       GetTextExtent(const tstring& text) const;
	TSize       GetTextExtent(TDC& dc, const tstring& text) const;
	tstring  GetFaceName() const;
	uint8        GetPitchAndFamily() const;

  LOGFONT GetObject() const;

#if defined(OWL5_COMPAT)

	TFont(const LOGFONT* logFont);
	bool GetObject(LOGFONT& logFont) const;

#endif

private:
	TFont& operator =(const TFont&);
};

//
/// \class TDefaultGUIFont
// ~~~~~ ~~~~~~~~~~~~~~~
/// Derived from TFont, TDefaultGUIFont encapsulates the font used for user
/// interface objects such as menus and dialog boxes.
//
class _OWLCLASS TDefaultGUIFont : public TFont {
public:
	TDefaultGUIFont();
};

//
/// \class TPalette
// ~~~~~ ~~~~~~~~
/// TPalette is the GDI Palette class derived from TGdiObject.  The TPalette
/// constructors can create palettes from explicit information or indirectly from
/// various color table types that are used by DIBs.
//
class _OWLCLASS TPalette : public TGdiObject {
public:
	// Class scoped types
	//
	typedef HPALETTE THandle;  ///< TPalette encapsulates an HPALETTE

	// Constructors
	//
	TPalette(HPALETTE handle, TAutoDelete autoDelete = NoAutoDelete);
	TPalette(const TClipboard&);
	TPalette(const TPalette& src);          ///< Deep copy whole palette
	TPalette(const LOGPALETTE& logPalette);
	TPalette(const PALETTEENTRY* entries, int count);
	TPalette(const BITMAPINFO& info, uint flags=0);    ///< Win 3.0 DIB hdr
	TPalette(const TDib& dib, uint flags=0);               ///<  DIB object
	TPalette(const tchar * fileName);  ///< read from file: *.dib,*.bmp,*.pal,*.aco,*.act

  //
  /// Creates a TPalette object with the entries from the given array.
  //
  template <size_t N>
  TPalette(const PALETTEENTRY (&entries)[N])
  {Init(&entries[0], static_cast<int>(N));}

	// Type Accessor & Conversion Operator
	//
	HPALETTE GetHandle() const;
	operator HPALETTE() const;

	// Palette functions
	//
	bool        ResizePalette(uint numEntries);
	void        AnimatePalette(uint start, uint count, const PALETTEENTRY * entries);
	uint        SetPaletteEntries(uint16 start, uint16 count, const PALETTEENTRY * entries);
	uint        SetPaletteEntry(uint16 index, const PALETTEENTRY & entry);
	uint        GetPaletteEntries(uint16 start, uint16 count, PALETTEENTRY * entries) const;
	uint        GetPaletteEntry(uint16 index, PALETTEENTRY & entry) const;
	uint        GetNearestPaletteIndex(const TColor& color) const;
	bool        GetObject(uint16 & numEntries) const;
	uint16      GetNumEntries() const;

	// Put this palette onto the clipboard
	//
	void        ToClipboard(TClipboard& Clipboard);
	bool        UnrealizeObject();

	// Write this palette into file: *.dib,*.bmp,*.pal,*.aco,*.act
	//
	bool        Write(const tchar * fileName);

#if defined(OWL5_COMPAT)

	TPalette(const LOGPALETTE* logPalette);
	TPalette(const BITMAPINFO* info, uint flags=0);    ///< Win 3.0 DIB hdr

#endif

protected:
	void        Create(const BITMAPINFO * info, uint flags);

	/// Read palette from file.
	bool        Read(const tchar * fileName);

	/// Read Microsoft palette from file.
	bool         Read_PAL(TRiffFile& file);
	/// Write Microsoft palette to file.
	bool         Write_PAL(TRiffFile& file);

	/// Read Adobe palette from file.
	bool         Read_ACO(TRiffFile& file);
	/// Write Adobe palette to file.
	bool         Write_ACO(TRiffFile& file);

	/// Read Adobe palette from file.
	bool         Read_ACT(TRiffFile& file);
	/// Write Adobe palette to file.
	bool         Write_ACT(TRiffFile& file);

	/// Read palette from BMP file.
	bool         Read_BMP(TRiffFile& file);
	/// Write palette to BMP 1x1 file.
	bool         Write_BMP(TRiffFile& file);

private:
	TPalette& operator =(const TPalette&);
  void Init(const PALETTEENTRY* entries, int count);
};

//
/// \class TBitmap
// ~~~~~ ~~~~~~~
/// TBitmap is the GDI bitmap class derived from TGdiObject. TBitMap can construct a
/// bitmap from many sources. TBitmap objects are DDBs (device-dependent bitmaps),
/// which are different from the DIBs (device-independent bitmaps) represented by
/// TDib objects.  This bitmap is the lowest level object that is actually selected
/// into a DC.
//
class _OWLCLASS TBitmap : public TGdiObject {
public:
	// Class scoped types
	//
	typedef HBITMAP THandle;  ///< TBitmap encapsulates an HBITMAP

	// Constructors
	//
	TBitmap(HBITMAP handle, TAutoDelete autoDelete = NoAutoDelete);
	TBitmap(const TClipboard& clipboard);
	TBitmap(const TBitmap& bitmap);
	TBitmap(int width, int height, uint8 planes=1, uint8 bitCount=1, const void * bits=0);
	TBitmap(const BITMAP& bitmap);
	TBitmap(const TDC&, int width, int height, bool discardable = false);
	TBitmap(const TDC&, const TDib& dib, uint32 usage=CBM_INIT);
	TBitmap(const TMetaFilePict& metaFile, TPalette& palette, const TSize& size);
	TBitmap(const TDib& dib, const TPalette* palette = 0);

	TBitmap(HINSTANCE, TResId);
	~TBitmap();

	// Type Accessor & Conversion Operator
	//
	HBITMAP  GetHandle() const;
	operator HBITMAP() const;

	// Get/set GDI Object information
	//
	bool        GetObject(BITMAP & bitmap) const;
	int         Width() const;
	int         Height() const;
	TSize       Size() const;
	int         Planes() const;
	int         BitsPixel() const;
	uint32      GetBitmapBits(uint32 count, void * bits) const;
	uint32      SetBitmapBits(uint32 count, const void * bits);
	bool        GetBitmapDimension(TSize& size) const;
	bool        SetBitmapDimension(const TSize& size, TSize * oldSize=0);

	// Put this bitmap onto the clipboard
	//
	void  ToClipboard(TClipboard& clipboard);

	
#if defined(OWL5_COMPAT)

  TBitmap(const BITMAP* bitmap);

#endif

protected:
	TBitmap();

	// Create a bitmap & fill in it's Handle
	//
	void Create(const TDib& dib, const TPalette& palette);
	void Create(const TBitmap& src);

private:
	TBitmap& operator =(const TBitmap&);
};

//
/// \class TRegion
// ~~~~~ ~~~~~~~
/// TRegion, derived from TGdiObject, represents GDI abstract shapes or regions.
/// TRegion can construct region objects with various shapes. Several operators are
/// provided for combining and comparing regions.
//
class _OWLCLASS TRegion : private TGdiBase {
public:
	// Class scoped types
	//
	typedef HRGN THandle;  ///< TRegion encapsulates an HRGN

	// Constructors
	//
	TRegion();
	TRegion(HRGN handle, TAutoDelete autoDelete = NoAutoDelete);
	TRegion(const TRegion& region);
	TRegion(const TRect& rect);
	
/// Defines the class-specific constant Ellipse, used to distinguish the ellipse
/// constructor from the rectangle copy constructor.
	enum TEllipse {Ellipse};
	
	TRegion(const TRect& e, TEllipse);
	TRegion(const TRect& rect, const TSize& corner);
	TRegion(const TPoint* points, int count, int fillMode);
	TRegion(const TPoint* points, const int* polyCounts, int count, int fillMode);

  //
  /// Creates a filled TRegion object from the polygon given by an array of points and fillMode.
  //
  template <size_t N>
  TRegion(const TPoint (&points)[N], int fillMode)
  {Init(&points[0], static_cast<int>(N), fillMode);}

  //
  /// Creates a filled TRegion object from the poly-polygons given by points and fillMode.
  /// The 'points' argument should point into an array of points for all the polygons, and 
  /// polyCounts should contain the number of points in each polygon.
  //
  template <size_t N>
	TRegion(const TPoint* points, const int (&polyCounts)[N], int fillMode)
  {Init(points, &polyCounts[0], static_cast<int>(N), fillMode);}

	~TRegion();

	// Other initialization
	//
	void        SetRectRgn(const TRect& rect);

	// Type Accessor & Conversion Operator
	//
	HRGN     GetHandle() const;
	operator HRGN() const;

	// Test and information functions/operators
	//
	bool        operator ==(const TRegion& other) const;
	bool        operator !=(const TRegion& other) const;
	bool        Contains(const TPoint& point) const;
	bool        Touches(const TRect& rect) const;
	int         GetRgnBox(TRect& box) const;
	TRect       GetRgnBox() const;

	// Assignment operators
	//
	TRegion&    operator =(const TRegion& source);
	TRegion&    operator +=(const TSize& delta);
	TRegion&    operator -=(const TSize& delta);
	TRegion&    operator -=(const TRegion& source);
	TRegion&    operator &=(const TRegion& source);
	TRegion&    operator &=(const TRect& source);
	TRegion&    operator |=(const TRegion& source);
	TRegion&    operator |=(const TRect& source);
	TRegion&    operator ^=(const TRegion& source);
	TRegion&    operator ^=(const TRect& source);

private:

  void Init(const TPoint* points, int count, int fillMode);
  void Init(const TPoint* points, const int* polyCounts, int count, int fillMode);
};

//
/// \class TIcon
// ~~~~~ ~~~~~
/// TIcon, derived from TGdiObject, represents the GDI object icon class. TIcon
/// constructors can create icons from a resource or from explicit information.
/// Because icons are not real GDI objects, the TIcon destructor overloads the base
/// destructor, ~TGdiObject.
//
class _OWLCLASS TIcon : private TGdiBase {
public:
	// Class scoped types
	//
	typedef HICON THandle;  ///< TIcon encapsulates an HICON

	// Constructors
	//
	TIcon(HICON handle, TAutoDelete autoDelete = NoAutoDelete);
	TIcon(HINSTANCE, const TIcon& icon);
	TIcon(HINSTANCE, TResId);
	TIcon(HINSTANCE, const tstring& filename, int index);
	TIcon(HINSTANCE, const TSize& size, int planes, int bitsPixel,
		const void * andBits, const void * xorBits);
	TIcon(const void* resBits, uint32 resSize);
	TIcon(const ICONINFO& iconInfo);
	~TIcon();

	// Type Accessor & Conversion Operator
	//
	HICON    GetHandle() const;
	operator HICON() const;

	bool     operator ==(const TIcon& other) const;

	ICONINFO GetIconInfo() const;

#if defined(OWL5_COMPAT)

	TIcon(const ICONINFO* iconInfo);
	bool GetIconInfo(ICONINFO* iconInfo) const;

#endif

private:
	TIcon(const TIcon&); // Protect against copying of icons
	TIcon& operator =(const TIcon&);

};

//
/// \class TCursor
// ~~~~~ ~~~~~~~
/// TCursor, derived from TGdiBase, represents the GDI cursor object class. TCursor
/// constructors can create cursors from a resource or from explicit information.
/// Because cursors are not real GDI objects, the TCursor destructor overrides the
/// base destructor, ~TGdiBase.
//
class _OWLCLASS TCursor : public TGdiBase {
public:
	// Class scoped types
	//
	typedef HCURSOR THandle;  ///< TCursor encapsulates an HCURSOR

	// Constructors
	//
	TCursor(HCURSOR handle, TAutoDelete autoDelete = NoAutoDelete);
	TCursor(HINSTANCE, const TCursor& cursor);
	TCursor(HINSTANCE, TResId);
	TCursor(HINSTANCE, const TPoint& hotSpot,
		const TSize& size, void * andBits, void * xorBits);
	TCursor(const void* resBits, uint32 resSize);
	TCursor(const ICONINFO& iconInfo);
	~TCursor();

	// Type Accessor & Conversion Operator
	//
	HCURSOR  GetHandle() const;
	operator HCURSOR() const;

	bool     operator ==(const TCursor& other) const;

	ICONINFO GetIconInfo() const;

#if defined(OWL5_COMPAT)

	TCursor(const ICONINFO* iconInfo);
	bool GetIconInfo(ICONINFO* iconInfo) const;

#endif

private:
	TCursor(const TCursor&); // Protect against copying of cursors
	TCursor& operator =(const TCursor&);

};

//
/// \class TDib
// ~~~~~ ~~~~
/// Pseudo-GDI object Device Independent Bitmap (DIB) class.  DIBs really have
/// no Window's handle, they are just a structure containing format and palette
/// information and a collection of bits (pixels).  This class provides a very
/// convenient way to work with DIBs like any other GDI object.
/// The memory for the DIB is in one GlobalAlloc'd chunk so it can be passed to
/// the Clipboard, OLE, etc.
/// Overloads the destructor since it is not a real GDI object.
///
/// This is what is really inside a .BMP file, what is in bitmap resources, and
/// what is put on the clipboard as a DIB.
//
class _OWLCLASS TDib : private TGdiBase {
public:
	// Class scoped types
	//
	typedef HANDLE THandle;  ///< TDib encapsulates an memory HANDLE w/ a DIB

	// Constructors and destructor
	//
	TDib(HGLOBAL handle, TAutoDelete autoDelete = NoAutoDelete);
	TDib(const TClipboard& clipboard);
	TDib(const TDib& src);

	TDib(int width, int height, uint32 nColors, uint16 mode=DIB_RGB_COLORS);
	TDib(HINSTANCE module, TResId resid);
	TDib(LPCTSTR filename);
	TDib(const tstring& filename);
	TDib(TFile& file, bool readFileHeader = true);
	TDib(std::istream& is, bool readFileHeader = false);
	TDib(const TBitmap& bitmap, const TPalette* pal = 0);
	virtual ~TDib();

	// Comparison operator
	//
	bool     operator ==(const TDib& other) const;

	// Access to the internal structures of the dib
	//
	const BITMAPINFO *      GetInfo() const;
	BITMAPINFO *            GetInfo();
	const BITMAPINFOHEADER *GetInfoHeader() const;
	BITMAPINFOHEADER *      GetInfoHeader();
	const TRgbQuad *        GetColors() const;
	TRgbQuad *              GetColors();
	const uint16 *          GetIndices() const;
	uint16 *                GetIndices();
	const void *           GetBits() const;
	void *                 GetBits();

	// Type convert this dib by returning pointers to internal structures
	//
	HANDLE   GetHandle() const;
	operator HANDLE() const;

	operator const BITMAPINFO *() const;
	operator BITMAPINFO *();
	operator const BITMAPINFOHEADER *() const;
	operator BITMAPINFOHEADER *();
	operator const TRgbQuad *() const;
	operator TRgbQuad *();

	// Put this Dib onto the clipboard
	//
	void      ToClipboard(TClipboard& clipboard);

	// Get info about this Dib
	//
	bool      IsOK() const;         // Is DIB OK & info available
	bool      IsPM() const;         // Is DIB stored in PM core format
	int       Width() const;        // Width in pixels
	int       Height() const;       // Height in pixels
	int       FlippedY(int y) const;// Y flipped the other direction
	TSize     Size() const;         // Width & Height in pixels
	int       BitsPixel() const;    // Bits per pixel: 2, 4, 8, 16, 24, 32
	int       Pitch() const;        // Width in bytes, or bytes per scan
	uint32    Compression() const;  // Compression & encoding flags
	uint32    SizeImage() const;    // Size of the DIB image bits in bytes

	long      NumColors() const;    // Number of colors in color table
	uint      StartScan() const;    // Starting scan line
	uint      NumScans() const;     // Number of scans
	int32     SizeColors() const;   // Size of the color table in bytes.
	uint32    SizeDib() const;      // Memory size of DIB in bytes
	uint16    Usage() const;        // The mode or usage of the color table

	int       XOffset(uint16 x) const;
	int       YOffset(uint16 y) const;

	void * PixelPtr(uint16 x, uint16 y);

	// Write this dib to a file by name, to file object or to an ostream
	//
	bool      WriteFile(const tstring& filename);
	bool      Write(TFile& file, bool writeFileHeader = false);
	bool      Write(std::ostream& os, bool writeFileHeader = false);

	// Work with the color table in RGB mode
	//
	TColor    GetColor(int entry) const;
	void      SetColor(int entry, const TColor& color);
	int       FindColor(const TColor& color);
	int       MapColor(const TColor& fromColor, const TColor& toColor, bool doAll = false);

	// Work with the color table in Palette relative mode
	//
	uint16    GetIndex(int entry) const;
	void      SetIndex(int entry, uint16 index);
	int       FindIndex(uint16 index);
	int       MapIndex(uint16 fromIndex, uint16 toIndex, bool doAll = false);

	// Change from RGB to Palette, or from Palette to RGB color table mode
	//
	bool      ChangeModeToPal(const TPalette& pal);
	bool      ChangeModeToRGB(const TPalette& pal);

	/// Map colors in color table matching stock UI colors to current UI colors
	//
	/// Enumerates the values for the part of the window whose color is to be set. You
	/// can OR these together to control the colors used for face shading on push
	/// buttons, the color of a selected control button, the edge shading on push
	/// buttons, text on push buttons, the color of the window frame, and the background
	/// color of the various parts of a window. The function MapUIColors uses one of
	/// these values to map the colors of various parts of a window to a specified
	/// color.
	//
	enum {
		MapFace      = 0x01,  // Or these together to control colors to map
		MapText      = 0x02,  // to current SysColor values
		MapShadow    = 0x04,
		MapHighlight = 0x08,
		MapFrame     = 0x10
	};
	void      MapUIColors(uint mapColors, const TColor* bkColor = 0);


	void MapToPalette(const TPalette& pal);

	/// Internal DIB file Reda/Write functions talk to these interfaces
	//
	class _OWLCLASS IFileIn {
	public:
		virtual long Read(void * buffer, long size) = 0;
		virtual void Skip(long size) = 0;
	};
	class _OWLCLASS IFileOut {
	public:
		virtual bool Write(void * buffer, long size) = 0;
	};

protected:
	using TGdiBase::CheckValid;  // make this function available to derivatives
	using TGdiBase::Handle;      // and these members too
	using TGdiBase::ShouldDelete;

	/// Protected ctor- derived classes need to fill in handle & get info
	//
	TDib();

	void      InfoFromHeader(const BITMAPINFOHEADER& infoHeader);
	void      InfoFromHandle();  // Get info & members from handle
	void      CopyOnWrite();     // Make sure we can write on info
	void      ResToMemHandle();  // Perform RO res handle to mem handle copy

	bool      ReadFile(const tstring& name);
	bool      Read(TFile& file, bool readFileHeader = false);
	bool      Read(std::istream& is, bool readFileHeader = false);

	bool      LoadResource(HINSTANCE, TResId);

	// virtuals for derived classes 
	virtual bool Read(IFileIn& in, bool readFileHeader = false);
	virtual bool Write(IFileOut& out, bool writeFileHeader = false);

	static int ScanBytes(long w, int bpp); ///< Width+bpp to dword aligned bytes

protected_data:
	BITMAPINFO * Info;       ///< Locked global alloc'd block, has sub ptrs:
	uint32 *     Mask;       ///<   Color mask[3] for 16 & 32 bpp bmps
	TRgbQuad *   Colors;     ///<   Color table[NumClrs] for any bmp
	void *      Bits;       ///<   Pointer to bits (pixels)
	long            NumClrs;    ///< Number of colors in color table
	uint16          Mode;       ///< Palette or RGB based pixels
	bool            IsResHandle;///< Is Handle a resource handle (vs. Memory)

private:
	// Prevent accidental copying of object
	//
	TDib& operator =(const TDib&);

};

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

//----------------------------------------------------------------------------
// Inline implementations for abstract GDI object class and derived classes.
//

#include <owl/dc.h>
#include <owl/clipboar.h>

namespace owl {

//
/// Get the palette object from the clipboard.
//
inline TClipboard& operator <<(TClipboard& clipboard, TPalette& palette)
{
	palette.ToClipboard(clipboard);
	return clipboard;
}

//
/// Get the bitmap object from the clipboard.
//
inline TClipboard& operator <<(TClipboard& clipboard, TBitmap& bitmap)
{
	bitmap.ToClipboard(clipboard);
	return clipboard;
}

//
/// Get the DIB from the clipboard.
//
inline TClipboard& operator <<(TClipboard& clipboard, TDib& dib)
{
	dib.ToClipboard(clipboard);
	return clipboard;
}

//
/// Returns the handle of the GDI object.
//
inline HGDIOBJ TGdiObject::GetGdiHandle() const
{
	return HGDIOBJ(Handle);
}

//
/// Typecasting operator that converts this GDI object handle to type HGDIOBJ.
//
inline TGdiObject::operator HGDIOBJ() const
{
	return GetGdiHandle();
}

//
/// Returns true if the handles are equal.  This is a binary compare.
//
inline bool TGdiObject::operator ==(const TGdiObject& other) const
{
	return Handle == other.Handle;
}

//
/// Retrieve the object's attributes into a buffer.
//
/// Obtains information about this GDI object and places it in the object buffer. If
/// the call succeeds and object is not 0, GetObject returns the number of bytes
/// copied to the object buffer. If the call succeeds and object is 0, GetObject
/// returns the number of bytes needed in the object buffer for the type of object
/// being queried. Depending on what type of GDI object is derived, this function
/// retrieves a LOGPEN, LOGBRUSH, LOGFONT, or BITMAP structure through object.
//
inline int TGdiObject::GetObject(int count, void * object) const
{
# if defined(UNICODE)
	return ::GetObjectW(Handle, count, object);
#  else
	return ::GetObjectA(Handle, count, object);
#  endif
}

//
/// Returns the type of the GDI object.
//
inline uint32 TGdiObject::GetObjectType() const
{
	return ::GetObjectType(GetGdiHandle());
}

//
/// Returns true if this represents a real GDI object.
//
inline bool TGdiObject::IsGDIObject() const
{
	return GetObjectType() != 0;
}

//
/// Returns the handle of the pen with type HPEN.
//
inline HPEN TPen::GetHandle() const
{
	return HPEN(GetGdiHandle());
}

//
/// Typecasting operator. Converts this pen's Handle to type HPEN (the data type
/// representing the handle to a logical pen).
//
inline TPen::operator HPEN() const
{
	return GetHandle();
}

#if defined(OWL5_COMPAT)

//
/// Retrieves information about this pen object and places it in the given LOGPEN
/// structure. Returns true if the call is successful, otherwise false.
/// This overload is deprecated. Use the overload that returns a LOGPEN instead.
//
inline bool TPen::GetObject(LOGPEN & logPen) const
{
	return TGdiObject::GetObject(sizeof(logPen), &logPen) != 0;
}

#endif

//
/// Returns the handle of the brush with type HBRUSH.
//
inline HBRUSH TBrush::GetHandle() const
{
	return HBRUSH(GetGdiHandle());
}

//
/// Typecasting operator. Converts this brush's Handle to type HBRUSH (the data type
/// representing the handle to a physical brush).
//
inline TBrush::operator HBRUSH() const
{
	return GetHandle();
}

#if defined(OWL5_COMPAT)

//
/// Retrieves information about this brush object and places it in the given
/// LOGBRUSH structure. Returns true if the call is successful; otherwise returns false.
/// This overload is deprecated. Use the overload that returns a LOGBRUSH instead.
//
inline bool TBrush::GetObject(LOGBRUSH & logBrush) const
{
	return TGdiObject::GetObject(sizeof(logBrush), &logBrush) != 0;
}

#endif


//
/// Returns the handle of the font with type HFONT.
//
inline HFONT TFont::GetHandle() const
{
	return HFONT(GetGdiHandle());
}

//
/// Typecasting operator that converts this font's Handle to type HFONT (the data
/// type representing the handle to a physical font).
//
inline TFont::operator HFONT() const
{
	return GetHandle();
}

#if defined(OWL5_COMPAT)

//
/// Retrieves information about this font object and places it in the given LOGFONT
/// structure. Returns true if successful and false if unsuccessful.
/// This overload is deprecated; use the overload that returns a LOGFONT instead.
//
inline bool TFont::GetObject(LOGFONT & logFont) const
{
	return TGdiObject::GetObject(sizeof(logFont), &logFont) != 0;
}

#endif

//
/// Returns the height of the font.
//
inline int TFont::GetHeight() const
{
	return int(GetTextMetrics().tmHeight);
}

//
/// Returns the height of the font if selected into the DC.
//
inline int TFont::GetHeight(TDC& dc) const
{
	return int(GetTextMetrics(dc).tmHeight);
}

//
/// Returns the average width of the characters in the font.
//
inline int TFont::GetAveWidth() const
{
	return int(GetTextMetrics().tmAveCharWidth);
}

//
/// Returns the average width of the characters in the font if selected into the DC.
//
inline int TFont::GetAveWidth(TDC& dc) const
{
	return int(GetTextMetrics(dc).tmAveCharWidth);
}

//
/// Returns the maximum width of the characters in the font.
//
inline int TFont::GetMaxWidth() const
{
	return int(GetTextMetrics().tmMaxCharWidth);
}

//
/// Returns the maximum width of the characters in the font if selected into
/// the DC.
//
inline int TFont::GetMaxWidth(TDC& dc) const
{
	return int(GetTextMetrics(dc).tmMaxCharWidth);
}

//
/// Returns the handle of the palette.
//
inline HPALETTE TPalette::GetHandle() const
{
	return HPALETTE(GetGdiHandle());
}

//
/// Typecasting operator. Converts this palette's Handle to type HPALETTE, which is
/// the data type representing the handle to a logical palette.
//
inline TPalette::operator HPALETTE() const
{
	return GetHandle();
}

//
/// Directs the GDI to completely remap the logical palette to the system palette on
/// the next RealizePalette(HDC) or TDC::RealizePalette call. Returns true if the
/// call is successful; otherwise false.
//
inline bool TPalette::UnrealizeObject()
{
	return ::UnrealizeObject(Handle);
}

//
/// Changes the size of this logical palette to the number given by numEntries.
/// Returns true if the call is successful; otherwise returns false.
//
inline bool TPalette::ResizePalette(uint numEntries)
{
	return ::ResizePalette(GetHandle(), numEntries);
}

//
/// Animate palette from entry 'start' for 'count' entries.
//
/// Replaces entries in this logical palette from the entries array of PALETTEENTRY
/// structures. The parameter start specifies the first entry to be animated, and
/// count gives the number of entries to be animated. The new entries are mapped
/// into the system palette immediately.
//
inline void TPalette::AnimatePalette(uint start, uint count, const PALETTEENTRY * entries)
{
	::AnimatePalette(GetHandle(), start, count, entries);
}

//
/// Sets the RGB color values in this palette from the entries array of PALETTEENTRY
/// structures. The start parameter specifies the first entry to be animated, and
/// count gives the number of entries to be animated. Returns the number of entries
/// actually set, or 0 if the call fails.
//
inline uint TPalette::SetPaletteEntries(uint16 start, uint16 count, const PALETTEENTRY * entries)
{
	//JJH - this is not very nice workaround... :o(, but good ones seems to be expensive
#ifdef WINELIB
	return ::SetPaletteEntries(GetHandle(), start, count, (PALETTEENTRY *)entries);
#else
	return ::SetPaletteEntries(GetHandle(), start, count, entries);
#endif
}

//
/// Sets the RGB color value at index in this palette from the entry argument. The
/// start parameter specifies the first entry to be animated, and count gives the
/// number of entries to be animated. Returns 1, the number of entries actually set
/// if successful, or 0 if the call fails.
//
inline uint TPalette::SetPaletteEntry(uint16 index, const PALETTEENTRY & entry)
{
	//JJH - this is not very nice workaround... :o(, but good ones seems to be expensive
#ifdef WINELIB
	return ::SetPaletteEntries(GetHandle(), index, 1, (PALETTEENTRY *)&entry);
#else
	return ::SetPaletteEntries(GetHandle(), index, 1, &entry);
#endif
}

//
/// Retrieves a range of entries in this logical palette and places them in the
/// entries array. The start parameter specifies the first entry to be retrieved,
/// and count gives the number of entries to be retrieved. Returns the number of
/// entries actually retrieved, or 0 if the call fails.
//
inline uint TPalette::GetPaletteEntries(uint16 start, uint16 count, PALETTEENTRY * entries) const
{
	return ::GetPaletteEntries(GetHandle(), start, count, entries);
}

//
/// Retrieves the entry in this logical palette at index and places it in the
/// entries array. Returns the number of entries actually retrieved: 1 if successful
/// or 0 if the call fails.
//
inline uint TPalette::GetPaletteEntry(uint16 index, PALETTEENTRY & entry) const
{
	return ::GetPaletteEntries(GetHandle(), index, 1, &entry);
}

//
/// Returns the index of the color entry that represents the closest color in this
/// palette to the given color.
//
inline uint TPalette::GetNearestPaletteIndex(const TColor& Color) const
{
	return ::GetNearestPaletteIndex(GetHandle(), Color);
}

//
/// Finds the number of entries in this logical palette and sets the value in the
/// numEntries argument. To find the entire LOGPALETTE structure, use
/// GetPaletteEntries. Returns true if the call is successful; otherwise returns
/// false.
//
inline bool TPalette::GetObject(uint16 & numEntries) const
{
	return TGdiObject::GetObject(sizeof(numEntries), &numEntries);
}

//
/// Returns the number of entries in this palette, or 0 if the call fails.
//
inline uint16 TPalette::GetNumEntries() const
{
	uint16 numEntries;
	if (TGdiObject::GetObject(sizeof(numEntries), &numEntries))
		return numEntries;
	return 0;
}

//
/// Returns the handle of the bitmap of type BITMAP.
//
inline HBITMAP TBitmap::GetHandle() const
{
	return HBITMAP(GetGdiHandle());
}

//
/// Typecasting operator. Converts this bitmap's Handle to type HBITMAP (the data
/// type representing the handle to a physical bitmap).
//
inline TBitmap::operator HBITMAP() const
{
	return GetHandle();
}

//
/// Copies up to count bits of this bitmap to the buffer bits.
//
inline uint32 TBitmap::GetBitmapBits(uint32 count, void * bits) const
{
	return ::GetBitmapBits(GetHandle(), count, bits);
}

//
/// Copies up to count bits from the bits buffer to this bitmap.
//
inline uint32 TBitmap::SetBitmapBits(uint32 count, const void * bits)
{
	return ::SetBitmapBits(GetHandle(), count, bits);
}

//
/// Retrieves the size of this bitmap (width and height, measured in tenths of
/// millimeters) and sets it in the size argument. Returns true if the call is
/// successful; otherwise returns false.
//
inline bool TBitmap::GetBitmapDimension(TSize& size) const
{
	return ::GetBitmapDimensionEx(GetHandle(), &size);
}

//
/// Sets the size of this bitmap from the given size argument (width and height,
/// measured in tenths of millimeters). The previous size is set in the oldSize
/// argument. Returns true if the call is successful; otherwise returns false.
//
inline bool TBitmap::SetBitmapDimension(const TSize& size, TSize * oldSize)
{
	return ::SetBitmapDimensionEx(GetHandle(), size.cx, size.cy, oldSize);
}

//
/// Retrieves data (width, height, and color format) for this bitmap and sets it in
/// the given BITMAP structure. To retrieve the bit pattern, use GetBitmapBits.
//
inline bool TBitmap::GetObject(BITMAP & Bitmap) const
{
	return TGdiObject::GetObject(sizeof(Bitmap), &Bitmap) != 0;
}

//
/// Returns the handle of the region with type HREGION.
//
inline HRGN TRegion::GetHandle() const{
	return HRGN(Handle);
}

//
/// Typecast operator. HRGN is the data type representing the handle to a physical
/// region.
//
inline TRegion::operator HRGN() const{
	return GetHandle();
}

//
/// Creates a rectangle of the size given by rect.
//
inline void TRegion::SetRectRgn(const TRect& rect)
{
	::SetRectRgn(GetHandle(), rect.left, rect.top, rect.right, rect.bottom);
}

//
/// Returns true if the regions are identical (equal in size and shape).
//
inline bool TRegion::operator ==(const TRegion& other) const
{
	return ::EqualRgn(GetHandle(), other);
}

//
/// Returns true if this region is not equal to the other region.
//
inline bool TRegion::operator !=(const TRegion& other) const
{
	return !::EqualRgn(GetHandle(), other);
}

//
/// Returns true if this region contains the given point.
//
inline bool TRegion::Contains(const TPoint& point) const
{
	return ::PtInRegion(GetHandle(), point.x, point.y);
}

//
/// Returns true if this region touches the given rectangle.
//
inline bool TRegion::Touches(const TRect& rect) const
{
	return ::RectInRegion(GetHandle(), (TRect*)&rect); // API <const> typecast
}

//
/// Finds the bounding rectangle (the minimum rectangle containing this region). The
/// resulting rectangle is placed in box and the returned values are as follows:
/// - \c \b  COMPLEXREGION	Region has overlapping borders.
/// - \c \b  NULLREGION	Region is empty.
/// - \c \b  SIMPLEREGION	Region has no overlapping borders.
//
inline int TRegion::GetRgnBox(TRect& box) const
{
	return ::GetRgnBox(GetHandle(), &box);
}

//
/// Returns the bounding rectangle of the region.
//
inline TRect TRegion::GetRgnBox() const
{
	TRect box;
	::GetRgnBox(GetHandle(), &box);
	return box;
}

//
/// Assigns the source region to this region. A reference to the result is returned,
/// allowing chained assignments.
//
inline TRegion& TRegion::operator =(const TRegion& source)
{
	::CombineRgn(GetHandle(), source, 0, RGN_COPY);
	return *this;
}

//
/// Adds the given delta to each point of this region to displace (translate) it by
/// delta.x and delta.y. Returns a reference to the resulting region.
//
inline TRegion& TRegion::operator +=(const TSize& delta)
{
	::OffsetRgn(GetHandle(), delta.cx, delta.cy);
	return *this;
}

//
/// Subtracts the given delta from each point of this region to
/// displace (translate) it by -delta.x and -delta.y. Returns a reference to the resulting region.
//
inline TRegion& TRegion::operator -=(const TSize& delta)
{
	::OffsetRgn(GetHandle(), -delta.cx, -delta.cy);
	return *this;
}

//
/// Creates a "difference" region consisting of all parts of this region that are not parts of
/// the source region. Returns a reference to the resulting region.
//
inline TRegion& TRegion::operator -=(const TRegion& source)
{
	::CombineRgn(GetHandle(), GetHandle(), source, RGN_DIFF);
	return *this;
}

//
/// Creates the intersection of this region with the given source region 
/// and returns a reference to the result.
//
inline TRegion& TRegion::operator &=(const TRegion& source)
{
	::CombineRgn(GetHandle(), GetHandle(), source, RGN_AND);
	return *this;
}

//
/// Creates the union of this region and the given source region , and
/// returns a reference to the result.
//
inline TRegion& TRegion::operator |=(const TRegion& source)
{
	::CombineRgn(GetHandle(), GetHandle(), source, RGN_OR);
	return *this;
}

//
/// Creates the exclusive-or of this region and the given source region. 
/// Returns a reference to the resulting region object.
//
inline TRegion& TRegion::operator ^=(const TRegion& source)
{
	::CombineRgn(GetHandle(), GetHandle(), source, RGN_XOR);
	return *this;
}

//
/// Returns the handle of the icon with type HICON.
//
inline HICON TIcon::GetHandle() const
{
	return HICON(Handle);
}

//
/// Typecasting operator that converts this icon's Handle to type HICON (the data
/// type representing the handle to an icon resource).
//
inline TIcon::operator HICON() const
{
	return GetHandle();
}

//
/// Returns true if the handles of two icons are identical.
//
inline bool TIcon::operator ==(const TIcon& other) const
{
	return Handle == other.Handle;
}

#if defined(OWL5_COMPAT)

//
/// Retrieves information about this icon and copies it into the given ICONINFO
/// structure. Returns true if the call is successful; otherwise returns false.
/// This overload is deprecated. Use the overload that returns ICONINFO instead.
//
inline bool TIcon::GetIconInfo(ICONINFO* IconInfo) const
{
	return ::GetIconInfo(GetHandle(), IconInfo);
}

//
/// Retrieves information about this cursor and copies it into the given ICONINFO
/// structure. Returns true if the call is successful; otherwise returns false.
/// This overload is deprecated. Use the overload that returns ICONINFO instead.
//
inline bool TCursor::GetIconInfo(ICONINFO* IconInfo) const
{
	return ::GetIconInfo((HICON)GetHandle(), IconInfo);
}

#endif

//
/// Returns the handle of the cursor with type HCURSOR.
//
inline HCURSOR TCursor::GetHandle() const
{
	return HCURSOR(Handle);
}

//
/// An inline typecasting operator. Converts this cursor's Handle to type HCURSOR
/// (the data type representing the handle to a cursor resource).
//
inline TCursor::operator HCURSOR() const
{
	return GetHandle();
}

//
/// Returns true if this cursor equals other; otherwise returns false.
//
inline bool TCursor::operator ==(const TCursor& other) const
{
	return Handle == other.Handle;
}

//
///  Compares two handles and returns true if this DIB's handle equals the other
/// (other) DIB's handle.
//
inline bool TDib::operator ==(const TDib& other) const
{
	return Handle == other.Handle;
}

//
/// Returns this DIB's Info field. A DIB's BITMAPINFO structure contains information
/// about the dimensions and color of the DIB and specifies an array of data types
/// that define the colors in the bitmap.
//
inline const BITMAPINFO * TDib::GetInfo() const
{
	return Info;
}

//
/// Returns this DIB's Info field. A DIB's BITMAPINFO structure contains information
/// about the dimensions and color of the DIB and specifies an array of data types
/// that define the colors in the bitmap.
//
inline BITMAPINFO * TDib::GetInfo()
{
	return Info;
}

//
/// Returns this DIB's bmiHeader field of the BITMAPINFO structure contains
/// information about the color and dimensions of this DIB.
//
inline const BITMAPINFOHEADER * TDib::GetInfoHeader() const
{
	return &Info->bmiHeader;
}

//
/// Returns this DIB's bmiHeader field of the BITMAPINFO structure contains
/// information about the color and dimensions of this DIB.
//
inline BITMAPINFOHEADER * TDib::GetInfoHeader()
{
	return &Info->bmiHeader;
}

//
/// Returns the bmiColors value of this DIB.
//
inline const TRgbQuad * TDib::GetColors() const
{
	return Colors;
}

//
/// Returns the bmiColors value of this DIB.
//
inline TRgbQuad * TDib::GetColors()
{
	return Colors;
}

//
/// Returns the bmiColors indexes of this DIB.
//
inline const uint16 * TDib::GetIndices() const
{
	return (const uint16*)Colors;
}

//
/// Returns the bmiColors indexes of this DIB.
//
inline uint16 * TDib::GetIndices()
{
	return (uint16*)Colors;
}

//
/// Returns the Bits data member for this DIB.
//
inline const void * TDib::GetBits() const
{
	return Bits;
}

//
/// Returns the Bits data member for this DIB.
//
inline void * TDib::GetBits()
{
	return Bits;
}

//
/// Returns the handle of the DIB with type HANDLE.
//
inline HANDLE TDib::GetHandle() const
{
	return Handle;
}

//
/// Return the handle of the DIB with type HANDLE.
//
inline TDib::operator HANDLE() const
{
	return GetHandle();
}

//
/// Typecasts this DIB by returning a pointer to its bitmap information structure
/// (BITMAPINFO) which contains information about this DIB's color format and
/// dimensions (size, width, height, resolution, and so on).
//
inline TDib::operator const BITMAPINFO *() const
{
	return GetInfo();
}

//
/// Typecasts this DIB by returning a pointer to its bitmap information structure
/// (BITMAPINFO) which contains information about this DIB's color format and
/// dimensions (size, width, height, resolution, and so on).
//
inline TDib::operator BITMAPINFO *()
{
	return GetInfo();
}

//
/// Typecasts this DIB by returning a pointer to its bitmap info header.
//
inline TDib::operator const BITMAPINFOHEADER *() const
{
	return GetInfoHeader();
}

//
/// Typecasts this DIB by returning a pointer to its bitmap info header.
//
inline TDib::operator BITMAPINFOHEADER *()
{
	return GetInfoHeader();
}

//
/// Typecasts this DIB by returning a pointer to its colors structure.
//
inline TDib::operator const TRgbQuad *() const
{
	return GetColors();
}

//
/// Typecasts this DIB by returning a pointer to its colors structure.
//
inline TDib::operator TRgbQuad *()
{
	return GetColors();
}

//
/// Returns false if Info is 0, otherwise returns true. If there is a problem with
/// the construction of the DIB, memory is freed and Info is set to 0. Therefore,
/// using Info is a reliable way to determine if the DIB is constructed correctly.
//
inline bool TDib::IsOK() const
{
	return Info != 0;
}

//
/// Returns true if IsCore is true; that is, if the DIB is an old-style PM DIB using
/// core headers. Otherwise returns false.
///
/// \note Returns false always since PM bitmaps are not supported.
//
inline bool TDib::IsPM() const
{
	return false;
}

//
/// Returns the width of the DIB.
//
inline int TDib::Width() const
{
	return (int)Info->bmiHeader.biWidth;
}

//
/// Returns the height of the DIB.
//
inline int TDib::Height() const
{
	return (int)Info->bmiHeader.biHeight;
}

//
/// Returns the coordinate of y if the direction of the y-axis was reversed.
//
inline int TDib::FlippedY(int y) const
{
	return int(Info->bmiHeader.biHeight - 1 - y);
}

//
/// Returns TSize(W,H), the size of this DIB.
//
inline TSize TDib::Size() const
{
	return TSize(Width(), Height());
}

//
/// Returns the number of bytes used to store a scanline for the DIB.
/// Rounded up to the nearest 32-bit boundary.
//
inline int TDib::ScanBytes(long w, int bpp)
{
	return (int)((w*bpp+31)&(~31))/8;
}

//
/// Return the number of bits (2, 4, 8, 16, 24, or 32) used to store a pixel for the DIB.
//
inline int TDib::BitsPixel() const
{
	return Info->bmiHeader.biBitCount;
}

//
/// Size of scan in bytes =
///   Pixel Width * bits per pixel rounded up to a uint32 boundary
//
inline int TDib::Pitch() const
{
	return ScanBytes(Width(), BitsPixel());
}

//
/// Returns type of compression and encoding for bottom-up DIBs.
//
inline uint32 TDib::Compression() const
{
	return Info->bmiHeader.biCompression;
}

//
/// Returns number of bytes used to store the image.
//
inline uint32 TDib::SizeImage() const
{
	return Info->bmiHeader.biSizeImage;
}

//
/// Returns NumClrs, the number of colors in this DIB's palette.
//
inline long TDib::NumColors() const
{
	return NumClrs;
}

//
/// Returns the starting scan line.
///
/// Always 0 because all DIBs are normalized.
//
inline uint TDib::StartScan() const
{
	return 0;
}

//
/// Returns the number of scans in this DIB.
///
/// Always same as height of the DIB.
//
inline uint TDib::NumScans() const
{
	return Height();
}

//
/// Return number of colors times the size of each entry in the table,
/// whether it is an RGB color table or palette index table.
//
inline int32 TDib::SizeColors() const
{
	return Mode == DIB_RGB_COLORS ?
		(long)NumColors() * sizeof(RGBQUAD) :  // RGB color table
	(long)NumColors() * sizeof(uint16);    // Palette index color table
}

//
/// Size of dib is measured as the end of the bits minus the start of the block
//
inline uint32 TDib::SizeDib() const
{
	return static_cast<uint32>(((char *)Bits + SizeImage()) - (char *)Info);
}

//
/// Returns the Mode for this DIB. This value tells GDI how to treat the color
/// table - whether the DIB has palette color entries or RGB color entries.
//
inline uint16 TDib::Usage() const
{
	return Mode;
}

//
/// Returns the byte offset from the start of the scan line to the xth pixel.
//
inline int TDib::XOffset(uint16 x) const
{
	return int((uint32)x * BitsPixel() / 8);
}

//
/// Returns the starting position of the scan line.
//
inline int TDib::YOffset(uint16 y) const
{
	return int((uint32)Pitch() * y);
}

//
/// Returns the byte of where the pixel is located.
//
inline void * TDib::PixelPtr(uint16 x, uint16 y)
{
	return (uint8 *)Bits + XOffset(x) + YOffset(y);
}

//
/// Copies the DIB.
//
inline void TDib::CopyOnWrite()
{
	if (IsResHandle)
		ResToMemHandle();
}

//
// Old GDI Orphan control macros
//

/// If orphan control is active (the default), OBJ_REF_ADD(handle, type) is defined
/// as TGdiObject::RefAdd((handle), (type)). The latter adds to the ObjInfoBag table
/// a reference entry for the object with the given handle and type, and sets its
/// count to 1. If orphan control is inactive, OBJ_REF_ADD(handle) is defined as
/// handle. This macro lets you write orphan control code that can be easily
/// deactivated with the single statement \#define NO_GDI_ORPHAN_CONTROL.
#define OBJ_REF_ADD(handle,type) TGdiObject::RefAdd((handle), (type))
	
/// If orphan control is active (the default), OBJ_REF_REMOVE(handle) is defined as
/// TGdiObject::RefRemove((handle)). The latter removes from the ObjInfoBag table
/// the reference entry for the object associated with handle. If orphan control is
/// inactive, OBJ_REF_REMOVE(handle) is defined as handle. This macro lets you write
/// orphan control code that can be easily deactivated with the single statement
/// \#define NO_GDI_ORPHAN_CONTROL.
#define OBJ_REF_REMOVE(handle) TGdiObject::RefRemove(handle)
	
/// If orphan control is active (the default), OBJ_REF_INC(handle) is defined as
/// TGdiObject::RefInc((handle)). The latter increments the reference count of the
/// object associated with handle. If orphan control is inactive,
/// OBJ_REF_DEC(handle) is defined as handle. This macro lets you write orphan
/// control code that can be easily deactivated with the single statement \#define
/// NO_GDI_ORPHAN_CONTROL.
#define OBJ_REF_INC(handle) TGdiObject::RefInc(handle)
	
/// If orphan control is active (the default), OBJ_REF_DEC(handle, wantDelete) is
/// defined as either TGdiObject::RefDec((handle)) or TGdiObject::RefDec((handle),
/// (wantDelete)). The latter format occurs only if _ _TRACE is defined.
/// RefDec(handle) decrements the reference count of the object associated with
/// handle and optionally deletes orphans or warns you of their existence. If orphan
/// control is inactive, OBJ_REF_DEC(handle) is defined as handle. This macro lets
/// you write orphan control code that can be easily deactivated with the single
/// statement \#define NO_GDI_ORPHAN_CONTROL.
#define OBJ_REF_DEC(handle, wantDelete) TGdiObject::RefDec((handle),(wantDelete))
	
/// If orphan control is active (the default), OBJ_REF_COUNT(handle) is defined as
/// TGdiObject::RefCount((handle)). The latter returns the reference count of the
/// object with the given handle, or -1 if no such object exists. If orphan control
/// is inactive, OBJ_REF_COUNT(handle) is defined as -1. This macro lets you write
/// orphan control code that can be easily deactivated with the single statement
/// \#define NO_GDI_ORPHAN_CONTROL.
#define OBJ_REF_COUNT(handle) TGdiObject::RefCount(handle)

} // OWL namespace

#endif  // OWL_GDIOBJEC_H
