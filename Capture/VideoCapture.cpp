#include "StdAfx.h"
#include "VideoCapture.h"
#include "Capture.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "strmbasd.lib");

const GUID MEDIASUBTYPE_I420 = 
{0x30323449,0x0000,0x0010,0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};


//// Implementation of CSampleGrabberCB
CVideoCapture::CSampleGrabberCB::CSampleGrabberCB(CVideoCapture* pSampleSink):
	m_pVCapturer(pSampleSink)
{
}

CVideoCapture::CSampleGrabberCB::~CSampleGrabberCB()
{
}

STDMETHODIMP CVideoCapture::CSampleGrabberCB::QueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv,E_POINTER);
	
	if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
	{
		*ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
		return NOERROR;
	}    

	return E_NOINTERFACE;
};

STDMETHODIMP CVideoCapture::CSampleGrabberCB::BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
{
	if(m_pVCapturer)
		m_pVCapturer->OnVideoSample(timeGetTime(),pBuffer,lBufferSize); // callback
	return 0;
}

CVideoCapture::CVideoCapture(void)
{
	CoInitialize(NULL);

	m_pVCaptureFilter = NULL;

	m_pControl = NULL;
	m_pEvent = NULL;

	m_pGraph = NULL;
	m_pBuilder = NULL;

	m_pVSampleGrabber = NULL;
	m_pVCB = NULL;

	m_pOnVideoSample = NULL;
}


CVideoCapture::~CVideoCapture(void)
{
	SAFE_STOP(m_pControl);
	
	NukeDownstream(m_pVCaptureFilter);
	SAFE_RELEASE(m_pVSampleGrabber);
	SAFE_RELEASE(m_pControl)
	SAFE_RELEASE(m_pEvent);
	SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pVCaptureFilter);
	SAFE_RELEASE(m_pBuilder);

	if(m_pVCB)
	{
		delete m_pVCB;
		m_pVCB = NULL;
	}

}

void CVideoCapture::setParent(void* pParents)
{
	m_pCapture = (CCapture*)pParents;
}

string CVideoCapture::getListDevice()
{
	string szList;
	ostringstream devList;
	devList.str("");

	HRESULT hr;

	IBaseFilter* pFilter = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker* pMoniker = NULL;
	ULONG cFetched;
	int nCapDevID = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
	IID_ICreateDevEnum, (void **)&pDevEnum);
	if(hr != 0)
	{
		cout <<"CTerminal::GetVideoInputFilter: system device enumerator Error"; 
	}

	// Create an enumerator for video capture devices.
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (hr != 0)
	{
		cout <<"CTerminal::GetVideoInputFilter: call CreateClassEnumerator failed";
	}

	pClassEnum->Reset();
	while(pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		IPropertyBag *pBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(hr==0)
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			
			TCHAR str[2048];		
			WideCharToMultiByte(CP_ACP,0,var.bstrVal, -1, (LPSTR)str, 2048, NULL, NULL);

			cout<< "\nInput video capture device "<<str;

			if(strcmp((LPSTR)str,"Pinnacle DCxx MJPEG Capture Filter")==0)
				continue;

			devList<<str<<";";

			if(m_pCapture)
				m_pCapture->updateListDev(nCapDevID, (LPSTR)str);

			pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
			pMoniker->Release();	

			nCapDevID++;
		}
		
	}

	szList = devList.str();

	return szList;
}

IBaseFilter* CVideoCapture::getCaptureFilter(string szDevice)
{
	HRESULT hr;

	m_sDeviceName = szDevice;

	IBaseFilter* pFilter = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker* pMoniker = NULL;
	ULONG cFetched;
	int nCapDevID = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
	IID_ICreateDevEnum, (void **)&pDevEnum);
	if(hr != 0)
	{
		cout <<"CTerminal::GetVideoInputFilter: system device enumerator Error"; 
	}

	// Create an enumerator for video capture devices.
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (hr != 0)
	{
		cout <<"CTerminal::GetVideoInputFilter: call CreateClassEnumerator failed";
	}

	pClassEnum->Reset();
	while(pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		IPropertyBag *pBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(hr==0)
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			
			TCHAR str[2048];		
			WideCharToMultiByte(CP_ACP,0,var.bstrVal, -1, (LPSTR)str, 2048, NULL, NULL);

			cout<< "\nInput video capture device "<<str;

			if(strcmp((LPSTR)str,"Pinnacle DCxx MJPEG Capture Filter")==0)
				continue;

			pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);

			if(strcmp((LPSTR)str,szDevice.c_str())==0)
			{
				pMoniker->Release();
				return pFilter;
			}
				
		}
		
	}		

	return NULL;
}

void CVideoCapture::getListDeviceCaps(string szDevice)
{
	m_pVCaptureFilter = getCaptureFilter(szDevice);

	int nId=0;
	int nIndex=-1;
	HRESULT hr;

	IEnumPins *pEnum = NULL;
	IPin      *pCamOutPin = NULL;

	m_pVCaptureFilter->EnumPins(&pEnum);

	hr = pEnum->Reset();
	hr = pEnum->Next(1, &pCamOutPin, NULL);
	//hr = pEnum->Next(1, &pCamOutPin, NULL);

	IAMStreamConfig *pConfig;
	IEnumMediaTypes *pMedia;
	AM_MEDIA_TYPE *pmt=NULL, *pftn=NULL;
	//CString cs, cs1, cs2;
	OLECHAR* value;
	hr= pCamOutPin->EnumMediaTypes(&pMedia);
	if(SUCCEEDED(hr))
	{
		while(pMedia->Next(1, &pmt, 0)==S_OK)
		{
			if(pmt->formattype==FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER * vt=(VIDEOINFOHEADER*) pmt->pbFormat;
				string sColorSpace;

				ostringstream convert;   
				string sHeight;
			    convert << vt->bmiHeader.biHeight;    
                sHeight = convert.str();

				string sWidth;
				ostringstream convert2;
				convert2 <<vt->bmiHeader.biWidth;
				sWidth=convert2.str();


				if(pmt->subtype==MEDIASUBTYPE_AYUV)
				   sColorSpace =" AYUV\0";
				else if(pmt->subtype==MEDIASUBTYPE_YUY2)
				  sColorSpace = "YUY2\0";
				else if(pmt->subtype==MEDIASUBTYPE_UYVY)
					sColorSpace="UYVY\0";
				else if(pmt->subtype==MEDIASUBTYPE_IMC1)
				   sColorSpace="IMC1\0";
				else if(pmt->subtype==MEDIASUBTYPE_IMC3)
				   sColorSpace="IMC2\0";
				else if(pmt->subtype==MEDIASUBTYPE_IMC2)
				   sColorSpace="IMC3\0";
				else if(pmt->subtype==MEDIASUBTYPE_IMC4)
				   sColorSpace="IMC4\0";
				else if(pmt->subtype==MEDIASUBTYPE_YV12)
				   sColorSpace="YV12\0";
				else if(pmt->subtype==MEDIASUBTYPE_NV12)
				   sColorSpace="NV12\0";
				else if(pmt->subtype==MEDIASUBTYPE_YVYU)
				   sColorSpace="YVYU\0";
				else if(pmt->subtype==MEDIASUBTYPE_RGB24)
				   sColorSpace="RGB24\0";
				else if(pmt->subtype==MEDIASUBTYPE_RGB32)
				   sColorSpace="RGB32\0";
				else if(pmt->subtype==MEDIASUBTYPE_RGB1)	
				   sColorSpace="RGB1\0";
				else if(pmt->subtype==MEDIASUBTYPE_RGB4)	
				   sColorSpace="RGB4\0";
				else if(pmt->subtype==MEDIASUBTYPE_RGB8)
				   sColorSpace="RGB8\0";
				else if(pmt->subtype==MEDIASUBTYPE_I420)
				   sColorSpace="I420\0";

				string sCap = sColorSpace + " - \0"+ sWidth+"x\0"+ sHeight;

				//UPDATE HERE
				if(m_pCapture)
					m_pCapture->updateListCaps(nId,sCap );

				nId++;
			}
		}
		pMedia->Release();
	}
	pCamOutPin->Release();
	pEnum->Release();
}

void CVideoCapture::setFormatIndex(int nIndex)
{
	m_nFormatIndex = nIndex;
}

void CVideoCapture::setVideoFormat(int nIndex)
{
	HRESULT hr;
	RECT rcDest;
	int nCount = 0;

	IPin* pCamOutPin = NULL;

	hr = m_pBuilder->FindPin(
			m_pVCaptureFilter,                   // Pointer to the filter to search.
			PINDIR_OUTPUT,          // Search for an output pin.
			&PIN_CATEGORY_CAPTURE,  // Search for a preview pin.
			&MEDIATYPE_Video,       // Search for a video pin.
			TRUE,                   // The pin must be unconnected. 
			0,                      // Return the first matching pin (index 0).
			&pCamOutPin);                 // This variable receives the IPin pointer.

	
    IAMStreamConfig* pConfig;
    IEnumMediaTypes *pMedia;
    AM_MEDIA_TYPE *pmt = NULL, *pfnt = NULL;
	VIDEOINFOHEADER* pVih = NULL;

    hr = pCamOutPin->EnumMediaTypes( &pMedia );
    if(SUCCEEDED(hr))
    {
        while(pMedia->Next(1, &pmt, 0) == S_OK)
        {
            if( pmt->formattype == FORMAT_VideoInfo )
            {
     //           VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)pmt->pbFormat;
     //           // printf("Size %i  %i\n", vih->bmiHeader.biWidth, vih->bmiHeader.biHeight );
     //           if( vih->bmiHeader.biWidth == width && vih->bmiHeader.biHeight == height )
     //           {
					//if((pmt->formattype == FORMAT_VideoInfo) && (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_I420)))
					//{
					//	pfnt = pmt;
					//
					//	// printf("found mediatype with %i %i\n", vih->bmiHeader.biWidth, vih->bmiHeader.biHeight );
					//	//char test[100];
					//	//sprintf(test,"Width=%d\nHeight=%d",vih->bmiHeader.biWidth, vih->bmiHeader.biHeight);
					//	//MessageBox(test);
					//	break;
					//}
     //           }

				if(nIndex == nCount)
				{
					pfnt = pmt;
					pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
					break;
				}

				nCount++;
                DeleteMediaType( pmt );
            }                        
        }
        pMedia->Release();
    }
    hr = pCamOutPin->QueryInterface( IID_IAMStreamConfig, (void **) &pConfig );
    if(SUCCEEDED(hr))
    {
        if( pfnt != NULL )
        {
            hr=pConfig->SetFormat( pfnt );

			//if(SUCCEEDED(hr))        
			//MessageBox("OK");

            DeleteMediaType( pfnt );
        }
        hr = pConfig->GetFormat( &pfnt );
        if(SUCCEEDED(hr))
        {
			
            m_nWidth = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biWidth;
            m_nHeight = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biHeight;
			
            DeleteMediaType( pfnt );
        }
    }


}

void CVideoCapture::getVideoSize(int* nWidth, int* nHeight)
{
	*nWidth = m_nWidth;
	*nHeight = m_nHeight;
}

HRESULT CVideoCapture::Run()
{
	if(m_pControl)
		return m_pControl->Run();

	return E_POINTER;
}

int CVideoCapture::startCapture()
{
	HRESULT hr;

	// Create the filter graph.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void **)&m_pGraph);
		
	if(hr!=0)
	{
		cout<< "Can not create filter graph!\n";
		return hr;
	}

	// Create the capture graph builder.
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **)&m_pBuilder);

	if(hr!=0)
	{
		cout<< "Can not create graph builder!\n";
		return hr;
	}
	
	hr = m_pBuilder->SetFiltergraph(m_pGraph);

	if(hr!=0)
	{
		cout<< "Can not set filter graph to graph builder!\n";
		return hr;
	}

	IBaseFilter *pVGrabFilter;
	
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (LPVOID *)&pVGrabFilter);
	if(hr!=0)
	{
		cout<< "Call CoCreateInstance CLSID_SampleGrabber got error.\n";
		return hr;
	}

	hr = m_pGraph->AddFilter(pVGrabFilter,L"Video Grabber");
	if(hr!=0)
	{
		cout<< "Call m_pGraph->AddFilter pVGrabFilter got error.\n";
		return hr;
	}
	
	//Get grab sample
	hr = pVGrabFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pVSampleGrabber);
	if(hr!=0)
	{
		cout<< "Call pVGrabFilter->QueryInterface IID_ISampleGrabber got error.\n";
		return hr;
	}

	//Get capture filter
	if(!m_pVCaptureFilter)
		m_pVCaptureFilter = getCaptureFilter(m_sDeviceName);

	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pEvent);
	if(hr!=0)
	{
		cout<< "Querry IMediaEvevnt error\n";
	}

	//add filter to graph
	hr = m_pGraph->AddFilter(m_pVCaptureFilter, L"Video Capture");
	if(hr!=0)
	{
		cout<< "Call m_pGraph->AddFilter m_pVCaptureFilter got error.\n";
		return hr;
	}
	
	//Set Video Format
	setVideoFormat(m_nFormatIndex);

	////////Set Video format
	//IAMStreamConfig* pISC = NULL;
	//AM_MEDIA_TYPE *pmt = 0;

	//hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pVCaptureFilter, IID_IAMStreamConfig, (void**)&pISC);
	//if(FAILED(hr))
	//{
	//	return -1;
	//}

	//if(SUCCEEDED(hr)&& pISC)
	//{
	//	int iFmt, iSize, i;
	//	BYTE* bbuf = 0;
	//	hr = pISC->GetNumberOfCapabilities(&iFmt, &iSize);
	//	if(SUCCEEDED(hr))
	//	{
	//		bbuf = new BYTE[iSize];

	//		for(i=0; i< iFmt; i++)
	//		{
	//			hr = pISC->GetStreamCaps(i, &pmt, bbuf);
	//			if(SUCCEEDED(hr))
	//			{
	//				VIDEOINFOHEADER *pvih = (VIDEOINFOHEADER*) pmt->pbFormat;
	//				if ((pvih->bmiHeader.biHeight == nHeight) && (pvih->bmiHeader.biWidth == nWidth))
	//				{
	//					if((pmt->formattype == FORMAT_VideoInfo) && (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_I420)))
	//					{
	//						pISC->SetFormat(pmt);
	//						DeleteMediaType(pmt);
	//						break;
	//					}
	//				}
	//			}//end if
	//			DeleteMediaType(pmt);
	//		}//end for
	//		if(bbuf)
	//			delete bbuf;
	//	}//End SUCCEEDED

	//	pISC->Release();
	//}

	hr = m_pBuilder->RenderStream(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		m_pVCaptureFilter,
		NULL,
		pVGrabFilter
		);

	pVGrabFilter->Release();


	////Init Video sample Callback
	if(m_pVCB)
	{
		delete m_pVCB;
		m_pVCB = NULL;
	}

	m_pVCB = new CSampleGrabberCB(this);
	hr = m_pVSampleGrabber->SetCallback(m_pVCB, 1);
	if(FAILED(hr))
	{
	}

	hr = m_pVSampleGrabber->SetOneShot(FALSE);      // disable one-shot mode
	if(FAILED(hr))
	{
	}

	hr = m_pVSampleGrabber->SetBufferSamples(TRUE); // uses buffer samples
	if(FAILED(hr))
	{
	}

	IMediaFilter *pMediaFilter;
	hr = m_pGraph->QueryInterface(IID_IMediaFilter,(void **)&pMediaFilter);
	if(FAILED(hr) || !pMediaFilter)
	{
		return hr;
	}

	hr = pMediaFilter->SetSyncSource(NULL); // Turn off the reference clock
	if(FAILED(hr))
	{
	}

	hr = pMediaFilter->Release();
	if(FAILED(hr))
	{
	}

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pControl);
	if(FAILED(hr))
	{
		return hr;
	}

	m_pControl->Run();

	return hr;
}

void CVideoCapture::stopCapture()
{
	if(m_pControl)
		m_pControl->Stop();

	SAFE_STOP(m_pControl);
	
	NukeDownstream(m_pVCaptureFilter);
	SAFE_RELEASE(m_pVSampleGrabber);
	SAFE_RELEASE(m_pControl)
	SAFE_RELEASE(m_pEvent);
	SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pVCaptureFilter);
	SAFE_RELEASE(m_pBuilder);

	if(m_pVCB)
	{
		delete m_pVCB;
		m_pVCB = NULL;
	}
}

void CVideoCapture::OnVideoSample(DWORD dwTimeStamp, BYTE* pBuffer, long lBufferSize)
{
	if(m_pCapture)
		m_pCapture->OnVideoSample(dwTimeStamp, pBuffer, lBufferSize);
}

void CVideoCapture::NukeDownstream(IBaseFilter* pBF)
{
	IPin *pP, *pTo;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;
	HRESULT hr = pBF->EnumPins(&pins);
	pins->Reset();
	while (hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if (pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if (hr == NOERROR)
				{
					if (pininfo.dir == PINDIR_INPUT)
					{
						NukeDownstream(pininfo.pFilter);
						m_pGraph->Disconnect(pTo);
						m_pGraph->Disconnect(pP);
						m_pGraph->RemoveFilter(pininfo.pFilter);
					}
					pininfo.pFilter->Release();
				}
				pTo->Release();
			}
			pP->Release();
		}
	}
	if (pins)
	pins->Release();
}
