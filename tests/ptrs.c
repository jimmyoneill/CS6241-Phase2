void easyPtrs()
{
	int ep[10];
	int *p;
	p = ep;
	ep[0] = -1;
	p[9] = 100;
}

void morePtrs()
{
	int ep[10];
	int *p1, *p2;
	p1 = ep;
	p2 = p1;
	p2[8] = 10;
	ep[0] = -2;
}

void multiDimPtrs()
{
	int ep[100][100];
	int **p1;
	p1 = ep;
	p1[0] = 10;
	p1[10][9] = 4;
	ep[99][98] = -3;		
}