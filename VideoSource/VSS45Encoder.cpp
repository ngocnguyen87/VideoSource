#include "stdafx.h"
#include ".\vss45encoder.h"
#include "./Include/v45sdk/v4e_api.h"

#pragma comment(lib, "libv4codecsdk-w32i-avc-mt.lib")


static int cpu_num_processors( void )
{
	DWORD_PTR vProcessCPUs;
	DWORD_PTR vSystemCPUs;
	int result = 0;

	if (GetProcessAffinityMask (GetCurrentProcess (),
								&vProcessCPUs, &vSystemCPUs))
	{
		DWORD_PTR bit;
		int CPUs = 0;

		for (bit = 1; bit != 0; bit <<= 1)
		{
			if (vProcessCPUs & bit)
			{
				CPUs++;
			}
		}
		result = CPUs;
	}
	else
	{
		result = 1;
	}

	return result;
}

CVSS45Encoder* CVSS45Encoder::m_pInstance = NULL; 

CVSS45Encoder::CVSS45Encoder(void)
{
	m_bFastUpdate = FALSE;
	m_params = new v4e_settings_t;
}

CVSS45Encoder::~CVSS45Encoder(void)
{
	Exit();
	delete m_params;
}

CVSS45Encoder* CVSS45Encoder::getInstance()
{
	if(!m_pInstance)
		m_pInstance = new CVSS45Encoder();
	return m_pInstance;
}

void CVSS45Encoder::removeInstance()
{
	if(m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

int CVSS45Encoder::setting(LPVOID pParam)
{
	memcpy(&m_Setting,pParam,sizeof(m_Setting));
	memset(m_params,0,sizeof(v4e_settings_t));
	v4e_settings_t* params = (v4e_settings_t*)m_params;
	params->size = sizeof(v4e_settings_t);

	v4e_default_settings(params);

	switch(m_Setting.nProfile)
	{
	case VSS_BALINE_PROFILE:
		params->profile_idc = 66; // base line
		break;
	case VSS_HIGHT_PROFILE:
		params->profile_idc = 100; // high line
		break;
	case VSS_MAIN_PROFILE:
		params->profile_idc = 77; // main line
		break;
	default:
		params->profile_idc = 66; // base line
		break;
	}

	params->level_idc = m_Setting.nLevel;
	params->frame_width = m_Setting.nWidth;
	params->frame_height = m_Setting.nHeight;
	params->bit_depth_luma = 8;
	params->bit_depth_chroma = 8;

	switch(m_Setting.nEntropyMode)
	{
	case VSS_ENTROPY_CAVLC:
		params->sym_mode = 0; // CAVLC
		break;
	case VSS_ENTROPY_CABAC:
		params->sym_mode = 1; // CABAC
		break;
	default:
		params->sym_mode = 0; // CAVLC
		break;
	}

	params->interlace_mode = 0;
	params->constrained_intra_pred = 0;
	params->poc_type = 0;
	params->direct_mode = 0;
	params->chroma_qp_offset = 0;
	params->interlace_flags = 0;
	params->weighted_pred_flag = 0;
	params->sps_id = 0;
	params->pps_id = 0;
	params->enc_flags = 0;
	params->cpu_disable = 0;

	params->input.colorspace = COLORSPACE_E_IYUV; // 0 = I420; 1 = YV12
	params->input.width = m_Setting.nWidth;
	params->input.height = m_Setting.nHeight;
	params->input.sample_size = 1;
	params->input.significant_bits = 8;

	params->preproc.enable = 0; // disable preprocessing

	params->gop.bframes = 0;
	params->gop.idr_period = 1;
	params->gop.keyframes = m_Setting.nIdrPicture;
	params->gop.num_units = 1001;
	params->gop.time_scale = 2*m_Setting.nFrameRate;
	params->gop.emulate_b = 0;
	params->gop.min_intra_period = 4;
	params->gop.aud = 0;
	params->gop.flags = 0;

	params->rc.type = RATE_CONTROL_CBR_WITH_FILLER;
	params->rc.kbps = m_Setting.nBitrate;
	params->rc.auto_qp = 1;
	params->rc.qp_intra   = 30;	// quant parameter for I-frames (0-51);
	params->rc.qp_delta_p = 2;	// base qp delta between I and P (0-51);
	params->rc.qp_delta_b = 3;	// base qp delta between P and B (0-51);
	params->rc.qp_min     = 1;	// minimum allowed QP for rate control (1-51);
	params->rc.qp_max     = 51;	// maximum allowed QP for rate control (1-51);
	params->rc.scene_detect   = 50;	// scene change detection threshold (0-100);
	params->rc.vbv_length  = 0;	// rate control buffer length in msec; will be set to default depending on type if 0
	params->rc.qp_modulation = 1;	// enable QP variation between macroblocks (0/1);
	params->rc.mb_update = 1;	// enable mb-level rate-control (0/1);
	params->rc.look_ahead = 0;       // number of look-ahead frames (0-8) //Currently only 0 or 1 are used
	params->rc.max_kbps   = 0;       // max allowed bitrate in vbr mode; default - 0 (means not set)
	params->rc.initial_cpb_removal_delay = -1; // Initial fullness of CBR buffer in 1/90000 sec; default: -1 (means calculated as 90*vbv_length/2)
	params->rc.dual_pass_param = 128; // dual-pass behavior parameter /< 0 - 256; 0 - CBR-like; 256 - "fixed qp"-like 

	params->me.max_refs = 1;
	params->me.use_idr_long_term = 0;
	params->me.subdiv = ALL_SUBDIV_MODES;
	params->me.flags = 0;
	params->me.search_range = 64;

	params->speed.automatic = 0;
	params->speed.i = 3;
	params->speed.p = 3;
	params->speed.b = 3;

	params->slice.mode = 2;	 // bytes per slice 
	params->slice.param = 1000; // 1000 bytes
	params->slice.i_param = 0;
	params->slice.b_param = 0;

	params->deblock.flag     = 0;	// Configure loop filter (0=parameter below ingored, 1=parameters sent)
	params->deblock.disable  = 0;	// Disable loop filter in slice header (0=Filter, 1=No Filter)
	params->deblock.alpha_c0 = 0;	// Alpha & C0 offset div. 2, {-6, -5, ... 0, +1, .. +6}
	params->deblock.beta_c0  = 0;	// Beta offset div. 2, {-6, -5, ... 0, +1, .. +6}

	params->sei.pic_timing_flag = 0;	// Picture timing and buffering period SEIs control (0/1/2); 0 - disable; 1 - put all picture SEIs in one NAL unit; 2 - Put each SEI in separate NAL unit   
	params->sei.film_grain_flag = 0; //Calculate parameters and add film-grain SEI (0/1/2)
	params->sei.post_filter_flag = 0; //Calculate parameters and add postfiltering SEI (0/1/2)
	params->sei.rec_point_flag = 0; //add recovery point SEI (0/1/2)
	params->sei.sbs_frame_packing_flag = 0; //Add side-by-side or top-and-bottom frame packing arrangement SEI (0 -off 1,2 - side-by-side; 3,4 - top-and-bottom )

	//----------------------- Film-grain SEI settings
	params->sei.film_grain_mode = 0; // 0 - automatic; 4 -manual, 1,2,3 - reserved

	params->mt.disable = 0;
	params->mt.num_threads = 0; // automatic

	params->er.enable = 0;

	
	// number of layers 0..7
	// 0 means no SVC 
	// otherwice count layers counter
	params->svc.num_layers = 0;
	params->svc.key_picture_period = 0;	// SVC and AVC key picture period
	params->svc.temporal_mode = 0;	// temporal scalability: 0=disabled, 1=enabled;
	params->svc.multistream_mode = 0;	// 0=SVC, 1=AVC, 2=MVC

	// Bitwise svc/mvc/multistream flags:
	//	1=Put MVC prefix-nal units into stream;
	//	2=Put MVC picture delimiter into stream;
	//       4=Use fast version of ParallelStream (not implemented)  
	//       8=Generate MVC SEI according to Blu Ray spec
	params->svc.flags = 0;

	params->vui.aspect_ratio_info_present_flag = 0;
	params->vui.aspect_ratio_idc = 0;
	params->vui.sar_width = 0;
	params->vui.sar_height = 0;
	params->vui.overscan_info_present_flag = 0;
	params->vui.overscan_appropriate_flag = 0;
	params->vui.video_signal_type_present_flag = 0;
	params->vui.video_format = 0;
	params->vui.video_full_range_flag = 0;
	params->vui.colour_description_present_flag = 0;
	params->vui.colour_primaries = 0;
	params->vui.transfer_characteristics = 0;
	params->vui.matrix_coefficients = 0;
	params->vui.chroma_loc_info_present_flag = 0;
	params->vui.chroma_sample_loc_type_top_field = 0;
	params->vui.chroma_sample_loc_type_bottom_field = 0;
	params->vui.timing_info_present_flag = 0;
	params->vui.fixed_frame_rate_flag = 0;
	params->vui.nal_hrd_parameters_present_flag = 0;
	params->vui.vcl_hrd_parameters_present_flag = 0;
	params->vui.low_delay_hrd_flag = 0;
	params->vui.pic_struct_present_flag = 0;
	params->vui.bitstream_restriction_flag = 0;
	params->vui.motion_vectors_over_pic_boundaries_flag = 255;
	params->vui.max_bytes_per_pic_denom = 255;
	params->vui.max_bits_per_mb_denom = 255;
	params->vui.log2_max_mv_length_vertical = 255;
	params->vui.log2_max_mv_length_horizontal = 255;
	params->vui.num_reorder_frames = 255;
	params->vui.max_dec_frame_buffering = 255;

	return 0;
}

int CVSS45Encoder::getVideoSetting(LPVOID pParam)
{
	memcpy(pParam,&m_Setting,sizeof(m_Setting));
	return 0;
}

int CVSS45Encoder::Init()
{
	CCommonAutoLock lock(&keyEncoder);
	m_nExit = 0;
	v4e_settings_t* params = (v4e_settings_t*)m_params;
	int ret = v4e_open(&m_hEncoder, params);
	if(ret != VSSH_OK || m_hEncoder == NULL)
		return -1;

	return 0;
}

int CVSS45Encoder::Exit()
{
	CCommonAutoLock lock(&keyEncoder);
	m_nExit = 1;
	if(m_hEncoder)
	{
		v4e_close(m_hEncoder);
		m_hEncoder = NULL;
	}

	return 0;
}

int CVSS45Encoder::encode(BYTE* pData, DWORD dwSize, DWORD dwTS)
{
	CCommonAutoLock lock(&keyEncoder);
	v4e_settings_t* params = (v4e_settings_t*)m_params;
	if(!m_hEncoder)
		return -1;

	int ret;

	raw_frame_t input_frame;
	memset(&input_frame, 0, sizeof(input_frame));
	input_frame.image = pData;
	input_frame.stride = params->frame_width;
	input_frame.timestamp = dwTS * 90;
	
	input_frame.modifier = NULL;
	frame_modifier_t mod;
	if(m_bFastUpdate)
	{
		memset(&mod,0,sizeof(mod));
		mod.idr_flag = 1;
		mod.qp = -1;
		mod.slice_type = -1;
		m_bFastUpdate = FALSE;
		for (int i=0; i<MAX_SVC_LAYERS; i++)
        {
            mod.svc_layers_qp[i] = -1;
        }

		input_frame.modifier = &mod;
	}

	ret = v4e_set_frame(m_hEncoder, &input_frame);
	if(ret != VSSH_OK)
		return -1;

	while(1) 
	{
		media_sample_t *ms;
		ret = v4e_get_nal(m_hEncoder, &ms);
		if (ret != VSSH_OK) break;
		if(m_pEncodedCallback && (m_nExit == 0) && ms)
		{
			m_pEncodedCallback((unsigned char*)ms->data, ms->used_size, ms->timestamp, ms->is_last_in_pict >= LAST_IN_FRAME, m_pUser);
			v4_free_media_sample(ms);
		}
	}

	return 0;
}

int CVSS45Encoder::changeBitrate(int nNewBitrate)
{
	CCommonAutoLock lock(&keyEncoder);
	v4e_settings_t* params = (v4e_settings_t*)m_params;
	int ret = v4e_change_bitrate(m_hEncoder, nNewBitrate);
	if(ret == VSSH_OK)
	{
		params->rc.kbps = nNewBitrate;
		return 0;
	}

	return -1;
}

int CVSS45Encoder::fastUpdate()
{
	m_bFastUpdate = TRUE;
	return 0;
}