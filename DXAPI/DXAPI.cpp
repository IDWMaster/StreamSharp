// DXAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <d3d.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <d3d11.h>
extern "C" {
	bool InitCapture_Screen() {
		ID3D11Device* dev;
		D3D_FEATURE_LEVEL featureLevel;
		ID3D11DeviceContext* ctx;
		bool retval = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, 0, 0, D3D11_SDK_VERSION, &dev, &featureLevel, &ctx) == S_OK;
		//Get DXGI device (used for direct access to pixel buffers)
		IDXGIDevice* dxdev;
		dev->QueryInterface(&dxdev);
		//Get DXGI adapter
		IDXGIAdapter* adapter;
		dxdev->GetAdapter(&adapter);
		//Find first output device (first display connected to graphics card)
		IDXGIOutput* output;
		adapter->EnumOutputs(0, &output);
		adapter->Release();
		adapter = 0;
		DXGI_OUTPUT_DESC videoFormat;
		IDXGIOutput1* graphicsOutput;
		output->QueryInterface(&graphicsOutput);

		output->GetDesc(&videoFormat); //Retrieve video format information
		//Mirror output to virtual display
		IDXGIOutputDuplication* vmon;
		graphicsOutput->DuplicateOutput(dev, &vmon);
		vmon->AcquireNextFrame() //TODO: For each frame; encode with help from this sample: https://msdn.microsoft.com/en-us/library/windows/desktop/ff819477(v=vs.85).aspx

		//dev->CreateRenderTargetView()
		return retval;
	}
}