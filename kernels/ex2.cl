
__kernel void add(
	__global float * data1,
	__global float * dataOut)
{
	const int i = get_global_id(0);

	dataOut[i] = data1[i];
}
