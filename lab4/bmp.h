#pragma once

#include <fstream>
#include <random>
#include <tchar.h>

constexpr int MEASUREMENTS_COUNT = 20;

struct Rect {
	int startRow;
	int endRow;
	int startCol;
	int endCol;
};

struct BlurParams
{
	const BYTE *src;
	BYTE *dst;
	int width;
	int height;
	int rowSize;
	std::vector<Rect> tiles;
};

struct ThreadArgs
{
	BlurParams *blurParams;
	DWORD startTime;
	int threadId;
};

inline HBITMAP LoadHBitmapFromFile(const LPCTSTR inputImage)
{
	return static_cast<HBITMAP>(LoadImage(
		NULL,
		inputImage,
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION));
}

inline BITMAP GetBitMap(const HBITMAP & hBmp)
{
	BITMAP bmp;
	if (!GetObject(hBmp, sizeof(BITMAP), &bmp)) {
		DeleteObject(hBmp);
		throw std::runtime_error("Error getting BitMap");
	}
	return bmp;
}

inline std::vector<BYTE> GetImagePixels(
	const HBITMAP hBmp,
	const int width,
	const int height,
	const BITMAPINFO& bmi)
{
	const int rowSize = (width * 3 + 3) / 4 * 4;
	const int imageSize = rowSize * height;
	std::vector<BYTE> pixels(imageSize);

	const HDC hdc = GetDC(NULL);
	if (!GetDIBits(
		hdc,
		hBmp,
		0,
		height,
		pixels.data(),
		const_cast<BITMAPINFO*>(&bmi),
		DIB_RGB_COLORS))
	{
		ReleaseDC(NULL, hdc);
		throw std::runtime_error("Error getting image pixels");
	}

	ReleaseDC(NULL, hdc);

	return pixels;
}

inline BITMAPINFO MakeBitmapInfo(const int width, const int height)
{
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = width * height;

	return bmi;
}

inline DWORD WINAPI Blur(LPVOID lpParam)
{
	auto *args = static_cast<ThreadArgs *>(lpParam);
	BlurParams *bluredImage = args->blurParams;

	std::string fileName = "output_CPUx3_2_" + std::to_string(args->threadId) + ".txt";
	std::ofstream file(fileName, std::ios::out | std::ios::trunc);

	int allPixelCount = 0;
	for (const auto& tile : bluredImage->tiles)
	{
		allPixelCount += (tile.endRow - tile.startRow) * (tile.endCol - tile.startCol);
	}

	int pixelCount = 0;
	for (const auto& tile : bluredImage->tiles)
	{
		for (int y = tile.startRow; y < tile.endRow; ++y)
		{
			for (int x = tile.startCol; x < tile.endCol; ++x)
			{
				++pixelCount;
				int r = 0, g = 0, b = 0, count = 0;
				const int blurRadius = 3;
				for (int dy = -blurRadius; dy <= blurRadius; ++dy)
				{
					for (int dx = -blurRadius; dx <= blurRadius; ++dx)
					{
						const int nearY = y + dy;
						const int nearX = x + dx;
						if (nearX >= 0 && nearX < bluredImage->width && nearY >= 0 && nearY < bluredImage->height)
						{
							const int index = nearY * bluredImage->rowSize + nearX * 3;
							b += bluredImage->src[index];
							g += bluredImage->src[index + 1];
							r += bluredImage->src[index + 2];
							++count;
						}
					}
				}

				int currentPixelIndex = y * bluredImage->rowSize + x * 3;
				bluredImage->dst[currentPixelIndex] = static_cast<BYTE>(b / count);
				bluredImage->dst[currentPixelIndex + 1] = static_cast<BYTE>(g / count);
				bluredImage->dst[currentPixelIndex + 2] = static_cast<BYTE>(r / count);

				if (pixelCount % ((allPixelCount) / MEASUREMENTS_COUNT) == 0)
				{
					std::string fileName = "output_CPUx3_2_" + std::to_string(args->threadId) + ".txt";
					std::ofstream file(fileName, std::ios::out | std::ios::app);

					const DWORD now = timeGetTime();
					const DWORD diff = now - args->startTime;

					file << std::to_string(args->threadId) + "|" + std::to_string(diff) + "\n";
				}

			}
		}
	}

	return 0;
}

void BlurImageMultiThread(
    const BYTE* src,
    BYTE* dst,
    const int width,
    const int height,
    const int rowSize,
    const int threadsCount,
    const int coreCount,
    DWORD startTime)
{
    int colsPerTile = width / threadsCount;
    int remCols = width % threadsCount;
    int rowsPerTile = height / threadsCount;
    int remRows = height % threadsCount;

    std::vector<Rect> allTiles;
    allTiles.reserve(threadsCount * threadsCount);

    int curY = 0;
    for (int i = 0; i < threadsCount; ++i)
    {
        int tileHeight = rowsPerTile + (i < remRows ? 1 : 0);
        int curX = 0;
        for (int j = 0; j < threadsCount; ++j) {
            int tileWidth = colsPerTile + (j < remCols ? 1 : 0);
            allTiles.push_back({curY, curY + tileHeight, curX, curX + tileWidth});
            curX += tileWidth;
        }
        curY += tileHeight;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(allTiles.begin(), allTiles.end(), g);

    DWORD_PTR affinityMask = (1ULL << coreCount) - 1;

    std::vector<HANDLE> threads(threadsCount);
    std::vector<BlurParams> params(threadsCount);
	std::vector<ThreadArgs> args(threadsCount);

    for (int i = 0; i < threadsCount; ++i)
    {
        params[i].src = src;
        params[i].dst = dst;
        params[i].width = width;
        params[i].height = height;
        params[i].rowSize = rowSize;
        params[i].tiles.clear();
        for (int k = 0; k < threadsCount; ++k)
        {
            params[i].tiles.push_back(allTiles[i * threadsCount + k]);
        }

		args[i].blurParams = &params[i];
    	args[i].startTime = startTime;
    	args[i].threadId = i;

        threads[i] = CreateThread(NULL, 0, Blur, &args[i], CREATE_SUSPENDED, NULL);
        if (threads[i] != NULL)
        {
            SetThreadAffinityMask(threads[i], affinityMask);
        }
    }

	SetThreadPriority(threads[0], THREAD_PRIORITY_ABOVE_NORMAL);
	SetThreadPriority(threads[1], THREAD_PRIORITY_NORMAL);
	SetThreadPriority(threads[2], THREAD_PRIORITY_BELOW_NORMAL);

	for (int i = 0; i < threadsCount; i++)
	{
		ResumeThread(threads[i]);
	}

    WaitForMultipleObjects(threadsCount, threads.data(), TRUE, INFINITE);
    for (auto& t : threads)
    {
        CloseHandle(t);
    }
}

HBITMAP CreateBitmapFromPixels(
	const BYTE* pixelsData,
	const BITMAPINFO& bmi,
	const int height)
{
	HBITMAP hBlurredBmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
	if (!hBlurredBmp) {
		throw std::runtime_error("Error create bitmap from pixels");
	}

	HDC hdc = GetDC(NULL);
	if (!SetDIBits(hdc, hBlurredBmp, 0, height, const_cast<BYTE*>(pixelsData), &bmi, DIB_RGB_COLORS)) {
		ReleaseDC(NULL, hdc);
		throw std::runtime_error("Error create bitmap from pixels");
	}
	ReleaseDC(NULL, hdc);

	return hBlurredBmp;
}

void SaveBlurredImage(
	const BYTE* blurredData,
	const _TCHAR* outputImage,
	int width,
	int height,
	int imageSize,
	const BITMAPINFO& bmi)
{
	BITMAPFILEHEADER file_header = {0};
	file_header.bfType = 0x4D42;
	file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	file_header.bfSize = file_header.bfOffBits + imageSize;

	BITMAPINFOHEADER info_header = bmi.bmiHeader;
	info_header.biHeight = height;
	info_header.biSizeImage = imageSize;

	HANDLE hFile = CreateFile(outputImage, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("CreateFile failed for output");
	}

	DWORD bytes_written;
	WriteFile(hFile, &file_header, sizeof(file_header), &bytes_written, NULL);
	WriteFile(hFile, &info_header, sizeof(info_header), &bytes_written, NULL);
	WriteFile(hFile, const_cast<BYTE*>(blurredData), imageSize, &bytes_written, NULL);
	CloseHandle(hFile);
}