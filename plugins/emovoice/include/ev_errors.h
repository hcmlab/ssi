/**
 *	@file	esmeralda_errors.h
 *	@author	Christian Plahl
 *	@date	Dez. 2004
 *
 *	@brief	definitions of error codes for dsp operations
 */

#ifndef __DSP_ERRORS_H_INCLUDED__
#define __DSP_ERRORS_H_INCLUDED__

/*
 * Error Codes
 */
#define DSP_SUCCESS					(0)		/* operation successful */

#define DSP_ERROR					(-1)	/* unspecific, general error */

#define DSP_ERROR_IO				(-100)	/* general IO-error */
#define DSP_ERROR_IO_FORMAT			(-101)	/* illegal data format */
#define DSP_ERROR_IO_EMPTY			(-102)	/* no data available */
#define DSP_ERROR_IO_TRUNCATED		(-103)	/* data truncated */
#define DSP_ERROR_IO_MISMATCH		(-104)	/* IO format and data do not match */
#define DSP_ERROR_IO_NOSUPPORT		(-105)	/* IO format not supported */
#define DSP_ERROR_IO_OUTOFRANGE		(-106)	/* data out of range for storage */

#define DSP_ERROR_DATA				(-200)	/* general errror in data format */
#define DSP_ERROR_DATA_EMPTY		(-201)	/* data stucture empty */
#define DSP_ERROR_DATA_NOSUPPORT	(-202)	/* operation not supported for data */
#define DSP_ERROR_DATA_SINGULAR		(-203)	/* data contain singularity */
#define DSP_ERROR_DATA_MISMATCH		(-204)	/* data don't match requirements */
#define DSP_ERROR_DATA_INVALID		(-205)	/* data isn't valid yet */

#define DSP_ERROR_ARG				(-300)	/* general error in argument(s) */
#define DSP_ERROR_ARG_ILLEGAL		(-301)	/* illegal parameter(s) */
#define DSP_ERROR_ARG_OUTOFRANGE	(-302)	/* parameter(s) out of range */

#define DSP_ERROR_OP				(-400)	/* general error for operation */
#define DSP_ERROR_OP_CANCELLED		(-401)	/* current operation not performed */

#endif /* __DSP_ERRORS_H_INCLUDED__ */
