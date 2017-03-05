// RtpTransmitter.h: interface for the CRtpTransmitter class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <afxtempl.h>
#include "UdpSocket_Pure.h"

enum RTPPayloadType {
    RTP_PT_ULAW = 0,
    RTP_PT_GSM = 3,
    RTP_PT_G723 = 4,
    RTP_PT_ALAW = 8,
    RTP_PT_S16BE_STEREO = 10,
    RTP_PT_S16BE_MONO = 11,
    RTP_PT_MPEGAUDIO = 14,
    RTP_PT_JPEG = 26,
    RTP_PT_H261 = 31,
    RTP_PT_MPEGVIDEO = 32,
    RTP_PT_MPEG2TS = 33,
    RTP_PT_H263 = 34, /* old H263 encapsulation */
    RTP_PT_PRIVATE = 96,
};

// Mang cac rtp type tuy theo codec
static int RtpTypeArray[] = 
{
//	NONE	H264	H264FD1	MP4		MP4FD1	H261	H263P	H263I
	0,		127,	127,	33,		33,		31,		34,		34,
//	G711A	G722	G723.1	G728	G729	AAC_MP4
	8,		9,		4,		15,		18,		0
};
// Sau khi modify RtpTypeArray can phai thay doi gia tri define sau day.
// So thanh phan Video codec. Chu y ko cong 1

#define VIDEO_HEADER_SIZE	8

typedef struct _NETWORK_TARGET
{
    int		ip;
    int		port;
} NETWORK_TARGET, *PNETWORK_TARGET;

class RtpTransmitter
{
	class RtpHeader
	{
	public:
		RtpHeader():
		  m_SSrcIdentifier(1234567)
		{
			RtpHdrStart.AsStruct.m_nVersion = 2;
			RtpHdrStart.AsStruct.m_fExtension = 0;
			RtpHdrStart.AsStruct.m_fPadding = 0;
			RtpHdrStart.AsStruct.m_nPayloadType = 96;
			RtpHdrStart.AsStruct.m_fMarker = 0;
			RtpHdrStart.AsStruct.m_nCSrcCount = 0;
		}

		void SetSequenceNumber(int nSequenceNumber)
		{
			RtpHdrStart.AsStruct.m_nSequenceNumPart1 = (nSequenceNumber >> 8) & 0xFF;
			RtpHdrStart.AsStruct.m_nSequenceNumPart2 = nSequenceNumber & 0xFF;
		}

		int GetSequenceNumber()
		{
			int out = (RtpHdrStart.AsStruct.m_nSequenceNumPart1 & 0xFF) << 8;

			out |= RtpHdrStart.AsStruct.m_nSequenceNumPart2 & 0xFF;

			return out;
		}

		void SetMarker(bool fMarker){ RtpHdrStart.AsStruct.m_fMarker = fMarker;}
		void SetTimeStamp(DWORD TimeStamp){ m_TimeStamp = TimeStamp;}
		DWORD GetTimeStamp() 
		{
			return m_TimeStamp;
		}

		void SetPadding(BOOL fPadding)
		{
			RtpHdrStart.AsStruct.m_fPadding = fPadding;
		}
		void SetPayloadType(int pt)
		{
			RtpHdrStart.AsStruct.m_nPayloadType = pt;
		}
		void SetSourceId(DWORD SourceId)
		{
			m_SSrcIdentifier=SourceId;
		}
		void FillBuffer(void* Buf)
		{
			DWORD* TempBuf = (DWORD*)Buf;
			TempBuf[0] = RtpHdrStart.AsDWORD;
			TempBuf[1] = htonl(m_TimeStamp );
			TempBuf[2] = htonl(m_SSrcIdentifier);
		}

	private:
		union
		{
			struct
			{
				DWORD	m_nCSrcCount		: 4; // CC
				DWORD	m_fExtension		: 1; // X
				DWORD	m_fPadding			: 1; // P
				DWORD	m_nVersion			: 2; // V
				DWORD	m_nPayloadType		: 7; // PT
				DWORD	m_fMarker			: 1; // M
				DWORD	m_nSequenceNumPart1	: 8;
				DWORD	m_nSequenceNumPart2	: 8;
			} AsStruct;

			DWORD AsDWORD;
		}
		RtpHdrStart;

		DWORD	m_TimeStamp;
		DWORD	m_SSrcIdentifier;		// SSRC
	};

public:
	RtpTransmitter();
	~RtpTransmitter();

	enum
	{
		RTP_HEADER_SIZE = 12,
		RAW_DATA_SIZE	= 64*1024
	};

	void SetPayloadType(int pt) { m_RtpHeader.SetPayloadType(pt); };

	// HungNV added for AES 128 at 2009-05-08 - 17:02:47
	//int PacketData(LPBYTE data, DWORD size, DWORD pts);
	int PacketData(LPBYTE data, DWORD size, DWORD pts, DWORD seq = -1);
	int PacketData(LPBYTE data, DWORD size, DWORD pts, unsigned char* pKey, int keyLen, int maker);
	int Packet(LPBYTE data, DWORD size, DWORD pts, BOOL marker, LPBYTE pDataOut, int &nSizeOut);
	void ForwardData(LPBYTE data, DWORD size);
	void SetVideoParam(int width, int height);

	int AddSendAddr(int ip, int port);
	BOOL RemoveSendAddr(int ip,int port);
	void RemoveAllAddr();
	
public:
	LPBYTE	m_pDataBuffer;

	DWORD	m_dwIndex;
	DWORD	m_dwPTS;
	bool	m_fSentLast;
	DWORD	m_SeqNumber;

	CUdpSocket	m_Sender;

	RtpHeader		m_RtpHeader;

	BYTE			m_VideoHeader[VIDEO_HEADER_SIZE];
	BOOL			m_fCopyVideoHeader;

	CTypedPtrArray<CPtrArray,PNETWORK_TARGET>	m_NetworkTargets;
};
