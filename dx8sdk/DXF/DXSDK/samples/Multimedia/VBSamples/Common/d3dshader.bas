Attribute VB_Name = "D3DShaders"

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DShader.bas
'  Content:    Shader constants
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

' destination/source parameter register type
Global Const D3DSI_COMMENTSIZE_SHIFT = 16
Global Const D3DSI_COMMENTSIZE_MASK = &H7FFF0000



'---------------------------------------------------------------------
' Vertex Shaders
'
'
'Vertex Shader Declaration
'
'The declaration portion of a vertex shader defines the static external
'interface of the shader.  The information in the declaration includes:
'
'- Assignments of vertex shader input registers to data streams.  These
'assignments bind a specific vertex register to a single component within a
'vertex stream.  A vertex stream element is identified by a byte offset
'within the stream and a type.  The type specifies the arithmetic data type
'plus the dimensionality (1, 2, 3, or 4 values).  Stream data which is
'less than 4 values are always expanded out to 4 values with zero or more
'0.F values and one 1.F value.
'
'- Assignment of vertex shader input registers to implicit data from the
'primitive tessellator.  This controls the loading of vertex data which is
'not loaded from a stream, but rather is generated during primitive
'tessellation prior to the vertex shader.
'
'- Loading data into the constant memory at the time a shader is set as the
'current shader.  Each token specifies values for one or more contiguous 4
'DWORD constant registers.  This allows the shader to update an arbitrary
'subset of the constant memory, overwriting the device state (which
'contains the current values of the constant memory).  Note that these
'values can be subsequently overwritten (between DrawPrimitive calls)
'during the time a shader is bound to a device via the
'SetVertexShaderConstant method.
'
'
'Declaration arrays are single-dimensional arrays of DWORDs composed of
'multiple tokens each of which is one or more DWORDs.  The single-DWORD
'token value 0xFFFFFFFF is a special token used to indicate the end of the
'declaration array.  The single DWORD token value 0x00000000 is a NOP token
'with is ignored during the declaration parsing.  Note that 0x00000000 is a
'valid value for DWORDs following the first DWORD for multiple word tokens.
'
'[31:29] TokenType
'    0x0 - NOP (requires all DWORD bits to be zero)
'    0x1 - stream selector
'    0x2 - stream data definition (map to vertex input memory)
'    0x3 - vertex input memory from tessellator
'    0x4 - constant memory from shader
'    0x5 - extension
'    0x6 - reserved
'    0x7 - end-of-array (requires all DWORD bits to be 1)
'
'NOP Token (single DWORD token)
'    [31:29] 0x0
'    [28:00] 0x0
'
'Stream Selector (single DWORD token)
'    [31:29] 0x1
'    [28]    indicates whether this is a tessellator stream
'    [27:04] 0x0
'    [03:00] stream selector (0..15)
'
'Stream Data Definition (single DWORD token)
'    Vertex Input Register Load
'      [31:29] 0x2
'      [28]    0x0
'      [27:20] 0x0
'      [19:16] type (dimensionality and data type)
'      [15:04] 0x0
'      [03:00] vertex register address (0..15)
'    Data Skip (no register load)
'      [31:29] 0x2
'      [28]    0x1
'      [27:20] 0x0
'      [19:16] count of DWORDS to skip over (0..15)
'      [15:00] 0x0
'    Vertex Input Memory from Tessellator Data (single DWORD token)
'      [31:29] 0x3
'      [28]    indicates whether data is normals or u/v
'      [27:24] 0x0
'      [23:20] vertex register address (0..15)
'      [19:16] type (dimensionality)
'      [15:04] 0x0
'      [03:00] vertex register address (0..15)
'
'Constant Memory from Shader (multiple DWORD token)
'    [31:29] 0x4
'    [28:25] count of 4*DWORD constants to load (0..15)
'    [24:07] 0x0
'    [06:00] constant memory address (0..95)
'
'Extension Token (single or multiple DWORD token)
'    [31:29] 0x5
'    [28:24] count of additional DWORDs in token (0..31)
'    [23:00] extension-specific information
'
'End-of-array token (single DWORD token)
'    [31:29] 0x7
'    [28:00] 0x1fffffff
'
'The stream selector token must be immediately followed by a contiguous set of stream data definition tokens.  This token sequence fully defines that stream, including the set of elements within the stream, the order in which the elements appear, the type of each element, and the vertex register into which to load an element.
'Streams are allowed to include data which is not loaded into a vertex register, thus allowing data which is not used for this shader to exist in the vertex stream.  This skipped data is defined only by a count of DWORDs to skip over, since the type information is irrelevant.
'The token sequence:
'Stream Select: stream=0
'Stream Data Definition (Load): type=FLOAT3; register=3
'Stream Data Definition (Load): type=FLOAT3; register=4
'Stream Data Definition (Skip): count=2
'Stream Data Definition (Load): type=FLOAT2; register=7
'
'defines stream zero to consist of 4 elements, 3 of which are loaded into registers and the fourth skipped over.  Register 3 is loaded with the first three DWORDs in each vertex interpreted as FLOAT data.  Register 4 is loaded with the 4th, 5th, and 6th DWORDs interpreted as FLOAT data.  The next two DWORDs (7th and 8th) are skipped over and not loaded into any vertex input register.   Register 7 is loaded with the 9th and 10th DWORDS interpreted as FLOAT data.
'Placing of tokens other than NOPs between the Stream Selector and Stream Data Definition tokens is disallowed.'
'
'

' Vertex Shader 1.0 register limits. D3D device must provide at least
' specified number of registers
'
Global Const D3DVS_INPUTREG_MAX_V1_1 = 16
Global Const D3DVS_TEMPREG_MAX_V1_1 = 12
' This max required number. Device could have more registers. Check caps.
Global Const D3DVS_CONSTREG_MAX_V1_1 = 96
Global Const D3DVS_TCRDOUTREG_MAX_V1_1 = 8
Global Const D3DVS_ADDRREG_MAX_V1_1 = 1
Global Const D3DVS_ATTROUTREG_MAX_V1_1 = 2
Global Const D3DVS_MAXINSTRUCTIONCOUNT_V1_1 = 128

' Pixel Shader DX8 register limits. D3D device will have at most these
' specified number of registers
'
Global Const D3DPS_INPUTREG_MAX_DX8 = 8
Global Const D3DPS_TEMPREG_MAX_DX8 = 8
Global Const D3DPS_CONSTREG_MAX_DX8 = 8
Global Const D3DPS_TEXTUREREG_MAX_DX8 = 8


Enum D3DVSD_TOKENTYPE
    D3DVSD_TOKEN_NOP = 0             ' NOP or extension
    D3DVSD_TOKEN_STREAM = 1          ' stream selector
    D3DVSD_TOKEN_STREAMDATA = 2      ' stream data definition (map to vertex input memory)
    D3DVSD_TOKEN_TESSELLATOR = 3     ' vertex input memory from tessellator
    D3DVSD_TOKEN_constMEM = 4        ' global constant memory from shader
    D3DVSD_TOKEN_EXT = 5             ' extension
    D3DVSD_TOKEN_END = 7             ' end-of-array (requires all DWORD bits to be 1)
End Enum


'3322 2222 2222 1111 1111 11
'1098 7654 3210 9876 5432 1098 7654 3210



Global Const D3DVSD_TOKENTYPESHIFT = 29
Global Const D3DVSD_TOKENTYPEMASK = &HE0000000 '  (7 << D3DVSD_TOKENTYPESHIFT)

Global Const D3DVSD_STREAMNUMBERSHIFT = 0
Global Const D3DVSD_STREAMNUMBERMASK = &HF&  '(0xF << D3DVSD_STREAMNUMBERSHIFT)

Global Const D3DVSD_DATALOADTYPESHIFT = 28
Global Const D3DVSD_DATALOADTYPEMASK = &H10000000 '(0x1 << D3DVSD_DATALOADTYPESHIFT)

Global Const D3DVSD_DATATYPESHIFT = 16
Global Const D3DVSD_DATATYPEMASK = &HF& * 2 ^ D3DVSD_DATATYPESHIFT '0xF<< D3DVSD_DATATYPESHIFT)

Global Const D3DVSD_SKIPCOUNTSHIFT = 16
Global Const D3DVSD_SKIPCOUNTMASK = &HF& * 2 ^ D3DVSD_SKIPCOUNTSHIFT '(0xF << D3DVSD_SKIPCOUNTSHIFT)

Global Const D3DVSD_VERTEXREGSHIFT = 0
Global Const D3DVSD_VERTEXREGMASK = &HF& * 2 ^ D3DVSD_VERTEXREGSHIFT '(0xF << D3DVSD_VERTEXREGSHIFT)

Global Const D3DVSD_VERTEXREGINSHIFT = 20
Global Const D3DVSD_VERTEXREGINMASK = &HF& * 2 ^ D3DVSD_VERTEXREGINSHIFT '(0xF << D3DVSD_VERTEXREGINSHIFT)

Global Const D3DVSD_CONSTCOUNTSHIFT = 25
Global Const D3DVSD_CONSTCOUNTMASK = &HF& * 2 ^ D3DVSD_CONSTCOUNTSHIFT ' (0xF << D3DVSD_global constCOUNTSHIFT)

Global Const D3DVSD_CONSTADDRESSSHIFT = 0
Global Const D3DVSD_CONSTADDRESSMASK = &H7F&    '(0x7F << D3DVSDCONSTADDRESSSHIFT)

Global Const D3DVSD_CONSTRSSHIFT = 16
Global Const D3DVSD_CONSTRSMASK = &H1FFF0000    '(0x1FFF << D3DVSDCONSTRSSHIFT)

Global Const D3DVSD_EXTCOUNTSHIFT = 24
Global Const D3DVSD_EXTCOUNTMASK = &H1F& * 2 ^ D3DVSD_EXTCOUNTSHIFT     '(0x1F << D3DVSD_EXTCOUNTSHIFT)

Global Const D3DVSD_EXTINFOSHIFT = 0
Global Const D3DVSD_EXTINFOMASK = &HFFFFFF                '(0xFFFFFF << D3DVSD_EXTINFOSHIFT)


' bit declarations for _Type fields
Global Const D3DVSDT_FLOAT1 = 0&        ' 1D float expanded to (value, 0., 0., 1.)
Global Const D3DVSDT_FLOAT2 = 1&        ' 2D float expanded to (value, value, 0., 1.)
Global Const D3DVSDT_FLOAT3 = 2&        ' 3D float expanded to (value, value, value, 1.)
Global Const D3DVSDT_FLOAT4 = 3&        ' 4D float
Global Const D3DVSDT_D3DCOLOR = 4&      ' 4D packed unsigned bytes mapped to 0. to 1. range                                    // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
Global Const D3DVSDT_UBYTE4 = 5&        ' 4D unsigned byte
Global Const D3DVSDT_SHORT2 = 6&        ' 2D signed short expanded to (value, value, 0., 1.)
Global Const D3DVSDT_SHORT4 = 7&        ' 4D signed short

' assignments of vertex input registers for fixed function vertex shader
'
Global Const D3DVSDE_POSITION = 0&
Global Const D3DVSDE_BLENDWEIGHT = 1&
Global Const D3DVSDE_BLENDINDICES = 2&
Global Const D3DVSDE_NORMAL = 3&
Global Const D3DVSDE_PSIZE = 4&
Global Const D3DVSDE_DIFFUSE = 5&
Global Const D3DVSDE_SPECULAR = 6&
Global Const D3DVSDE_TEXCOORD0 = 7&
Global Const D3DVSDE_TEXCOORD1 = 8&
Global Const D3DVSDE_TEXCOORD2 = 9&
Global Const D3DVSDE_TEXCOORD3 = 10&
Global Const D3DVSDE_TEXCOORD4 = 11&
Global Const D3DVSDE_TEXCOORD5 = 12&
Global Const D3DVSDE_TEXCOORD6 = 13&
Global Const D3DVSDE_TEXCOORD7 = 14&
Global Const D3DVSDE_POSITION2 = 15&
Global Const D3DVSDE_NORMAL2 = 16&

' Maximum supported number of texture coordinate sets
Global Const D3DDP_MAXTEXCOORD = 8

'---------------------------------------------------------------------
'
' Pixel Shader (PS) & Vertex Shader (VS) Instruction Token Definition
'
' **** Version Token ****
' [07:00] minor version number
' [15:08] major version number
' [31:16]
'    PS 0xFFFF
'    VS 0xFFFE
'
' **** End Token ****
' [31:00] 0x0000FFFF
'
' **** Comment Token ****
' [15:00] 0xFFFE
' [30:16] DWORD Length (up to 2^15 DWORDS = 128KB)
' [31] 0x0
'
' **** Instruction Token ****
' [15:00] Opcode (D3DSIO_*)
' [23:16] Opcode-Specific Controls
' [30:24] Reserved 0x0
' [31] 0x0
'
' **** Destination Parameter Token ****
' [07:00] Register Number (offset in register file)
' [15:08] Reserved 0x0
' [19:16] Write Mask
'   [16] Component 0 (X;Red)
'   [17] Component 1 (Y;Green)
'   [18] Component 2 (Z;Blue)
'   [19] Component 3 (W;Alpha)
' [23:20]
'   PS Result Modifier (bias;..)
'   VS Reserved 0x0
' [27:24]
'   PS Result Shift Scale (signed )
'   VS Reserved 0x0
' [30:28] Register Type
'   [0x0] Temporary Register File
'   [0x1] Reserved
'   [0x2] Reserved
'   [0x3]
'      VS Address Register (reg num must be zero)
'      PS Reserved
'   [0x4]
'      VS Rasterizer Output Register File
'      PS Reserved
'   [0x5]
'      VS Attribute Output Register File
'      PS Reserved
'   [0x6]
'      VS Texture Coordinate Register File
'      PS Reserved
'   [0x7] Reserved
' [31] 0x1
'
' **** Source Parameter Token *****
' [12:00] Register Number (offset in register file)
' [14:13]
'   VS Relative Address Register Component
'   PS Reserved 0x0
' [15]
'   VS Relative Address
'   PS Reserved 0x0
' [23:16] Source Component Swizzle
'   [17:16] Component 0 Swizzle
'   [19:18] Component 1 Swizzle
'   [21:20] Component 2 Swizzle
'   [23:22] Component 3 Swizzle
' [27:24] Source Modifier
'     [0x0] None
'     [0x1] Negate
'     [0x2] Complement
'     [0x3-0xf] Reserved
' [30:28] Register Type
'   [0x0] Temporary Register File
'   [0x1] Input Register File
'   [0x2] global constant Register File
'   [0x3-0x7] Reserved
' [31] 0x1
'

'
' Instruction Token Bit Definitions
'
Global Const D3DSI_OPCODE_MASK = &HFFFF&

Enum D3DSHADER_INSTRUCTION_OPCODE_TYPE

    D3DSIO_NOP = 0            ' PS/VS
    D3DSIO_MOV = 1                ' PS/VS
    D3DSIO_ADD = 2                ' PS/VS
    D3DSIO_SUB = 3                ' PS
    D3DSIO_MAD = 4                ' PS/VS
    D3DSIO_MUL = 5                ' PS/VS
    D3DSIO_RCP = 6                ' VS
    D3DSIO_RSQ = 7                ' VS
    D3DSIO_DP3 = 8                ' PS/VS
    D3DSIO_DP4 = 9                ' VS
    D3DSIO_MIN = 10                ' VS
    D3DSIO_MAX = 11                ' VS
    D3DSIO_SLT = 12                ' VS
    D3DSIO_SGE = 13                ' VS
    D3DSIO_EXP = 14                ' VS
    D3DSIO_LOG = 15                ' VS
    D3DSIO_LIT = 16                ' VS
    D3DSIO_DST = 17                ' VS
    D3DSIO_LRP = 18                ' PS
    D3DSIO_FRC = 19                ' VS
    D3DSIO_M4x4 = 20               ' VS
    D3DSIO_M4x3 = 21               ' VS
    D3DSIO_M3x4 = 22               ' VS
    D3DSIO_M3x3 = 23               ' VS
    D3DSIO_M3x2 = 24               ' VS

    D3DSIO_TEXCOORD = 64           ' PS
    D3DSIO_TEXKILL = 65            ' PS
    D3DSIO_TEX = 66                ' PS
    D3DSIO_TEXBEM = 67             ' PS
    D3DSIO_TEXBEML = 68            ' PS
    D3DSIO_TEXREG2AR = 69          ' PS
    D3DSIO_TEXREG2GB = 70          ' PS
    D3DSIO_TEXM3x2PAD = 71         ' PS
    D3DSIO_TEXM3x2TEX = 72         ' PS
    D3DSIO_TEXM3x3PAD = 73         ' PS
    D3DSIO_TEXM3x3TEX = 74         ' PS
    D3DSIO_TEXM3x3DIFF = 75        ' PS
    D3DSIO_TEXM3x3SPEC = 76        ' PS
    D3DSIO_TEXM3x3VSPEC = 77       ' PS
    D3DSIO_EXPP = 78               ' VS
    D3DSIO_LOGP = 79               ' VS
    D3DSIO_CND = 80                ' PS
    D3DSIO_DEF = 81                ' PS

    D3DSIO_COMMENT = &HFFFE&
    D3DSIO_END = &HFFFF&
End Enum

'
' Co-Issue Instruction Modifier - if set then this instruction is to be
' issued in parallel with the previous instruction(s) for which this bit
' is not set.
'
Global Const D3DSI_COISSUE = &H40000000


'
' Parameter Token Bit Definitions
'
Global Const D3DSP_REGNUM_MASK = &HFFF&

' destination parameter write mask
Global Const D3DSP_WRITEMASK_0 = &H10000           ' Component 0 (X;Red)
Global Const D3DSP_WRITEMASK_1 = &H20000           ' Component 1 (Y;Green)
Global Const D3DSP_WRITEMASK_2 = &H40000           ' Component 2 (Z;Blue)
Global Const D3DSP_WRITEMASK_3 = &H80000           ' Component 3 (W;Alpha)
Global Const D3DSP_WRITEMASK_ALL = &HF0000         ' All Components

' destination parameter modifiers
Global Const D3DSP_DSTMOD_SHIFT = 20
Global Const D3DSP_DSTMOD_MASK = &HF00000

Enum D3DSHADER_PARAM_DSTMOD_TYPE

    D3DSPDM_NONE = 0 * 2 ^ D3DSP_DSTMOD_SHIFT '0<<D3DSP_DSTMOD_SHIFT, ' nop
    D3DSPDM_SATURATE = 1 * 2 ^ D3DSP_DSTMOD_SHIFT '1<<D3DSP_DSTMOD_SHIFT, 'clamp 0 to 1 range

End Enum

' destination parameter
Global Const D3DSP_DSTSHIFT_SHIFT = 24
Global Const D3DSP_DSTSHIFT_MASK = &HF000000

' destination/source parameter register type
Global Const D3DSP_REGTYPE_SHIFT = 28
Global Const D3DSP_REGTYPE_MASK = &H70000000

Global Const D3DVSD_STREAMTESSSHIFT = 28
Global Const D3DVSD_STREAMTESSMASK = 2 ^ D3DVSD_STREAMTESSSHIFT '(1 << D3DVSD_STREAMTESSSHIFT)

Enum D3DSHADER_PARAM_REGISTER_TYPE

    D3DSPR_TEMP = &H0&              '0<<D3DSP_REGTYPE_SHIFT, ' Temporary Register File
    D3DSPR_INPUT = &H20000000       '1<<D3DSP_REGTYPE_SHIFT, ' Input Register File
    D3DSPR_CONST = &H40000000       '2<<D3DSP_REGTYPE_SHIFT, ' global constant Register File
    D3DSPR_ADDR = &H60000000        '3<<D3DSP_REGTYPE_SHIFT, ' Address Register (VS)
    D3DSPR_TEXTURE = &H60000000     '3<<D3DSP_REGTYPE_SHIFT, ' Texture Register File (PS)
    D3DSPR_RASTOUT = &H80000000     '4<<D3DSP_REGTYPE_SHIFT, ' Rasterizer Register File
    D3DSPR_ATTROUT = &HA0000000     '5<<D3DSP_REGTYPE_SHIFT, ' Attribute Output Register File
    D3DSPR_TEXCRDOUT = &HC0000000   '6<<D3DSP_REGTYPE_SHIFT, ' Texture Coordinate Output Register File
End Enum

' Register offsets in the Rasterizer Register File
'
Enum D3DVS_RASTOUT_OFFSETS
    D3DSRO_POSITION = 0
    D3DSRO_FOG = 1
    D3DSRO_POINT_SIZE = 2
End Enum

' Source operand addressing modes

Global Const D3DVS_ADDRESSMODE_SHIFT = 13
Global Const D3DVS_ADDRESSMODE_MASK = (2 ^ D3DVS_ADDRESSMODE_SHIFT)

Enum D3DVS_ADRRESSMODE_TYPE
    D3DVS_ADDRMODE_ABSOLUTE = 0     '(0 << D3DVS_ADDRESSMODE_SHIFT),
    D3DVS_ADDRMODE_RELATIVE = 2 ^ D3DVS_ADDRESSMODE_SHIFT '(1 << D3DVS_ADDRESSMODE_SHIFT)   ' Relative to register A0
End Enum

' Source operand swizzle definitions
'
Global Const D3DVS_SWIZZLE_SHIFT = 16
Global Const D3DVS_SWIZZLE_MASK = &HFF0000

' The following bits define where to take component X:

Global Const D3DVS_X_X = (0 * 2 ^ D3DVS_SWIZZLE_SHIFT)   '(0 << D3DVS_SWIZZLE_SHIFT)
Global Const D3DVS_X_Y = (1 * 2 ^ D3DVS_SWIZZLE_SHIFT)   '(1 << D3DVS_SWIZZLE_SHIFT)
Global Const D3DVS_X_Z = (2 * 2 ^ D3DVS_SWIZZLE_SHIFT)   '(2 << D3DVS_SWIZZLE_SHIFT)
Global Const D3DVS_X_W = (3 * 2 ^ D3DVS_SWIZZLE_SHIFT)   '(3 << D3DVS_SWIZZLE_SHIFT)

' The following bits define where to take component Y:

Global Const D3DVS_Y_X = (0 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 2))     '  (0 << (D3DVS_SWIZZLE_SHIFT + 2))
Global Const D3DVS_Y_Y = (1 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 2))      ' (1 << (D3DVS_SWIZZLE_SHIFT + 2))
Global Const D3DVS_Y_Z = (2 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 2))      ' (2 << (D3DVS_SWIZZLE_SHIFT + 2))
Global Const D3DVS_Y_W = (3 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 2))      ' (3 << (D3DVS_SWIZZLE_SHIFT + 2))

' The following bits define where to take component Z:

Global Const D3DVS_Z_X = (0 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 4))         '(0 << (D3DVS_SWIZZLE_SHIFT + 4))
Global Const D3DVS_Z_Y = (1 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 4))        '(1 << (D3DVS_SWIZZLE_SHIFT + 4))
Global Const D3DVS_Z_Z = (2 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 4))         '(2 << (D3DVS_SWIZZLE_SHIFT + 4))
Global Const D3DVS_Z_W = (3 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 4))        '(3 << (D3DVS_SWIZZLE_SHIFT + 4))

' The following bits define where to take component W:

Global Const D3DVS_W_X = (0 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 6))            '(0 << (D3DVS_SWIZZLE_SHIFT + 6))
Global Const D3DVS_W_Y = (1 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 6))            '(1 << (D3DVS_SWIZZLE_SHIFT + 6))
Global Const D3DVS_W_Z = (2 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 6))            '(2 << (D3DVS_SWIZZLE_SHIFT + 6))
Global Const D3DVS_W_W = (3 * 2 ^ (D3DVS_SWIZZLE_SHIFT + 6))             '(3 << (D3DVS_SWIZZLE_SHIFT + 6))

' Value when there is no swizzle (X is taken from X, Y is taken from Y,
' Z is taken from Z, W is taken from W
'
Global Const D3DVS_NOSWIZZLE = (D3DVS_X_X Or D3DVS_Y_Y Or D3DVS_Z_Z Or D3DVS_W_W)

' source parameter swizzle
Global Const D3DSP_SWIZZLE_SHIFT = 16
Global Const D3DSP_SWIZZLE_MASK = &HFF0000

Global Const D3DSP_NOSWIZZLE = _
    ((0 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 0)) Or _
      (1 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 2)) Or _
      (2 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 4)) Or _
      (3 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 6)))

' pixel-shader swizzle ops
Global Const D3DSP_REPLICATEALPHA = _
    ((3 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 0)) Or _
      (3 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 2)) Or _
      (3 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 4)) Or _
      (3 * 2 ^ (D3DSP_SWIZZLE_SHIFT + 6)))
      

' source parameter modifiers
Global Const D3DSP_SRCMOD_SHIFT = 24
Global Const D3DSP_SRCMOD_MASK = &HF000000

Enum D3DSHADER_PARAM_SRCMOD_TYPE
    D3DSPSM_NONE = 0 * 2 ^ D3DSP_SRCMOD_SHIFT    '0<<D3DSP_SRCMOD_SHIFT, ' nop
    D3DSPSM_NEG = 1 * 2 ^ D3DSP_SRCMOD_SHIFT     ' negate
    D3DSPSM_BIAS = 2 * 2 ^ D3DSP_SRCMOD_SHIFT    ' bias
    D3DSPSM_BIASNEG = 3 * 2 ^ D3DSP_SRCMOD_SHIFT ' bias and negate
    D3DSPSM_SIGN = 4 * 2 ^ D3DSP_SRCMOD_SHIFT    ' sign
    D3DSPSM_SIGNNEG = 5 * 2 ^ D3DSP_SRCMOD_SHIFT ' sign and negate
    D3DSPSM_COMP = 6 * 2 ^ D3DSP_SRCMOD_SHIFT    ' complement
End Enum



'''''''''''''''''''''''''''''''''''
'PIXELSHADER FUNCTIONS
'''''''''''''''''''''''''''''''''''


' pixel shader version token
Function D3DPS_VERSION(Major As Long, Minor As Long) As Long
    D3DPS_VERSION = (&HFFFF0000 Or ((Major) * 2 ^ 8) Or (Minor))
End Function


' vertex shader version token
Function D3DVS_VERSION(Major As Long, Minor As Long) As Long
    D3DVS_VERSION = (&HFFFE0000 Or ((Major) * 2 ^ 8) Or (Minor))
End Function


' extract major/minor from version cap
Function D3DSHADER_VERSION_MAJOR(Version As Long) As Long
    D3DSHADER_VERSION_MAJOR = (((Version) \ 8) And &HFF&)
End Function
    
Function D3DSHADER_VERSION_MINOR(Version As Long) As Long
    D3DSHADER_VERSION_MINOR = (((Version)) And &HFF&)
End Function
    

Function D3DSHADER_COMMENT(DWordSize As Long) As Long
    D3DSHADER_COMMENT = ((((DWordSize) * 2 ^ D3DSI_COMMENTSIZE_SHIFT) And D3DSI_COMMENTSIZE_MASK) Or D3DSIO_COMMENT)
End Function
    

' pixel/vertex shader end token
Function D3DPS_END() As Long
    D3DPS_END = &HFFFF&
End Function

Function D3DVS_END() As Long
   D3DVS_END = &HFFFF&
End Function





'''''''''''''''''''''''''''''''''''
'VERTEXSHADER FUNCTIONS
'''''''''''''''''''''''''''''''''''


Function D3DVSD_MAKETOKENTYPE(tokenType As Long) As Long
    Dim out As Long
    Select Case tokenType
        Case D3DVSD_TOKEN_NOP    '0
            out = 0
        Case D3DVSD_TOKEN_STREAM '1          ' stream selector
            out = &H20000000
        Case D3DVSD_TOKEN_STREAMDATA     '2      ' stream data definition (map to vertex input memory)
            out = &H40000000
        Case D3DVSD_TOKEN_TESSELLATOR ' 3     ' vertex input memory from tessellator
            out = &H60000000
        Case D3DVSD_TOKEN_constMEM  '4        ' global constant memory from shader
            out = &H80000000
        Case D3DVSD_TOKEN_EXT    '5             ' extension
            out = &HA0000000
        Case D3DVSD_TOKEN_END   '7             ' end-of-array (requires all DWORD bits to be 1)
            out = &HFFFFFFFF
    End Select
    D3DVSD_MAKETOKENTYPE = out And D3DVSD_TOKENTYPEMASK
End Function


' macros for generation of CreateVertexShader Declaration token array

' Set current stream
' _StreamNumber [0..(MaxStreams-1)] stream to get data from
'
Function D3DVSD_STREAM(StreamNumber As Long) As Long
    D3DVSD_STREAM = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) Or (StreamNumber))
End Function


' Set tessellator stream
'
Function D3DVSD_STREAM_TESS() As Long
    D3DVSD_STREAM_TESS = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) Or (D3DVSD_STREAMTESSMASK))
End Function
    

' bind single vertex register to vertex element from vertex stream
'
' VertexRegister [0..15] address of the vertex register
' dataType [D3DVSDT_*] dimensionality and arithmetic data type
Function D3DVSD_REG(VertexRegister As Long, dataType As Long) As Long
    D3DVSD_REG = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) Or _
     ((dataType) * 2 ^ D3DVSD_DATATYPESHIFT) Or (VertexRegister))
End Function

' Skip DWORDCount DWORDs (vb longs) in vertex
'
Function D3DVSD_SKIP(DWORDCount As Long) As Long
    D3DVSD_SKIP = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) Or &H10000000 Or _
     ((DWORDCount) * 2 ^ D3DVSD_SKIPCOUNTSHIFT))
End Function
    
' load data into vertex shader global constant memory
'
' _constantAddress [0..95] - address of global constant array to begin filling data
' _Count [0..15] - number of global constant vectors to load (4 DWORDs each)
' followed by 4*_Count DWORDS of data
'
Function D3DVSD_CONST(constantAddress As Long, count As Long) As Long
    D3DVSD_CONST = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_constMEM) Or _
     ((count) * 2 ^ D3DVSD_CONSTCOUNTSHIFT) Or (constantAddress))
End Function

' enable tessellator generated normals
'
'  VertexRegisterIn  [0..15] address of vertex register whose input stream
'                            will be used in normal computation
'  VertexRegisterOut [0..15] address of vertex register to output the normal to
'
Function D3DVSD_TESSNORMAL(VertexRegisterIn As Long, VertexRegisterOut As Long) As Long
    D3DVSD_TESSNORMAL = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_TESSELLATOR) Or _
     ((VertexRegisterIn) * 2 ^ D3DVSD_VERTEXREGINSHIFT) Or _
     ((&H2&) * 2 ^ D3DVSD_DATATYPESHIFT) Or (VertexRegisterOut))
End Function
   

' enable tessellator generated surface parameters
'
' VertexRegister [0..15] address of vertex register to output parameters
'
Function D3DVSD_TESSUV(VertexRegister As Long) As Long
    D3DVSD_TESSUV = (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_TESSELLATOR) Or &H10000000 Or _
     ((&H1&) * 2 ^ D3DVSD_DATATYPESHIFT) Or (VertexRegister))
End Function

' Generates END token
'
Function D3DVSD_END() As Long
        D3DVSD_END = &HFFFFFFFF
End Function

Function D3DVSD_NOP() As Long
    D3DVSD_NOP = 0
End Function


