void allocateAssignSingleDim()
{
	int a1[10];
	a1[1] = 5;
}

void allocateAssignMultiDim()
{
	int a2[10][15];
	a2[2][3] = 5;
}

void swap()
{
	int a4[100],a5[100]; 
	int i, temp;
	for(i = 0; i < 100; i++)
	{
		temp = a4[i];
		a4[i] = a5[i];
		a4[i] = temp;
	}
}
