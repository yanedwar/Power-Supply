#ifndef INC_ANALOGSYSTEMCONSTANTS_H_
#define INC_ANALOGSYSTEMCONSTANTS_H_


#define ADC_BUF_LEN			2048
#define ADC_REF 			3.3f
#define ADC_RES 			4096
#define ADC_VOUT_VDIV 		(12.4f/2.4f)

#define DAC_OUT_MAX 		4095
#define DAC_OUT_MIN 		0

#define KP 					52.0f
#define KI 					52.0f
#define KD 					0.0f

#define TIMESCALE 			1.0f

#define ADC_VOUT_CAL_SLOPE 		1.0034f
#define ADC_VOUT_CAL_OFFSET 	-0.2872f

#define ADC_ISENSE_CAL_SLOPE	0.00109996f
#define ADC_ISENSE_CAL_OFFSET 	-0.33443958f

#define DAC_CAL_SLOPE			-249.8845539f
#define DAC_CAL_OFFSET			4000.94725f

//#define DISABLE_CURRENT_LIMIT	1
//#define USE_FILTERED_ISENSE		1

#endif
