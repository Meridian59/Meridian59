//-----------------------------------------------------------------------------
// Name: Lighting Direct3D Sample
// 
// Copyright (c) 1995-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Lighting samples shows how to use D3D lights when rendering.  It shows
   the difference between the various types of lights (ambient, point, 
   directional, spot), how to configure these lights, and how to enable and
   disable them.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Lighting
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   The following keys are implemented. The dropdown menus can be used for the
   same controls.
      <Enter>     Starts and stops the scene
      <Space>     Advances the scene by a small increment
      <F2>        Prompts user to select a new rendering device or display mode
      <Alt+Enter> Toggles between fullscreen and windowed modes
      <Esc>       Exits the app.
	

Programming Notes
=================
   D3D lights can be used to shade and color objects that are rendered.  They
   modify the color at each vertex of the primitives rendered while the lights
   are active.  Note that the walls and floor of this sample contain many 
   vertices, so the lighting is fairly detailed.  If the vertices were only at
   the corners of the walls and floor, the lighting effects would be much 
   rougher because of the reduced number of vertices.  You can modify the m_m
   and m_n member variables in this program to see how the vertex count affects 
   the lighting of the surfaces.

   There are ways to generate lighting effects other than by using D3D lights.
   Techniques like light mapping can create lighting effects that are not 
   limited to being computed only at each vertex.  Vertex shaders can generate
   more unusual, realistic, or customized lighting at each vertex.  Pixel shaders
   can perform lighting computation at each pixel of the polygons being rendered.
   
   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

