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
#include <codecapi.h>
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
			return S_OK;
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
	void(*bw)(const void* bytes, ULONG count, void* callback, void* state);
	void(*ew)(void*, ULONG*);
	bool read;
	ByteStream(void(*BeginWrite)(const void* bytes, ULONG count, void* callback, void* state), void(*EndWrite)(void*, ULONG*), bool read = false) {
		bw = BeginWrite;
		ew = EndWrite;
		this->read = read;
		refcount = 1;
	}
	HRESULT STDMETHODCALLTYPE GetCapabilities(
		/* [out] */ __RPC__out DWORD *pdwCapabilities) {
		
		*pdwCapabilities = (bw == 0 ? 0 : MFBYTESTREAM_IS_WRITABLE) | (read ? MFBYTESTREAM_IS_READABLE : 0);
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
		bw(pb, cb, pCallback, punkState); //What a punk
		
		return S_OK;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE EndWrite(
		/* [in] */ IMFAsyncResult *pResult,
		/* [annotation][out] */
		_Out_  ULONG *pcbWritten) {
		ew(pResult, pcbWritten);
		return S_OK;
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
	__declspec(dllexport) void PlaybackStream() {
		MFStartup(MF_VERSION);
		IMFMediaSession* session = 0;
		MFCreateMediaSession(0, &session);
		ByteStream* bs = new ByteStream(0, 0, true);
		IMFSourceResolver* resolver = 0;
		MFCreateSourceResolver(&resolver);
		MF_OBJECT_TYPE type;
		IUnknown* idunno = 0;
		//Retrieve media source object from bytestream
		resolver->CreateObjectFromByteStream(bs, 0, MF_RESOLUTION_MEDIASOURCE, 0, &type, &idunno);
		IMFMediaSource* mediaSource = 0;
		idunno->QueryInterface(&mediaSource);

		bs->Release();
		session->Release();
	}
	
	__declspec(dllexport) void CompleteIO(IMFAsyncCallback* cb, IUnknown* brute) {
		IMFAsyncResult* res = 0;
		MFCreateAsyncResult(0, cb, brute, &res);
		cb->Invoke(res);
	}
	__declspec(dllexport) bool InitCapture_Screen(void(*streamcb)(void*),void(*BeginWrite)(const void* bytes,ULONG count,void* callback, void* state),void(*EndWrite)(void*,ULONG*)) {
		MFStartup(MF_VERSION);
		ID3D11Device* dev;
		D3D_FEATURE_LEVEL featureLevel;
		ID3D11DeviceContext* ctx;
		bool retval = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, 0, 0, D3D11_SDK_VERSION, &dev, &featureLevel, &ctx) == S_OK;
		HWND desktop = GetDesktopWindow();
		RECT desktopSize;
		GetWindowRect(desktop, &desktopSize);
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

		//Create staging texture that we can copy active frame to (we can't lock the backbuffer from userspace)
		ID3D11Texture2D* stagingTexture = 0;

		
		//TODO: Create byte stream
		ByteStream* bs = new ByteStream(BeginWrite,EndWrite);
		streamcb(bs);
		IMFSinkWriter* outputstream;
		IMFAttributes* attribs = 0;
		MFCreateAttributes(&attribs, 1);
		attribs->SetGUID(MF_TRANSCODE_CONTAINERTYPE, MFTranscodeContainerType_FMPEG4);
		HRESULT res = MFCreateSinkWriterFromURL(0, bs, attribs, &outputstream); //TODO: For now; we will just output to a simple file
		
		IMFMediaType* mediaType;
		MFCreateMediaType(&mediaType);
		mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
		mediaType->SetUINT32(MF_MT_AVG_BITRATE, 16000000); //Bits per second
		mediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
		MFSetAttributeSize(mediaType, MF_MT_FRAME_SIZE, desktopSize.right, desktopSize.bottom); //TODO: Dynamic based on screen size
		MFSetAttributeRatio(mediaType, MF_MT_FRAME_RATE, 60, 1); //60 frames per second
		MFSetAttributeRatio(mediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
		DWORD videoStream;
		HRESULT streamStatus = outputstream->AddStream(mediaType, &videoStream);
		mediaType->Release();
		mediaType = 0;
		MFCreateMediaType(&mediaType);
		mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
		mediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
		MFSetAttributeSize(mediaType, MF_MT_FRAME_SIZE, desktopSize.right, desktopSize.bottom); //TODO: Dynamic based on screen size
		MFSetAttributeRatio(mediaType, MF_MT_FRAME_RATE, 60, 1); //60 frames per second
		MFSetAttributeRatio(mediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
		HRESULT inputStatus = outputstream->SetInputMediaType(videoStream, mediaType, 0);
		mediaType->Release();
		mediaType = 0;
		HRESULT oval = outputstream->BeginWriting();
		
		while (true) {
			
			DXGI_OUTDUPL_FRAME_INFO frameinfo;
			IDXGIResource* frame = 0;
			HRESULT res = vmon->AcquireNextFrame(-1, &frameinfo, &frame);
			IDXGISurface* surface = 0;
			frame->QueryInterface(&surface);
			DXGI_SURFACE_DESC surfdesc;
			surface->GetDesc(&surfdesc);

			ID3D11Texture2D* backbufferTexture = 0;
			surface->QueryInterface(&backbufferTexture);
			if (stagingTexture == 0) {
				D3D11_TEXTURE2D_DESC desc;
				backbufferTexture->GetDesc(&desc);
				//Clone texture
				//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.Usage = D3D11_USAGE_STAGING;
				desc.MiscFlags = 0;
				desc.BindFlags = 0;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; //Allow CPU access to buffer from userspace
				dev->CreateTexture2D(&desc, 0, &stagingTexture);
				
			}
			ctx->CopyResource(stagingTexture, backbufferTexture);

			vmon->ReleaseFrame();
			backbufferTexture->Release();
			IMFSample* sample = 0;
			MFCreateSample(&sample);
			
			//Copy from GPU to CPU memory
			HRESULT status = 0;
			IMFMediaBuffer* cpubuffer = 0;
			/*D3D11_MAPPED_SUBRESOURCE gpumem;
			ctx->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &gpumem);
			
			MFCreateMemoryBuffer(gpumem.DepthPitch, &cpubuffer);
			BYTE* cpumem = 0;
			cpubuffer->Lock(&cpumem, 0, 0);
			memcpy(cpumem, gpumem.pData, gpumem.DepthPitch);
			cpubuffer->Unlock();
			cpubuffer->SetCurrentLength(gpumem.DepthPitch);
			ctx->Unmap(stagingTexture, 0);
			*/
			//GPU direct
			MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), stagingTexture, 0, false, &cpubuffer);
			IMF2DBuffer* im2d = 0;
			cpubuffer->QueryInterface(&im2d);
			DWORD len = 0;
			im2d->GetContiguousLength(&len);
			im2d->Release();
			im2d = 0;
			cpubuffer->SetCurrentLength(len);
			status = sample->AddBuffer(cpubuffer);
			
			cpubuffer->Release();


			sample->SetSampleDuration(10 * 1000 * 1000 / 60); //1 frame
			sample->SetSampleTime(0);
			status = outputstream->WriteSample(0, sample);
			sample->Release();
		}
		outputstream->Finalize();
		//TODO: For each frame; encode with help from this sample: https://msdn.microsoft.com/en-us/library/windows/desktop/ff819477(v=vs.85).aspx

		//dev->CreateRenderTargetView()
		return retval;
	}
}