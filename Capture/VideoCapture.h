#pragma once

#include <string>
#include <qedit.h>
#include <dshow.h>
#include <Mmsystem.h>

#include <iostream>
#include <sstream>
#include <streams.h>

using namespace std;

#ifndef SAFE_STOP
#define SAFE_STOP(x) if(x) (x)->Stop();
#endif//SAFE_STOP

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x) { (x)->Release(); (x) = NULL; }
#endif//SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(x) { delete (x); (x) = NULL; }
#endif//SAFE_DELETE


//call back function
typedef void (*POnVideoSample)(DWORD, BYTE*, long);

class CCapture;

class CVideoCapture
{
		//////////////////////////////////////////////////////////////////////////////////
	/// CSampleGrabberCB class for video samples grabber callback
	class CSampleGrabberCB : public ISampleGrabberCB
	{
	public:
		CSampleGrabberCB(CVideoCapture* pVCapture);
		virtual ~CSampleGrabberCB();
		// Fake out any COM ref counting
		//
		STDMETHODIMP_(ULONG) AddRef() { return 2; };
		STDMETHODIMP_(ULONG) Release() { return 1; };

		// Fake out any COM QI'ing
		//
		STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);

		// The sample grabber is calling us back on its deliver thread
		// This is NOT the main app thread!
		STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
		{
			return 0;
		};


		// The sample grabber is calling us back on its deliver thread.
		// This is NOT the main app thread!
		//
		STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize );

	protected:
		CVideoCapture* m_pVCapturer;
	};

public:
	CVideoCapture(void);
	~CVideoCapture(void);

	void setParent(void* pParent);

	int startCapture();
	void stopCapture();

	HRESULT Run();

	string getListDevice(); //device1;device2;
	IBaseFilter* getCaptureFilter(string szDevice);

	void getListDeviceCaps(string szDevice);
	void getVideoSize(int* nWidth, int* nHeight);

	void setVideoFormat(int nIndex);
	void setFormatIndex(int nIndex);

	void OnVideoSample(DWORD dwTimeStamp, BYTE* pBuffer, long lBufferSize);

	void setVideoCallback(void* pCallBackFunc){m_pOnVideoSample = (POnVideoSample)pCallBackFunc;};

	void NukeDownstream(IBaseFilter* pBF);
protected:
	CCapture*								m_pCapture;

	IBaseFilter*                            m_pVCaptureFilter;

	IMediaControl*                          m_pControl;
	IMediaEventEx*                          m_pEvent;

	IGraphBuilder*                          m_pGraph;
	ICaptureGraphBuilder2*                  m_pBuilder;

	ISampleGrabber*                         m_pVSampleGrabber;
	CSampleGrabberCB*                       m_pVCB;

	POnVideoSample							m_pOnVideoSample;

	int		m_nWidth;
	int		m_nHeight;

	int m_nFormatIndex;

	string m_sDeviceName;
	int test;
};

