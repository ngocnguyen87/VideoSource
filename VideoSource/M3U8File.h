#pragma once

#include <string>
#include <vector>

using namespace std;

class M3U8File
{
public:
	M3U8File(void);
	~M3U8File(void);

private:
	string m_sPath; //URI

	vector<string> m_vListFile;

	HANDLE m_hMutex;

	int m_nSequenceMedia;

public:
	void setPath(string szPath);

	void addFile(string sFileName);

	void releaseFile(bool bEnd = false);

};

