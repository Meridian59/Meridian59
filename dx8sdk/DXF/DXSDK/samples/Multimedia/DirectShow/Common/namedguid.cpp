//------------------------------------------------------------------------------
// File: NamedGuid.cpp
//
// Desc: DirectShow sample code - helps in converting GUIDs to strings
//
// Copyright (c) 1996-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <stdafx.h>
#include <dshow.h>
#include <dmo.h>

#include "namedguid.h"

//
// Create a large table to allow searches by CLSID, media type, IID, etc.
//
const NamedGuid rgng[] =
{
    {&MEDIASUBTYPE_AIFF, TEXT("MEDIASUBTYPE_AIFF")},
    {&MEDIASUBTYPE_AU, TEXT("MEDIASUBTYPE_AU")},
    {&MEDIASUBTYPE_AnalogVideo_NTSC_M, TEXT("MEDIASUBTYPE_AnalogVideo_NTSC_M")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_B, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_B")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_D, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_D")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_G, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_G")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_H, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_H")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_I, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_I")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_M, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_M")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_N, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_N")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_B, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_B")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_D, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_D")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_G, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_G")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_H, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_H")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_K")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K1, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_K1")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_L, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_L")},
    {&MEDIASUBTYPE_Asf, TEXT("MEDIASUBTYPE_Asf")},
    {&MEDIASUBTYPE_Avi, TEXT("MEDIASUBTYPE_Avi")},
    {&MEDIASUBTYPE_CFCC, TEXT("MEDIASUBTYPE_CFCC")},
    {&MEDIASUBTYPE_CLJR, TEXT("MEDIASUBTYPE_CLJR")},
    {&MEDIASUBTYPE_CPLA, TEXT("MEDIASUBTYPE_CPLA")},
    {&MEDIASUBTYPE_DOLBY_AC3, TEXT("MEDIASUBTYPE_DOLBY_AC3")},
    {&MEDIASUBTYPE_DVCS, TEXT("MEDIASUBTYPE_DVCS")},
    {&MEDIASUBTYPE_DVD_LPCM_AUDIO, TEXT("MEDIASUBTYPE_DVD_LPCM_AUDIO")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_DSI, TEXT("MEDIASUBTYPE_DVD_NAVIGATION_DSI")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PCI, TEXT("MEDIASUBTYPE_DVD_NAVIGATION_PCI")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, TEXT("MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER")},
    {&MEDIASUBTYPE_DVD_SUBPICTURE, TEXT("MEDIASUBTYPE_DVD_SUBPICTURE")},
    {&MEDIASUBTYPE_DVSD, TEXT("MEDIASUBTYPE_DVSD")},
    {&MEDIASUBTYPE_DssAudio, TEXT("MEDIASUBTYPE_DssAudio")},
    {&MEDIASUBTYPE_DssVideo, TEXT("MEDIASUBTYPE_DssVideo")},
    {&MEDIASUBTYPE_IF09, TEXT("MEDIASUBTYPE_IF09")},
    {&MEDIASUBTYPE_IJPG, TEXT("MEDIASUBTYPE_IJPG")},
    {&MEDIASUBTYPE_Line21_BytePair, TEXT("MEDIASUBTYPE_Line21_BytePair")},
    {&MEDIASUBTYPE_Line21_GOPPacket, TEXT("MEDIASUBTYPE_Line21_GOPPacket")},
    {&MEDIASUBTYPE_Line21_VBIRawData, TEXT("MEDIASUBTYPE_Line21_VBIRawData")},
    {&MEDIASUBTYPE_MDVF, TEXT("MEDIASUBTYPE_MDVF")},
    {&MEDIASUBTYPE_MJPG, TEXT("MEDIASUBTYPE_MJPG")},
    {&MEDIASUBTYPE_MPEG1Audio, TEXT("MEDIASUBTYPE_MPEG1Audio")},
    {&MEDIASUBTYPE_MPEG1AudioPayload, TEXT("MEDIASUBTYPE_MPEG1AudioPayload")},
    {&MEDIASUBTYPE_MPEG1Packet, TEXT("MEDIASUBTYPE_MPEG1Packet")},
    {&MEDIASUBTYPE_MPEG1Payload, TEXT("MEDIASUBTYPE_MPEG1Payload")},
    {&MEDIASUBTYPE_MPEG1System, TEXT("MEDIASUBTYPE_MPEG1System")},
    {&MEDIASUBTYPE_MPEG1Video, TEXT("MEDIASUBTYPE_MPEG1Video")},
    {&MEDIASUBTYPE_MPEG1VideoCD, TEXT("MEDIASUBTYPE_MPEG1VideoCD")},
    {&MEDIASUBTYPE_MPEG2_AUDIO, TEXT("MEDIASUBTYPE_MPEG2_AUDIO")},
    {&MEDIASUBTYPE_MPEG2_PROGRAM, TEXT("MEDIASUBTYPE_MPEG2_PROGRAM")},
    {&MEDIASUBTYPE_MPEG2_TRANSPORT, TEXT("MEDIASUBTYPE_MPEG2_TRANSPORT")},
    {&MEDIASUBTYPE_MPEG2_VIDEO, TEXT("MEDIASUBTYPE_MPEG2_VIDEO")},
    {&MEDIASUBTYPE_None, TEXT("MEDIASUBTYPE_None")},
    {&MEDIASUBTYPE_Overlay, TEXT("MEDIASUBTYPE_Overlay")},
    {&MEDIASUBTYPE_PCM, TEXT("MEDIASUBTYPE_PCM")},
    {&MEDIASUBTYPE_PCMAudio_Obsolete, TEXT("MEDIASUBTYPE_PCMAudio_Obsolete")},
    {&MEDIASUBTYPE_Plum, TEXT("MEDIASUBTYPE_Plum")},
    {&MEDIASUBTYPE_QTJpeg, TEXT("MEDIASUBTYPE_QTJpeg")},
    {&MEDIASUBTYPE_QTMovie, TEXT("MEDIASUBTYPE_QTMovie")},
    {&MEDIASUBTYPE_QTRle, TEXT("MEDIASUBTYPE_QTRle")},
    {&MEDIASUBTYPE_QTRpza, TEXT("MEDIASUBTYPE_QTRpza")},
    {&MEDIASUBTYPE_QTSmc, TEXT("MEDIASUBTYPE_QTSmc")},
    {&MEDIASUBTYPE_RGB1, TEXT("MEDIASUBTYPE_RGB1")},
    {&MEDIASUBTYPE_RGB24, TEXT("MEDIASUBTYPE_RGB24")},
    {&MEDIASUBTYPE_RGB32, TEXT("MEDIASUBTYPE_RGB32")},
    {&MEDIASUBTYPE_RGB4, TEXT("MEDIASUBTYPE_RGB4")},
    {&MEDIASUBTYPE_RGB555, TEXT("MEDIASUBTYPE_RGB555")},
    {&MEDIASUBTYPE_RGB565, TEXT("MEDIASUBTYPE_RGB565")},
    {&MEDIASUBTYPE_RGB8, TEXT("MEDIASUBTYPE_RGB8")},
    {&MEDIASUBTYPE_TVMJ, TEXT("MEDIASUBTYPE_TVMJ")},
    {&MEDIASUBTYPE_UYVY, TEXT("MEDIASUBTYPE_UYVY")},
    {&MEDIASUBTYPE_VPVBI, TEXT("MEDIASUBTYPE_VPVBI")},
    {&MEDIASUBTYPE_VPVideo, TEXT("MEDIASUBTYPE_VPVideo")},
    {&MEDIASUBTYPE_WAKE, TEXT("MEDIASUBTYPE_WAKE")},
    {&MEDIASUBTYPE_WAVE, TEXT("MEDIASUBTYPE_WAVE")},
    {&MEDIASUBTYPE_Y211, TEXT("MEDIASUBTYPE_Y211")},
    {&MEDIASUBTYPE_Y411, TEXT("MEDIASUBTYPE_Y411")},
    {&MEDIASUBTYPE_Y41P, TEXT("MEDIASUBTYPE_Y41P")},
    {&MEDIASUBTYPE_YUY2, TEXT("MEDIASUBTYPE_YUY2")},
    {&MEDIASUBTYPE_YV12, TEXT("MEDIASUBTYPE_YV12")},
    {&MEDIASUBTYPE_YVU9, TEXT("MEDIASUBTYPE_YVU9")},
    {&MEDIASUBTYPE_YVYU, TEXT("MEDIASUBTYPE_YVYU")},
    {&MEDIASUBTYPE_dvhd, TEXT("MEDIASUBTYPE_dvhd")},
    {&MEDIASUBTYPE_dvsd, TEXT("MEDIASUBTYPE_dvsd")},
    {&MEDIASUBTYPE_dvsl, TEXT("MEDIASUBTYPE_dvsl")},
    {&MEDIATYPE_AUXLine21Data, TEXT("MEDIATYPE_AUXLine21Data")},
    {&MEDIATYPE_AnalogAudio, TEXT("MEDIATYPE_AnalogAudio")},
    {&MEDIATYPE_AnalogVideo, TEXT("MEDIATYPE_AnalogVideo")},
    {&MEDIATYPE_Audio, TEXT("MEDIATYPE_Audio")},
    {&MEDIATYPE_DVD_ENCRYPTED_PACK, TEXT("MEDIATYPE_DVD_ENCRYPTED_PACK")},
    {&MEDIATYPE_DVD_NAVIGATION, TEXT("MEDIATYPE_DVD_NAVIGATION")},
    {&MEDIATYPE_File, TEXT("MEDIATYPE_File")},
    {&MEDIATYPE_Interleaved, TEXT("MEDIATYPE_Interleaved")},
    {&MEDIATYPE_LMRT, TEXT("MEDIATYPE_LMRT")},
    {&MEDIATYPE_MPEG1SystemStream, TEXT("MEDIATYPE_MPEG1SystemStream")},
    {&MEDIATYPE_MPEG2_PES, TEXT("MEDIATYPE_MPEG2_PES")},
    {&MEDIATYPE_Midi, TEXT("MEDIATYPE_Midi")},
    {&MEDIATYPE_ScriptCommand, TEXT("MEDIATYPE_ScriptCommand")},
    {&MEDIATYPE_Stream, TEXT("MEDIATYPE_Stream")},
    {&MEDIATYPE_Text, TEXT("MEDIATYPE_Text")},
    {&MEDIATYPE_Timecode, TEXT("MEDIATYPE_Timecode")},
    {&MEDIATYPE_URL_STREAM, TEXT("MEDIATYPE_URL_STREAM")},
    {&MEDIATYPE_Video, TEXT("MEDIATYPE_Video")},
    {&WMMEDIASUBTYPE_Base, TEXT("WMMEDIASUBTYPE_Base")},
    {&WMMEDIASUBTYPE_RGB1, TEXT("WMMEDIASUBTYPE_RGB1")},
    {&WMMEDIASUBTYPE_RGB4, TEXT("WMMEDIASUBTYPE_RGB4")},
    {&WMMEDIASUBTYPE_RGB8, TEXT("WMMEDIASUBTYPE_RGB8")},
    {&WMMEDIASUBTYPE_RGB565, TEXT("WMMEDIASUBTYPE_RGB565")},
    {&WMMEDIASUBTYPE_RGB555, TEXT("WMMEDIASUBTYPE_RGB555")},
    {&WMMEDIASUBTYPE_RGB24, TEXT("WMMEDIASUBTYPE_RGB24")},
    {&WMMEDIASUBTYPE_RGB32, TEXT("WMMEDIASUBTYPE_RGB32")},
    {&WMMEDIASUBTYPE_I420, TEXT("WMMEDIASUBTYPE_I420")},
    {&WMMEDIASUBTYPE_IYUV, TEXT("WMMEDIASUBTYPE_IYUV")},
    {&WMMEDIASUBTYPE_YV12, TEXT("WMMEDIASUBTYPE_YV12")},
    {&WMMEDIASUBTYPE_YUY2, TEXT("WMMEDIASUBTYPE_YUY2")},
    {&WMMEDIASUBTYPE_UYVY, TEXT("WMMEDIASUBTYPE_UYVY")},
    {&WMMEDIASUBTYPE_YVYU, TEXT("WMMEDIASUBTYPE_YVYU")},
    {&WMMEDIASUBTYPE_YVU9, TEXT("WMMEDIASUBTYPE_YVU9")},
    {&WMMEDIASUBTYPE_MP43, TEXT("WMMEDIASUBTYPE_MP43")},
    {&WMMEDIASUBTYPE_MP4S, TEXT("WMMEDIASUBTYPE_MP4S")},
    {&WMMEDIASUBTYPE_WMV1, TEXT("WMMEDIASUBTYPE_WMV1")},
    {&WMMEDIASUBTYPE_MSS1, TEXT("WMMEDIASUBTYPE_MSS1")},
    {&WMMEDIASUBTYPE_PCM, TEXT("WMMEDIASUBTYPE_PCM")},
    {&WMMEDIASUBTYPE_DRM, TEXT("WMMEDIASUBTYPE_DRM")},
    {&WMMEDIASUBTYPE_WMAudioV7, TEXT("WMMEDIASUBTYPE_WMAudioV7")},
    {&WMMEDIASUBTYPE_WMAudioV2, TEXT("WMMEDIASUBTYPE_WMAudioV2")},
    {&WMMEDIASUBTYPE_ACELPnet, TEXT("WMMEDIASUBTYPE_ACELPnet")},
    {&WMMEDIATYPE_Audio, TEXT("WMMEDIATYPE_Audio")},
    {&WMMEDIATYPE_Video, TEXT("WMMEDIATYPE_Video")},
    {&WMMEDIATYPE_Script, TEXT("WMMEDIATYPE_Script")},
    {&WMMEDIATYPE_Image, TEXT("WMMEDIATYPE_Image")},
    {&WMFORMAT_VideoInfo, TEXT("WMFORMAT_VideoInfo")},
    {&WMFORMAT_WaveFormatEx, TEXT("WMFORMAT_WaveFormatEx")},
    {&WMFORMAT_Script, TEXT("WMFORMAT_Script")},
    {&WMSCRIPTTYPE_TwoStrings, TEXT("WMSCRIPTTYPE_TwoStrings")},
    {&PIN_CATEGORY_ANALOGVIDEOIN, TEXT("PIN_CATEGORY_ANALOGVIDEOIN")},
    {&PIN_CATEGORY_CAPTURE, TEXT("PIN_CATEGORY_CAPTURE")},
    {&PIN_CATEGORY_CC, TEXT("PIN_CATEGORY_CC")},
    {&PIN_CATEGORY_EDS, TEXT("PIN_CATEGORY_EDS")},
    {&PIN_CATEGORY_NABTS, TEXT("PIN_CATEGORY_NABTS")},
    {&PIN_CATEGORY_PREVIEW, TEXT("PIN_CATEGORY_PREVIEW")},
    {&PIN_CATEGORY_STILL, TEXT("PIN_CATEGORY_STILL")},
    {&PIN_CATEGORY_TELETEXT, TEXT("PIN_CATEGORY_TELETEXT")},
    {&PIN_CATEGORY_TIMECODE, TEXT("PIN_CATEGORY_TIMECODE")},
    {&PIN_CATEGORY_VBI, TEXT("PIN_CATEGORY_VBI")},
    {&PIN_CATEGORY_VIDEOPORT, TEXT("PIN_CATEGORY_VIDEOPORT")},
    {&PIN_CATEGORY_VIDEOPORT_VBI, TEXT("PIN_CATEGORY_VIDEOPORT_VBI")},
    {&CLSID_ACMWrapper, TEXT("CLSID_ACMWrapper")},
    {&CLSID_AVICo, TEXT("CLSID_AVICo")},
    {&CLSID_AVIDec, TEXT("CLSID_AVIDec")},
    {&CLSID_AVIDoc, TEXT("CLSID_AVIDoc")},
    {&CLSID_AVIDraw, TEXT("CLSID_AVIDraw")},
    {&CLSID_AVIMIDIRender, TEXT("CLSID_AVIMIDIRender")},
    {&CLSID_ActiveMovieCategories, TEXT("CLSID_ActiveMovieCategories")},
    {&CLSID_AnalogVideoDecoderPropertyPage, TEXT("CLSID_AnalogVideoDecoderPropertyPage")},
    {&CLSID_WMAsfReader, TEXT("CLSID_WMAsfReader")},
    {&CLSID_WMAsfWriter, TEXT("CLSID_WMAsfWriter")},
    {&CLSID_AsyncReader, TEXT("CLSID_AsyncReader")},
    {&CLSID_AudioCompressorCategory, TEXT("CLSID_AudioCompressorCategory")},
    {&CLSID_AudioInputDeviceCategory, TEXT("CLSID_AudioInputDeviceCategory")},
    {&CLSID_AudioProperties, TEXT("CLSID_AudioProperties")},
    {&CLSID_AudioRecord, TEXT("CLSID_AudioRecord")},
    {&CLSID_AudioRender, TEXT("CLSID_AudioRender")},
    {&CLSID_AudioRendererCategory, TEXT("CLSID_AudioRendererCategory")},
    {&CLSID_AviDest, TEXT("CLSID_AviDest")},
    {&CLSID_AviMuxProptyPage, TEXT("CLSID_AviMuxProptyPage")},
    {&CLSID_AviMuxProptyPage1, TEXT("CLSID_AviMuxProptyPage1")},
    {&CLSID_AviReader, TEXT("CLSID_AviReader")},
    {&CLSID_AviSplitter, TEXT("CLSID_AviSplitter")},
    {&CLSID_CAcmCoClassManager, TEXT("CLSID_CAcmCoClassManager")},
    {&CLSID_CDeviceMoniker, TEXT("CLSID_CDeviceMoniker")},
    {&CLSID_CIcmCoClassManager, TEXT("CLSID_CIcmCoClassManager")},
    {&CLSID_CMidiOutClassManager, TEXT("CLSID_CMidiOutClassManager")},
    {&CLSID_CMpegAudioCodec, TEXT("CLSID_CMpegAudioCodec")},
    {&CLSID_CMpegVideoCodec, TEXT("CLSID_CMpegVideoCodec")},
    {&CLSID_CQzFilterClassManager, TEXT("CLSID_CQzFilterClassManager")},
    {&CLSID_CVidCapClassManager, TEXT("CLSID_CVidCapClassManager")},
    {&CLSID_CWaveOutClassManager, TEXT("CLSID_CWaveOutClassManager")},
    {&CLSID_CWaveinClassManager, TEXT("CLSID_CWaveinClassManager")},
    {&CLSID_CameraControlPropertyPage, TEXT("CLSID_CameraControlPropertyPage")},
    {&CLSID_CaptureGraphBuilder, TEXT("CLSID_CaptureGraphBuilder")},
    {&CLSID_CaptureProperties, TEXT("CLSID_CaptureProperties")},
    {&CLSID_Colour, TEXT("CLSID_Colour")},
    {&CLSID_CrossbarFilterPropertyPage, TEXT("CLSID_CrossbarFilterPropertyPage")},
    {&CLSID_DSoundRender, TEXT("CLSID_DSoundRender")},
    {&CLSID_DVDHWDecodersCategory, TEXT("CLSID_DVDHWDecodersCategory")},
    {&CLSID_DVDNavigator, TEXT("CLSID_DVDNavigator")},
    {&CLSID_DVDecPropertiesPage, TEXT("CLSID_DVDecPropertiesPage")},
    {&CLSID_DVEncPropertiesPage, TEXT("CLSID_DVEncPropertiesPage")},
    {&CLSID_DVMux, TEXT("CLSID_DVMux")},
    {&CLSID_DVMuxPropertyPage, TEXT("CLSID_DVMuxPropertyPage")},
    {&CLSID_DVSplitter, TEXT("CLSID_DVSplitter")},
    {&CLSID_DVVideoCodec, TEXT("CLSID_DVVideoCodec")},
    {&CLSID_DVVideoEnc, TEXT("CLSID_DVVideoEnc")},
    {&CLSID_DirectDraw, TEXT("CLSID_DirectDraw")},
    {&CLSID_DirectDrawClipper, TEXT("CLSID_DirectDrawClipper")},
    {&CLSID_DirectDrawProperties, TEXT("CLSID_DirectDrawProperties")},
    {&CLSID_Dither, TEXT("CLSID_Dither")},
    {&CLSID_DvdGraphBuilder, TEXT("CLSID_DvdGraphBuilder")},
    {&CLSID_FGControl, TEXT("CLSID_FGControl")},
    {&CLSID_FileSource, TEXT("CLSID_FileSource")},
    {&CLSID_FileWriter, TEXT("CLSID_FileWriter")},
    {&CLSID_FilterGraph, TEXT("CLSID_FilterGraph")},
    {&CLSID_FilterGraphNoThread, TEXT("CLSID_FilterGraphNoThread")},
    {&CLSID_FilterMapper, TEXT("CLSID_FilterMapper")},
    {&CLSID_FilterMapper2, TEXT("CLSID_FilterMapper2")},
    {&CLSID_InfTee, TEXT("CLSID_InfTee")},
    {&CLSID_LegacyAmFilterCategory, TEXT("CLSID_LegacyAmFilterCategory")},
    {&CLSID_Line21Decoder, TEXT("CLSID_Line21Decoder")},
    {&CLSID_MOVReader, TEXT("CLSID_MOVReader")},
    {&CLSID_MPEG1Doc, TEXT("CLSID_MPEG1Doc")},
    {&CLSID_MPEG1PacketPlayer, TEXT("CLSID_MPEG1PacketPlayer")},
    {&CLSID_MPEG1Splitter, TEXT("CLSID_MPEG1Splitter")},
    {&CLSID_MediaPropertyBag, TEXT("CLSID_MediaPropertyBag")},
    {&CLSID_MemoryAllocator, TEXT("CLSID_MemoryAllocator")},
    {&CLSID_MidiRendererCategory, TEXT("CLSID_MidiRendererCategory")},
    {&CLSID_ModexProperties, TEXT("CLSID_ModexProperties")},
    {&CLSID_ModexRenderer, TEXT("CLSID_ModexRenderer")},
    {&CLSID_OverlayMixer, TEXT("CLSID_OverlayMixer")},
    {&CLSID_PerformanceProperties, TEXT("CLSID_PerformanceProperties")},
    {&CLSID_PersistMonikerPID, TEXT("CLSID_PersistMonikerPID")},
    {&CLSID_ProtoFilterGraph, TEXT("CLSID_ProtoFilterGraph")},
    {&CLSID_QualityProperties, TEXT("CLSID_QualityProperties")},
    {&CLSID_SeekingPassThru, TEXT("CLSID_SeekingPassThru")},
    {&CLSID_SmartTee, TEXT("CLSID_SmartTee")},
    {&CLSID_SystemClock, TEXT("CLSID_SystemClock")},
    {&CLSID_SystemDeviceEnum, TEXT("CLSID_SystemDeviceEnum")},
    {&CLSID_TVAudioFilterPropertyPage, TEXT("CLSID_TVAudioFilterPropertyPage")},
    {&CLSID_TVTunerFilterPropertyPage, TEXT("CLSID_TVTunerFilterPropertyPage")},
    {&CLSID_TextRender, TEXT("CLSID_TextRender")},
    {&CLSID_URLReader, TEXT("CLSID_URLReader")},
    {&CLSID_VBISurfaces, TEXT("CLSID_VBISurfaces")},
    {&CLSID_VPObject, TEXT("CLSID_VPObject")},
    {&CLSID_VPVBIObject, TEXT("CLSID_VPVBIObject")},
    {&CLSID_VfwCapture, TEXT("CLSID_VfwCapture")},
    {&CLSID_VideoCompressorCategory, TEXT("CLSID_VideoCompressorCategory")},
    {&CLSID_VideoInputDeviceCategory, TEXT("CLSID_VideoInputDeviceCategory")},
    {&CLSID_VideoProcAmpPropertyPage, TEXT("CLSID_VideoProcAmpPropertyPage")},
    {&CLSID_VideoRenderer, TEXT("CLSID_VideoRenderer")},
    {&CLSID_VideoStreamConfigPropertyPage, TEXT("CLSID_VideoStreamConfigPropertyPage")},
    {&FORMAT_AnalogVideo, TEXT("FORMAT_AnalogVideo")},
    {&FORMAT_DVD_LPCMAudio, TEXT("FORMAT_DVD_LPCMAudio")},
    {&FORMAT_DolbyAC3, TEXT("FORMAT_DolbyAC3")},
    {&FORMAT_DvInfo, TEXT("FORMAT_DvInfo")},
    {&FORMAT_MPEG2Audio, TEXT("FORMAT_MPEG2Audio")},
    {&FORMAT_MPEG2Video, TEXT("FORMAT_MPEG2Video")},
    {&FORMAT_MPEG2_VIDEO, TEXT("FORMAT_MPEG2_VIDEO")},
    {&FORMAT_MPEGStreams, TEXT("FORMAT_MPEGStreams")},
    {&FORMAT_MPEGVideo, TEXT("FORMAT_MPEGVideo")},
    {&FORMAT_None, TEXT("FORMAT_None")},
    {&FORMAT_VIDEOINFO2, TEXT("FORMAT_VIDEOINFO2")},
    {&FORMAT_VideoInfo, TEXT("FORMAT_VideoInfo")},
    {&FORMAT_VideoInfo2, TEXT("FORMAT_VideoInfo2")},
    {&FORMAT_WaveFormatEx, TEXT("FORMAT_WaveFormatEx")},
    {&TIME_FORMAT_BYTE, TEXT("TIME_FORMAT_BYTE")},
    {&TIME_FORMAT_FIELD, TEXT("TIME_FORMAT_FIELD")},
    {&TIME_FORMAT_FRAME, TEXT("TIME_FORMAT_FRAME")},
    {&TIME_FORMAT_MEDIA_TIME, TEXT("TIME_FORMAT_MEDIA_TIME")},
    {&TIME_FORMAT_SAMPLE, TEXT("TIME_FORMAT_SAMPLE")},
    {&AMPROPSETID_Pin, TEXT("AMPROPSETID_Pin")},
    {&AM_INTERFACESETID_Standard, TEXT("AM_INTERFACESETID_Standard")},
    {&AM_KSCATEGORY_AUDIO, TEXT("AM_KSCATEGORY_AUDIO")},
    {&AM_KSCATEGORY_CAPTURE, TEXT("AM_KSCATEGORY_CAPTURE")},
    {&AM_KSCATEGORY_CROSSBAR, TEXT("AM_KSCATEGORY_CROSSBAR")},
    {&AM_KSCATEGORY_DATACOMPRESSOR, TEXT("AM_KSCATEGORY_DATACOMPRESSOR")},
    {&AM_KSCATEGORY_RENDER, TEXT("AM_KSCATEGORY_RENDER")},
    {&AM_KSCATEGORY_TVAUDIO, TEXT("AM_KSCATEGORY_TVAUDIO")},
    {&AM_KSCATEGORY_TVTUNER, TEXT("AM_KSCATEGORY_TVTUNER")},
    {&AM_KSCATEGORY_VIDEO, TEXT("AM_KSCATEGORY_VIDEO")},
    {&AM_KSPROPSETID_AC3, TEXT("AM_KSPROPSETID_AC3")},
    {&AM_KSPROPSETID_CopyProt, TEXT("AM_KSPROPSETID_CopyProt")},
    {&AM_KSPROPSETID_DvdSubPic, TEXT("AM_KSPROPSETID_DvdSubPic")},
    {&AM_KSPROPSETID_TSRateChange, TEXT("AM_KSPROPSETID_TSRateChange")},
    {&IID_IAMDirectSound, TEXT("IID_IAMDirectSound")},
    {&IID_IAMLine21Decoder, TEXT("IID_IAMLine21Decoder")},
    {&IID_IBaseVideoMixer, TEXT("IID_IBaseVideoMixer")},
    {&IID_IDDVideoPortContainer, TEXT("IID_IDDVideoPortContainer")},
    {&IID_IDirectDraw, TEXT("IID_IDirectDraw")},
    {&IID_IDirectDraw2, TEXT("IID_IDirectDraw2")},
    {&IID_IDirectDrawClipper, TEXT("IID_IDirectDrawClipper")},
    {&IID_IDirectDrawColorControl, TEXT("IID_IDirectDrawColorControl")},
    {&IID_IDirectDrawKernel, TEXT("IID_IDirectDrawKernel")},
    {&IID_IDirectDrawPalette, TEXT("IID_IDirectDrawPalette")},
    {&IID_IDirectDrawSurface, TEXT("IID_IDirectDrawSurface")},
    {&IID_IDirectDrawSurface2, TEXT("IID_IDirectDrawSurface2")},
    {&IID_IDirectDrawSurface3, TEXT("IID_IDirectDrawSurface3")},
    {&IID_IDirectDrawSurfaceKernel, TEXT("IID_IDirectDrawSurfaceKernel")},
    {&IID_IDirectDrawVideo, TEXT("IID_IDirectDrawVideo")},
    {&IID_IFullScreenVideo, TEXT("IID_IFullScreenVideo")},
    {&IID_IFullScreenVideoEx, TEXT("IID_IFullScreenVideoEx")},
    {&IID_IKsDataTypeHandler, TEXT("IID_IKsDataTypeHandler")},
    {&IID_IKsInterfaceHandler, TEXT("IID_IKsInterfaceHandler")},
    {&IID_IKsPin, TEXT("IID_IKsPin")},
    {&IID_IMixerPinConfig, TEXT("IID_IMixerPinConfig")},
    {&IID_IMixerPinConfig2, TEXT("IID_IMixerPinConfig2")},
    {&IID_IMpegAudioDecoder, TEXT("IID_IMpegAudioDecoder")},
    {&IID_IQualProp, TEXT("IID_IQualProp")},
    {&IID_IVPConfig, TEXT("IID_IVPConfig")},
    {&IID_IVPControl, TEXT("IID_IVPControl")},
    {&IID_IVPNotify, TEXT("IID_IVPNotify")},
    {&IID_IVPNotify2, TEXT("IID_IVPNotify2")},
    {&IID_IVPObject, TEXT("IID_IVPObject")},
    {&IID_IVPVBIConfig, TEXT("IID_IVPVBIConfig")},
    {&IID_IVPVBINotify, TEXT("IID_IVPVBINotify")},
    {&IID_IVPVBIObject, TEXT("IID_IVPVBIObject")},
    {&LOOK_DOWNSTREAM_ONLY, TEXT("LOOK_DOWNSTREAM_ONLY")},
    {&LOOK_UPSTREAM_ONLY, TEXT("LOOK_UPSTREAM_ONLY")},
    {0, 0},
};



void GetGUIDString(TCHAR *szString, GUID *pGUID)
{
    int i=0;

    // Find format GUID's name in the named guids table
    while (rgng[i].pguid != 0)
    {
        if(*pGUID == *(rgng[i].pguid))
        {
            wsprintf(szString, TEXT("%s\0"), rgng[i].psz);
            return;
        }
        i++;
    }

    // If we got here, there was no match
    wsprintf(szString, TEXT("GUID_NULL\0"));
}


void GetFormatString(TCHAR *szFormat, DMO_MEDIA_TYPE *pType)
{
    int i=0;

    // Find format GUID's name in the named guids table
    while (rgng[i].pguid != 0)
    {
        if(pType->formattype == *(rgng[i].pguid))
        {
            wsprintf(szFormat, TEXT("%s\0"), rgng[i].psz);
            return;
        }
        i++;
    }

    // If we got here, there was no match
    wsprintf(szFormat, TEXT("Format_None\0"));
}


void GetTypeSubtypeString(TCHAR *szCLSID, DMO_PARTIAL_MEDIATYPE& aList)
{
    HRESULT hr;
    CString strType, strSubtype;
    int j;

    LPOLESTR szType, szSubtype;

    // Convert binary CLSID to a readable version
    hr = StringFromCLSID(aList.type, &szType);
    if(FAILED(hr))
        return;
    hr = StringFromCLSID(aList.subtype, &szSubtype);
    if(FAILED(hr))
        return;

    // Set default type/subtype strings to their actual GUID values
    strType = szType;
    strSubtype = szSubtype;

    // Find type GUID's name in the named guids table
    j=0;
    while (rgng[j].pguid != 0)
    {
        if(aList.type == *(rgng[j].pguid))
        {
            strType = rgng[j].psz;  // Save type name
            break;
        }
        j++;
    }

    // Find subtype GUID's name in the named guids table
    j=0;
    while (rgng[j].pguid != 0)
    {
        if(aList.subtype == *(rgng[j].pguid))
        {
            strSubtype = rgng[j].psz; // Save subtype name
            break;
        }
        j++;
    }

    // Build a string with the type/subtype information.
    // If a friendly name was found, it will be used.
    // Otherwise, the type/subtype's GUID will be displayed.
    wsprintf(szCLSID, TEXT("%s\t%s"), strType, strSubtype);
}


