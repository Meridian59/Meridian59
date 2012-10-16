//-----------------------------------------------------------------------------
// 
// Sample Name: Maze Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  This is a DirectPlay 8 client/server sample. The client comes in two flavors.  
  A console based version, and a D3D client.  The D3D client can optionally 
  be run as screen saver by simply copying mazeclient.exe to your 
  \winnt\system32\ and renaming it to mazeclient.scr.  This will make 
  it a screen saver that will be detected by the display control panel.  

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\VoicePosition 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  How to use MazeClient:
    If you run "mazeclient.exe" then you'll be in "settings mode".  
        A dialog box with various settings appears.
        
    If you run "mazeclient.exe /t" then you'll be in "Test mode" where 
        you don't have to connect to a server, or you can search and join 
        a server game. There are few simple commands you can type when running 
        mazeclient in "test mode":
          'a': Will turn auto-pilot on/off, but when moving about.
               There's isn't colision detection code so you can go through walls.
          'r': Will turn reflections on/off.
                      
    If you run "mazeclient.exe /s" then you'll be in "Screen Saver mode" on
        auto-pilot.  This mode will auto-connect to a server, using the settings 
        from "Setttings Mode".  If a server is not found, it will run without 
        connecting to a server.  It will also exit upon mouse/keyboard input.

  How to use MazeConsoleClient:
    Run it from the cmd line, and it will automatically search for a 
    session on the local subnet.  If you specify a IP address at the cmd 
    prompt, as in "MazeConsoleClient 255.255.255.255", it will search for 
    a session at that IP address.  If a server is not found or the session 
    is lost, then it will exit automatically.  Hit Crtl-C to close the 
    session.

  How to use MazeServer:
    Run it from the cmd line and it will automatically creates a host session 
    that clients can join.  
    
    MazeServer.exe takes an optional command line parameter to set the size of the maze. 
    For example:
    
        mazeserver.exe /size 16 128
        
    will set the maze to be 16 wide, and 128 high.  The width and height are restricted to 
    these numbers: 16, 32, 64, or 128.  
    
    Once started, the server will display a simple command prompt to control the server.  
    Here is the list of commands:

    "help"
        A list of simple commands you can do.  
    "stop"
        Shutdown the server.
    "stats"
        How many players are connected.
    "sr" or "serverreliable": 
        Set what percent of the packets going from the server to the client have 
        the DPNSEND_GUARANTEED flag.
    "cr" or "clientreliable" 
        Set what percent of the packets going from every client to the server 
        have the DPNSEND_GUARANTEED flag.
    "cu" or "clientupdate" 
        Set how many milliseconds pass between updates from each client. 
    "ct" or "clienttimeout" 
        Sets the timeout value of packets sent by the clients
    "st" or "servertimeout" 
        Sets the timeout value of packets sent by the server
    "ci" or "connectioninfo" 
        Displays info about the connection from the server to a client.  Ex. "ci 00300003"		        
    "loglevel" 
        Set how much extra info it spews about what is happening behind the scenes.  

