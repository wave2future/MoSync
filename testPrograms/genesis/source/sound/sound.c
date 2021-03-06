/***************************************************************************************
 *  Genesis Plus 1.2a
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  modified by Eke-Eke (compatibility fixes & additional code), GC/Wii port
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Sound Hardware
 ****************************************************************************************/

#include "shared.h"
#include "samplerate.h"

/* generic functions */
int  (*_YM2612_Write)(unsigned char adr, unsigned char data);
int  (*_YM2612_Read)(void);
void (*_YM2612_Update)(int **buf, int length);
int (*_YM2612_Reset)(void);

/* cycle-accurate samples */
static double m68cycles_per_sample[2];
static double z80cycles_per_sample[2];

/* libsamplerate buffers (max. is 488 cycles per line x 313 lines / 144) */
static SRC_DATA src_data;
static float src_in[1061*2];
static int src_buffer[2][1061];

/* YM2612 register arrays */
int fm_reg[2][0x100];

/* return the number of samples that should have been rendered so far */
static inline uint32 fm_sample_cnt(uint8 is_z80)
{
	if (is_z80) return (uint32) ((double)(count_z80 + current_z80 - z80_ICount) / z80cycles_per_sample[0]);
	else return (uint32) ((double) count_m68k / m68cycles_per_sample[0]);
}

static inline uint32 psg_sample_cnt(uint8 is_z80)
{
	if (is_z80) return (uint32) ((double)(count_z80 + current_z80 - z80_ICount) / z80cycles_per_sample[1]);
	else return (uint32) ((double) count_m68k / m68cycles_per_sample[1]);
}

/* update FM samples */
static inline void fm_update()
{
	if(snd.fm.curStage - snd.fm.lastStage > 0)
	{
		int *tempBuffer[2];
    
    if (config.hq_fm && !config.fm_core)
    {
      tempBuffer[0] = src_buffer[0] + snd.fm.lastStage;
		  tempBuffer[1] = src_buffer[1] + snd.fm.lastStage;
    }
    else
    {
      tempBuffer[0] = snd.fm.buffer[0] + snd.fm.lastStage;
		  tempBuffer[1] = snd.fm.buffer[1] + snd.fm.lastStage;
    }

		_YM2612_Update(tempBuffer, snd.fm.curStage - snd.fm.lastStage);
		snd.fm.lastStage = snd.fm.curStage;
	}
}

/* update PSG samples */
static inline void psg_update()
{
	if(snd.psg.curStage - snd.psg.lastStage > 0)
	{
		int16 *tempBuffer = snd.psg.buffer + snd.psg.lastStage;
		SN76489_Update (0, tempBuffer, snd.psg.curStage - snd.psg.lastStage);
		snd.psg.lastStage = snd.psg.curStage;
	}
}

void sound_init(int rate)
{
	double vclk = Master_Clock / 7.0;  /* 68000 and YM2612 clock */
	double zclk = Master_Clock / 15.0; /* Z80 and SN76489 clock  */

	/* cycle-accurate FM samples */
  if (config.hq_fm && !config.fm_core)
  {
    m68cycles_per_sample[0] = 144.0;
    z80cycles_per_sample[0] = (144.0 * 7.0) / 15.0;

    /* initialize samplerate converter data */
    src_data.data_in        = src_in;
    src_data.data_out       = snd.fm.src_out;
    src_data.input_frames   = (int)(((double)m68cycles_per_line * (double)lines_per_frame / 144.0) + 0.5);
    src_data.output_frames  = rate / vdp_rate;
    src_data.src_ratio      = (double)src_data.output_frames  / (double)src_data.input_frames;
  }
  else
  {
    m68cycles_per_sample[0] = ((double)m68cycles_per_line * (double)lines_per_frame) / (double) (rate / vdp_rate);
	  z80cycles_per_sample[0] = ((double)z80cycles_per_line * (double)lines_per_frame) / (double) (rate / vdp_rate);
  }

  /* cycle-accurate PSG samples */
  m68cycles_per_sample[1] = ((double)m68cycles_per_line * (double)lines_per_frame) / (double) (rate / vdp_rate);
  z80cycles_per_sample[1] = ((double)z80cycles_per_line * (double)lines_per_frame) / (double) (rate / vdp_rate);

	/* initialize sound chips */
	SN76489_Init(0, (int)zclk, rate);
	SN76489_Config(0, MUTE_ALLON, VOL_FULL, FB_SEGAVDP, SRW_SEGAVDP, 0);

	if (config.fm_core)
	{
		_YM2612_Write  = YM2612_Write;
		_YM2612_Read   = YM2612_Read;
		_YM2612_Update = YM2612_Update;
		_YM2612_Reset  = YM2612_Reset;
		YM2612_Init((int)vclk, rate, config.hq_fm);
	}
	else
	{
		_YM2612_Write  = YM2612Write;
		_YM2612_Read   = YM2612Read;
		_YM2612_Update = YM2612UpdateOne;
		_YM2612_Reset  = YM2612ResetChip;
		YM2612Init ((int)vclk, rate);
	}
} 

void sound_update(void)
{
	/* finalize sound buffers */
	snd.fm.curStage  = (config.hq_fm && !config.fm_core) ? src_data.input_frames : snd.buffer_size;
	snd.psg.curStage = snd.buffer_size;

	/* update last samples (if needed) */
	fm_update();
	psg_update();

  /* Resampling */
  if (config.hq_fm && !config.fm_core)
  {
    double scaled_value ;
    int len = src_data.input_frames;

	  /* this is basically libsamplerate "src_int_to_float_array" function, adapted to interlace samples */
    while (len)
	  {
      len -- ;
		  src_in[len*2]      = (float) (src_buffer[0] [len] / (8.0 * 0x10000000));
		  src_in[len*2 + 1]  = (float) (src_buffer[1] [len] / (8.0 * 0x10000000));
		}

    /* samplerate conversion */
    src_simple (&src_data, svp ? SRC_LINEAR : SRC_SINC_FASTEST, 2);

	  /* this is basically libsamplerate "src_float_to_int_array" function, adapted to interlace samples */
    len = snd.buffer_size;
    while (len)
    {
      len -- ;

      scaled_value = snd.fm.src_out[len*2] * (8.0 * 0x10000000);
      if (scaled_value >= (1.0 * 0x7FFFFFFF))
        snd.fm.buffer[0][len] = 0x7fffffff;
      else if (scaled_value <= (-8.0 * 0x10000000))
        snd.fm.buffer[0][len] = -1 - 0x7fffffff;
      else
        snd.fm.buffer[0][len] = (long)scaled_value;

      scaled_value = snd.fm.src_out[len*2+1] * (8.0 * 0x10000000);
      if (scaled_value >= (1.0 * 0x7FFFFFFF))
        snd.fm.buffer[1][len] = 0x7fffffff;
      else if (scaled_value <= (-8.0 * 0x10000000))
        snd.fm.buffer[1][len] = -1 - 0x7fffffff;
      else
        snd.fm.buffer[1][len] = (long)scaled_value;
      }
    }

  /* reset samples count */
	snd.fm.curStage   = 0;
	snd.fm.lastStage  = 0;
	snd.psg.curStage  = 0;
	snd.psg.lastStage = 0;
}

/* YM2612 control */
/* restore FM registers */
void fm_restore(void)
{
	int i;

	_YM2612_Reset();

	/* feed all the registers and update internal state */
	for(i = 0; i < 0x100; i++)
	{
		_YM2612_Write(0, i);
		_YM2612_Write(1, fm_reg[0][i]);
		_YM2612_Write(2, i);
		_YM2612_Write(3, fm_reg[1][i]);
	}
}

/* write FM chip */
void fm_write(unsigned int cpu, unsigned int address, unsigned int data)
{
  if (address & 1)
  {
    snd.fm.curStage = fm_sample_cnt(cpu);
	  fm_update();
  }
  _YM2612_Write(address & 3, data);
}

/* read FM status */
unsigned int fm_read(unsigned int cpu, unsigned int address)
{
	snd.fm.curStage = fm_sample_cnt(cpu);
	fm_update();
	return (_YM2612_Read() & 0xff);
}


/* PSG write */
void psg_write(unsigned int cpu, unsigned int data)
{
	snd.psg.curStage = psg_sample_cnt(cpu);
	psg_update();
	SN76489_Write(0, data);
}
