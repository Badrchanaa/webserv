#include <iostream>

class A
{
	public:
	A(){}
	virtual void test() = 0;
};

class B: public A
{
	public:
	void test()
	{
		std::cout << "this is a test" << std::endl;
	}
};

int main()
{
	B b;

	A &a = b;

	a.test();
	return 0;
}
