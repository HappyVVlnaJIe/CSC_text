#include "server.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "�� ������ ����" << std::endl;
		return 0;
	}

	BaseServer server(argv[1]);
	server.Run();
	return 0;
}