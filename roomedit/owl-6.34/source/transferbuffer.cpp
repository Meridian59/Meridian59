//
/// \file transferbuffer.cpp
/// Safe transfer buffers
//
// Copyright © 2010 Vidar Hasfjord 
// Distributed under the OWLNext License (see http://owlnext.sourceforge.net).
//

#if defined(__BORLANDC__) && __BORLANDC__ < 0x600
# pragma option -w-inl // Disable warning in standard library; "Functions containing 'statement' are not expanded inline".
#endif

#include <owl/pch.h>

#include <owl/transferbuffer.h>
#include <owl/validate.h>
#include <map>
#include <algorithm>

namespace owl {

namespace // Local implementation utilities
{ 

  //
  /// Stores information about a bound field in a window's transfer buffer.
  //
  struct TFieldInfo
  {
    size_t Offset;
    size_t Size;
  };

  //
  /// Type of collection of bound controls
  /// Public so that the class implementation can use free functions.
  //
  typedef std::map<TWindow*, TFieldInfo, std::less<TWindow*> > TFieldDetails; // NB! Default arg is for BC++ 5.0x compatibility.

  //
  /// Functor for find_if with TFieldDetails
  //
  struct TOffsetEq
  {
    size_t Offset;
    TOffsetEq(size_t ofs) : Offset(ofs) {}

    bool operator()(const TFieldDetails::value_type& v)
    {return v.second.Offset == Offset;}
  };

  //
  /// Functor for find_if with TFieldDetails
  //
  struct TIdEq
  {
    int Id;
    TIdEq(int id) : Id(id) {}

    bool operator()(const TFieldDetails::value_type& v)
    {return v.first->GetId() == Id;}
  };

  //
  /// Argument for ForEach
  //
  struct TTransferIterInfo 
  {
    TFieldDetails* Details;
    void* Buffer;
    TTransferDirection Direction;
  };

  //
  /// Function for ForEach
  /// Calculates the field address and dispatches to the given child.
  /// Checks that all participating children are bound to a field; throws if not.
  /// Checks that the transferred size matches the bound field size; throws if not.
  /// Forbids validator meddling; throws if a validator wants to override data transfer.
  //
  // TODO: Replace TXWindow (IDS_TRANSFERBUFFERSIZEMISMATCH) with a local excption class.
  //
  void TransferChild(TWindow* child, void* arg) 
  {
    PRECONDITION(child);
    if (!child->IsFlagSet(wfTransfer)) return; // disabled
    if (TEdit* edit = TYPESAFE_DOWNCAST(child, TEdit))
      if (TValidator* v = edit->GetValidator())
        if (v->HasOption(voTransfer)) throw TTransferBufferWindowBase::TXMeddlingValidator(*edit);

    const TTransferIterInfo& t = *static_cast<TTransferIterInfo*>(arg);
    CHECK(t.Details);
    TFieldDetails::const_iterator i = t.Details->find(child);
    if (i == t.Details->end()) throw TTransferBufferWindowBase::TXUnboundControl(*child);
    const TFieldInfo& f = (*i).second;
    void* field = static_cast<char*>(t.Buffer) + f.Offset;
    uint size = child->Transfer(field, tdSizeData);
    if (size != f.Size) TXWindow::Raise(child, IDS_TRANSFERBUFFERSIZEMISMATCH); 
    if (t.Direction == tdSizeData) return; // done
    uint transferred_size = child->Transfer(field, t.Direction);
    if (transferred_size != size) TXWindow::Raise(child, IDS_TRANSFERBUFFERSIZEMISMATCH);
  }

  //
  /// See TXPolygamousControl.
  //
  tstring MakeMessageForTXPolygamousControl(TWindow& c)
  {
    tostringstream s;
    TWindow* p = c.GetParentO();
    s << _T("Control #") << c.GetId() << _T(" (") << GetFullTypeName(&c) << _T(") ")
      << _T(" in window ") << GetFullTypeName(p) << _T(" ")
      << _T(" is trying to bind to more than one field.\n\n")
      << _T("Ensure that every control is bound to a separate field in the transfer buffer, ")
        _T("and that TTransferBufferWindow::Bind is not called more than once for the same control.\n");
    return s.str();
  }

  //
  /// See TXFieldConflict.
  //
  tstring MakeMessageForTXFieldConflict(TWindow& c)
  {
    tostringstream s;
    TWindow* p = c.GetParentO();
    s << _T("Control #") << c.GetId() << _T(" (") << GetFullTypeName(&c) << _T(") ")
      << _T(" in window ") << GetFullTypeName(p) << _T(" ")
      << _T(" is trying to bind to an already bound field.\n\n")
      << _T("Ensure that every control is bound to a separate field in the transfer buffer, ")
        _T("and that TTransferBufferWindow::Bind is not called more than once for the same control.\n");
    return s.str();
  }

  //
  /// See TXUnboundControl.
  //
  tstring MakeMessageForTXUnboundControl(TWindow& c)
  {
    tostringstream s;
    TWindow* p = c.GetParentO();
    s << _T("Transfer requested by unbound control #") 
      << c.GetId() << _T(" (") << GetFullTypeName(&c) << _T(") ")
      << _T(" in window ") << GetFullTypeName(p) << _T(".\n\n")
      << _T("Use TTransferBufferWindow::Bind to bind the control to a field in the window's transfer buffer, ")
        _T("or disable transfer for this control using TWindow::DisableTransfer.\n");
    return s.str();
  }

  //
  /// See TXMeddlingValidator.
  //
  tstring MakeMessageForTXMeddlingValidator(TEdit& c)
  {
    tostringstream s;
    TWindow* p = c.GetParentO();
    s << _T("Validator ") << GetFullTypeName(c.GetValidator()) 
      << _T(" is meddling with the transfer of control #") 
      << c.GetId() << _T(" (") << GetFullTypeName(&c) << _T(") ")
      << _T(" in window ") << GetFullTypeName(p) << _T(".\n\n")
      << _T("Validators are not allowed to take part in transfers in a TTransferBufferWindow. ")
        _T("Turn off transfer meddling for the validator by calling TValidator::UnsetOption(voTransfer). ")
        _T("You must use a custom control if you need to transfer numeric buffer fields.\n");
    return s.str();
  }

} // namespace

//
/// Private implementation class for TTransferBufferWindowBase
/// Stores the buffer field associations.
//
class TTransferBufferWindowBase::TImpl
{
public:
  TFieldDetails FieldDetails;
};

TTransferBufferWindowBase::TTransferBufferWindowBase()
: pimpl(new TImpl())
{}

TTransferBufferWindowBase::~TTransferBufferWindowBase()
{
  delete pimpl;
}

void TTransferBufferWindowBase::TransferData(TTransferDirection direction)
{
  Transfer(GetTransferBuffer(), direction);
}

uint TTransferBufferWindowBase::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer) return 0; // nothing to do
  TTransferIterInfo info = {&pimpl->FieldDetails, buffer, direction};
  ForEach(TransferChild, &info);
  return GetTransferBufferSize();
}

void TTransferBufferWindowBase::AssignField(TWindow& c, size_t offset, size_t size)
{
  PRECONDITION(size > 0);

  TFieldDetails& f = pimpl->FieldDetails;
  if (std::find_if(f.begin(), f.end(), TOffsetEq(offset)) != f.end()) 
    throw TXFieldConflict(c);
  if (std::find_if(f.begin(), f.end(), TIdEq(c.GetId())) != f.end()) 
    throw TXPolygamousControl(c);

  TFieldInfo info = {offset, size};
  typedef std::pair<TFieldDetails::iterator, bool> TInsertResult;
  TInsertResult r = f.insert(TFieldDetails::value_type(&c, info));
  if (!r.second) throw TXPolygamousControl(c);

  c.EnableTransfer();
}

TTransferBufferWindowBase::TXPolygamousControl::TXPolygamousControl(TWindow& c) 
  : TXOwl(MakeMessageForTXPolygamousControl(c)) 
{}

TTransferBufferWindowBase::TXFieldConflict::TXFieldConflict(TWindow& c) 
  : TXOwl(MakeMessageForTXFieldConflict(c)) 
{}

TTransferBufferWindowBase::TXUnboundControl::TXUnboundControl(TWindow& c) 
  : TXOwl(MakeMessageForTXUnboundControl(c)) 
{}

TTransferBufferWindowBase::TXMeddlingValidator::TXMeddlingValidator(TEdit& c) 
  : TXOwl(MakeMessageForTXMeddlingValidator(c)) 
{}

} // OWL namespace
