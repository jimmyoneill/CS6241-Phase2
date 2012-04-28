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
	struct myStruct mStruct;
	mStruct.i[0] = 1;
	mStruct.f[0] = 2.0;

	struct myStruct sArr[10];
	sArr[1].i[2] = 1;
	sArr[3].f[4] = 2.0;

	struct myStruct mSArr[10][10];
	mSArr[5][6].i[7] = 1;
	mSArr[8][9].f[10] = 2.0; // boom

	return 0;
}