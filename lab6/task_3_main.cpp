#include <chrono>
#include <iostream>

constexpr int exponent = 10000;

int main()
{
	srand(time(0));
	const auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::vector<int>> matrix1(exponent);
	std::vector<std::vector<int>> matrix2(exponent);
	std::vector<std::vector<int>> result(exponent);

	#pragma omp parallel for
	for (int i = 0; i < exponent; i++)
	{
		matrix1[i].resize(exponent);
		matrix2[i].resize(exponent);
		result[i].resize(exponent);

		for (int j = 0; j < exponent; j++)
		{
			matrix1[i][j] = rand() % 100;
			matrix2[i][j] = rand() % 100;
		}
	}


	#pragma omp parallel for collapse(2)
	for(int i = 0; i < exponent; i++)
	{
		for(int j = 0; j < exponent; j++)
		{
			result[i][j] = matrix1[i][j] * matrix2[i][j];
		}
	}

	const auto end = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> diff = end - start;
	std::cout << diff.count() << std::endl;

	// for(int i = 0; i < exponent; i++)
	// {
	// 	for(int j = 0; j < exponent; j++)
	// 	{
	// 		std::cout << result[i][j] << " ";
	// 	}
	//
	// 	std::cout << std::endl;
	// }

	return 0;
}