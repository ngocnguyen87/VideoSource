

class CVideoRender;

class __declspec(dllexport)  CRender
{
public:

	CRender(void);
	CRender(void* pUser);

	~CRender(void);

	int InitVideoRender(HWND hWnd);

	void UpdateOverlay(RECT rect);
	void writeData(int nStream, BYTE* pBuffer, int width, int height, unsigned long timestamp);

protected:

	void* m_pUser;
	
	CVideoRender* m_pVideoRender;
};