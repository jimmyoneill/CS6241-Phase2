int main(int argc, char const *argv[])
{
	unsigned int i = 0;
	int a[10];
	int x = 0;
	for(i = 0; i < 10; i++)
	{
		a[i] = 10 + i;
		x += a[i];
	}
	return 0;
}
