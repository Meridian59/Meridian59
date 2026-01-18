/////////////////////////////////////////////////////////
//
// OLE Drag Drop support
//
//
#if !defined(COOLPRJ_DRAWDROP_H)
#define COOLPRJ_DRAWDROP_H

#include <coolprj/defs.h>

typedef DWORD DROPEFFECT;

//
//
//
class _COOLCLASS TOleInitializer{
  public:
    static void Initialize();
    TOleInitializer();
    ~TOleInitializer();
};
//
//
//
class _COOLCLASS TFormatEtc: public FORMATETC{
  public:
    TFormatEtc();
    TFormatEtc(const TFormatEtc& etc);
    TFormatEtc(const FORMATETC& etc);
    TFormatEtc(CLIPFORMAT cfmt, DWORD tmd);
    bool operator ==(const TFormatEtc& other) const;
    bool operator !=(const TFormatEtc& other) const;
};
//
//
//
class _COOLCLASS TStorageMedium: public STGMEDIUM{
  public:
    TStorageMedium();
};
//
//
//
class _COOLCLASS TDragDropProxy {
  public:
    struct TDataEntry{
      TDataEntry(){}
      TFormatEtc      FormatEtc;
      TStorageMedium  StgMedium;
    };
    enum TNoValue{
      NOVALUE = ((owl::uint)-1),
    };
    enum TTestResult {
      UDROP_NONE        = 0x0000,      //Exclusive
      UDROP_CLIENT      = 0x0001,      //Inclusive
      UDROP_INSETLEFT   = 0x0002,      //L/R are exclusive
      UDROP_INSETRIGHT  = 0x0004,
      UDROP_INSETHORZ   = (UDROP_INSETLEFT | UDROP_INSETRIGHT),
      UDROP_INSETTOP    = 0x0008,      //T/B are exclusive
      UDROP_INSETBOTTOM = 0x0010,
      UDROP_INSETVERT   = (UDROP_INSETTOP | UDROP_INSETBOTTOM),
    };
  public:
    TDragDropProxy(owl::TWindow*  parent);
    virtual ~TDragDropProxy();

    // DragSource functions
    virtual STDMETHODIMP QueryContinueDrag(BOOL, DWORD);
    virtual STDMETHODIMP GiveFeedback(DWORD);

    // DragTarget functions
    virtual STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
    virtual STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
    virtual STDMETHODIMP DragLeave();
    virtual STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);

    // DataObject functions
    virtual STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
    virtual STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
    virtual STDMETHODIMP QueryGetData(LPFORMATETC);
    virtual STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC);
    virtual STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
    virtual STDMETHODIMP EnumFormatEtc(DWORD, LPENUMFORMATETC*);
    virtual STDMETHODIMP DAdvise(LPFORMATETC, DWORD,  LPADVISESINK, DWORD*);
    virtual STDMETHODIMP DUnadvise(DWORD);
    virtual STDMETHODIMP EnumDAdvise(LPENUMSTATDATA*);

    void SetDragData(CLIPFORMAT cfFormat, HGLOBAL hGlobal);
    void ResetDragData();

    virtual void ShowDropIndicator(owl::TPoint&){}
    virtual void HideDropIndicator(){}
    virtual owl::uint IsDroppable(const owl::TPoint& point, const owl::TRect& clientRect);
    virtual bool PerformDrop(IDataObject* dataObj, const owl::TPoint& point, DROPEFFECT de);
    virtual bool StartDragDrop();
    virtual void EndDragDrop(DROPEFFECT de);

  protected:
    TDataEntry* GetDataEntry(TFormatEtc& formatEtc);
    TDataEntry* CreateDataEntry(TFormatEtc& formatEtc);

  protected:
    TDataEntry* DataArray;
    owl::uint        DataSize;
    owl::uint        ItemsReserved;

    owl::TWindow*      Parent;
    IDataObject*  DataObject;
    owl::TPoint        SavedPos;
    owl::uint          LastTest;
    owl::uint32        TimeLast;           //Ticks on last DragOver
    int HScrollCode; // L/R on scroll repeat?
    int VScrollCode; // U/D on scroll repeat?
    bool          PendingRepaint;
    bool          ShowIndicator;
    static owl::uint   ScrollInset;
    static owl::uint   ScrollDelay;
};

//
//
//
class _COOLCLASS TEnumFormatEtc : public IEnumFORMATETC{
  public:
    TEnumFormatEtc();
    ~TEnumFormatEtc();

    bool Init(TDragDropProxy::TDataEntry* array, owl::uint size);

    //IUnknown members that delegate to m_pUnkOuter.
    STDMETHODIMP         QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    //IEnumFORMATETC members
    STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG*);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumFORMATETC **);

  private:
    ULONG       RefCnt;
    TFormatEtc* DataArray;
    owl::uint        DataSize;
    owl::uint        m_iCur;
};

//
// class TDragDropSupport
// ~~~~~ ~~~~~~~~~~~~~~~~
//
class _COOLCLASS TDragDropSupport: public IUnknown {
  public:
    TDragDropSupport(TDragDropProxy* proxy);
    virtual ~TDragDropSupport();

    TDragDropProxy* SetProxy(TDragDropProxy* proxy);
    bool Register(owl::TWindow& parent);
    bool UnRegister(owl::TWindow& parent);
    bool CanDragDrop(const owl::TPoint& startPoint, const owl::TPoint& endPoint);
    owl::uint GetDelay(){ return DragDelay; }

    DROPEFFECT DoDragDrop(DWORD dwEffects = DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK);

  protected:
    //IUnknown interface members
    STDMETHODIMP QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    class _COOLCLASS TDropSource: public IDropSource {
        public:
          //IDropSource interface members
          STDMETHODIMP QueryInterface(REFIID, LPVOID*);
          STDMETHODIMP_(ULONG) AddRef(void);
          STDMETHODIMP_(ULONG) Release(void);

          STDMETHODIMP QueryContinueDrag(BOOL, DWORD);
          STDMETHODIMP GiveFeedback(DWORD);
        }DropSource;
    friend class TDropSource;

    class _COOLCLASS TDropTarget : public IDropTarget {
      public:
        //IDropTarget interface members
        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
        STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
        STDMETHODIMP DragLeave();
        STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
    }DropTarget;
    friend class TDropTarget;
    class _COOLCLASS TDataObject: public IDataObject {
      public:
        //IUnknown members that delegate to m_pUnkOuter.
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        //IDataObject members
        STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP QueryGetData(LPFORMATETC);
        STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC);
        STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
        STDMETHODIMP EnumFormatEtc(DWORD, LPENUMFORMATETC*);
        STDMETHODIMP DAdvise(LPFORMATETC, DWORD,  LPADVISESINK, DWORD*);
        STDMETHODIMP DUnadvise(DWORD);
        STDMETHODIMP EnumDAdvise(LPENUMSTATDATA*);
    }DataObject;
    friend class TDataObject;

  protected:
    ULONG            RefCnt;  //Interface reference count
    TDragDropProxy* Proxy;
    static owl::uint      DragDelay;
    static owl::uint      DragMinDist;
};

//
//
//
#endif // COOLPRJ_DRAWDROP_H