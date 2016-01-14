
// map the value in from the range in_low-in_high to the range out_low-out_high
long map(long in, 
	 long in_low, long in_high,
	 long out_low, long out_high)
{
	long in_range = in_high - in_low;
	long out_range = out_high - out_low;
	long in_weight = in - in_low;
	long out_weight = (in_weight * out_range) / in_range;

	return out_weight + out_low;
}

