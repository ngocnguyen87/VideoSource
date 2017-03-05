// RtpTransmitter.cpp: implementation of the CRtpTransmitter class.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "RtpTransmitter.h"

//-----------------------------------------------------------------------------
// Construction/Destruction
//-----------------------------------------------------------------------------

RtpTransmitter::RtpTransmitter():
	m_dwIndex(0xFFFF),
	m_fSentLast(false),
	m_SeqNumber(0),
	m_dwPTS(0)
{
	m_pDataBuffer = new BYTE[RTP_HEADER_SIZE + RAW_DATA_SIZE];
}

RtpTransmitter::~RtpTransmitter()
{
	while (m_NetworkTargets.GetSize())
	{
		delete m_NetworkTargets.GetAt(0);
		m_NetworkTargets.RemoveAt(0);
	}
	delete[] m_pDataBuffer;
}

int RtpTransmitter::Packet(LPBYTE data, DWORD size, DWORD pts, BOOL marker, LPBYTE pDataOut, int &nSizeOut)
{
	//lamm thu ham nay de test
	m_dwPTS = pts;
	m_RtpHeader.SetTimeStamp(m_dwPTS);
	m_RtpHeader.SetPadding(FALSE);
	DWORD sent_size;

	m_dwIndex = RTP_HEADER_SIZE;
	sent_size = min(size, 2048);
	m_RtpHeader.SetPadding(FALSE);

	memcpy(m_pDataBuffer + m_dwIndex, data, sent_size);
	
	m_dwIndex += sent_size;

	m_RtpHeader.SetMarker(marker);
	m_RtpHeader.SetSequenceNumber(++m_SeqNumber);
	m_RtpHeader.FillBuffer(m_pDataBuffer);

	memcpy(pDataOut, m_pDataBuffer, m_dwIndex);
	nSizeOut = m_dwIndex;

	
	return 0;
}

int RtpTransmitter::PacketData(LPBYTE data, DWORD size, DWORD pts, DWORD seq)
{
	m_dwPTS = pts;
	m_RtpHeader.SetTimeStamp(m_dwPTS);
	DWORD sent_size;
	BOOL first=TRUE;
	int result = 0; // turn off warning
	while(size)
	{
		m_dwIndex = RTP_HEADER_SIZE;
		sent_size = min(size, 1408); // must be multiple of 16 for AES 128
		m_RtpHeader.SetMarker(FALSE);
		if(seq != -1)
			m_RtpHeader.SetSequenceNumber(seq);
		else
			m_RtpHeader.SetSequenceNumber(m_SeqNumber++);

		memcpy(m_pDataBuffer + m_dwIndex, data, sent_size);
		size -= sent_size;
		data += sent_size;

		m_RtpHeader.SetPadding(FALSE);

		m_dwIndex += sent_size;

		m_RtpHeader.FillBuffer(m_pDataBuffer);

		//TRACE("Gui di RTP co Sequence = %d \n", m_SeqNumber);
		for (int i=0; i < m_NetworkTargets.GetSize(); i++)
		{
			result = m_Sender.SendTo(m_pDataBuffer, m_dwIndex,
					m_NetworkTargets[i]->ip,
					m_NetworkTargets[i]->port);
		}
	}
	
	return result;
}

int RtpTransmitter::PacketData(LPBYTE data, DWORD size, DWORD pts, unsigned char* pKey, int keyLen, int maker)
{
	m_dwPTS = pts;
	m_RtpHeader.SetTimeStamp(m_dwPTS);
	DWORD sent_size;
	BOOL first=TRUE;
	int result = 0; // turn off warning
	while(size)
	{
		m_dwIndex = RTP_HEADER_SIZE;
		sent_size = min(size, 1408); // must be multiple of 16 for AES 128
		m_RtpHeader.SetMarker(maker);
		m_RtpHeader.SetSequenceNumber(m_SeqNumber++);

		memcpy(m_pDataBuffer + m_dwIndex, data, sent_size);
		size -= sent_size;
		data += sent_size;

		//if(pKey)
		//{
		//	int byte_padding = 0;
		//	//Khoi tao IV
		//	if(m_bIVInit)
		//	{
		//		unsigned short sequence = htons((u_short)m_RtpHeader.GetSequenceNumber());
		//		unsigned long timeStamp = htonl(m_RtpHeader.GetTimeStamp());
		//			
		//		int i = 0;
		//		while(i<18)
		//		{
		//			memcpy(&m_iv[i], &sequence, sizeof(sequence));
		//			i += sizeof(sequence);
		//				
		//			memcpy(&m_iv[i], &timeStamp, sizeof(timeStamp));
		//			i += sizeof(timeStamp);
		//		}
		//	}
		//	else
		//	{
		//		//IV ban dau cu bang 0 da.
		//		m_bIVInit = TRUE;
		//		memset(m_iv,0,sizeof(m_iv));
		//	}

		//	
		//	BYTE* buffer = m_pDataBuffer + m_dwIndex;

		//	byte_padding = 16 - (sent_size % 16);

		//	if(byte_padding == 16)
		//	{
		//		byte_padding = 0;
		//	}
		//	else
		//	{
		//		memset(buffer + sent_size, 0, byte_padding);
		//		sent_size += byte_padding;
		//		buffer[sent_size - 1] = byte_padding;
		//	}

		//	m_RtpHeader.SetPadding(byte_padding != 0);
		//	//Set Key ma hoa
		//	aes_setkey_enc(&m_aes, pKey, 128);
		//	aes_crypt_cbc(&m_aes, AES_ENCRYPT, sent_size, m_iv, (unsigned char*)buffer, (unsigned char*)buffer);
		//}
		
		{
			m_RtpHeader.SetPadding(FALSE);
		}

		m_dwIndex += sent_size;

		m_RtpHeader.FillBuffer(m_pDataBuffer);

		//TRACE("Gui di RTP co Sequence = %d \n", m_SeqNumber);
		for (int i=0; i < m_NetworkTargets.GetSize(); i++)
		{
			result = m_Sender.SendTo(m_pDataBuffer, m_dwIndex,
					m_NetworkTargets[i]->ip,
					m_NetworkTargets[i]->port);
		}
	}
	
	return result;
}

void RtpTransmitter::ForwardData(LPBYTE data, DWORD size)
{
	for (int i=0; i < m_NetworkTargets.GetSize(); i++)
	{
		m_Sender.SendTo(data, size, m_NetworkTargets[i]->ip,
			m_NetworkTargets[i]->port);
	}
}

int RtpTransmitter::AddSendAddr(int ip, int port)
{
	for (int i=0; i < m_NetworkTargets.GetSize(); i++)
	{
		if (m_NetworkTargets[i]->ip == ip && m_NetworkTargets[i]->port == port)
			return 0;
	}

	PNETWORK_TARGET pTarget = new NETWORK_TARGET;
	pTarget->ip = ip;
	pTarget->port = port;
	m_NetworkTargets.Add(pTarget);

	TRACE("Add new address: %x, port : %d\n ", ip, port);
	return 0;
}

BOOL RtpTransmitter::RemoveSendAddr(int ip, int port)
{
	for (int i=0; i<m_NetworkTargets.GetSize(); i++)
	{
		if ((m_NetworkTargets[i]->ip == ip) &&
			(m_NetworkTargets[i]->port == port))
		{
			delete m_NetworkTargets[i];
			m_NetworkTargets[i] = NULL;
			m_NetworkTargets.RemoveAt(i);			
			break;
		}
	}

	TRACE("remove address: %x, port: %d==============================\n ", ip, port);

	return TRUE;
}

void RtpTransmitter::RemoveAllAddr()
{
	while (m_NetworkTargets.GetSize()>0)
	{
		delete m_NetworkTargets[0];
		m_NetworkTargets.RemoveAt(0);
	}
	
	TRACE("remove all address=============================================\n");
}

void RtpTransmitter::SetVideoParam(int width, int height)
{
	memcpy(m_VideoHeader, &width, sizeof(int));
	memcpy(m_VideoHeader + sizeof(int), &height, sizeof(int));
}
