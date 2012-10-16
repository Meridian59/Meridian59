//------------------------------------------------------------------------------
// File: SourceInfo.cpp
//
// Desc: DirectShow sample code
//       Implementation of CMediaList, play-list of media files
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "SourceInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMediaList::CMediaList() :
                m_avgDuration(0L),
                m_N(0),
                m_ppSI(NULL)
{
}

CMediaList::~CMediaList()
{
    Clean();
}

//------------------------------------------------------------------------------
void CMediaList::Clean()
{
    for( int i=0; i<m_N; i++)
    {
        delete m_ppSI[i];
        m_ppSI[i] = NULL;
    }// for

    delete[] m_ppSI;
    m_ppSI = NULL;
    m_N = 0;
}


//------------------------------------------------------------------------------
// CMediaList::Add
// Desc: adds a new source to the media list
// return: true if success, false otherwise
//------------------------------------------------------------------------------
bool CMediaList::Add( SourceInfo * pSI)
{
    SourceInfo ** ppSInew = NULL;

    ppSInew = new SourceInfo*[m_N+1];
    if( !ppSInew )
        return false;

    for( int i=0; i<m_N; i++)
    {
        ppSInew[i] = m_ppSI[i];
    }// for

    ppSInew[m_N] = pSI;
    delete[] m_ppSI;
    m_ppSI = ppSInew;
    m_N++;

    return true;
}
    
//------------------------------------------------------------------------------
// CMediaList::Shuffle()
// Desc: randomly shuffles media list content to 
//       have different playback settings every time
//------------------------------------------------------------------------------
void CMediaList::Shuffle()
{
    SourceInfo *pSIaux = NULL;
    int n1;
    int n2;

    for(int i=0; i< m_N; i++)
    {
        n1 = rand() * (m_N+1) / RAND_MAX;
        n2 = rand() * (m_N+1) / RAND_MAX;

        n1 = ( n1<0) ? 0 : n1;
        n2 = ( n2<0) ? 0 : n2;

        n1 = ( n1>m_N-1) ? m_N-1 : n1;
        n2 = ( n2>m_N-1) ? m_N-1 : n2;

        if( n1 == n2 )
            continue;

        pSIaux = m_ppSI[n1];
        m_ppSI[n1] = m_ppSI[n2];
        m_ppSI[n2] = pSIaux;
    }
}

//------------------------------------------------------------------------------
// Name: SetInitialParameters
// Purpose: calculates initial demonstration parameters for each media file --
//          destination rectangles and alpha levels
//          Note that these values are used as parameters in CDemonstration and 
//          that changing them can cause different appearence of the demonstration          
//------------------------------------------------------------------------------
void CMediaList::SetInitialParameters()
{
    NORMALIZEDRECT rectD;
    double Alpha;
    int i;
    // set alpha levels and destination rectangles here
    for( i=0; i< m_N; i++)
    {
        Alpha = 0.3 + 0.68 * (double)rand() / RAND_MAX; // random in [0.3; 0.98]
        
        rectD.left = rectD.top = 0.1f;
        rectD.right = rectD.bottom = 0.9f;

        this->GetItem(i)->m_fAlpha = Alpha;
        this->GetItem(i)->m_rD = rectD;
    }// for
}

//------------------------------------------------------------------------------
// Name: Initialize
// Purpose: go through m_szMediaFolder and retrieve all media files into
// Parameters: none
// Return: true if folder contains at least one valid file;
//         false otherewise
//------------------------------------------------------------------------------
bool CMediaList::Initialize(char *szFolder)
{
    struct _finddata_t fileinfo;
    long filehandle = -1L;
    int nRes;
    int nCounter = 0; // to prevent loading huge number of files,
                      // let's restrict it to 50
    char szMask[MAX_PATH];
    char szExt[] = "*.AVI;*.MOV;*.MPG;*.MPEG;*.VOB;*.QT;";
    char szCurExt[MAX_PATH];
    char szFilePath[MAX_PATH];
    char *psz = NULL;

    // clean the list
    Clean();

    if( !_strcmpi(szFolder,""))
        return false;

    do
    {
        strcpy(szCurExt,szExt);
        psz = strstr(szCurExt,";");
        if( psz)
        {
            *psz = 0;
            psz = NULL;
            psz = strstr( szExt, ";");
            if( psz )
            {
                strcpy( szExt, psz+1);
            }
        }
        else
        {
            strcpy( szExt, "");
        }
        sprintf(szMask, "%s%s", szFolder, szCurExt);

        filehandle = _findfirst(szMask, &fileinfo);

        if( filehandle == -1L)
            continue;
        
        SourceInfo * pSI = NULL;
        pSI = new SourceInfo;
        if( !pSI )
        {
            return false;
        }
        sprintf( szFilePath, "%s%s", szFolder, fileinfo.name);
        strcpy( pSI->m_szPath, (const char*)szFilePath);
        MultiByteToWideChar(CP_ACP, 0, (const char*)szFilePath, -1, pSI->m_wszPath, _MAX_PATH); 
        
        Add( pSI );
        nCounter++;

        nRes = _findnext(filehandle, &fileinfo);

        while( -1L != nRes )
        {
            pSI = NULL;
            pSI = new SourceInfo;
            if( !pSI )
            {
                return false;
            }
            sprintf( szFilePath, "%s%s", szFolder,fileinfo.name);
            strcpy( pSI->m_szPath, (const char*)szFilePath);
            MultiByteToWideChar(CP_ACP, 0, (const char*)szFilePath, -1, pSI->m_wszPath, _MAX_PATH); 

            Add( pSI );
            nCounter++;
            nRes = _findnext(filehandle, &fileinfo); 
        }// while

    } while( _strcmpi(szExt, "") && nCounter < 50 );

    if( 0 == Size() )
    {
        return false;
    }
    else
    {
        return true;
    }
}

//------------------------------------------------------------------------------
// Name: Clone
// Purpose: copies elements (nStart; nStart+n) to the list pML
// Parameters: n      - number of elements
//             pML    - cloned media list
//             nStart - start position in (this) list; default: 0
// Return: true if success;
//         false otherewise
//------------------------------------------------------------------------------
bool CMediaList::Clone(int n, CMediaList *pML, int nStart /* = 0 */)
{
    bool bRes = true;

    if( n < 1 || nStart < 0 || nStart + n > m_N)
        return false;

    pML->Clean();

    for(int i = nStart; i< nStart + n; i++)
    {
        SourceInfo *pSI = NULL;
        
        pSI = new SourceInfo;

        if( !pSI )
            goto cleanup;

        bRes = bRes && GetItem(i)->CopyTo( pSI);

        if( false == bRes)
        {
            delete pSI;
            goto cleanup;
        }

        pSI->m_bInUse = false;

        bRes = bRes && pML->Add(pSI);
        if( false == bRes)
            goto cleanup;
    }// for
    return true;

cleanup:
    pML->Clean();
    return false;
}

//------------------------------------------------------------------------------
// Name: AdjustDuration
// Purpose: calculates demonstration time. Here, it is duration of the longest file
//          Change this function to set a fixed time, average time etc.         
//------------------------------------------------------------------------------
void CMediaList::AdjustDuration()
{
    m_avgDuration = 0L;
    
    if( 0 == m_N )
    {
        return;
    }

    for( int i=0; i<m_N; i++)
    {
        
        m_avgDuration = (this->GetItem(i)->m_llDuration > m_avgDuration) ?
            this->GetItem(i)->m_llDuration :
            m_avgDuration;
    }// for

}


