//-----------------------------------------------------------------------------
// 
// Sample Name: AddressOverride Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  AddressOverride shows how to programmatically tell DirectPlay the addressing 
  in order to host or connect to another session on the network, instead of just 
  letting DirectPlay popup a dialog box to ask the user the connection settings.  
  Most games will typically follow this method so they can customize the UI.
  
Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\AddressOverride 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Enter the player's name, session, and check or uncheck the 'host sesion' 
  checkbox.  
  
  Choose a service provider, and adapter.  If the TCP/IP SP is chosen, and 
  hosting then enter port or it leave blank to have DirectPlay automatically 
  pick a port.  If joining, then enter the port and the IP address of the host, 
  or leave blank to search the subnet.  If the modem SP is chosen, then enter 
  the phone number. If the IPX SP is chosen, then enter the port. If the 
  serial SP is chosen, then the simple dialog will use the default DirectPlay 
  dialog to query the user for the serial settings.
  
  Click OK to start or join a game.  Once in a session, it presents a 
  simple game similiar to SimplePeer.  

Programming Notes
=================
  The AddressOverride sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  The AddressOverride differs by letting programmatically telling DirectPlay
  the address instead of just letting DirectPlay popup a dialog box.  
  
  * The "OK" button is pressed.  See LaunchMultiplayerGame().
        1. It gathers if the user wants to host or join.
        2. It gathers the SP guid from the UI
        3. If hosting it creates a IDirectPlay8Address* called pHostAddress and 
           calls IDirectPlay8Address::SetSP() on it.
        4. If joining or hosting it creates a IDirectPlay8Address* called 
           pDeviceAddress and calls IDirectPlay8Address::SetSP() on it.
        5. If gathers from the UI if an adapter was selected.  If it was it calls
           IDirectPlay8Address::SetDevice() on the pDeviceAddress.
        6. It then fills in the pDeviceAddress and the pHostAddress depending
           on the SP selected, and if joining or hosting by calling
           IDirectPlay8Address::AddComponent.  Here's how that works:
           
           - If the TCP/IP SP was selected and hosting, then:
                - If a port was entered it calls 
                  IDirectPlay8Address::AddComponent( DPNA_KEY_PORT, ... )
                  on the pDeviceAddress to add the port.
          - If the TCP/IP SP was selected and joining, then:
                - If a IP address was entered it calls 
                  IDirectPlay8Address::AddComponent( DPNA_KEY_HOSTNAME, ... )
                  on the pHostAddress to add the IP address.
                - If a port was entered it calls 
                  IDirectPlay8Address::AddComponent( DPNA_KEY_PORT, ... )
                  on the pHostAddress to add the port.
                  
          - If the IPX SP was selected and hosting, then:
                - If a port was entered it calls 
                  IDirectPlay8Address::AddComponent( DPNA_KEY_PORT, ... )
                  on the pDeviceAddress to add the port.
          - If the IPX SP was selected and joining, then:
                - If a port was entered it calls 
                  IDirectPlay8Address::AddComponent( DPNA_KEY_PORT, ... )
                  on the pHostAddress to add the port.
                  
          - If the modem SP was selected and joining, then:
                - If a phone number was entered it calls 
                  IDirectPlay8Address::AddComponent( DPNA_KEY_PHONENUMBER, ... )
                  on the pHostAddress to add the phone number.
                  
        7. It then connects in a similar manner as SimplePeer, but using 
           the pDeviceAddress and pHostAddress it filled to connect or join
           to the session.
           
  
   
