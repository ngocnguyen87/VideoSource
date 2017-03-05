
/**
 * @file v4_substr_extract.h
 * Public SVC/MVC Sub-stream Extractor API
 *
 * For details see:
 *  ITU-T Recommendation H.264 (03/2009 or later)
 *   G.8.8 Specification of bitstream subsets (SVC)
 *   H.8.5 Specification of bitstream subsets (MVC)
 *
 * Project:	VSofts H.264 Codec V4
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

/**
* Create Sub-stream Extractor instance
*
* @param p_id - target priority id, [-1, 0 .. 63],  -1 means don't care;
* @param t_id - target temporal id, [-1, 0 .. 7],   -1 means don't care;
* @param dq_id- target combined dependency/quality id, dq_id = (d_id << 4) + q_id
*									[-1, 0 .. 127], -1 means don't care;
* @param mode - extraction mode, [0, 1]
*		0: Extract by specified p_id, t_id and dq_id;
*		1: Extract first n layers/views, where n = dq_id (not implemented);
*
* @return pointer to a structure created or NULL in case of out of memory;
*/
void *v4d_substream_extractor_create(int p_id, int t_id, int dq_id, int mode);

/**
* Release Sub-stream Extractor instance
* @param handle - Sub-stream Extractor context pointer;
*/
void v4d_substream_extractor_close(void *handle);

/**
* Put the next NAL unit to the inner buffer. Pass NULL to indicate end of stream.
* @param handle - Sub-stream Extractor context pointer;
* @param ms - input NAL unit (or NULL);
* @return 0=sample accepted, 1=sample cannot be loaded;
*/
int v4d_substream_extractor_feed_nalu(void *handle, media_sample_t *ms);

/**
* Get the next NAL unit from the buffer
* @param handle - Sub-stream Extractor context pointer;
* @return NAL unit or NULL if no NAL is available;
*/
media_sample_t *v4d_substream_extractor_get_nalu(void *handle);
