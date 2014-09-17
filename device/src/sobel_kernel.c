__kernel void sobel_kern(
	uint n,
	uint line,
	__global char* gaa,
	__global char* gbb
	)
{
	int i,k;

	k = get_global_id(0);

	int n16 = n/16;
	int m16 = n%16;

	int ifirst = k*n16 + ((k>m16)? 0:k);
	int iend   = ifirst + n16 + ((k<m16)? 1:0);

	int tmp;

	char bb[iend-ifirst];

	// Use 12 tiles for each block. The image is 640x480, so 1/16th of this image is 19,200 pixels (30 lines).
	// This is split into 12, 1600 pixel (2.5 line) tiles. The data for each tile is moved into local memory.
	// The entire output is stored in local memory.
	int tile;
	for (tile=0;tile<12;tile++)
	{

		char aa[2882]; // 1600 pixels, plus 1 line before and 1 line after, plus 2 extra border pixels

		for(i=0;i<2882;i++)
			aa[i] = gaa[ifirst+tile*1600+i-line-1];

		for(i=line+1; i<2241; i++) {

			tmp = abs(-     aa[i-line-1]
			          - 2 * aa[i-1]
			          -     aa[i+line-1]
			          +     aa[i-line+1]
			          + 2 * aa[i+1]
			          +     aa[i+line+1])
			    + abs(      aa[i-line-1]
			          + 2 * aa[i-line]
			          +     aa[i-line+1]
			          -     aa[i+line-1]
			          - 2 * aa[i+line]
			          -     aa[i+line+1]);

			bb[tile*1600+i-line-1] = tmp > 255 ? 255 : tmp;
		}
	}

	for(i=ifirst; i<iend; i++)
		gbb[i] = bb[i-ifirst];
}

