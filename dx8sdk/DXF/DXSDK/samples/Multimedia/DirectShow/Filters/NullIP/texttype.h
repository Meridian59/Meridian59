//------------------------------------------------------------------------------
// File: TextType.h
//
// Desc: DirectShow sample code - definition of CTextMediaType class.  This
//       is a class that can return itself as text.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


class CTextMediaType : public CMediaType {

public:

    CTextMediaType(AM_MEDIA_TYPE mt):CMediaType(mt) {}

    void AsText(LPTSTR szType,
                unsigned int iLen,
                LPTSTR szAfterMajor,
                LPTSTR szAfterOthers,
                LPTSTR szAtEnd);

private:

    // Provide a string description for this format block

    void Format2String(LPTSTR szBuffer,
                       UINT iLength,
                       const GUID* pGuid,
                       BYTE* pFormat,
                       ULONG lFormatLength);

    // Convert this CLSID into a meaningful string

    void CLSID2String(LPTSTR szBuffer,
                       UINT iLength,
                       const GUID* pGuid);
};

