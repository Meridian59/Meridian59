//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TUIMetric class.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/uimetric.h>
#include <owl/system.h>

namespace owl {

//
// System metric constant objects
//
const TUIMetric TUIMetric::CxScreen = 0;
const TUIMetric TUIMetric::CyScreen = 1;
const TUIMetric TUIMetric::CxVScroll = 2;
const TUIMetric TUIMetric::CyHScroll = 3;
const TUIMetric TUIMetric::CyCaption = 4;
const TUIMetric TUIMetric::CxBorder = 5;
const TUIMetric TUIMetric::CyBorder = 6;
const TUIMetric TUIMetric::CxFixedFrame = 7;
const TUIMetric TUIMetric::CyFixedFrame = 8;
const TUIMetric TUIMetric::CyVThumb = 9;
const TUIMetric TUIMetric::CxHThumb = 10;
const TUIMetric TUIMetric::CxIcon = 11;
const TUIMetric TUIMetric::CyIcon = 12;
const TUIMetric TUIMetric::CxCursor = 13;
const TUIMetric TUIMetric::CyCursor = 14;
const TUIMetric TUIMetric::CyMenu = 15;
const TUIMetric TUIMetric::CxFullScreen = 16;
const TUIMetric TUIMetric::CyFullScreen = 17;
const TUIMetric TUIMetric::CyKanjiWindow = 18;
const TUIMetric TUIMetric::MousePresent = 19;
const TUIMetric TUIMetric::CyVScroll = 20;
const TUIMetric TUIMetric::CxHScroll = 21;
const TUIMetric TUIMetric::Debug = 22;
const TUIMetric TUIMetric::SwapButton = 23;
const TUIMetric TUIMetric::Reserved1 = 24;
const TUIMetric TUIMetric::Reserved2 = 25;
const TUIMetric TUIMetric::Reserved3 = 26;
const TUIMetric TUIMetric::Reserved4 = 27;
const TUIMetric TUIMetric::CxMin = 28;
const TUIMetric TUIMetric::CyMin = 29;
const TUIMetric TUIMetric::CxSize = 30;
const TUIMetric TUIMetric::CySize = 31;
const TUIMetric TUIMetric::CxSizeFrame = 32;
const TUIMetric TUIMetric::CySizeFrame = 33;
const TUIMetric TUIMetric::CxMinTrack = 34;
const TUIMetric TUIMetric::CyMinTrack = 35;
const TUIMetric TUIMetric::CxDoubleClk = 36;
const TUIMetric TUIMetric::CyDoubleClk = 37;
const TUIMetric TUIMetric::CxIconSpacing = 38;
const TUIMetric TUIMetric::CyIconSpacing = 39;
const TUIMetric TUIMetric::MenuDropAlignment = 40;
const TUIMetric TUIMetric::PenWindows = 41;
const TUIMetric TUIMetric::DbcsEnabled = 42;
const TUIMetric TUIMetric::CMouseButtons = 43;
const TUIMetric TUIMetric::Secure = 44;
const TUIMetric TUIMetric::CxEdge = 45;
const TUIMetric TUIMetric::CyEdge = 46;
const TUIMetric TUIMetric::CxMinSpacing = 47;
const TUIMetric TUIMetric::CyMinSpacing = 48;
const TUIMetric TUIMetric::CxSmIcon = 49;
const TUIMetric TUIMetric::CySmIcon = 50;
const TUIMetric TUIMetric::CySmCaption = 51;
const TUIMetric TUIMetric::CxSmSize = 52;
const TUIMetric TUIMetric::CySmSize = 53;
const TUIMetric TUIMetric::CxMenuSize = 54;
const TUIMetric TUIMetric::CyMenuSize = 55;
const TUIMetric TUIMetric::Arrange = 56;
const TUIMetric TUIMetric::CxMinimized = 57;
const TUIMetric TUIMetric::CyMinimized = 58;
const TUIMetric TUIMetric::CxMaxTrack = 59;
const TUIMetric TUIMetric::CyMaxTrack = 60;
const TUIMetric TUIMetric::CxMaximized = 61;
const TUIMetric TUIMetric::CyMaximized = 62;
const TUIMetric TUIMetric::ShowSounds = 63;
const TUIMetric TUIMetric::KeyboardPref = 64;
const TUIMetric TUIMetric::HighContrast = 65;
const TUIMetric TUIMetric::ScreenReader = 66;
const TUIMetric TUIMetric::CleanBoot = 67;
const TUIMetric TUIMetric::CxDrag = 68;
const TUIMetric TUIMetric::CyDrag = 69;
const TUIMetric TUIMetric::Network = 70;
const TUIMetric TUIMetric::CxMenuCheck = 71;
const TUIMetric TUIMetric::CyMenuCheck = 72;
const TUIMetric TUIMetric::SlowMachine = 73;
const TUIMetric TUIMetric::MideastEnabled = 74;
const TUIMetric TUIMetric::MouseWheelPresent = 75; // only NT or WinVer > 5.0
//#if(WINVER >= 0x0500)
const TUIMetric TUIMetric::XVirtualScreen = 76;
const TUIMetric TUIMetric::YVirtualScreen = 77;
const TUIMetric TUIMetric::CxVirtualScreen = 78;
const TUIMetric TUIMetric::CyVirtualScreen = 79;
const TUIMetric TUIMetric::CMonitors = 80;
const TUIMetric TUIMetric::SameDisplayFormat = 81;
//#endif /* WINVER >= 0x0500 */
//#if (WINVER < 0x0500) && (!defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0400))
const TUIMetric TUIMetric::CMetrics = 83;
//#else
//const TUIMetric TUIMetric::CMetrics = 83
//#endif

} // OWL namespace
/* ========================================================================== */
