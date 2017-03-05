#include "StdAfx.h"
#include "M3U8File.h"


M3U8File::M3U8File(void)
{
	m_hMutex = CreateMutex(NULL, FALSE, NULL);

	m_vListFile.clear();

	m_nSequenceMedia = 1;
}


M3U8File::~M3U8File(void)
{
}

void M3U8File::setPath(string szPath)
{
	m_sPath = szPath;
}

void M3U8File::addFile(string sFileName)
{
	WaitForSingleObject(m_hMutex, INFINITE);

	//Dam bao luon chi co 5 segment trong playlist
	if(m_vListFile.size() == 5)
	{
		m_vListFile.erase(m_vListFile.begin());

		m_nSequenceMedia++;
	}

	m_vListFile.push_back(sFileName);

	ReleaseMutex(m_hMutex);
}

void M3U8File::releaseFile(bool bEnd)
{
	WaitForSingleObject(m_hMutex, INFINITE);

	FILE* file = fopen("index.m3u8", "w");

	fprintf(file, "#EXTM3U\n");
	fprintf(file, "#EXT-X-VERSION:3\n");
	fprintf(file, "#EXT-X-TARGETDURATION:10\n");
	fprintf(file, "#EXT-X-MEDIA-SEQUENCE:%d\n", m_nSequenceMedia);

	fprintf(file, "\n\n\n\n");

	//List file
	for(int i = 0; i< m_vListFile.size(); i++)
	{
		fprintf(file, "#EXTINF:10.00,\n");

		fprintf(file, m_sPath.c_str());
		fprintf(file,  m_vListFile.at(i).c_str());

		fprintf(file, "\n\n");
	}

	if(bEnd)
		fprintf(file, "#EXT-X-ENDLIST");

	fclose(file);

	ReleaseMutex(m_hMutex);
}



