#include <iostream>
class A
{
	public:
	void Afunction()
	{
		std::cout << "Afunction" << std::endl;
	}
};

class B
{
	public:
	int c[200];
	void Bfunction()
	{
		c[1] = 'A';
		std::cout << "Bfunction" << std::endl;
		std::cout << c[1] << std::endl;
	}
	int d[200];
};

int main()
{

	A a;

	void *ptr = &a;

	B *aptr = static_cast<B*>(ptr);

	aptr->Bfunction();
	return 0;
}