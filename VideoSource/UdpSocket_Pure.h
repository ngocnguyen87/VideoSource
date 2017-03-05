// UdpSocket.h: interface for the CUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

typedef void (*PSOCKETRECVDATA)(LPBYTE,int,int,LPVOID);

class CUdpSocket  
{
public:
	SOCKET m_hRecvSocket;
	HANDLE m_hRecvThread;
	LPVOID m_UserData;
	void (*m_pOnRecvData)(LPBYTE,int,int,LPVOID);
	BOOL m_fExit;

	SOCKET m_hSendSocket;
public:
	int SendTo(LPVOID data,int size,int ip,int port);
	CUdpSocket();
	virtual ~CUdpSocket();
	int	StartRecvData(int port,void (*OnRecvData)(LPBYTE,int,int,LPVOID),LPVOID UserData);
	void StopRecvData();
	static UINT RecvThread(LPVOID lpParam);
};

class CMulticastSocket : public CUdpSocket
{
protected:
	CString		m_sGroupIP;
	UINT		m_nGroupPort;
	ip_mreq		m_mrMReq;
	BOOL		m_fJoinGroup;

public:
	BOOL LeaveGroup();
	BOOL JoinGroup(CString sGroupIP,UINT nGroupPort, PSOCKETRECVDATA pRecvFunc, LPVOID UserData, UINT TTL=5,BOOL bLoopBack=FALSE);
};
