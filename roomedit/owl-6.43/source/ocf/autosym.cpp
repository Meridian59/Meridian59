//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// OLE Automation Server Implementation, except TServedObject (in typelib.cpp)
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/appdesc.h>
#include <ocf/ocreg.h>
#include <ocf/occtrl.h>

namespace ocf {

using namespace owl;

TAutoType TAutoVoid       ::ClassInfo = {atVoid};
TAutoType TAutoByte       ::ClassInfo = {atByte};
TAutoType TAutoShort      ::ClassInfo = {atShort};
TAutoType TAutoLong       ::ClassInfo = {atLong};
TAutoType TAutoFloat      ::ClassInfo = {atFloat};
TAutoType TAutoDouble     ::ClassInfo = {atDouble};
TAutoType TAutoCurrency   ::ClassInfo = {atCurrency};
TAutoType TAutoDate       ::ClassInfo = {atDatetime};
TAutoType TAutoString     ::ClassInfo = {atString};
TAutoType TAutoBool       ::ClassInfo = {atBool};
TAutoType TAutoUnknown    ::ClassInfo = {atUnknown};
TAutoType TAutoDispatch   ::ClassInfo = {atObject};
TAutoType TAutoVariant    ::ClassInfo = {atVariant};
TAutoType TAutoSafeArray  ::ClassInfo = {atSafeArray};
TAutoType TAutoShortRef   ::ClassInfo = {atByRef|atShort};
TAutoType TAutoLongRef    ::ClassInfo = {atByRef|atLong};
TAutoType TAutoFloatRef   ::ClassInfo = {atByRef|atFloat};
TAutoType TAutoDoubleRef  ::ClassInfo = {atByRef|atDouble};
TAutoType TAutoCurrencyRef::ClassInfo = {atByRef|atCurrency};
TAutoType TAutoDateRef    ::ClassInfo = {atByRef|atDatetime};
TAutoType TAutoStringRef  ::ClassInfo = {atByRef|atString};
TAutoType TAutoVariantRef ::ClassInfo = {atByRef|atVariant};
TAutoType TAutoBoolRef    ::ClassInfo = {atByRef|atBool};
TAutoType TAutoByteRef    ::ClassInfo = {atByRef|atByte};


_OCFFUNC(void) SendObituary(const void * obj, const std::type_info& typeInfo)
{
  TAppDescriptor* appDesc = ocf::GetAppDescriptor();
  if (appDesc)
    appDesc->InvalidateObject(ocf::MostDerived(obj, typeInfo));
}
//----------------------------------------------------------------------------
// TAutoClass implementation
//

TAutoClass::TClassList TAutoClass::ClassList = {0,0,0};// MUST BE MODULE GLOBAL

TAutoClass::TAutoClass(TAutoSymbol* table, TAutoSymbol* classSymbol,
             const std::type_info& typeInfo, TAggregator aggregator)
           : Table(table), ClassSymbol(classSymbol), TypeInfo(typeInfo),
             Aggregator(aggregator), AutoIds (TRUE)
{
  Type = atObject | atAutoClass;
  NextClass = ClassList.List;
  ClassList.List = this;
  ClassList.Count++;
}

TAutoClass::~TAutoClass()  // do we really need to support dynamic AutoClass?
{
  for (TAutoClass** link = &ClassList.List; *link != 0; link = &(*link)->NextClass)
    if (*link == this) {
      *link = NextClass;
      break;
    }
}

short TAutoClass::CountCommands()
{
  TAutoSymbol* sym;
  if (!CommandCount) {
    for (sym = Table; !sym->IsTerminator(); sym++) {
      int attr = sym->GetFlags();
      if (attr & asAnyCommand) {
        CommandCount++;
        if (attr & asOleType) {
          if ((attr & asGetSet) == asGetSet)
            VariableCount++;
          else
            FunctionCount++;
        }
      } else if (sym->TestFlag(asClass)) {
        TAutoClass* cls = sym->GetClass();
        if (!sym->SymCount)
          sym->SymCount = cls->CountCommands();
        CommandCount  += cls->CommandCount;
        VariableCount += cls->VariableCount;
        FunctionCount += cls->FunctionCount;
      }
    }
  }
  return CommandCount;
}

TAutoSymbol* TAutoClass::FindId(long id, ObjectPtr& obj)
{
  TAutoSymbol* sym;
  long cmdId;
  if ((id <= 0) || !(AutoIds)) {
    // reserved dispatch ID if negative or zero or AutoIDs is false
    for (sym = Table; !sym->IsTerminator(); sym++) {
      if (sym->TestFlag(asAnyCommand) && sym->DispId == id)
        return sym;

      if (sym->TestFlag(asClass)) {
        ObjectPtr adjObj = sym->Convert(obj);  // this pointer adjustment
        TAutoSymbol* fsym = sym->GetClass()->FindId(id, adjObj);
        if (fsym) {
          obj = adjObj;
          return fsym;
        }
      }
    }
  } else {
    for (cmdId = 0, sym = Table; !sym->IsTerminator(); sym++) {
      if (sym->TestFlag(asClass)) {
        if (!sym->SymCount)
          sym->SymCount = sym->GetClass()->CountCommands();
        if (cmdId + sym->SymCount >= id) {    // symbol in nested class
          obj = sym->Convert(obj);
          return sym->GetClass()->FindId(id-cmdId, obj);
        }
        cmdId += sym->SymCount;
      }
      else if (sym->TestFlag(asAnyCommand)) {
        cmdId++;
        if (cmdId == id) {
          if (sym->DispId == -1)
            return sym;
          else
            break;
        }
      }
    }
  }
  return 0;
}

TAutoSymbol* TAutoClass::FindFunction(unsigned index, MEMBERID& retId)
{
  TAutoSymbol* sym;
  int funcCount = 0;
  long cmdId = retId;
  for (sym = Table; !sym->IsTerminator(); sym++) {
    int attr = sym->GetFlags();
    if (attr & asAnyCommand) {
      cmdId++;
      if ((attr & asOleType) != 0 && (attr & asGetSet) != asGetSet) {
        if (funcCount++ == (int)index) {
          retId = (sym->DispId == -1L) ? cmdId : sym->DispId;
          return sym;
        }
      }
    } else if (sym->TestFlag(asClass)) {
      TAutoClass* cls = sym->GetClass();
      if (!sym->SymCount)
        sym->SymCount = cls->CountCommands();
      if (funcCount + cls->FunctionCount > (int)index) {
        retId = int(cmdId);
        return cls->FindFunction(index - funcCount, retId);
      }
      funcCount += cls->FunctionCount;
      cmdId     += cls->CommandCount;
    }
  }
  return 0;  // should never happen unless caller overruns total count
}

TAutoSymbol* TAutoClass::FindVariable(unsigned index, MEMBERID& retId)
{
  TAutoSymbol* sym;
  int varCount = 0;
  long cmdId = retId;
  for (sym = Table; !sym->IsTerminator(); sym++) {
    int attr = sym->GetFlags();
    if (attr & asAnyCommand) {
      cmdId++;
      if ((attr & asGetSet) == asGetSet) {
        if (varCount++ == (int)index) {
          retId = (sym->DispId == -1L) ? cmdId : sym->DispId;
          return sym;
        }
      }
    } else if (sym->TestFlag(asClass)) {
      TAutoClass* cls = sym->GetClass();
      if (!sym->SymCount)
        sym->SymCount = cls->CountCommands();
      if (varCount + cls->VariableCount > (int)index){
        retId = int(cmdId);
        return cls->FindVariable(index - varCount, retId);
      }
      varCount += cls->VariableCount;
      cmdId    += cls->CommandCount;
    }
  }
  return 0;  // should never happen unless caller overruns total count
}

short TAutoClass::GetArgCount(TAutoSymbol& sym)
{
  short count = 0;
  TAutoSymbol* arg = &sym;
  while ((++arg)->TestFlag(asArgument))
    count++;
  return count;
}

TAutoSymbol* TAutoClass::Lookup(_TCHAR * name, TLangId lang, short symflags,
                                long & retid)
{
  long cmdId = 0;
  for (TAutoSymbol* sym = Table; !sym->IsTerminator(); sym++) {
    if (sym->TestFlag(asAnyCommand))
      cmdId++;
    if (sym->TestFlag(symflags) && sym->Name.Compare(name, lang) == 0) {
      retid = sym->DispId == -1 ? cmdId : sym->DispId;
      return sym;
    }
    else if (sym->TestFlag(asClass)) {
      TAutoClass* cls = sym->GetClass();
      if (!sym->SymCount)
        sym->SymCount = cls->CountCommands();
      long id;
      TAutoSymbol* found = cls->Lookup(name, lang, symflags, id);
      if (found) {
        retid = id > 0 ? id + (long)cmdId : id;
        return found;
      }
      cmdId += sym->SymCount;
    }
  }
  return 0;
}

TAutoSymbol* TAutoClass::LookupArg(_TCHAR * name, TLangId lang,
                                   TAutoSymbol* sym, long & retid)
{
  PRECONDITION(sym);

   for (int i = 0; (++sym)->TestFlag(asArgument); ++i)
     if (sym->Name.Compare(name, lang) == 0) {
       retid = (long)i;
       return sym;
     }
   return 0;
}

TXAuto::TError TAutoClass::Dispatch(ObjectPtr obj, TAutoCreator& creator,
                                    TUnknown& owner, int attr,
                                    TAutoStack& args, TAutoVal* retval)
{
  TAutoCommand*  cmdobj = 0;
  TAutoIterator* iterator = 0;
  try {
    if (args.Symbol->IsIterator()) {
      iterator = args.Symbol->BuildIter(obj, creator, owner, args.LangId);
      iterator->SetSymbol(args.Symbol);
      iterator->Init();
      *retval = (IUnknown*)*iterator; // remains until RefCnt->0
    } else {
      cmdobj = args.Symbol->Build(obj, attr, args);
      cmdobj->SetSymbol(args.Symbol);
      if (args.ArgCount>0 && !cmdobj->Validate()) {// no validate for prop get
        delete cmdobj;
        return TXAuto::xValidateFailure;
      }
      cmdobj->Invoke();
      if ((args.ErrorCode = cmdobj->Report()) != 0) {
        args.ErrorMsg = TAutoCommand::LookupError(args.ErrorCode);
        if (!args.ErrorMsg && args.Symbol)  // if no error message available
          args.ErrorMsg = args.Symbol->Name.Translate(args.LangId);
        delete cmdobj;
        return TXAuto::xErrorStatus;
      }
      if (retval) {
        cmdobj->Return(*retval);
        if (args.Symbol->IsEnum())
          args.Symbol->GetEnum()->Convert(*retval, args.LangId);
        TObjectDescriptor objDesc;
        if (retval->GetObjDesc(objDesc)) {
          if (!objDesc.Object)     // null pointer returned from function
            // there are three choices for behavior here:
            // 1. Allow a dead object to be returned, fail when passed back
            // 2. Fail now, however this prevents testing for null pointer
            // 3. Return an empty variant, causing script to fail when used
            *retval = TAutoVoid();  // return an empty value if no object
          else
            *retval = creator.CreateDispatch(objDesc);
        }
      }
      delete cmdobj;
    }
  }
  catch(TXAuto& xobj) {
    delete cmdobj;
    delete iterator;
    return xobj.ErrorCode;
  }
  return TXAuto::xNoError;
}

TAutoClass::TExtLink::TExtLink(TClassList* list, HINSTANCE module)
           : Classes(list), Module(module), Next(0)
{
  for (Prev = &ClassList.Link; *Prev; Prev = &(*Prev)->Next)
    ;               // link to end of list
}

TAutoClass::TExtLink::~TExtLink()
{
  *Prev = Next;
  if (Next)
    Next->Prev = Prev;
}

//
//
//
int
TAutoClass::TClassList::CountAutoClasses()
{
  int count = Count;
  for (TExtLink* link = Link; link; link = link->Next)
    count += link->Classes->CountAutoClasses();
  return count;
}

//
//
//
TAutoClass::TAutoClassRef*
TAutoClass::TClassList::MergeAutoClasses(TAutoClass::TAutoClassRef* array)
{
  for (TAutoClass* cls = List; cls; cls = cls->NextClass, array++)
    array->Class = cls;
  for (TExtLink* link = Link; link; link = link->Next)
    array = link->Classes->MergeAutoClasses(array);
  return array;
}

// ----------------------------------------------------------------------
//  Helper routines
//

//
//
TAutoCommand*
AutoQuitBuild(ObjectPtr obj, int/*attr*/, TAutoStack& args)
{
  TServedObject& owner = *args.Owner;

  // if the automation object is not in control of the app, execute a no-op
  //
  if (owner.Destruct == TObjectDescriptor::Quiet)
    return new TAutoCommand(0);

  // if registered as the active object, free it to release OLE's refcnt
  //
  if (owner.Creator.GetAppDesc().IsActiveObject(&owner))
    owner.Creator.GetAppDesc().UnregisterObject();

  // disconnect automation from app to prevent further access
  //
  owner.Object = 0;
  owner.RootObject = 0;

  // build command object for destructor, will either delete or PostQuitMsg
  //
  return owner.Class->GetDestructor()(obj, owner.Destruct);
}

#if defined(BI_COMP_BORLANDC)
} // OCF namespace
//
// Temporary defines for using std::type_info with dynamic cast
//
void* __cdecl __DynamicCast(void* object, void* vtable,
                            void* srctyp, void* dsttyp,
                            int reference = 0);
struct tpid {int s; short m; short n; int VptrOffs; int Flags;};  // partial
namespace ocf {
#endif
//
//
const void*
DynamicCast(const void* obj, const std::type_info& src, const std::type_info& dst)
{
#if defined(__clang__) // TODO: This needs review!
  return __DynamicCast((void*)obj, 0, (void*)&src, (void*)&dst, 0);
#elif defined(BI_COMP_BORLANDC)
  int vtblOff;
  if (!obj)
    return obj;
  else if ((vtblOff = src.tpp->VptrOffs) == -1)
    return src==dst ? obj : 0;
  else
    return __DynamicCast(const_cast<void *>(obj),
                         *(void **)((char*)obj+vtblOff), src.tpp,dst.tpp);
#else
  return __RTDynamicCast((void*)obj,0,(void*)&src,(void*)&dst,0);
#endif  //BI_COMP_BORLANDC
}


//
//
const void *
MostDerived(const void * obj, const std::type_info& src)
{
#if defined(BI_COMP_BORLANDC) && !defined(__clang__) // TODO: This needs review for Clang!
  int vtblOff;
  if (!obj || (vtblOff = src.tpp->VptrOffs) == -1)
    return obj;
  else
    return __DynamicCast(const_cast<void *>(obj),
                         *(void **)((char*)obj+vtblOff), src.tpp, 0);
#else
  return obj;
#endif
}


//____________________________________________________________________________
//
// TAutoCommand implementation - inlined to allow definition of _AUTOCLASS
//____________________________________________________________________________

_OCFDATA(TAutoCommand::TErrorMsgHook) TAutoCommand_ErrorLookup = 0; // module static
_OCFDATA(TAutoCommand::TCommandHook)   TAutoCommand_InvokeHook  = 0; // module static

} // OCF namespace

//==============================================================================


