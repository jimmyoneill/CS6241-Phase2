#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct myStruct
{
	int i[10];
	float f[10];
};

int main(int argc, char const *argv[])
{
	int oneDim[10];
	int twoDim[15][1000];
	float twoDimF[15][66];
	int threeDim[9][10][11];
	char fourDim[12][12][12][12];

	oneDim[0] = 10;
	twoDim[10][999] = 9;
	twoDimF[0][59] = 8.0;
	threeDim[8][9][10] = 7;
	fourDim[1][1][1][1] = 9;

	// Structs are currently broken.
	// struct myStruct sArr[10];
	// sArr[0].i[0] = 1;
	// sArr[0].f[0] = 2.0;

	// struct myStruct mSArr[10][10];
	// mSArr[0][0].i[0] = 1;
	// mSArr[0][0].f[0] = 2.0;

	return 0;
}