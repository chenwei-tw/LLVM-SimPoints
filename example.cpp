int main()
{
	int x, y, z;

	x = 12;
	y = x + 22; // load value of X that was just stored
	z = y + 33; // load value of Y that was just stored

	volatile int v;
	while (v < 50)
	{
		v++;
	}

	return 0;
}
