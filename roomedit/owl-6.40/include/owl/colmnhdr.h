//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TColumnHeader and helper classes
//----------------------------------------------------------------------------

#if !defined(OWL_COLMNHDR_H)
#define OWL_COLMNHDR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/commctrl.h>
#include <owl/contain.h>
#include <owl/template.h>


namespace owl {

class _OWLCLASS THdrItem;
class _OWLCLASS THdrNotify;
class _OWLCLASS TColumnHeader;

#if !defined(HDI_ALL)   // Mentioned in docs. but missing from headers
# define HDI_ALL (HDI_BITMAP|HDI_FORMAT|HDI_HEIGHT|HDI_LPARAM|HDI_TEXT|HDI_WIDTH)
#endif

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{
/// \class THdrItem
// ~~~~~ ~~~~~~~~
/// THdrItem contains information about an item in a header control.
//
class _OWLCLASS THdrItem : public HD_ITEM {
  public:

    /// \name Constructors to be used for adding, inserting or updating a header control's item.
    /// 'str' specifies the title of the item. Note: The string pointed to must outlive the object!
    /// 'hbm' specifies a handle to a bitmap to be used in the item.
    /// @{

    THdrItem(LPCTSTR str);
    THdrItem(LPCTSTR str, HBITMAP hbm);
    THdrItem(HBITMAP hbm);

    /// @}

    /// Constructors used primarily to construct a 'THdrItem' structure
    /// which will be used to retrieve information about an item within
    /// a header control. 'buffer' specifies the location to receive the
    /// item's title and 'len' indicates the size of that buffer. The 'msk'
    /// flags indicates which members of the structure should be updated.
    /// @{

    THdrItem(LPTSTR buffer, int len, uint msk = HDI_TEXT);
    THdrItem(uint msk = 0);

    /// @}

    /// Methods to set the data members of the HD_ITEM structure.
    /// 
    void          SetText(LPCTSTR str);
    void          SetBitmap(HBITMAP hbm);
    void          SetWidth(int cx);
    void          SetHeight(int cy);
    void          SetItemData(TParam2 data);
    void          SetMask(int msk);
    // new Version 4.0
    void          SetImage(int item);
    void          SetOrder(int order);

    static  void  SetDefStringSize(int size);
    static  void  SetDefBitmapSize(int size);

  protected:
    static  int  DefStringItemSize;
    static  int  DefBitmapItemSize;
};


//
/// \class THeaderHitTestInfo
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// THeaderHitTestInfo contains hittest information for the header control.
/// It is used with the TColumnHeader::HitTest method.
//
class THeaderHitTestInfo : public HD_HITTESTINFO {
  public:
    THeaderHitTestInfo(const TPoint& pt);
    THeaderHitTestInfo(int x, int y);
};

//
/// \class TColumnHeader
// ~~~~~ ~~~~~~~~~~~~~
///  TColumnHeader encapsulates the 'header control', a window
///  usually positioned above columns of text or numbers.
//
class _OWLCLASS TColumnHeader : public TControl {
  public:
    TColumnHeader(TWindow*        parent,
                  int             id,
                  int x, int y, int w, int h,
                  TModule*        module = 0);

    TColumnHeader(TWindow* parent, int resourceId, TModule* module = 0);
    ~TColumnHeader();

    // Position Column Header
    //
    bool          Layout(TRect& boundingRect, WINDOWPOS& winPos);
    bool          Layout(uint swpFlags = 0);

    // Column Header attributes
    //
    int           GetCount() const;

    // Operations on individual column header items
    //
    int           Add(const THdrItem& item);
    int           Insert(const THdrItem& item, int index);
    bool          Delete(int index);
    bool          GetItem(THdrItem&, int index, uint mask = 0);
    bool          SetItem(const THdrItem& itemInfo, int index);

    // Hit-testing
    //
    int           HitTest(THeaderHitTestInfo& ht);

    // New IE 4.0 functionality
    //
    // Image List manipulation
    //
    HIMAGELIST    CreateDragImage(int index) const;
    HIMAGELIST    GetImageList() const;
    HIMAGELIST    SetImageList(HIMAGELIST iml);

    bool           GetItemRect(TRect& rect, int index) const;

    //
    // Order array manipulation
    //
    bool          SetOrderArray(int size, int* array);
    bool          GetOrderArray(int size, int* array) const;
    TResult       OrderToIndex(int order);

    int           SetHotDivider(uint32 value, bool flag = true);

    // Override virtual method to save/restore contents of columnHeader
    //
    uint          Transfer(void* buffer, TTransferDirection direction);

  protected:
    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TColumnHeader(const TColumnHeader&);
    TColumnHeader& operator =(const TColumnHeader&);

///YB  DECLARE_STREAMABLE_FROM_BASE(_OWLCLASS, owl::TColumnHeader, 1); // not implemented
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>



//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Version 4.70 functionality.
///
/// Creates a semi-transparent version of an item's image for use as a dragging
/// image. Returns a handle to an image list that contains the new image as its only
/// element. The
/// image assigned to this item is the basis for the transparent image. 
/// \param index is a zero-based index of the item within the header control. 
//
inline HIMAGELIST TColumnHeader::CreateDragImage(int index) const{
  return HIMAGELIST(CONST_CAST(TColumnHeader*,this)->SendMessage(
                    HDM_CREATEDRAGIMAGE,TParam1(index)));
}

//
/// Version 4.70 functionality.
///
/// Retrieves the handle to the image list that has been set for an existing header
/// control. 
//
inline HIMAGELIST TColumnHeader::GetImageList() const{
  return HIMAGELIST(CONST_CAST(TColumnHeader*,this)->SendMessage(HDM_GETIMAGELIST));
}

//
/// Version 4.70 functionality.
///
/// Assigns an image list to an existing header control. Returns the handle to the
/// image list previously associated with the control. Returns NULL upon failure or
/// if no image list was set previously. 
//
inline HIMAGELIST TColumnHeader::SetImageList(HIMAGELIST iml){
  return HIMAGELIST(SendMessage(HDM_SETIMAGELIST,0,TParam2(iml)));
}


//
/// Version 4.70 functionality.
///
/// Retrieves the bounding rectangle for a given item in a header control. Returns
/// nonzero if successful, or zero otherwise.
//
inline bool TColumnHeader::GetItemRect(TRect& rect, int index) const{
  return ToBool(CONST_CAST(TColumnHeader*,this)->SendMessage(HDM_GETITEMRECT,
                           TParam1(index), TParam2(&rect)));
}

//
/// Version 4.70 functionality.
///
/// Sets the left-to-right order of header items. Returns nonzero if successful, or
/// zero otherwise.
/// 
/// \param size 	Size of the buffer at lpiArray, in elements. This value must equal the
/// value returned by GetItemCount().
/// \param array 	Address of an array that specifies the order in which items should be
/// displayed, from left to right. For example, if the contents of the array are
/// { 2,0,1}, the control displays item 2, item 0, and item 1, from left to right.
//
inline bool TColumnHeader::SetOrderArray(int size, int* array){
  return ToBool(SendMessage(HDM_SETORDERARRAY, TParam1(size), TParam2(array)));
}

//
/// Version 4.70 functionality.
///
/// Retrieves the current left-to-right order of items in a header control. Returns
/// nonzero if successful, and the buffer at lpiArray receives the item number for
/// each item in the header control in the order in which they appear from left to
/// right. Otherwise, the message returns zero.
/// 
/// \param size 	Number of integer elements that array can hold. This value must be equal
/// to the number of items in the control (see GetCount().
/// \param array 	Address of an array of integers that receive the index values for items
/// in the header. The number of elements in this array is specified in size and
/// must be equal to or greater than the number of items in the control. For
/// example, the following code fragment will reserve enough memory to hold the
/// index values.
/// \code
/// int	items;
/// int *array;
/// 
/// // Get memory for buffer.
/// items = GetCount();
/// if(!(array = calloc(items,sizeof(int))))
///    MessageBox("Out of memory.","Error", MB_OK);
/// \endcode
//
inline bool TColumnHeader::GetOrderArray(int size, int* array) const{
  return ToBool(CONST_CAST(TColumnHeader*,this)->SendMessage(HDM_GETORDERARRAY,
                           TParam1(size), TParam2(array)));
}

//
/// Version 4.70 functionality.
///
/// Retrieves an index value for an item based on its order in the header control.
/// Returns int that indicates the item index. If order is invalid (negative or too
/// large), the return equals order.
/// 
/// \param order 	Order in which the item appears within the header control, from left to
/// right. For example, the index value of the item in the far left column would be
/// 0. The value for the next item to the right would be 1, and so on.
//
inline TResult TColumnHeader::OrderToIndex(int order){
  return SendMessage(HDM_ORDERTOINDEX,TParam1(order));
}

//
/// Version 4.70 functionality.
///
/// Changes the color of a divider between header items to indicate the destination
/// of an external drag-and-drop operation. Returns a value equal to the index of
/// the divider that the control highlighted.
/// 
/// \param flag 	Value specifying the type of value represented by dwInputValue. This value
/// can be one of the following:
/// - \c \b  true	Indicates that value holds the client coordinates of the pointer.
/// - \c \b  false	Indicates that value holds a divider index value.
/// \param value 	Value held in dwInputValue is interpreted depending on the value of
/// flag.
/// If flag is true, dwInputValue represents the x- and y-coordinates of the
/// pointer. The x-coordinate is in the low word, and the y-coordinate is in the
/// high word. When the header control receives the message, it highlights the
/// appropriate divider based on the dwInputValue coordinates.
/// If flag is false, dwInputValue represents the integer index of the divider to be
/// highlighted.
/// 
/// This message creates an effect that a header control automatically produces when
/// it has the HDS_DRAGDROP style. This function is intended to be used when the
/// owner of the control handles drag-and-drop operations manually.
//
inline int TColumnHeader::SetHotDivider(uint32 value, bool flag){
  return static_cast<int>(SendMessage(HDM_SETHOTDIVIDER,TParam1(flag),TParam2(value)));
}


//
/// Updates the mask member of the structure. The latter indicates
/// which other members of the structure contain valid data.
//
inline void THdrItem::SetMask(int msk)
{
  mask = msk;
}

//
/// Initializes the client ooordinates being hit-tested with the
/// specified TPoint parameter.
//
inline THeaderHitTestInfo::THeaderHitTestInfo(const TPoint& point)
{
  pt.x = point.x;
  pt.y = point.y;
}

//
/// Initializes the client ooordinates being hit-tested with the
/// specified 'x' and 'y' locations.
//
inline THeaderHitTestInfo::THeaderHitTestInfo(int x, int y)
{
  pt.x = x;
  pt.y = y;
}


} // OWL namespace


#endif  // OWL_COLMNHDR_H
