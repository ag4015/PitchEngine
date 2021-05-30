
#include <iostream>
#include <chrono>
#include <random>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

int main()
{
	// Dummy variable initialization
	auto tp1rt    = std::chrono::high_resolution_clock::now();
	auto tp1ct    = std::chrono::high_resolution_clock::now();
    auto tp2rt    = std::chrono::high_resolution_clock::now();
    auto tp2ct    = std::chrono::high_resolution_clock::now();
    auto exTimeRt = std::chrono::duration_cast<std::chrono::nanoseconds>(tp1rt - tp1rt);
    auto exTimeCt = std::chrono::duration_cast<std::chrono::nanoseconds>(tp1ct - tp1ct);

	int numIterations = 100;
	constexpr int size = 300;
	constexpr int size2 = 100;
	Matrix<float, Dynamic, Dynamic> mrt1;
	Matrix<float, Dynamic, Dynamic> mrt2;
	Matrix<float, size, size2> mct1;
	Matrix<float, size2, size> mct2;

	for (int count = 0; count < numIterations; count++)
	{
		mrt1 = Matrix<float, Dynamic, Dynamic>::Random(size, size2);
		mrt2 = Matrix<float, Dynamic, Dynamic>::Random(size2, size);
		tp1rt = std::chrono::high_resolution_clock::now();
		auto rrt = mrt1 * mrt2;
		tp2rt = std::chrono::high_resolution_clock::now();
		exTimeRt += std::chrono::duration_cast<std::chrono::nanoseconds>(tp2rt - tp1rt);

		mct1 = Matrix<float, size, size2>::Random();
		mct2 = Matrix<float, size2, size>::Random();
		tp1ct = std::chrono::high_resolution_clock::now();
		auto rct = mct1 * mct2;
		tp2ct = std::chrono::high_resolution_clock::now();
		exTimeCt += std::chrono::duration_cast<std::chrono::nanoseconds>(tp2ct - tp1ct);
	}

	std::cout << "Number of executions: " << numIterations << std::endl;
	std::cout << "Average execution time: " << std::endl;
	std::cout << "Runtime memory test: " << exTimeRt.count()/numIterations << "ns." << std::endl;
	std::cout << "Compile time memory test: " << exTimeCt.count()/numIterations << "ns." << std::endl;
	return 0;
}
