//-----------------------------------------------------------------------------
// File: TexArgs.h
//
// Desc: Header file to declare texture stage state arguments used for the app.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TEX_ARGS_H
#define TEX_ARGS_H




//-----------------------------------------------------------------------------
// String arrays for filling the form's combo boxes
//-----------------------------------------------------------------------------
TCHAR* g_astrPresetEffects[] =
{
    _T("Modulate"), _T("Modulate Alpha"), _T("Add"), _T("Decal Alpha"), 
	_T("Colored Light Map"), _T("Inverse Colored Light Map"), 
	_T("Single Channel Light Map"), _T("Modulate and Late Add"),
    _T("Linear Blend using Texture Alpha"), _T("Linear Blend using Diffuse Alpha"),
    _T("Add with smooth saturation"), _T("Multitexture Subtract"),
    _T("Add Diffuse to light map then modulate"), _T("Detail Modulate"), 
	_T("Detail Add"),
    NULL
};

TCHAR* g_astrTextureOps[] =
{
	// Basics
    _T("Disable"),
	_T("Select Arg 1"),
	_T("Select Arg 2"),

	// Modulate
    _T("Modulate"),
	_T("Modulate 2x"),
	_T("Modulate 4x"), 
	
    // Add
	_T("Add"),
	_T("Add Signed"),
	_T("Add Signed 2x"), 
	_T("Subtract"),
	_T("Add Smooth"), 
	
    // Linear alpha blend: Arg1*(Alpha) + Arg2*(1-Alpha)
	_T("Blend Diffuse"),
	_T("Blend Texture"),
    _T("Blend Factor"),

    // Linear alpha blend with pre-multiplied arg1 input: Arg1 + Arg2*(1-Alpha)
	_T("Blend Texture PM"),
	_T("Blend Current PM"),
    
    // Specular mapping
	_T("Premodulate"), 
	_T("Modulate Alpha Add Color"),
	_T("Modulate Color Add Alpha"), 
	_T("Modulate Inv Alpha Add Color"),
	_T("Modulate Inv Color Add Alpha"),

    // Bump mapping
	_T("Bump EnvMap"),
	_T("Bump EnvMap w/Luminance"),

    // DotProduct4
	_T("DotProduct3"),

	NULL
};

TCHAR* g_astrTextureArgs1[] =
{
    _T("Diffuse"), _T("Current"), _T("Texture"),
    _T("Diffuse Inv"), _T("Current Inv"), _T("Texture Inv"),
    _T("Tex w/Alpha"), NULL
};

TCHAR* g_astrTextureArgs2[] =
{
    _T("Diffuse"), _T("Current"), _T("Factor"),
    _T("Diffuse Inv"), _T("Current Inv"), _T("Factor Inv"),
    _T("Diffuse w/Alpha"), _T("Current w/Alpha"), _T("Factor w/Alpha"),
    NULL
};




//-----------------------------------------------------------------------------
// D3DTOP values corresponding to the strings above for the dropdown lists
//-----------------------------------------------------------------------------
WORD aTexOps[] =
{
    NULL, D3DTOP_DISABLE, D3DTOP_SELECTARG1, D3DTOP_SELECTARG2,
    D3DTOP_MODULATE, D3DTOP_MODULATE2X, D3DTOP_MODULATE4X,
    D3DTOP_ADD, D3DTOP_ADDSIGNED, D3DTOP_ADDSIGNED2X, D3DTOP_SUBTRACT, 
	D3DTOP_ADDSMOOTH, D3DTOP_BLENDDIFFUSEALPHA, D3DTOP_BLENDTEXTUREALPHA, 
	D3DTOP_BLENDFACTORALPHA, D3DTOP_BLENDTEXTUREALPHAPM, 
	D3DTOP_BLENDCURRENTALPHA, D3DTOP_PREMODULATE, 
	D3DTOP_MODULATEALPHA_ADDCOLOR, D3DTOP_MODULATECOLOR_ADDALPHA,
    D3DTOP_MODULATEINVALPHA_ADDCOLOR, D3DTOP_MODULATEINVCOLOR_ADDALPHA,
    D3DTOP_BUMPENVMAP, D3DTOP_BUMPENVMAPLUMINANCE, D3DTOP_DOTPRODUCT3
};

WORD aTexArgs1[] =
{
    NULL, D3DTA_DIFFUSE, D3DTA_CURRENT, D3DTA_TEXTURE,
    D3DTA_DIFFUSE|D3DTA_COMPLEMENT, D3DTA_CURRENT|D3DTA_COMPLEMENT,
    D3DTA_TEXTURE|D3DTA_COMPLEMENT, D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE
};

WORD aTexArgs2[] =
{
    NULL, D3DTA_DIFFUSE, D3DTA_CURRENT, D3DTA_TFACTOR,
    D3DTA_COMPLEMENT|D3DTA_DIFFUSE, D3DTA_COMPLEMENT|D3DTA_CURRENT,
        D3DTA_COMPLEMENT|D3DTA_TFACTOR,
    D3DTA_ALPHAREPLICATE|D3DTA_DIFFUSE, D3DTA_ALPHAREPLICATE|D3DTA_CURRENT,
        D3DTA_ALPHAREPLICATE|D3DTA_TFACTOR,
};





//-----------------------------------------------------------------------------
// Enumerated values to relate the combo box strings (listed above) with the
// combo box indices.
//-----------------------------------------------------------------------------
enum _TEXOPS { toDISABLE, toSELECTARG1, toSELECTARG2,
               toMODULATE, toMODULATE2X, toMODULATE4X, toADD,
               toADDSIGNED, toADDSIGNED2X, toSUBTRACT,
               toADDSMOOTH, toBLENDDIFFUSE, toBLENDTEXTURE,
               toBLENDFACTOR };

enum _TEXARGS1 { ta1DIFFUSE, ta1CURRENT, ta1TEXTURE, ta1INVDIFFUSE,
                ta1INVCURRENT, ta1INVTEXTURE, ta1ALPHATEXTURE };

enum _TEXARGS2 { ta2DIFFUSE, ta2CURRENT, ta2FACTOR,
                 ta2INVDIFFUSE, ta2INVCURRENT, ta2INVFACTOR,
                 ta2ALPHADIFFUSE, ta2ALPHACURRENT, ta2ALPHAFACTOR };




#endif

