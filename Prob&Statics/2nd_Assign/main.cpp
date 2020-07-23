#include <iostream>
#include <math.h>
#include <windows.h>

#pragma warning(disable : 4996)

using namespace std;

void DrawCDF(float cdf[256], int x_origin, int y_origin);

HWND hwnd;
HDC hdc;


inline UCHAR limit(const UCHAR& value)
{
	return ((value > 255) ? 255 : ((value < 0) ? 0 : value)); // �ȼ��� max, min�� ����
}

void MemoryClear(UCHAR **buf) {
	if (buf) {
		free(buf[0]);
		free(buf);
		buf = NULL;
	}
}

UCHAR** memory_alloc2D(int width, int height)
{
	UCHAR** ppMem2D = 0;
	int	i;

	//arrary of pointer
	ppMem2D = (UCHAR**)calloc(sizeof(UCHAR*), height);
	if (ppMem2D == 0) {
		return 0;
	}

	*ppMem2D = (UCHAR*)calloc(sizeof(UCHAR), height * width);
	if ((*ppMem2D) == 0) {//free the memory of array of pointer        
		free(ppMem2D);
		return 0;
	}

	for (i = 1; i < height; i++) {
		ppMem2D[i] = ppMem2D[i - 1] + width;
	}

	return ppMem2D;
}

void MakeHistogramCDF(float histogram[256], float cdf[256], int width, int height)
{
	int size = width * height;
	cdf[0] = histogram[0];

	for (int i = 1; i < 256; i++) // histogram�� CDF ���ϱ�
		cdf[i] = cdf[i - 1] + histogram[i];
}

UCHAR** MakeHistogramEqualization(UCHAR** imgbuf, float histogram[256], float Equal_histogram[256], int width, int height)
{
	int size = width * height;
	UCHAR cnt[256] = { 0, };

	// histogram�� CDF���� ��ȯ
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			cnt[static_cast<UCHAR>(limit(histogram[imgbuf[i][j]] * 255))]++;
	}

	// ��ȯ�� CDF���� ������ ��Ȱȭ
	for (int i = 0; i < width; i++)
	{
		Equal_histogram[i] = static_cast<float>(cnt[i]) / size;
	}

	return imgbuf;
}

void MakeHistogram(UCHAR** imgbuf, float histogram[256], int width, int height)
{
	int size = width * height;
	int cnt[256] = { 0, };
	
	// �Է¹��� �̹����� ���� �ȼ��� ī��Ʈ
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			cnt[imgbuf[i][j]]++;
	}

	// cnt�� ������ histogram ����
	for (int i = 0; i < width; i++)
	{
		histogram[i] = static_cast<float>(cnt[i]) / size;
	}
}

void DrawCDF(float cdf[256], int x_origin, int y_origin) {
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

void DrawHistogram(float histogram[256], int x_origin, int y_origin) {
	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin + 255, y_origin);

	MoveToEx(hdc, x_origin, 100, 0);
	LineTo(hdc, x_origin, y_origin);

	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < histogram[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			LineTo(hdc, x_origin + CurX, y_origin - histogram[CurX] * 5000);
		}
	}
}

int main(void)
{
	system("color F0");
	hwnd = GetForegroundWindow();
	hdc = GetWindowDC(hwnd);

	int width = 256;
	int height = 256;

	float Image_Histogram[256] = { 0, };
	float Image_equal_Histogram[256] = { 0, };
	float Image_Histogram_CDF[256] = { 0, };


	FILE* fp_InputImg = fopen("gLenna256_256.raw", "rb");
	if (!fp_InputImg) {
		printf("Can not open file.");
	}

	UCHAR** Input_imgBuf = memory_alloc2D(width, height); // �޸� �����Ҵ�
	fread(&Input_imgBuf[0][0], sizeof(UCHAR), width * height, fp_InputImg); // 2���� �迭�� �̹��� �о����

	MakeHistogram(Input_imgBuf, Image_Histogram, width, height);

	DrawHistogram(Image_Histogram, 30, 400); // ������׷� �׷���

	MakeHistogramCDF(Image_Histogram, Image_Histogram_CDF, width, height);

	DrawCDF(Image_Histogram_CDF, 30, 400); // CDF �׷���

	MakeHistogramEqualization(Input_imgBuf, Image_Histogram_CDF, Image_equal_Histogram, width, height);

	DrawHistogram(Image_equal_Histogram, 400, 400); // ������׷� ��Ȱȭ �׷���

	FILE* fp_outputImg = fopen("output.raw", "wb"); // ��� �����ϱ�
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			Input_imgBuf[i][j] = Image_equal_Histogram[Input_imgBuf[i][j]];
			fwrite(&Input_imgBuf[i][j], sizeof(UCHAR), width * height, fp_outputImg); // 2���� �迭�� �̹��� ��������
		}
	}

	//MemoryClear(Input_imgBuf);
	//MemoryClear(Histogram_imgBuf);
	fclose(fp_outputImg);
	fclose(fp_InputImg);
	return 0;
}