const sampler_t simple_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void modify(
	__read_only image2d_t image,
	__write_only image2d_t output,
	int W,
	int H) {

	const int i = get_global_id(0);
	const int j = get_global_id(1);

	if (i >= 0 && i < W && j >= 0 && j < H) {
		write_imagef(output, (int2)(i, j), read_imagef(image, simple_sampler, (int2)(i, j)));
	}

}
