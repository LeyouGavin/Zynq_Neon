#include "string.h"
#include "xtime_l.h"
#include "arm_neon.h"

XTime tStart[3] = { 0 };
XTime tEnd[3] = { 0 };
float fTime[3] = { 0 };

const float fB = 104.0f;
const float fG = 117.0f;
const float fR = 123.0f;
const float fScale = 0.5f;

void __attribute__((optimize("O3"))) fLimit(float* pfPix, float fMin, float fMax);
void __attribute__((optimize("O3"))) C_Gray2BGR(unsigned char* pucImgDataGray, unsigned char* pucImgDataBGR);
void __attribute__((optimize("O3"))) Neon_Gray2BGR(unsigned char* pucImgDataGray, unsigned char* pucImgDataBGR);
void __attribute__((optimize("O3"))) Neon_Gray2BGR2(unsigned char* pucImgDataGray, unsigned char* pucImgDataBGR);

unsigned char pucImgDataGray[300 * 300];
unsigned char pucImgDataBGR_C[300 * 300 * 3];
unsigned char pucImgDataBGR_Neon[300 * 300 * 3];
unsigned char pucImgDataBGR_Neon2[300 * 300 * 3];

int main(void)
{
	int i = 0;
	int iErr = 0;

	for(i = 0; i < sizeof(pucImgDataGray); ++i)
	{
		pucImgDataGray[i] = (unsigned char)i;
	}

	XTime_GetTime(&tStart[0]);
	C_Gray2BGR(pucImgDataGray, pucImgDataBGR_C);
	XTime_GetTime(&tEnd[0]);

	XTime_GetTime(&tStart[1]);
	Neon_Gray2BGR(pucImgDataGray, pucImgDataBGR_Neon);
	XTime_GetTime(&tEnd[1]);

	XTime_GetTime(&tStart[2]);
	Neon_Gray2BGR2(pucImgDataGray, pucImgDataBGR_Neon2);
	XTime_GetTime(&tEnd[2]);

	fTime[0] = 1000.0f * (tEnd[0] - tStart[0]) / COUNTS_PER_SECOND;
	fTime[1] = 1000.0f * (tEnd[1] - tStart[1]) / COUNTS_PER_SECOND;
	fTime[2] = 1000.0f * (tEnd[2] - tStart[2]) / COUNTS_PER_SECOND;

	for(i = 0; i < sizeof(pucImgDataBGR_C); ++i)
	{
		if(pucImgDataBGR_Neon[i] != pucImgDataBGR_C[i])
		{
			iErr = 1;
		}
		if(pucImgDataBGR_Neon2[i] != pucImgDataBGR_C[i])
		{
			iErr = 2;
		}
	}

	while(1);

	return iErr;
}


void fLimit(float* pfPix, float fMin, float fMax)
{
	float fVal = *pfPix;

	if(fVal < fMin)
	{
		fVal = fMin;
	}
	else if(fVal > fMax)
	{
		fVal = fMax;
	}
	else
	{}

	*pfPix = fVal;
}


void C_Gray2BGR(unsigned char* pucImgDataGray, unsigned char* pucImgDataBGR)
{
	int i = 0;
	float fVal = 0.0f;
	signed char scVal = 0;

	for(i = 0; i < 300 * 300; ++i)
	{
		fVal = (*(pucImgDataGray + i) - fB) * fScale;
		fLimit(&fVal, -128.0f, 127.0f);
		scVal = fVal;
		*(pucImgDataBGR + i * 3 + 0) = scVal;

		fVal = (*(pucImgDataGray + i) - fG) * fScale;
		fLimit(&fVal, -128.0f, 127.0f);
		scVal = fVal;
		*(pucImgDataBGR + i * 3 + 1) = scVal;

		fVal = (*(pucImgDataGray + i) - fR) * fScale;
		fLimit(&fVal, -128.0f, 127.0f);
		scVal = fVal;
		*(pucImgDataBGR + i * 3 + 2) = scVal;
	}
}


void Neon_Gray2BGR(unsigned char* pucImgDataGray, unsigned char* pucImgDataBGR)
{
	int i = 0;

	float32x4x3_t	neonfBGRMean;
	float32x4_t		neonfBGRScale;
	float32x4_t		neonfScMin;
	float32x4_t		neonfScMax;

	float32_t pfPix[16];
	float32x4x4_t neonfPix;
	float32x4x4_t neonfChannelBGR[3];
	int16x8_t neonsChannelBGR1;
	int16x8_t neonsChannelBGR2;
	uint8x16x3_t neonucChannelBGR;

	neonfBGRMean.val[0] = vdupq_n_f32(fB);
	neonfBGRMean.val[1] = vdupq_n_f32(fG);
	neonfBGRMean.val[2] = vdupq_n_f32(fR);

	neonfBGRScale = vdupq_n_f32(fScale);
	neonfScMin = vdupq_n_f32(-128.0f);
	neonfScMax = vdupq_n_f32(127.0f);

	for(i = 0; i < 300 * 300 / 16; ++i)
	{
		//依次存取16个像素
		pfPix[0] = (float)*(pucImgDataGray + i * 16 + 0);
		pfPix[1] = (float)*(pucImgDataGray + i * 16 + 1);
		pfPix[2] = (float)*(pucImgDataGray + i * 16 + 2);
		pfPix[3] = (float)*(pucImgDataGray + i * 16 + 3);
		pfPix[4] = (float)*(pucImgDataGray + i * 16 + 4);
		pfPix[5] = (float)*(pucImgDataGray + i * 16 + 5);
		pfPix[6] = (float)*(pucImgDataGray + i * 16 + 6);
		pfPix[7] = (float)*(pucImgDataGray + i * 16 + 7);
		pfPix[8] = (float)*(pucImgDataGray + i * 16 + 8);
		pfPix[9] = (float)*(pucImgDataGray + i * 16 + 9);
		pfPix[10] = (float)*(pucImgDataGray + i * 16 + 10);
		pfPix[11] = (float)*(pucImgDataGray + i * 16 + 11);
		pfPix[12] = (float)*(pucImgDataGray + i * 16 + 12);
		pfPix[13] = (float)*(pucImgDataGray + i * 16 + 13);
		pfPix[14] = (float)*(pucImgDataGray + i * 16 + 14);
		pfPix[15] = (float)*(pucImgDataGray + i * 16 + 15);
		memcpy(&neonfPix, &pfPix, 16 * 4);

		//B通道计算
		neonfChannelBGR[0].val[0] = vmulq_f32(vsubq_f32(neonfPix.val[0], neonfBGRMean.val[0]), neonfBGRScale);
		neonfChannelBGR[0].val[0] = vminq_f32(neonfChannelBGR[0].val[0], neonfScMax);
		neonfChannelBGR[0].val[0] = vmaxq_f32(neonfChannelBGR[0].val[0], neonfScMin);
		neonfChannelBGR[0].val[1] = vmulq_f32(vsubq_f32(neonfPix.val[1], neonfBGRMean.val[0]), neonfBGRScale);
		neonfChannelBGR[0].val[1] = vminq_f32(neonfChannelBGR[0].val[1], neonfScMax);
		neonfChannelBGR[0].val[1] = vmaxq_f32(neonfChannelBGR[0].val[1], neonfScMin);
		neonfChannelBGR[0].val[2] = vmulq_f32(vsubq_f32(neonfPix.val[2], neonfBGRMean.val[0]), neonfBGRScale);
		neonfChannelBGR[0].val[2] = vminq_f32(neonfChannelBGR[0].val[2], neonfScMax);
		neonfChannelBGR[0].val[2] = vmaxq_f32(neonfChannelBGR[0].val[2], neonfScMin);
		neonfChannelBGR[0].val[3] = vmulq_f32(vsubq_f32(neonfPix.val[3], neonfBGRMean.val[0]), neonfBGRScale);
		neonfChannelBGR[0].val[3] = vminq_f32(neonfChannelBGR[0].val[3], neonfScMax);
		neonfChannelBGR[0].val[3] = vmaxq_f32(neonfChannelBGR[0].val[3], neonfScMin);

		neonsChannelBGR1 = vcombine_s16(vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[0].val[0])), \
				vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[0].val[1])));

		neonsChannelBGR2 = vcombine_s16(vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[0].val[2])), \
						vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[0].val[3])));

		neonucChannelBGR.val[0] = vreinterpretq_u8_s8(vcombine_s8(vqmovn_s16(neonsChannelBGR1), vqmovn_s16(neonsChannelBGR2)));

		//G通道计算
		neonfChannelBGR[1].val[0] = vmulq_f32(vsubq_f32(neonfPix.val[0], neonfBGRMean.val[1]), neonfBGRScale);
		neonfChannelBGR[1].val[0] = vminq_f32(neonfChannelBGR[1].val[0], neonfScMax);
		neonfChannelBGR[1].val[0] = vmaxq_f32(neonfChannelBGR[1].val[0], neonfScMin);
		neonfChannelBGR[1].val[1] = vmulq_f32(vsubq_f32(neonfPix.val[1], neonfBGRMean.val[1]), neonfBGRScale);
		neonfChannelBGR[1].val[1] = vminq_f32(neonfChannelBGR[1].val[1], neonfScMax);
		neonfChannelBGR[1].val[1] = vmaxq_f32(neonfChannelBGR[1].val[1], neonfScMin);
		neonfChannelBGR[1].val[2] = vmulq_f32(vsubq_f32(neonfPix.val[2], neonfBGRMean.val[1]), neonfBGRScale);
		neonfChannelBGR[1].val[2] = vminq_f32(neonfChannelBGR[1].val[2], neonfScMax);
		neonfChannelBGR[1].val[2] = vmaxq_f32(neonfChannelBGR[1].val[2], neonfScMin);
		neonfChannelBGR[1].val[3] = vmulq_f32(vsubq_f32(neonfPix.val[3], neonfBGRMean.val[1]), neonfBGRScale);
		neonfChannelBGR[1].val[3] = vminq_f32(neonfChannelBGR[1].val[3], neonfScMax);
		neonfChannelBGR[1].val[3] = vmaxq_f32(neonfChannelBGR[1].val[3], neonfScMin);

		neonsChannelBGR1 = vcombine_s16(vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[1].val[0])), \
				vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[1].val[1])));

		neonsChannelBGR2 = vcombine_s16(vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[1].val[2])), \
						vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[1].val[3])));

		neonucChannelBGR.val[1] = vreinterpretq_u8_s8(vcombine_s8(vqmovn_s16(neonsChannelBGR1), vqmovn_s16(neonsChannelBGR2)));

		//G通道计算
		neonfChannelBGR[2].val[0] = vmulq_f32(vsubq_f32(neonfPix.val[0], neonfBGRMean.val[2]), neonfBGRScale);
		neonfChannelBGR[2].val[0] = vminq_f32(neonfChannelBGR[2].val[0], neonfScMax);
		neonfChannelBGR[2].val[0] = vmaxq_f32(neonfChannelBGR[2].val[0], neonfScMin);
		neonfChannelBGR[2].val[1] = vmulq_f32(vsubq_f32(neonfPix.val[1], neonfBGRMean.val[2]), neonfBGRScale);
		neonfChannelBGR[2].val[1] = vminq_f32(neonfChannelBGR[2].val[1], neonfScMax);
		neonfChannelBGR[2].val[1] = vmaxq_f32(neonfChannelBGR[2].val[1], neonfScMin);
		neonfChannelBGR[2].val[2] = vmulq_f32(vsubq_f32(neonfPix.val[2], neonfBGRMean.val[2]), neonfBGRScale);
		neonfChannelBGR[2].val[2] = vminq_f32(neonfChannelBGR[2].val[2], neonfScMax);
		neonfChannelBGR[2].val[2] = vmaxq_f32(neonfChannelBGR[2].val[2], neonfScMin);
		neonfChannelBGR[2].val[3] = vmulq_f32(vsubq_f32(neonfPix.val[3], neonfBGRMean.val[2]), neonfBGRScale);
		neonfChannelBGR[2].val[3] = vminq_f32(neonfChannelBGR[2].val[3], neonfScMax);
		neonfChannelBGR[2].val[3] = vmaxq_f32(neonfChannelBGR[2].val[3], neonfScMin);

		neonsChannelBGR1 = vcombine_s16(vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[2].val[0])), \
				vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[2].val[1])));

		neonsChannelBGR2 = vcombine_s16(vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[2].val[2])), \
						vqmovn_s32(vcvtq_s32_f32(neonfChannelBGR[2].val[3])));

		neonucChannelBGR.val[2] = vreinterpretq_u8_s8(vcombine_s8(vqmovn_s16(neonsChannelBGR1), vqmovn_s16(neonsChannelBGR2)));

		vst3q_u8(pucImgDataBGR + i * 16 * 3, neonucChannelBGR);
	}
}


void Neon_Gray2BGR2(unsigned char* pucImgDataGray, unsigned char* pucImgDataBGR)
{
	int i = 0;

	float32x4_t neonfScaleP;
	float32x4_t neonfScaleN;
	float32x4_t neonfBxS;
	float32x4_t neonfGxS;
	float32x4_t neonfRxS;

	float pfPix[16];
	float32x4x4_t neonfPix;
	float32x4x4_t neonfChannelBGR;
	int32x4x4_t neoniChannelBGR;
	int16x4x4_t neonsChannelBGR;
	int16x8x2_t neonsChannelBGR2;
	int8x8x2_t neonscChannelBGR2;
	int8x16x3_t neonscChannelBGR;

	neonfScaleP = vdupq_n_f32(fScale);
	neonfScaleN = vdupq_n_f32(-fScale);

	neonfBxS = vmulq_f32(vdupq_n_f32(fB), neonfScaleN);
	neonfGxS = vmulq_f32(vdupq_n_f32(fG), neonfScaleN);
	neonfRxS = vmulq_f32(vdupq_n_f32(fR), neonfScaleN);

	for(i = 0; i < 300 * 300 / 16; ++i)
	{
		//依次存取16个像素
		pfPix[0] = (float)*(pucImgDataGray + i * 16 + 0);
		pfPix[1] = (float)*(pucImgDataGray + i * 16 + 1);
		pfPix[2] = (float)*(pucImgDataGray + i * 16 + 2);
		pfPix[3] = (float)*(pucImgDataGray + i * 16 + 3);
		pfPix[4] = (float)*(pucImgDataGray + i * 16 + 4);
		pfPix[5] = (float)*(pucImgDataGray + i * 16 + 5);
		pfPix[6] = (float)*(pucImgDataGray + i * 16 + 6);
		pfPix[7] = (float)*(pucImgDataGray + i * 16 + 7);
		pfPix[8] = (float)*(pucImgDataGray + i * 16 + 8);
		pfPix[9] = (float)*(pucImgDataGray + i * 16 + 9);
		pfPix[10] = (float)*(pucImgDataGray + i * 16 + 10);
		pfPix[11] = (float)*(pucImgDataGray + i * 16 + 11);
		pfPix[12] = (float)*(pucImgDataGray + i * 16 + 12);
		pfPix[13] = (float)*(pucImgDataGray + i * 16 + 13);
		pfPix[14] = (float)*(pucImgDataGray + i * 16 + 14);
		pfPix[15] = (float)*(pucImgDataGray + i * 16 + 15);
		memcpy(&neonfPix, pfPix, sizeof(pfPix));

		//B通道
		neonfChannelBGR.val[0] = vmlaq_f32(neonfBxS, neonfPix.val[0], neonfScaleP);
		neoniChannelBGR.val[0] = vcvtq_s32_f32(neonfChannelBGR.val[0]);
		neonsChannelBGR.val[0] = vmovn_s32(neoniChannelBGR.val[0]);

		neonfChannelBGR.val[1] = vmlaq_f32(neonfBxS, neonfPix.val[1], neonfScaleP);
		neoniChannelBGR.val[1] = vcvtq_s32_f32(neonfChannelBGR.val[1]);
		neonsChannelBGR.val[1] = vmovn_s32(neoniChannelBGR.val[1]);

		neonfChannelBGR.val[2] = vmlaq_f32(neonfBxS, neonfPix.val[2], neonfScaleP);
		neoniChannelBGR.val[2] = vcvtq_s32_f32(neonfChannelBGR.val[2]);
		neonsChannelBGR.val[2] = vmovn_s32(neoniChannelBGR.val[2]);

		neonfChannelBGR.val[3] = vmlaq_f32(neonfBxS, neonfPix.val[3], neonfScaleP);
		neoniChannelBGR.val[3] = vcvtq_s32_f32(neonfChannelBGR.val[3]);
		neonsChannelBGR.val[3] = vmovn_s32(neoniChannelBGR.val[3]);

		neonsChannelBGR2.val[0] = vcombine_s16(neonsChannelBGR.val[0], neonsChannelBGR.val[1]);
		neonscChannelBGR2.val[0] = vqmovn_s16(neonsChannelBGR2.val[0]);

		neonsChannelBGR2.val[1] = vcombine_s16(neonsChannelBGR.val[2], neonsChannelBGR.val[3]);
		neonscChannelBGR2.val[1] = vqmovn_s16(neonsChannelBGR2.val[1]);

		neonscChannelBGR.val[0] = vcombine_s8(neonscChannelBGR2.val[0], neonscChannelBGR2.val[1]);

		//G通道
		neonfChannelBGR.val[0] = vmlaq_f32(neonfGxS, neonfPix.val[0], neonfScaleP);
		neoniChannelBGR.val[0] = vcvtq_s32_f32(neonfChannelBGR.val[0]);
		neonsChannelBGR.val[0] = vmovn_s32(neoniChannelBGR.val[0]);

		neonfChannelBGR.val[1] = vmlaq_f32(neonfGxS, neonfPix.val[1], neonfScaleP);
		neoniChannelBGR.val[1] = vcvtq_s32_f32(neonfChannelBGR.val[1]);
		neonsChannelBGR.val[1] = vmovn_s32(neoniChannelBGR.val[1]);

		neonfChannelBGR.val[2] = vmlaq_f32(neonfGxS, neonfPix.val[2], neonfScaleP);
		neoniChannelBGR.val[2] = vcvtq_s32_f32(neonfChannelBGR.val[2]);
		neonsChannelBGR.val[2] = vmovn_s32(neoniChannelBGR.val[2]);

		neonfChannelBGR.val[3] = vmlaq_f32(neonfGxS, neonfPix.val[3], neonfScaleP);
		neoniChannelBGR.val[3] = vcvtq_s32_f32(neonfChannelBGR.val[3]);
		neonsChannelBGR.val[3] = vmovn_s32(neoniChannelBGR.val[3]);

		neonsChannelBGR2.val[0] = vcombine_s16(neonsChannelBGR.val[0], neonsChannelBGR.val[1]);
		neonscChannelBGR2.val[0] = vqmovn_s16(neonsChannelBGR2.val[0]);

		neonsChannelBGR2.val[1] = vcombine_s16(neonsChannelBGR.val[2], neonsChannelBGR.val[3]);
		neonscChannelBGR2.val[1] = vqmovn_s16(neonsChannelBGR2.val[1]);

		neonscChannelBGR.val[1] = vcombine_s8(neonscChannelBGR2.val[0], neonscChannelBGR2.val[1]);

		//R通道
		neonfChannelBGR.val[0] = vmlaq_f32(neonfRxS, neonfPix.val[0], neonfScaleP);
		neoniChannelBGR.val[0] = vcvtq_s32_f32(neonfChannelBGR.val[0]);
		neonsChannelBGR.val[0] = vmovn_s32(neoniChannelBGR.val[0]);

		neonfChannelBGR.val[1] = vmlaq_f32(neonfRxS, neonfPix.val[1], neonfScaleP);
		neoniChannelBGR.val[1] = vcvtq_s32_f32(neonfChannelBGR.val[1]);
		neonsChannelBGR.val[1] = vmovn_s32(neoniChannelBGR.val[1]);

		neonfChannelBGR.val[2] = vmlaq_f32(neonfRxS, neonfPix.val[2], neonfScaleP);
		neoniChannelBGR.val[2] = vcvtq_s32_f32(neonfChannelBGR.val[2]);
		neonsChannelBGR.val[2] = vmovn_s32(neoniChannelBGR.val[2]);

		neonfChannelBGR.val[3] = vmlaq_f32(neonfRxS, neonfPix.val[3], neonfScaleP);
		neoniChannelBGR.val[3] = vcvtq_s32_f32(neonfChannelBGR.val[3]);
		neonsChannelBGR.val[3] = vmovn_s32(neoniChannelBGR.val[3]);

		neonsChannelBGR2.val[0] = vcombine_s16(neonsChannelBGR.val[0], neonsChannelBGR.val[1]);
		neonscChannelBGR2.val[0] = vqmovn_s16(neonsChannelBGR2.val[0]);

		neonsChannelBGR2.val[1] = vcombine_s16(neonsChannelBGR.val[2], neonsChannelBGR.val[3]);
		neonscChannelBGR2.val[1] = vqmovn_s16(neonsChannelBGR2.val[1]);

		neonscChannelBGR.val[2] = vcombine_s8(neonscChannelBGR2.val[0], neonscChannelBGR2.val[1]);

		vst3q_s8((signed char*)(pucImgDataBGR + i * 16 * 3), neonscChannelBGR);
	}
}
