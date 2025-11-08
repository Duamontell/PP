#include <chrono>
#include <cmath>
#include <iostream>
#include <valarray>

int main()
{
	const auto start = std::chrono::system_clock::now();

	double result = 0;
	constexpr int count = 1000000000;

	for (int i = 0; i < count; ++i)
	{
		const double chastnoe = pow(-1, i);
		const double delimoe = 2 * i + 1;
		const double microResult = chastnoe / delimoe;
		result += microResult;
	}

	result = result * 4;

	std::cout << result << "\n";
	const auto end = std::chrono::system_clock::now();
	const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << elapsed.count() << "\n";

	return 0;
}
