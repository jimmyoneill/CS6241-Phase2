void populate(int a[])
{
	a[0] = 1;
}

void sameNameAsPopulate()
{
	int a[10];
	a[0] = 1;
}

int main(int argc, char const *argv[])
{
	int a[5];
	populate(a);
	
	return 0;
}