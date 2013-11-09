// Defining types for the expected input arguments
// changed intptr_t to int instead of unsigned int and sum2 from us long to us int
typedef unsigned int intptr_t;
typedef unsigned char pixel;
typedef unsigned short sum_t;
typedef unsigned long sum2_t;

// Define an inputarray that points to the start of the data memory
pixel datamem[128];

// BIT_DEPTH is 8 always, so if else statement is removed
#define BIT_DEPTH 8

// #define BITS_PER_SUM (8 * sizeof(sum_t))
#define BITS_PER_SUM (8 * sizeof(sum_t))

// HADAMARD4 function exported from the original pixel.c file
#define HADAMARD4(d0, d1, d2, d3, s0, s1, s2, s3) {\
    sum2_t t0 = s0 + s1;\
    sum2_t t1 = s0 - s1;\
    sum2_t t2 = s2 + s3;\
    sum2_t t3 = s2 - s3;\
    d0 = t0 + t2;\
    d2 = t0 - t2;\
    d1 = t1 + t3;\
    d3 = t1 - t3;\
}

// abs function exported from the original pixel.c file
static sum2_t abs2(sum2_t a);

// x264_pixel_satd_8x4 exported from the original pixel.c file
int x264_pixel_satd_8x4();

// x264_pixel_satd_4x4 exported from the original pixel.c file
int x264_pixel_satd_4x4();

char main()
{
 	// run the x264_pixel_satd_8x4 by passing on the pixel values. 
 	int result, i;
 	result = 0;
 	i = 0;
	
	// calculate the result
	if (datamem[0x40] == 0x44)
	{
		result = x264_pixel_satd_4x4();
	}
	else if (datamem[0x40] == 0x84)
	{
		result = x264_pixel_satd_8x4();
	}
	else result = 0xdead;
	
	
	// clean result array
	for (i = 0; i < 4; i++)
	{
		datamem[i] = 0x00;
	}
	
	// write result in reverse endianess
	datamem[0x00] = (result >> 24) & 0xFF;
	datamem[0x01] = (result >> 16) & 0xFF;
	datamem[0x02] = (result >> 8) & 0xFF;
	datamem[0x03] = result & 0xFF;
	
	// DEBUG
	// datamem[0x40] = 0xfe;
	// datamem[0x41] = 0xed;
	
	return 0;
}

static sum2_t abs2(sum2_t a)
{
	sum2_t s = ((a>>(BITS_PER_SUM-1))&(((sum2_t)1<<BITS_PER_SUM)+1))*((sum_t)-1);
	return (a+s)^s;
}

int x264_pixel_satd_8x4()
{	
    	sum2_t tmp[4][4];
    	sum2_t a0, a1, a2, a3;
	sum2_t sum = 0;
	int i = 0;
	
	// Strides are set values and do not have to be given as input arguments
	// Stride is 16 to compute 2 rows of 8 bytes of pixel array elements
	intptr_t stride = 16;

	// Pointer to data memory
	pixel* datapoint = datamem;

	// Adjust the for loop so it can fetch data from data memory
    for(i = 0; i < 4; i++, datapoint += stride)
    {
        a0 = (datapoint[0] - datapoint[8]) + ((sum2_t)(datapoint[4] - datapoint[12]) << BITS_PER_SUM);
        a1 = (datapoint[1] - datapoint[9]) + ((sum2_t)(datapoint[5] - datapoint[13]) << BITS_PER_SUM);
        a2 = (datapoint[2] - datapoint[10]) + ((sum2_t)(datapoint[6] - datapoint[14]) << BITS_PER_SUM);
        a3 = (datapoint[3] - datapoint[11]) + ((sum2_t)(datapoint[7] - datapoint[15]) << BITS_PER_SUM);
        HADAMARD4( tmp[i][0], tmp[i][1], tmp[i][2], tmp[i][3], a0,a1,a2,a3 );
    }
    for(i = 0; i < 4; i++ )
    {
        HADAMARD4( a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i] );
        sum += abs2(a0) + abs2(a1) + abs2(a2) + abs2(a3);
    }
    return (((sum_t)sum) + (sum>>BITS_PER_SUM)) >> 1;;
}

int x264_pixel_satd_4x4()
{
    sum2_t tmp[4][2];
    sum2_t a0, a1, a2, a3, b0, b1;
    sum2_t sum = 0;
    int i = 0;
    
    	// Strides are set values and do not have to be given as input arguments
	// Stride is 8 to compute 2 rows of 4 bytes of pixel array elements
	intptr_t stride = 8;
    
	// Pointer to data memory
	pixel* datapoint = datamem;
    
    // instead of 8x4 method, it writes per 4 bytes
    for( i = 0; i < 4; i++, datapoint +=stride )
    {
        a0 = datapoint[0] - datapoint[4];
        a1 = datapoint[1] - datapoint[5];
        b0 = (a0+a1) + ((a0-a1)<<BITS_PER_SUM);
        a2 = datapoint[2] - datapoint[6];
        a3 = datapoint[3] - datapoint[7];
        b1 = (a2+a3) + ((a2-a3)<<BITS_PER_SUM);
        tmp[i][0] = b0 + b1;
        tmp[i][1] = b0 - b1;
    }
    for( i = 0; i < 2; i++ )
    {
        HADAMARD4( a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i] );
        a0 = abs2(a0) + abs2(a1) + abs2(a2) + abs2(a3);
        sum += ((sum_t)a0) + (a0>>BITS_PER_SUM);
    }
    return sum >> 1;
}
