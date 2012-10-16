/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Jul 23 18:31:14 2001
 */
/* Compiler settings for D:\DXSDK\samples\Multimedia\DirectShowXP\VideoControl\CPPVideoControl\CPPVideoControl.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __CPPVideoControl_h__
#define __CPPVideoControl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ICompositeControl_FWD_DEFINED__
#define __ICompositeControl_FWD_DEFINED__
typedef interface ICompositeControl ICompositeControl;
#endif 	/* __ICompositeControl_FWD_DEFINED__ */


#ifndef __CompositeControl_FWD_DEFINED__
#define __CompositeControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class CompositeControl CompositeControl;
#else
typedef struct CompositeControl CompositeControl;
#endif /* __cplusplus */

#endif 	/* __CompositeControl_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ICompositeControl_INTERFACE_DEFINED__
#define __ICompositeControl_INTERFACE_DEFINED__

/* interface ICompositeControl */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ICompositeControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3E119535-D5AB-4520-B0E1-495B322E2A1A")
    ICompositeControl : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ICompositeControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICompositeControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICompositeControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICompositeControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICompositeControl __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICompositeControl __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICompositeControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICompositeControl __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } ICompositeControlVtbl;

    interface ICompositeControl
    {
        CONST_VTBL struct ICompositeControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICompositeControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICompositeControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICompositeControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICompositeControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICompositeControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICompositeControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICompositeControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICompositeControl_INTERFACE_DEFINED__ */



#ifndef __CPPVIDEOCONTROLLib_LIBRARY_DEFINED__
#define __CPPVIDEOCONTROLLib_LIBRARY_DEFINED__

/* library CPPVIDEOCONTROLLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_CPPVIDEOCONTROLLib;

EXTERN_C const CLSID CLSID_CompositeControl;

#ifdef __cplusplus

class DECLSPEC_UUID("CDDFD429-EDFD-4C72-AE9C-B70FE6955051")
CompositeControl;
#endif
#endif /* __CPPVIDEOCONTROLLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
