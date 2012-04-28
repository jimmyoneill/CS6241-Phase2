void spin()
{
	int a[100];
	int i;

	for(i = 0; i < 100; i++)
	{
		a[i] = i+1;
	}
}

void spinTwoDim()
{
	int a[100][100];
	int i,j;

	for(i = 0; i < 100; i++)
	{
		for(j = 0; j < 100; j++)
		{
			a[i][j] = i+j+1;
		}
	}
}