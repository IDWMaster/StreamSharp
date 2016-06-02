// DXAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <d3d.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <d3d9.h>
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

class ByteStream :public IMFByteStream {
public:
	ULONG refcount;
	HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) {
		if (ppvObject == 0) {
			return E_INVALIDARG;
		}
		if (riid == IID_IMFByteStream || riid == IID_IUnknown) {
			*ppvObject = (PVOID)this;
			AddRef();
			return NOERROR;
		}
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) {
		refcount++;
		return refcount;
	}

	ULONG STDMETHODCALLTYPE Release(void) {
		refcount--;
		if (refcount == 0) {
			delete this;
			return 0;
		}
		return refcount;
	}
	ByteStream() {
		refcount = 1;
	}
	HRESULT STDMETHODCALLTYPE GetCapabilities(
		/* [out] */ __RPC__out DWORD *pdwCapabilities) {
		*pdwCapabilities = MFBYTESTREAM_IS_READABLE | MFBYTESTREAM_IS_REMOTE;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetLength(
		/* [out] */ __RPC__out QWORD *pqwLength) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetLength(
		/* [in] */ QWORD qwLength) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetCurrentPosition(
		/* [out] */ __RPC__out QWORD *pqwPosition) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetCurrentPosition(
		/* [in] */ QWORD qwPosition) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE IsEndOfStream(
		/* [out] */ __RPC__out BOOL *pfEndOfStream) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Read(
		/* [size_is][out] */ __RPC__out_ecount_full(cb) BYTE *pb,
		/* [in] */ ULONG cb,
		/* [out] */ __RPC__out ULONG *pcbRead) {
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE BeginRead(
		/* [annotation][out] */
		_Out_writes_bytes_(cb)  BYTE *pb,
		/* [in] */ ULONG cb,
		/* [in] */ IMFAsyncCallback *pCallback,
		/* [in] */ IUnknown *punkState) {
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE EndRead(
		/* [in] */ IMFAsyncResult *pResult,
		/* [annotation][out] */
		_Out_  ULONG *pcbRead) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Write(
		/* [size_is][in] */ __RPC__in_ecount_full(cb) const BYTE *pb,
		/* [in] */ ULONG cb,
		/* [out] */ __RPC__out ULONG *pcbWritten) {
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE BeginWrite(
		/* [annotation][in] */
		_In_reads_bytes_(cb)  const BYTE *pb,
		/* [in] */ ULONG cb,
		/* [in] */ IMFAsyncCallback *pCallback,
		/* [in] */ IUnknown *punkState) {
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE EndWrite(
		/* [in] */ IMFAsyncResult *pResult,
		/* [annotation][out] */
		_Out_  ULONG *pcbWritten) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Seek(
		/* [in] */ MFBYTESTREAM_SEEK_ORIGIN SeekOrigin,
		/* [in] */ LONGLONG llSeekOffset,
		/* [in] */ DWORD dwSeekFlags,
		/* [out] */ __RPC__out QWORD *pqwCurrentPosition) {
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Flush(void) {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Close(void) {
		return S_OK;
	}


};


extern "C" {
	__declspec(dllexport) bool InitCapture_Screen() {
		MFStartup(MF_VERSION);
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

		//TODO: Create byte stream
		ByteStream* bs = new ByteStream();
		IMFSinkWriter* outputstream;
		IMFAttributes* attribs = 0;
		MFCreateAttributes(&attribs, 1);
		attribs->SetGUID(MF_TRANSCODE_CONTAINERTYPE, MFTranscodeContainerType_MPEG4);
		MFCreateSinkWriterFromURL(0, bs, attribs, &outputstream);

		while (true) {
			DXGI_OUTDUPL_FRAME_INFO frameinfo;
			IDXGIResource* frame = 0;
			HRESULT res = vmon->AcquireNextFrame(-1, &frameinfo, &frame);
			IDXGISurface* surface = 0;
			frame->QueryInterface(&surface);
			DXGI_SURFACE_DESC surfdesc;
			surface->GetDesc(&surfdesc);
			IMFSample* sample = 0;
			MFCreateSample(&sample);
			IMFMediaBuffer* buffer = 0; //TODO: MF seems to support direct-GPU scenarios; see if we can use these somehow?
			BYTE* cpumem;
			DXGI_MAPPED_RECT gpumem;
			MFCreateMemoryBuffer(surfdesc.Width*surfdesc.Height * 4, &buffer);
			buffer->Lock(&cpumem, 0, 0);
			HRESULT status = vmon->MapDesktopSurface(&gpumem); //TODO: This function doesn't really work. We have to copy to a GPU texture with CPU read access enabled, then copy from that texture to CPU memory....

			memcpy(cpumem, gpumem.pBits, surfdesc.Width*surfdesc.Height * 4); //Copy from GPU to CPU memory
			vmon->UnMapDesktopSurface();
			buffer->Unlock();
			buffer->SetCurrentLength(surfdesc.Width*surfdesc.Height * 4);
			status = sample->AddBuffer(buffer);
			outputstream->WriteSample(0, sample);
			buffer->Release();
			sample->Release();
			vmon->ReleaseFrame();
		}
		//TODO: For each frame; encode with help from this sample: https://msdn.microsoft.com/en-us/library/windows/desktop/ff819477(v=vs.85).aspx

		//dev->CreateRenderTargetView()
		return retval;
	}
}