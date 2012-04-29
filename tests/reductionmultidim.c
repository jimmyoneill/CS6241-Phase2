int main(int argc, char const *argv[])
{
	int a[10][5];
	int x = 0;

	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 5; j++)
		{
			a[i][j] = i+j;
			x += a[i][j];
		}
	}
	return 0;
}