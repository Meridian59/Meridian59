// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

d3d_driver_profile	gD3DDriverProfile;

DWORD	gBlendCaps = (D3DPBLENDCAPS_SRCCOLOR | D3DPBLENDCAPS_INVSRCCOLOR |
						D3DPBLENDCAPS_DESTCOLOR | D3DPBLENDCAPS_INVDESTCOLOR |
						D3DPBLENDCAPS_SRCALPHA | D3DPBLENDCAPS_INVSRCALPHA |
						D3DPBLENDCAPS_DESTALPHA | D3DPBLENDCAPS_INVDESTALPHA);

DWORD	gZCmpCaps = (D3DPCMPCAPS_ALWAYS | D3DPCMPCAPS_LESSEQUAL);
DWORD	gAlphaCmpCaps = (D3DPCMPCAPS_ALWAYS | D3DPCMPCAPS_GREATEREQUAL);
DWORD	gTextureCaps = (D3DPTEXTURECAPS_ALPHA);
DWORD	gTextureAddressCaps = (D3DPTADDRESSCAPS_CLAMP | D3DPTADDRESSCAPS_WRAP);
DWORD	gTextureOpCaps = (D3DTOP_DISABLE |
						  D3DTOP_MODULATE |
						  D3DTOP_SELECTARG1 |
						  D3DTOP_SELECTARG2);

DWORD	gFogCaps = (D3DPRASTERCAPS_FOGTABLE |
						  D3DPRASTERCAPS_ZFOG);

D3DPRESENT_PARAMETERS	gPresentParam;

Bool D3DDriverProfileInit(void)
{
	FILE					*pFile;
	Bool					bProblem = FALSE;
	Bool					bDisable = FALSE;
	char					string[255];

	pFile = fopen("d3dlog.txt", "w+t");
	assert(pFile);

	gD3DEnabled = FALSE;

	// first check to make sure user isn't forcing software rendering
	GetPrivateProfileString("config", "softwarerenderer", "error", string, 255, "./config.ini");
	strlwr(string);

	if (0 == strcmp(string, "true"))
	{
		gD3DDriverProfile.bSoftwareRenderer = TRUE;

		return FALSE;
	}

	memset(&gD3DDriverProfile, 0, sizeof(d3d_driver_profile));

	HRESULT error = IDirect3D9_CheckDeviceType(gpD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                              D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, TRUE);

	if (FAILED(error))
	{
		fprintf(pFile, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n");
		MessageBox(hMain, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n\nSwitching to software renderer",
			NULL, MB_OK);
		fclose(pFile);

		gD3DDriverProfile.bSoftwareRenderer = TRUE;

		return FALSE;
	}

   // This looks wrong.. "TRUE" stencil format??
	error = IDirect3D9_CheckDepthStencilMatch(
      gpD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      D3DFMT_X8R8G8B8, D3DFMT_D24S8, (D3DFORMAT) TRUE);

	if (config.large_area)
	{
		gScreenWidth = 800;
		gScreenHeight = 600;
	}
	else
	{
		gScreenWidth = 512;
		gScreenHeight = 384;
	}

	memset(&gPresentParam, 0, sizeof(gPresentParam));
	gPresentParam.Windowed = TRUE;
	gPresentParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
	gPresentParam.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	gPresentParam.BackBufferWidth = gScreenWidth;
	gPresentParam.BackBufferHeight = gScreenHeight;
	gPresentParam.BackBufferFormat = D3DFMT_A8R8G8B8;
	gPresentParam.BackBufferCount = 1;
	gPresentParam.EnableAutoDepthStencil = TRUE;
	gPresentParam.AutoDepthStencilFormat = D3DFMT_D24S8;
	gPresentParam.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// first try hardware vertex processing
	error = IDirect3D9_CreateDevice(gpD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                   hMain, D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                   &gPresentParam, &gpD3DDevice);

	// if not, try mixed
	if (FAILED(error) || !gpD3DDevice)
	{
		fprintf(pFile, "Failed hardware vertex processing device creation...  Trying mixed\n");
		error = IDirect3D9_CreateDevice(gpD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                      hMain, D3DCREATE_MIXED_VERTEXPROCESSING,
                                      &gPresentParam, &gpD3DDevice);
	}

	// as last resort, try software
	if (FAILED(error) || !gpD3DDevice)
	{
		fprintf(pFile, "Failed mixed vertex processing device creation...  Trying software\n");
		error = IDirect3D9_CreateDevice(gpD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                      hMain, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &gPresentParam, &gpD3DDevice);
	}

	// switch to sw renderer
	if (FAILED(error) || !gpD3DDevice)
	{
		fprintf(pFile, "Failed software vertex processing device creation\n");
		fprintf(pFile, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n");
		MessageBox(hMain, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n\nSwitching to software renderer",
			NULL, MB_OK);

		gD3DDriverProfile.bSoftwareRenderer = TRUE;

		fclose(pFile);

		return FALSE;
	}

	IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &gD3DDriverProfile.d3dCaps);

	// now run through and check for everything we need
	IDirect3D9_GetAdapterIdentifier(gpD3D, D3DADAPTER_DEFAULT, 0,
                                   &gD3DDriverProfile.adapterID);

	fprintf(pFile, "Video Hardware Detected\nDriver:  %s\nDescription:  %s\nProduct:  %d\nVersion:  %d\nSubversion:  %d\nBuild:  %d\nGUID:  %x, %x, %x, %s\n",
		gD3DDriverProfile.adapterID.Driver,
		gD3DDriverProfile.adapterID.Description,
		HIWORD(gD3DDriverProfile.adapterID.DriverVersion.HighPart),
		LOWORD(gD3DDriverProfile.adapterID.DriverVersion.HighPart),
		HIWORD(gD3DDriverProfile.adapterID.DriverVersion.LowPart),
		LOWORD(gD3DDriverProfile.adapterID.DriverVersion.LowPart),
		gD3DDriverProfile.adapterID.DeviceIdentifier.Data1,
		gD3DDriverProfile.adapterID.DeviceIdentifier.Data2,
		gD3DDriverProfile.adapterID.DeviceIdentifier.Data3,
		gD3DDriverProfile.adapterID.DeviceIdentifier.Data4);

	if ((gD3DDriverProfile.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0)
	{
		fprintf(pFile, "Failed hardware transform and light check...  Trying software tnl\n");

		if ((gD3DDriverProfile.d3dCaps.PrimitiveMiscCaps & D3DPMISCCAPS_CLIPTLVERTS) == 0)
		{
			// don't even bother, gonna look like crap anyway
			fprintf(pFile, "Failed post tnl clip check\n");
			fprintf(pFile, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n");
			MessageBox(hMain, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n\nSwitching to software renderer",
				NULL, MB_OK);

			gD3DDriverProfile.bSoftwareRenderer = TRUE;

			fclose(pFile);

			return FALSE;
		}
		else
		{
			// destroy device, recreate as software, and shoot the fricken' driver writers
			IDirect3DDevice9_Release(gpD3DDevice);
			
			error = IDirect3D9_CreateDevice(gpD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                         hMain, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                         &gPresentParam, &gpD3DDevice);

			// if failed, switch to sw renderer, really
			if (FAILED(error) || !gpD3DDevice)
			{
				fprintf(pFile, "Failed software vertex processing device creation\n");
				fprintf(pFile, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n");
				MessageBox(hMain, "Incompatible device and/or desktop settings.  Please check the following:\n\n- Your video card is installed properly\n- You have the latest drivers\n- Your desktop color depth is set to 32bit\n\nSwitching to software renderer",
					NULL, MB_OK);

				gD3DDriverProfile.bSoftwareRenderer = TRUE;

				fclose(pFile);

				return FALSE;
			}
			else
			{
				// repoll caps
				IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &gD3DDriverProfile.d3dCaps);

				// now run through and check for everything we need
				IDirect3D9_GetAdapterIdentifier(gpD3D, D3DADAPTER_DEFAULT, 0,
                                            &gD3DDriverProfile.adapterID);

				gD3DDriverProfile.vertexProcessFlag = D3DUSAGE_SOFTWAREPROCESSING;
			}
		}
	}

	// if max streams == 0, driver is not dx9 compliant
	if (gD3DDriverProfile.d3dCaps.MaxStreams <= 0)
	{
		fprintf(pFile, "Driver is not a DirectX 9 compliant driver.  Please verify your video card and latest drivers are DirectX 9 compliant.\n");
		bProblem = TRUE;
	}

	// right now, we require at least 2 stages of multitexture
	if ((gD3DDriverProfile.d3dCaps.MaxTextureBlendStages < 2) ||
		(gD3DDriverProfile.d3dCaps.MaxSimultaneousTextures < 2))
	{
		fprintf(pFile, "No multitexture hardware detected.  Please verify your video card and latest drivers support multitexture.\n");
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.ZCmpCaps & gZCmpCaps) != gZCmpCaps)
	{
		fprintf(pFile, "ZCmpCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.ZCmpCaps);
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.AlphaCmpCaps & gAlphaCmpCaps) != gAlphaCmpCaps)
	{
		fprintf(pFile, "AlphaCmpCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.AlphaCmpCaps);
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.SrcBlendCaps & gBlendCaps) != gBlendCaps)
	{
		fprintf(pFile, "SrcBlendCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.SrcBlendCaps);
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.DestBlendCaps & gBlendCaps) != gBlendCaps)
	{
		fprintf(pFile, "DestBlendCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.DestBlendCaps);
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.TextureCaps & gTextureCaps) != gTextureCaps)
	{
		fprintf(pFile, "TextureCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.TextureCaps);
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.TextureAddressCaps & gTextureAddressCaps) != gTextureAddressCaps)
	{
		fprintf(pFile, "TextureAddressCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.TextureAddressCaps);
		bProblem = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.TextureOpCaps & gTextureOpCaps) != gTextureOpCaps)
	{
		fprintf(pFile, "TextureOpCaps check failed, %d\n", gD3DDriverProfile.d3dCaps.TextureOpCaps);
		bProblem = TRUE;
	}

	// optional caps
	if ((gD3DDriverProfile.d3dCaps.PrimitiveMiscCaps & D3DPMISCCAPS_CULLCCW) == 0)
	{
		fprintf(pFile, "Culling check failed\n");
		gD3DDriverProfile.bCull = FALSE;
		bDisable = TRUE;
	}

	if ((gD3DDriverProfile.d3dCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) == 0)
	{
		fprintf(pFile, "Anisotropic filtering check failed\n");
		gD3DDriverProfile.minFilter = D3DTEXF_LINEAR;
		gD3DDriverProfile.magFilter = D3DTEXF_LINEAR;
		bDisable = TRUE;
	}
	else
	{
		gD3DDriverProfile.minFilter = D3DTEXF_ANISOTROPIC;
		gD3DDriverProfile.magFilter = D3DTEXF_ANISOTROPIC;
	}

	if ((gD3DDriverProfile.d3dCaps.RasterCaps & gFogCaps) != gFogCaps)
	{
		fprintf(pFile, "Hardware fog check failed\n");
		gD3DDriverProfile.bFogEnable = FALSE;
	}
	else
	{
		gD3DDriverProfile.bFogEnable = TRUE;
	}

	gD3DDriverProfile.maxAnisotropy = gD3DDriverProfile.d3dCaps.MaxAnisotropy;

	if ((gD3DDriverProfile.d3dCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS) == 0 ||
       (gD3DDriverProfile.d3dCaps.RasterCaps & D3DRS_SLOPESCALEDEPTHBIAS) == 0)
	{
		fprintf(pFile, "Zbias check failed\n");
		gD3DDriverProfile.bZBias = FALSE;
		bDisable = TRUE;
	}

	fclose(pFile);

	if (bProblem)
	{
		char	string[255];

		GetPrivateProfileString("config", "rendererfailedonce", "error", string, 255, "./config.ini");
		strlwr(string);

		if (0 != strcmp(string, "true"))
			MessageBox(hMain, "A problem was detected with your video card drivers and/or hardware.  Please see d3dlog.txt for more information.\n\nSwitching to software renderer",
				NULL, MB_OK);

		WritePrivateProfileString("config", "rendererfailedonce", "true", "./config.ini");
		WritePrivateProfileString("config", "softwarerenderer", "true", "./config.ini");
		gD3DDriverProfile.bSoftwareRenderer = TRUE;

		return FALSE;
	}
	else if (bDisable)
	{
		char	string[255];

		GetPrivateProfileString("config", "rendererfailedonce", "error", string, 255, "./config.ini");
		strlwr(string);

		if (0 != strcmp(string, "true"))
			MessageBox(hMain, "A problem was detected with your video card drivers and/or hardware.  Please see d3dlog.txt for more information.\n\nDirect3D will continue with certain features disabled.  If the resulting rendering is not satisfactory, you may force software rendering in the Configuration Menu",
				NULL, MB_OK);

		WritePrivateProfileString("config", "rendererfailedonce", "true", "./config.ini");
	}
	else
		WritePrivateProfileString("config", "rendererfailedonce", "false", "./config.ini");

	gD3DDriverProfile.texMemTotal = IDirect3DDevice9_GetAvailableTextureMem(gpD3DDevice);

	if (gD3DDriverProfile.texMemTotal < (32 * 1024 * 1024))
//	if (1)
	{
//		gD3DDriverProfile.texMemTotal = (32 * 1024 * 1024);
		gD3DDriverProfile.bManagedTextures = TRUE;
	}
	else
		gD3DDriverProfile.bManagedTextures = FALSE;

//	gD3DDriverProfile.texMemTotal = (32 * 1024 * 1024);
	gD3DDriverProfile.texMemTotal /= 4;

	gD3DDriverProfile.texMemWorldStatic = gD3DDriverProfile.texMemTotal * 0.40f;
	gD3DDriverProfile.texMemWorldDynamic = gD3DDriverProfile.texMemTotal * 0.13f;
	gD3DDriverProfile.texMemLMapStatic = gD3DDriverProfile.texMemTotal * 0.20f;
	gD3DDriverProfile.texMemLMapDynamic = gD3DDriverProfile.texMemTotal * 0.10f;
	gD3DDriverProfile.texMemObjects = gD3DDriverProfile.texMemTotal * 0.17f;

	return TRUE;
}
