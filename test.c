void print(int* A)
{
	int c=10;
	for(int i=0;i<10;i+=4)
	{
		A[i+2]=c+i+2;
		A[i+1]=c+i+1;
		A[i+3]=c+i+3;
		A[i]=c+i;
	}
}

