DirectShow Sample -- ASFCopy
----------------------------

Transcodes one or more files to an ASF file.

To build this sample, you must install the Microsoft Windows Media Format SDK 
and obtain a software certificate.  After you obtain the software certificate, 
build the sample by linking two additional libraries: WMStub.lib and WMVCore.lib.


Usage:

ASFCopy [/v] /p profile file1 [file2 ...] target

The following command-line switches are supported:
    /p Specifies the profile number. 
    /v Verbose mode. 

Specify an ASF profile using the /p switch.  If you omit this switch, ASFCopy 
displays a list of the standard system profiles.

Specify the name of one or more source files, and the name of the target file. 
If you specify more than one source file, the application multiplexes all of 
the source files.  You must specify a profile that matches the streams 
contained in the source files, or else the application will not work correctly. 
For example, if you specify Video for Web Servers (56 Kbps), the combined 
source files must have exactly one video stream and one audio stream.

