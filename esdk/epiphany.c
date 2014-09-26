/*
Copyright (c) 2014, Steven Clukey
Copyright (c) 2014, Shodruky Rhyammer
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "e_lib.h"
#include "common.h"

#define BUF_ADDRESS 0x8f000000
#define BUF_IN_ADDRESS 0x2000
#define BUF_OUT_ADDRESS 0x4000

typedef union
{
	uint8_t color_parts[4];
	uint32_t color;
} fb_color;

int main(void)
{
	msg_block_t msg;
	msg_block_t *shared_msg = (msg_block_t *)(BUF_ADDRESS);
	e_coreid_t coreid = e_get_coreid();
	unsigned int row, col, core, cores;
	e_coords_from_coreid(coreid, &row, &col);
	core = row * e_group_config.group_cols + col;
	cores = e_group_config.group_rows * e_group_config.group_cols;
	unsigned int frame = 1;
	//uint8_t new_color[4];
	//new_color[3] = 0xff;

	fb_color new_color;
	new_color.color_parts[3] = 0xff;

	// to fix timing bug
	e_wait(E_CTIMER_0, 0x10000000);
	// get init data
	e_dma_copy(&msg.msg_init, &shared_msg->msg_init, sizeof(msg_init_t));
	unsigned int line_bytes = msg.msg_init.width * msg.msg_init.pixel_bytes;
	unsigned int next_line = line_bytes * cores;
	unsigned int fb_next_line = msg.msg_init.line_length * (cores - 1) * msg.msg_init.out_scale;
	uint8_t *depth_start_address = (uint8_t *)BUF_ADDRESS + msg.msg_init.start_offset + core * line_bytes;
	// clear local framebuffer
	uint64_t *clear = (uint64_t *)BUF_OUT_ADDRESS;
	unsigned int i;
	for (i = 0; i < (msg.msg_init.line_length >> 3); i++)
	{
		*clear = 0;
		clear++;
	}

	uint8_t tmpO;
	int tmp;
	unsigned int x, y;

	while (1)
	{
		msg.msg_d2h[core].coreid = coreid;
		msg.msg_d2h[core].value[0] = frame;
		uint8_t *fb_address = (uint8_t *)msg.msg_init.smem_start + core * msg.msg_init.line_length * msg.msg_init.out_scale + msg.msg_init.out_xoff * 4 + msg.msg_init.out_yoff * msg.msg_init.line_length;
		size_t fb_length = msg.msg_init.width * msg.msg_init.out_scale * 4;

		uint8_t *src_address = depth_start_address - line_bytes - 1;
		for (y = 0; y < msg.msg_init.height; y += cores)
		{
			// get depth data
			e_dma_copy((uint8_t *)BUF_IN_ADDRESS, src_address, line_bytes*3+2);
			uint8_t *depth = (uint8_t *)(BUF_IN_ADDRESS + line_bytes+1);
			uint32_t *pix = (uint32_t *)BUF_OUT_ADDRESS;
			// render
			for (x = 0; x < msg.msg_init.width; x++)
			{
				tmp = abs(      *(depth - line_bytes+1)
				          + 2 * *(depth + 1)
				          +     *(depth + line_bytes+1)
				          -     *(depth - line_bytes-1)
				          - 2 * *(depth - 1)
				          -     *(depth + line_bytes-1))
				    + abs(      *(depth - line_bytes-1)
				          + 2 * *(depth - line_bytes)
				          +     *(depth - line_bytes+1)
				          -     *(depth + line_bytes-1)
				          - 2 * *(depth + line_bytes)
				          -     *(depth + line_bytes+1));

				tmpO = tmp > 255 ? 255 : tmp;

				for (i = 0; i < msg.msg_init.out_scale; i++)
				{
					new_color.color_parts[0] = tmpO;
					new_color.color_parts[1] = tmpO;
					new_color.color_parts[2] = tmpO;
					*(pix + i) = new_color.color;
				}
				pix += msg.msg_init.out_scale;
				depth++;
			}
			// copy to framebuffer
			for (i = 0; i < msg.msg_init.out_scale; i++)
			{
				e_dma_copy(fb_address, (uint8_t *)BUF_OUT_ADDRESS, fb_length);
				fb_address += msg.msg_init.line_length;
			}
			src_address += next_line;
			fb_address += fb_next_line;
		}

		// notify completion
		e_dma_copy(&shared_msg->msg_d2h[core], &msg.msg_d2h[core], sizeof(msg_dev2host_t));

		frame++;
		// sleep
		__asm__ __volatile__ ("trap 4");
	}

	return 0;
}
