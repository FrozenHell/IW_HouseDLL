// ArrayDLL.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <stdio.h>
#include <assert.h>
#include <Math.h>

// --------------------- ��������� --------------------------
template<typename DataType>
struct TArray
{
	DataType* Data;

	int Num()
	{
		return ArrayNum;
	}

	void Reallocate(int NewNum, bool bCompact = false)
	{
		ArrayNum = NewNum;
		if(ArrayNum > ArrayMax || bCompact)
		{
			ArrayMax = ArrayNum;
			Data = (DataType*)(*ReallocFunctionPtr)(Data, ArrayMax * sizeof(DataType), 8);
		}
	}
private:
	int ArrayNum;
	int ArrayMax;
};

struct NaviStruct
{
	TArray<int> NaviData;
};

struct vector
{
	int x, y, z;

	int cells()
	{
		return x * y * z;
	}

	int length() {
		return ((int)sqrt((float)(x * x + y * y + z * z)));
	}
};

struct rotator
{
	int Pich, Yaw, Roll;
};

//----------------------------------- ������ --------------------------------------------

long myrand_seed = 1021;
int myrand_rand()
{
	__asm
	{
		push	edx
		push	ecx
		mov		eax,	[myrand_seed]

		cmp		eax,	0		// ���� ���� ����� ���� 
		jnz		noninit
		mov		eax,	1021	// �� ������� � ���� ����� 1021

	noninit:
		xor		edx,	edx
		mov		ecx,	127773
		div		ecx
		mov		ecx,	eax
		mov		eax,	16807
		mul		edx
		mov		edx,	ecx
		mov		ecx,	eax
		mov		eax,	2836
		mul		edx
		sub		ecx,	eax
		xor		edx,	edx
		mov		eax,	ecx
		mov		[myrand_seed], ecx
		mov		ecx,	100000
		div		ecx
		mov		eax,	edx
		pop		ecx
		pop		edx
	}
}

// ������� ��������� ������� ��� ���� �� num ������� � ������� pos*2 
byte get2bit(int IN_num, byte IN_pos)
{
	__asm
	{
		push	ecx
		mov		eax,	[IN_num]
		mov		cl,		[IN_pos]
		shl		cl,		1		// �������� �������� ������ �� 2
		shr		eax,	cl		// �������� �������������� �����
		and		eax,	0x3		// �������� ��� ���� ����� ����� ����
		pop		ecx
	}
}

// ������� ��������� ���������� ��� ������ ���� bits ����� num ������� � ������� pos*2
int set2bit(int IN_num, byte IN_bits, byte IN_pos)
{
	__asm
	{
		push	edx
		push	ecx
		mov		eax,	[IN_num]
		mov		cl,		[IN_pos]
		shl		cl,		1		// �������� �������� �������� �� 2
		ror		eax,	cl		// ������� �������������� �����
		and		eax,	-0x4	// �������� ������ ����
		xor		edx,	edx		// �������� edx
		mov		dl,		[IN_bits]
		add		eax,	edx		// ������������� ���� � ������ ��������
		rol		eax,	cl		// ������� ����� � �������� �������
		pop		ecx
		pop		edx
	}
}

// ������� ��������� ���������� ��� ������ ���� bits ����� num ������� � ������� pos*2
void change2bit(int* INOUT_num, byte IN_bits, byte IN_pos)
{
	__asm
	{
		push	edx
		push	ecx
		mov		eax,	[INOUT_num]
		mov		eax,	[eax]	// �������� �������� �� ������
		mov		cl,		[IN_pos]
		shl		cl,		1		// �������� �������� �������� �� 2
		ror		eax,	cl		// ������� �������������� �����
		and		eax,	-0x4	// �������� ������ ����
		xor		edx,	edx		// �������� edx
		mov		dl,		[IN_bits]
		add		eax,	edx		// ������������� ���� � ������ ��������
		rol		eax,	cl		// ������� ����� � �������� �������
		mov		edx,	[INOUT_num]
		mov		[edx],	eax		// ���������� ����� �� ������
		pop		ecx
		pop		edx
	}
}

// ��� ������� ������� �� type
// type == 0 - ����, 1 - �����, 2 - �����, 3 - ������

byte binrand(int IN_type)
{
	byte exiting = 0;
	switch (IN_type)
	{
		case 1: // ����� �� ����� �� ������� �� ������ �����
			exiting = (myrand_rand() % 2) == 1 ? 2 : 0; // ����� ��� �����
			break;
		case 2: // ����� �� ����� �� ������� �� 2+ �����
			exiting = 0; // ������ ����
			break;
		case 3: // ����� ����� ���������
			exiting = myrand_rand() % 3 + 1; // �����, ����� ��� ������
			break;
	}
	return exiting;
}

// ��������������� ������������� ����
void generichouse(int* OUT_walls, int IN_type, int len, int wid, int hei)
{
	// x ����������� � �������, y - � ��������

	int j = 0; // i - ������� �� x, j - ������� �� y
	int nfloor = 1;
	for (int i = 0; i < (len * wid * hei); i++)
	{ // ��� �� ���� ������� ������
		if (nfloor == 1)
		{ // ���� ��� ������ ����
			if (i % len == 0)
			{ // ���� ��� ����� ����� ������
				if  (IN_type == 2)
				{ // ����, � ������������ � �����, ��� ����� ��������� � ������� ������
					OUT_walls[i] = binrand(3) << 6; //*64
				}
				else
				{
					OUT_walls[i] = binrand(1) << 6; //*64
				}
			}
			else // ���� ����� �� �����
			{ // ���� ���������� � ����� �� �������� ������
				OUT_walls[i] = get2bit(OUT_walls[i - 1], 1) << 6;
			}

			if (j == 0)
			{ // ���� ��� �������� ����� ������
				OUT_walls[i] += binrand(1) << 4;
			}
			else
			{ // ���� ����� �� ��������
				// ���� ���������� � ����� �� �������� ������
				OUT_walls[i] += get2bit(OUT_walls[i - len], 0) << 4;
			}

			if (i % len == len - 1)
			{ // ���� ��� �������� ����� ������
				if  (IN_type == 0) // ���� �� ����� ���� � ������� �����
				{
					OUT_walls[i] += binrand(1) << 2;
				}
				else // ���� ���, ������� �� ���������� - ��� ���� ������� ������
				{				
					OUT_walls[i] += binrand(3) << 2;
				}
			}
			else // ���� ��� �� �������� ����� ������
			{
				OUT_walls[i] += binrand(3) << 2;
			}

			if (j == wid - 1)
			{ // ���� ��� ��������� ����� ������
				OUT_walls[i] += binrand(1);
			}
			else
			{ // ���� ��� �� ��������� ����� ������
				OUT_walls[i] += binrand(3);
			}
		}
		else
		{ // 2� ���� � ����
			if (i % len == 0)
			{ // ���� ��� ����� �����
				if  (IN_type <= 1)
				{ // ���� ��� ������� �����
					OUT_walls[i] = binrand(2) << 6;
				}
				else
				{ // ���� ��� ����� ��������� � ������� ������
					OUT_walls[i] = binrand(3) << 6;
				}
			}
			else
			{ // ���� ��� �� ����� �����, ���� ���������� � ����� �� �������� ������
				OUT_walls[i] = get2bit(OUT_walls[i - 1], 1) << 6;
			}

			if (i % len == len - 1)
			{ // ���� ��� �������� �����
				if (IN_type == 0)
				{// ���� ��� ������� �����
					OUT_walls[i] += binrand(2) << 2;
				}
				else
				{ // ���� ��� ����� ��������� � ��������� ������
					OUT_walls[i] += binrand(3) << 2;
				}
			}
			else
			{ // ���� ��� �� �������� ����� ������
				OUT_walls[i] += binrand(3) << 2;
			}

			if (j == 0)
			{ // ���� ��� ��������� �����
				OUT_walls[i] += binrand(2) << 4;
			}
			else
			{	// ���� ����� �� ���������, �� ���� ���������� � ����� �� �������� ������
				OUT_walls[i] += get2bit(OUT_walls[i - len], 0) << 4;
			}

			if (j == wid - 1)
			{ // ���� ��� �������� �����
				OUT_walls[i] += binrand(2);
			}
			else
			{
				OUT_walls[i] += binrand(3);
			}
		}

		// --------------------
		if (i % len == len - 1)
		{	// ���� �� ����� �� ���� �� X, ���������� �� ������ ��������� ������ �� Y
			j++;
		}
		if (j == wid)
		{ // ���� �� ����� �� ���� �� Y, �� �������� ��������� ���� (�� Z)
			nfloor++;
			j = 0;
		}
	}
}

// ---------------------��������-----------------------------
void genStairs(byte* OUT_stCoords, int* INOUT_Walls, int len, int wid, int hei, int stColls)
{
	// ����������� ����� �������� ���������� i, j � accepted
	{
		int i = 0, j;
		bool accepted;
		// ���� �� ��������� ��� ��������
		while (i < stColls)
		{
			// ����������� ����� ��������
			OUT_stCoords[i * 2] = myrand_rand() % len;
			OUT_stCoords[i * 2 + 1] = myrand_rand() % (wid - 1);
			accepted = true;

			// ��������� � ��������� ����������
			for (j = 0; j < i; j++)
			{
				// ���� �������� �� ����� �����
				if (OUT_stCoords[i * 2] == OUT_stCoords[j * 2])
				{	
					// ���� �������� ����������� ����-�����
					if (OUT_stCoords[i * 2 + 1] <= OUT_stCoords[j * 2 + 1] + 1
						&&
						OUT_stCoords[i * 2 + 1] >= OUT_stCoords[j * 2 + 1] - 1)
					{
						// ��������� ����� ����� ��� ���� ��������
						accepted = false;
					}
				}
			}

			// ���� ����� �������� ��������
			if (accepted)
			{
				// ��������� � ��������� ��������
				i++;
			}
		}
	}
	// ��� ��� ���������� i, j � accepted ��������� ��������������

	// ����������� ����� ������ �������
	for (int i = 0; i < stColls; i++)
	{
		int addr;
		for (int nfloor = 0; nfloor < hei; nfloor++)
		{
			addr = OUT_stCoords[i * 2] + OUT_stCoords[i * 2 + 1] * len + nfloor * wid * len;
			INOUT_Walls[addr] = 0x56;

			if (OUT_stCoords[i * 2] != 0)
			{
				change2bit(&INOUT_Walls[addr - 1], 1, 1);
			}

			if (OUT_stCoords[i * 2] != len - 1)
			{
				change2bit(&INOUT_Walls[addr + 1], 1, 3);
			}

			if (OUT_stCoords[i * 2 + 1] != 0)
			{
				change2bit(&INOUT_Walls[addr - len], 1, 0);
			}

			// �������� �� ����� ���� � �������� �����, �.�. �������� �� �����
			change2bit(&INOUT_Walls[addr + len], 2, 2);
		}
	}
}

// �������� ����� ������� ����
void NewFloor(byte* INOUT_flArr, byte IN_flCount, byte x1, byte y1, byte x2, byte y2)
{
	INOUT_flArr[IN_flCount * 4] = x1;
	INOUT_flArr[IN_flCount * 4 + 1] = y1;
	INOUT_flArr[IN_flCount * 4 + 2] = x2;
	INOUT_flArr[IN_flCount * 4 + 3] = y2;
}

// ������ ������������������ ���
// ���������� ���������� �������� ����
byte gfloor(byte* OUT_flArr, byte* IN_stArr, byte IN_stCount, byte IN_sizeX, byte IN_sizeY)
{
	// ���������� ������ ����
	byte OUT_flCount = 0;

	// ������ ��������� ������ �������
	byte* stairs = new byte[IN_stCount];
	for (byte i = 0; i < IN_stCount; i++)
		stairs[i] = i;

	// � ����������� �� ����� � ������ ������ ����� ��� ����� ��� ������
	if (IN_sizeX < IN_sizeY)
	{
		// ��������� �������� � ������ �������
		// ���������� ������� �������� ������
		int min_i, x;
		for (byte i = 0; i < IN_stCount - 1; i++)
		{
			min_i = i;
			for (byte j = i + 1; j < IN_stCount; j++)
			{
				if (IN_stArr[stairs[min_i] * 2] + IN_stArr[stairs[min_i] * 2 + 1] * IN_sizeX > IN_stArr[stairs[j] * 2] + IN_stArr[stairs[j] * 2 + 1] * IN_sizeX)
					min_i = j;
			}
			if(min_i != i)
			{
				x = stairs[i];
				stairs[i] = stairs[min_i];
				stairs[min_i] = x;  
			}
		}

		// ��������� ��� ����� ����
		int prew_x = IN_sizeX, prew_y = -1;
		for (byte i = 0; i < IN_stCount; i++)
		{
			// ���� ���������� �����
			if (IN_stArr[stairs[i] * 2 + 1] == prew_y)
			{
				// ���� ���� �����
				if (IN_stArr[stairs[i] * 2] != prew_x)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, IN_stArr[stairs[i] * 2], prew_y + 1);
					prew_x = IN_stArr[stairs[i] * 2] + 1;
				}
			}
			else // ����� �����
			{
				// ���� �����, ����������� ���������� �����
				if (prew_x < IN_sizeX)
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, IN_sizeX, prew_y + 1);

				// ���� ���� �����, ��������� �����
				if (IN_stArr[stairs[i] * 2 + 1] > prew_y + 1)
					NewFloor(OUT_flArr, OUT_flCount++, 0, prew_y + 1, IN_sizeX, IN_stArr[stairs[i] * 2 + 1]);

				// ���� �������� �� � ������ ���� ��������� ���
				if (IN_stArr[stairs[i] * 2] != 0)
				{
					NewFloor(OUT_flArr, OUT_flCount++, 0, IN_stArr[stairs[i] * 2 + 1], IN_stArr[stairs[i] * 2], IN_stArr[stairs[i] * 2 + 1] + 1);
				}
				prew_y = IN_stArr[stairs[i] * 2 + 1];
				prew_x = IN_stArr[stairs[i] * 2] + 1;
			}
		}
		// ���� �����, ����������� ���������� �����
		if (prew_x < IN_sizeX)
			NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, IN_sizeX, prew_y + 1);

		// ���� ���� �����, ��������� �����
		if (IN_sizeY > prew_y + 1)
			NewFloor(OUT_flArr, OUT_flCount++, 0, prew_y + 1, IN_sizeX, IN_sizeY);
	}
	else // SIZE_X >= SIZE_Y
	{
		// ��������� �������� � ������ �������
		// ���������� ������� �������� ������
		int min_i, x;
		for (byte i = 0; i < IN_stCount - 1; i++)
		{
			min_i = i;
			for (byte j = i + 1; j < IN_stCount; j++)
			{
				if (IN_stArr[stairs[min_i] * 2] * IN_sizeX + IN_stArr[stairs[min_i] * 2 + 1] > IN_stArr[stairs[j] * 2] * IN_sizeX + IN_stArr[stairs[j] * 2 + 1])
					min_i = j;
			}
			if (min_i != i)
			{
				x = stairs[i];
				stairs[i] = stairs[min_i];
				stairs[min_i] = x;  
			}
		}

		// ��������� ��� ����� ����
		int prew_y = IN_sizeY, prew_x = -1;
		for (byte i = 0; i < IN_stCount; i++)
		{
			// ���� ���������� �����
			if (IN_stArr[stairs[i] * 2] == prew_x)
			{
				// ���� ���� �����
				if (IN_stArr[stairs[i] * 2 + 1] != prew_y)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, prew_x + 1, IN_stArr[stairs[i] * 2 + 1]);
					prew_y = IN_stArr[stairs[i] * 2 + 1] + 1;
				}
			}
			else // ����� �����
			{
				// ���� �����, ����������� ���������� �����
				if (prew_y < IN_sizeY)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, prew_x + 1, IN_sizeY);
				}

				// ���� ���� �����, ��������� �����
				if (IN_stArr[stairs[i] * 2] > prew_x + 1)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x + 1, 0, IN_stArr[stairs[i] * 2], IN_sizeY);
				}

				// ���� �������� �� � ������ ���� ��������� ���
				if (IN_stArr[stairs[i] * 2 + 1] != 0)
				{
					NewFloor(OUT_flArr, OUT_flCount++, IN_stArr[stairs[i] * 2], 0, IN_stArr[stairs[i] * 2] + 1, IN_stArr[stairs[i] * 2 + 1]);
				}
				prew_x = IN_stArr[stairs[i] * 2];
				prew_y = IN_stArr[stairs[i] * 2 + 1] + 1;
			}
		}
		// ���� �����, ����������� ���������� �����
		if (prew_y < IN_sizeY)
		{
			NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, prew_x + 1, IN_sizeY);
		}

		// ���� ���� �����, ��������� �����
		if (IN_sizeX > prew_x + 1)
		{
			NewFloor(OUT_flArr, OUT_flCount++, prew_x + 1, 0, IN_sizeX, IN_sizeY);
		}
	}

	// ������������ ������ ���������� �������
	delete[] stairs;

	return OUT_flCount;
}

// ������ ������ ������
void ghouse(int* OUT_houseParts, int IN_type, byte len, byte wid, int hei, byte stCount, int seed)
{
	myrand_seed = seed;
	int offset = 0;
	int* walls = new int[len * wid * hei];
	byte* stairs = new byte[stCount * 2];
	byte* floors = new byte[stCount * 12 + 4];
	byte flCount;

	// ������ ��� ���������� � ������
	generichouse(walls, IN_type, len, wid, hei);
	genStairs(stairs, walls, len, wid, hei, stCount);
	flCount = gfloor(floors, stairs, stCount, len, wid);

	// ������� ���������� � ���������
	for (int i = 0; i < stCount * 2; i++)
	{
		OUT_houseParts[i] = stairs[i];
	}
	offset = stCount * 2;

	// ������� ���������� � ���������� �������� ����
	OUT_houseParts[offset] = flCount;
	offset++;

	// ������� ���������� � ������ ����
	for (int i = 0; i < flCount * 4; i++)
	{
		OUT_houseParts[offset + i] = floors[i];
	}
	offset += flCount * 4;

	// ������� ���������� � ������ ������
	for (int i = 0; i < len * wid * hei; i++)
	{
		OUT_houseParts[offset + i] = walls[i];
	}

	// ������� ������
	delete[] walls;
	delete[] stairs;
	delete[] floors;
}

//---------------------------------------------------------------------------------------------------------
// ��� ������ ��������, ����� �� ���������� ������ ���� � ������ ������.

void genview(int* house, int* cells, vector* info)
{
	// info[0] - ������, ������� �������� ������ ���� � ������
	// info[1] - ������, ������� �������� ������ �����
	// info[2] - ������, ���������� ������������� ���������� ������
	// house - ������, ���������� ���������� � ������� ����
	// cells - ������, ���������� ���������� � ��������� ������ ������ (����� ����������� � �������)

	int sx = (int)floor(info[0].x * info[1].x * 0.5); // ���������� ����
	int sy = (int)floor(info[0].y * info[1].y * 0.5); // �������������� ...
	int sz = (int)floor(info[0].z * info[1].z * 0.5); // ..
	int vx, vy, vz;

	if (info[2].x > sx)
	{ // �� ����� ���� �� ������ ����
		vx = 1;
	} else if (info[2].x < -sx)
	{ // �� ����� ��������������� ���������� �����
		vx = -1;
	}
	else
	{ // �� ����� �� ���� �� ���������� ����
		vx = 0;
	}

	if (info[2].y > sy)
	{ // �� ����� ���� �� ������ ����
		vy = 1;
	}
	else if (info[2].y < -sy)
	{ // �� ����� ��������������� ���������� �����
		vy = -1;
	}
	else
	{ // �� ����� �� ���� �� ���������� ����
		vy = 0;
	}

	if (info[2].z > sz)
	{ // �� ����� ����� ����
		vz = 1;
	}
	else if (info[2].z < -sz)
	{ // �� ��� �����
		vz = -1;
	}
	else
	{ // �� ���� �����, �� �� ��� �����
		vz = 0;
	}

	int x, y, z, lenx = info[0].x, leny = info[0].x, addr;

	if (vx == 0 && vy == 0 && vz == 0)
	{ // ���� �� ������ ����
		for (z = 0; z < info[0].z; z++)
			for (y = 0; y < leny; y++)
				for (x = 0; x < lenx; x++)
					cells[z * lenx * leny + y * lenx + x] = 2; // ���� ������ "�������� ��"
	}
	else
	{
		for (z = 0; z < info[0].z; z++)
			for (y = 0; y < leny; y++)
				for (x = 0; x < lenx; x++)
				{
					addr=z*lenx*leny+y*lenx+x;
					cells[addr] = 0; // �� ��������� ������ ������
					if (vx == 1 && x == lenx - 1) cells[addr] = 2; // �� ���� ��� � ���� - �������� �
					if (vx == -1 && x == 0) cells[addr] = 2;
					if (vy == 1 && y == leny - 1) cells[addr] = 2;
					if (vy == -1 && y == 0) cells[addr] = 2;
					if (vz == 1 && z == info[0].z - 1) cells[addr] = 2;
				}
	}
}

//---------------------------------------------------------------------------------------------------------

extern "C"
{
	typedef void* (*ReallocFunctionPtrType)(void* Original, DWORD Count, DWORD Alignment);

	ReallocFunctionPtrType ReallocFunctionPtr = NULL;

	struct FDLLBindInitData
	{
		INT Version;
		ReallocFunctionPtrType ReallocFunctionPtr;
	};

	__declspec(dllexport) void DLLBindInit(FDLLBindInitData* InitData)
	{
		ReallocFunctionPtr = InitData->ReallocFunctionPtr;
	}

	__declspec(dllexport) void GetNavData(struct NaviStruct* NavData, int type, int len, int wid, int hei, int stairscoll, int seed)
	{
		NavData->NaviData.Reallocate(14 * stairscoll + 5 + len * wid * hei);

		ghouse(NavData->NaviData.Data, type, len, wid, hei, stairscoll, seed);
	}

	__declspec(dllexport) void GetNavData2(struct NaviStruct* NavData, struct NaviStruct* NavData2,int len,int wid,int hei,int posx,int posy,int posz)
	{
		NavData2->NaviData.Reallocate(len*wid*hei);

		vector* inf = new vector[3];

		inf[0].x = len;
		inf[0].y = wid;
		inf[0].z = hei;

		inf[1].x = 600;
		inf[1].y = 600;
		inf[1].z = 250;

		inf[2].x = posx;
		inf[2].y = posy;
		inf[2].z = posz;

		genview(NavData->NaviData.Data, NavData2->NaviData.Data, inf);
	}
}
