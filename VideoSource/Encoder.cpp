#include "StdAfx.h"
#include ".\encoder.h"


/*static char lst[256];
CStdioFile logFile;
int CEncoder::m_cEncoders=0;*/


CEncoder::CEncoder(void)
{
	m_pEncodedCallback = NULL;
	m_pUser = NULL;
	m_nLogLevel = 1;
	/*m_cEncoders++;
	if (m_cEncoders == 1)
	{
		if (!logFile.Open("EncLog.txt", CFile::modeWrite | CFile::shareDenyWrite))
			logFile.Open("EncLog.txt", CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
	}*/
}

CEncoder::~CEncoder(void)
{
	/*m_cEncoders--;
	if (m_cEncoders == 0)
		logFile.Close();*/
}


void CEncoder::Log(int nLevel, char* psz, ...)
{
	/*CString s;
	if (nLevel > m_nLogLevel)
		return;
	va_list marker;
	va_start(marker,psz);
	vsprintf(lst,psz,marker);
	va_end(marker);

	if (m_pLog != NULL)
	{
		m_pLog(m_pUser, nLevel, lst);
	}
	else
	{
		s.Format("(%x):\t%s", this, lst);
		logFile.WriteString(s);
		logFile.WriteString("\n");
	}*/
}
