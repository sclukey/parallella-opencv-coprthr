__kernel void sobel_kern(
	uint n,
	uint line,
	__global char* aa,
	__global char* bb
	)
{
	int i,k;

	k = get_global_id(0);

	int n16 = n/16;
	int m16 = n%16;

	int ifirst = k*n16 + ((k>m16)? 0:k);
	int iend   = ifirst + n16 + ((k<m16)? 1:0);

	int tmp;

	for(i=ifirst; i<iend; i++) {
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
		bb[i] = tmp > 255 ? 255 : tmp;
	}
}

