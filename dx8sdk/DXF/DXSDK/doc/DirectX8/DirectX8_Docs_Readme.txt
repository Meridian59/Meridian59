
            Microsoft DirectX 8.1 SDK Readme File
                        October 2001
  (c) Microsoft Corporation, 2001. All rights reserved.

This document provides late-breaking or other information that supplements the Microsoft DirectX 8.1 SDK documentation.

------------------------
How to Use This Document
------------------------

To view the Readme file on-screen in Windows Notepad, maximize the Notepad window. On the Edit menu, click Word Wrap. To print the Readme file, open it in Notepad or another word processor, and then use the Print command on the File menu.

---------
CONTENTS
---------

1.  PRINTING ISSUES

2.  DIRECTX GRAPHICS
    2.1  IDirect3D8::CheckDeviceFormat Method
    2.2  IDirect3D8::CreateVertexShader Method
    2.3  D3DXMATRIXA16
    2.4  Pixel Shader Instruction texm3x3spec
    2.5  Vertex Shader Macros
    2.6  D3DXVec2Transform Function

3.  DIRECTPLAY
    3.1  IDirectPlay8Peer::EnumHosts and IDirectPlay8Client::EnumHosts


-------------------

1.  PRINTING ISSUES

If you try to print a heading and all of its subtopics, there are several known issues.

- There will be no formatting on the pages.
- If you are using Microsoft Internet Explorer 5.5, content might be missing from your printed pages. Use Internet Explorer 5.0 or 6.0 instead.

If you print only one topic at a time, the printed page should contain all the formatting and content.  This is the recommended printing method.

To change the font for printing, click the Font button. Successive clicks will take you through the various font options. It is recommended that you select the smallest font size for printing.
 

2.   DIRECTX GRAPHICS 

2.1  IDirect3D8::CheckDeviceFormat Method

The IDirect3D8::CheckDeviceFormat method takes the Usage parameter, which is missing a value in the documentation. D3DUSAGE_DYNAMIC may also be used.

2.2  IDirect3D8::CreateVertexShader Method

For mixed mode devices, CreateVertexShader might fail validation if three things happen.

- The device is in hardware vertex-processing mode. 
- The application calls CreateVertexShader with software usage.
- The number of streams supported by hardware is different than the number of streams declared by the vertex shader declaration. 

The failure is a result of the CreateVertexShader validation incorrectly using the hardware device to check the number of data streams, instead of using the number of streams in the vertex shader declaration. The workaround in version 8.1 is to enable the software vertex processing mode before calling CreateVertexShader.


2.3   D3DXMATRIXA16  

A 16-byte (quad WORD) aligned D3DXMATRIX. A 16-byte aligned matrix with a size and a starting memory address that are multiples of 16. Some CPUs use an aligned matrix to optimize the performance of D3DX math functions.

The following list shows which compilers support aligned matrices.

- Microsoft Visual Studio 7 compilers guarantee 16-byte aligned D3DXMATRIXA16 variables.

- Visual Studio 6 compilers with the Visual Studio 6 Processor Pack installed guarantee 16-byte aligned D3DXMATRIXA16 variables. In order to get _ALIGN_16 and D3DXMATRIXA16 defined, you must manually edit the D3dx8math.h file and remove the check for Visual Studio 7 (#if _MSC_VER >= 1300).

- All compilers guarantee 16-byte aligned D3DXMATRIXA16 variables allocated on the heap. 

- Non-Visual Studio compilers do not guarantee 16-byte aligned D3DXMATRIXA16 variables when they are allocated as a global variable or as a local variable on the stack.

D3DX does not define 16-byte aligned vectors or quaternions. It is suggested that you use _ALIGN_16 on your vectors and quaternions with D3DX math functions, because 16-byte alignment greatly improves performance for some processors. For a definition of _ALIGN_16, see D3dx8math.h.


2.4  Pixel Shader Instruction texm3x3spec

The reference page for the pixel shader instruction texm3x3spec incorrectly lists three source registers as input. This instruction takes only two source registers as inputs, as shown in the table following:
 
                                 Registers
Argument  Description            vn   cn   tn    rn    Version
--------  -----------           -------------------    -------
dest      destination register              x          1.0, 1.1, 1.2, 1.3, 1.4
src0      source register                   x          1.0, 1.1, 1.2, 1.3, 1.4
src1      source register              x               1.0, 1.1, 1.2, 1.3, 1.4


2.5  Vertex Shader Macros
Be careful when using vertex shader macros m3x2, m3x3, m3x4, m4x3, and m4x4 because the output of these macros is undefined if the input and output parameters use the same register. 


2.6  D3DXVec2Transform Function

In the documentation for the function D3DXVec2Transform, the first parameter type is incorrectly listed as D3DXVector4** pOut (a pointer to a pointer). The first parameter type should be D3DXVector4* pOut (a pointer). Therefore, the correct function signature is:
 
D3DXVECTOR4* WINAPI D3DXVec2Transform(
   D3DXVECTOR4 *pOut, 
   CONST D3DXVECTOR2 *pV, 
   CONST D3DXMATRIX *pM );


3.   DIRECTPLAY

3.1  IDirectPlay8Peer::EnumHosts and IDirectPlay8Client::EnumHosts

The documentation is missing a possible return value. DPNERR_USERCANCEL will be returned if the application stops the enumeration. 

Also, the following sentence in the fourth paragraph:

"Asynchronous enumerations can be stopped at any time by calling IDirectPlay8Peer::CancelAsyncOperation and either passing the handle returned in the pAsyncHandle parameter or setting the DPENUM_CANCEL flag in the dwFlags parameter."

should read:

"Asynchronous enumerations can be stopped at any time by calling IDirectPlay8Peer::CancelAsyncOperation and either passing the handle returned in the pAsyncHandle parameter or setting the DPNCANCEL_ENUM flag in the dwFlags parameter."
