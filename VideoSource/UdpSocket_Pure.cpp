// RtpSocket.cpp: implementation of the CRtpSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UdpSocket_Pure.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUdpSocket::CUdpSocket()
{
	m_hRecvSocket=m_hSendSocket=INVALID_SOCKET;	
	m_hRecvThread=NULL;
}

CUdpSocket::~CUdpSocket()
{
	m_fExit=TRUE;
	WaitForSingleObject(m_hRecvThread,INFINITE);
	StopRecvData();

	if(m_hSendSocket!=INVALID_SOCKET)
		closesocket(m_hSendSocket);
}

int	CUdpSocket::StartRecvData(int port,void (*OnRecvData)(LPBYTE,int,int,LPVOID),LPVOID UserData)
{
	m_hRecvSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
	if(m_hRecvSocket==INVALID_SOCKET)
		return WSAGetLastError();

	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_port=htons(port);
	int err=bind(m_hRecvSocket,(sockaddr*)&addr,sizeof(addr));
	ASSERT(err==0);
	if(err!=0)
	{
		closesocket(m_hRecvSocket);
		m_hRecvSocket=INVALID_SOCKET;
		return WSAGetLastError();
	}

	int rcv_buf=1024*64;
	setsockopt(m_hRecvSocket,SOL_SOCKET,SO_RCVBUF,(char*)&rcv_buf,sizeof(rcv_buf));

	m_pOnRecvData=OnRecvData;
	m_UserData=UserData;

	if(!m_hRecvThread)
	{
		m_fExit=FALSE;
		m_hRecvThread=AfxBeginThread(RecvThread,this)->m_hThread;
	}

	return 0;
}

void CUdpSocket::StopRecvData()
{
	if(m_hRecvSocket!=INVALID_SOCKET)
	{
		closesocket(m_hRecvSocket);
		m_hRecvSocket=INVALID_SOCKET;
		m_pOnRecvData=NULL;
	}
}

UINT CUdpSocket::RecvThread(LPVOID lpParam)
{
	char *buf=new char[1024*64];
	CUdpSocket* lpSocket=(CUdpSocket*)lpParam;
	int result;
	u_long bytes_in_queue;
	sockaddr_in addr;
	int addr_len;

	while(!lpSocket->m_fExit)
	{
		result=ioctlsocket(lpSocket->m_hRecvSocket,FIONREAD,&bytes_in_queue);
		if((bytes_in_queue>0) && (result==0))
		{
			addr_len=sizeof(addr);
			result=recvfrom(lpSocket->m_hRecvSocket,buf,1024*64,0,(sockaddr*)&addr,&addr_len);
			if((result>0) && lpSocket->m_pOnRecvData)
				lpSocket->m_pOnRecvData(LPBYTE(buf),result,addr.sin_addr.s_addr,lpSocket->m_UserData);
		}
		else
			Sleep(1);
	}

	delete buf;

	return 0;
}

int CUdpSocket::SendTo(LPVOID data, int size,int ip,int port)
{
	int result;
	sockaddr_in addr;
	
	if(m_hSendSocket==INVALID_SOCKET)
	{
		m_hSendSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);

		memset(&addr,0,sizeof(addr));
		addr.sin_family=AF_INET;
		addr.sin_addr.s_addr=INADDR_ANY;
		addr.sin_port=0;
		bind(m_hSendSocket,(sockaddr*)&addr,sizeof(addr));

		//  [8/28/2008]
		int send_buf = 1024*1024;
		setsockopt(m_hRecvSocket, SOL_SOCKET, SO_SNDBUF,(char*)&send_buf,sizeof(send_buf));

		//u_long argp = 1;
		//ioctlsocket(m_hSendSocket, FIONBIO, &argp);
	}

	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=ip;
	addr.sin_port=htons(port);
	result=sendto(m_hSendSocket,(char*)data,size,0,(sockaddr*)&addr,sizeof(addr));

	return result;
}

//===================== CMulticastSocket=====================

BOOL CMulticastSocket::LeaveGroup()
{
	if(m_hRecvSocket==INVALID_SOCKET)
		return TRUE;
	setsockopt(m_hRecvSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&m_mrMReq, sizeof(m_mrMReq));
	m_fJoinGroup=FALSE;
	closesocket(m_hRecvSocket);
	m_hRecvSocket=INVALID_SOCKET;

	return TRUE;
}

BOOL CMulticastSocket::JoinGroup(CString sGroupIP,UINT nGroupPort, PSOCKETRECVDATA pRecvFunc, LPVOID UserData, UINT TTL,BOOL bLoopBack)
{
	sockaddr_in addr;
	TCHAR szError[100];   

	if(m_hRecvSocket!=INVALID_SOCKET)
		closesocket(m_hRecvSocket);

	m_hRecvSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
	if(m_hRecvSocket==INVALID_SOCKET)
		return FALSE;

	//BOOL fReuseAddr=1;
	//if(setsockopt(m_hRecvSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&fReuseAddr,sizeof(BOOL))!=0)
	//{
	//	closesocket(m_hRecvSocket);
	//	m_hRecvSocket=INVALID_SOCKET;
	//	return FALSE;
	//}

	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr = htonl (INADDR_ANY);
	addr.sin_port=htons(nGroupPort);

	if(bind(m_hRecvSocket,(sockaddr*)&addr,sizeof(addr))!=0)
	{
		closesocket(m_hRecvSocket);
		m_hRecvSocket=INVALID_SOCKET;
		return FALSE;
	}	

	/* Join the multicast group */
	// ip_mreq m_mrMReq
	m_mrMReq.imr_multiaddr.s_addr = inet_addr(sGroupIP);	
	m_mrMReq.imr_interface.s_addr = htons(INADDR_ANY);		
	if (setsockopt (m_hRecvSocket, 
					IPPROTO_IP, 
					IP_ADD_MEMBERSHIP, 
					(char FAR *)&m_mrMReq, 
					sizeof (m_mrMReq)) == SOCKET_ERROR)
	{
		wsprintf (szError, TEXT("setsockopt failed! Error: %d"), 
				WSAGetLastError ());
		MessageBox (NULL, szError, TEXT("Error"), MB_OK);
		closesocket(m_hRecvSocket);
		m_hRecvSocket=INVALID_SOCKET;
		return FALSE;
	}

	int nRecvTimeout=500;
	setsockopt(m_hRecvSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nRecvTimeout,sizeof(nRecvTimeout));

	m_sGroupIP=sGroupIP;
	m_nGroupPort=nGroupPort;
	m_fExit = false;
	m_pOnRecvData=pRecvFunc;
	m_UserData=UserData;

	m_fJoinGroup=TRUE;
	if(!m_hRecvThread)
	{
		m_fExit=FALSE;
		m_hRecvThread=AfxBeginThread(RecvThread,this)->m_hThread;
	}

	return TRUE;
}