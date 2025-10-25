#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <windows.h>
#include "bmp.h"
#include "tchar.h"

void ParseArguments(
	const int argc,
	_TCHAR ** argv,
	int& threadsCount,
	int& coreCount,
	const _TCHAR*& inputImage,
	const _TCHAR*& outputImage)
{
	if (argc != 5) {
		throw std::invalid_argument("Usage: <thread count> <core count> <input filename> <output filename>");
	}
	threadsCount = _ttoi(argv[1]);
	coreCount = _ttoi(argv[2]);
	inputImage = argv[3];
	outputImage = argv[4];
}


int _tmain(int argc, _TCHAR *argv[])
{
	auto start = std::chrono::high_resolution_clock::now();

	const DWORD startTime = timeGetTime();
	try
	{
		int threadsCount, coreCount;
		const _TCHAR *inputImage, *outputImage;
		ParseArguments(argc, argv, threadsCount, coreCount, inputImage, outputImage);

		HBITMAP hBmp = LoadHBitmapFromFile(inputImage);
		BITMAP bmp = GetBitMap(hBmp);

		const int width = bmp.bmWidth;
		const int height = bmp.bmHeight;

		BITMAPINFO bmi = MakeBitmapInfo(width, height);
		std::vector<BYTE> pixels = GetImagePixels(hBmp, width, height, bmi);

		std::vector<BYTE> blurredPixels = pixels;
		const int rowSize = (width * 3 + 3) / 4 * 4;
		BlurImageMultiThread(pixels.data(), blurredPixels.data(), width, height, rowSize, threadsCount, coreCount, startTime);

		HBITMAP hBlurredBmp = CreateBitmapFromPixels(blurredPixels.data(), bmi, height);

		const int imageSize = rowSize * height;
		SaveBlurredImage(blurredPixels.data(), outputImage, width, height, imageSize, bmi);

		DeleteObject(hBmp);
		DeleteObject(hBlurredBmp);
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}

	const auto end = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> diff = end - start;
	std::cout << diff.count() << " seconds" << std::endl;

	return 0;
}
