// ArrayDLL.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <stdio.h>
#include <assert.h>
#include <Math.h>

// --------------------- структуры --------------------------
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

//----------------------------------- методы --------------------------------------------

long myrand_seed = 1021;
int myrand_rand()
{
	__asm
	{
		push	edx
		push	ecx
		mov		eax,	[myrand_seed]

		cmp		eax,	0		// если семя равно нулю 
		jnz		noninit
		mov		eax,	1021	// то заносим в него число 1021

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

// Функция позволяет вернуть два бита из num начиная с позиции pos*2 
byte get2bit(int IN_num, byte IN_pos)
{
	__asm
	{
		push	ecx
		mov		eax,	[IN_num]
		mov		cl,		[IN_pos]
		shl		cl,		1		// умножаем величину сдвига на 2
		shr		eax,	cl		// сдвигаем обрабатываемое число
		and		eax,	0x3		// обнуляем все биты кроме наших двух
		pop		ecx
	}
}

// Функция позволяет установить два первых бита bits числу num начиная с позиции pos*2
int set2bit(int IN_num, byte IN_bits, byte IN_pos)
{
	__asm
	{
		push	edx
		push	ecx
		mov		eax,	[IN_num]
		mov		cl,		[IN_pos]
		shl		cl,		1		// умножаем величину поворота на 2
		ror		eax,	cl		// вращаем обрабатываемое число
		and		eax,	-0x4	// обнуляем нужные биты
		xor		edx,	edx		// обнуляем edx
		mov		dl,		[IN_bits]
		add		eax,	edx		// устанавливаем биты в нужные значения
		rol		eax,	cl		// вращаем число в исходную позицию
		pop		ecx
		pop		edx
	}
}

// Функция позволяет установить два первых бита bits числу num начиная с позиции pos*2
void change2bit(int* INOUT_num, byte IN_bits, byte IN_pos)
{
	__asm
	{
		push	edx
		push	ecx
		mov		eax,	[INOUT_num]
		mov		eax,	[eax]	// получаем значение из адреса
		mov		cl,		[IN_pos]
		shl		cl,		1		// умножаем величину поворота на 2
		ror		eax,	cl		// вращаем обрабатываемое число
		and		eax,	-0x4	// обнуляем нужные биты
		xor		edx,	edx		// обнуляем edx
		mov		dl,		[IN_bits]
		add		eax,	edx		// устанавливаем биты в нужные значения
		rol		eax,	cl		// вращаем число в исходную позицию
		mov		edx,	[INOUT_num]
		mov		[edx],	eax		// записываем ответ по адресу
		pop		ecx
		pop		edx
	}
}

// что ставить зависит от type
// type == 0 - окно, 1 - стена, 2 - дверь, 3 - ничего

byte binrand(int IN_type)
{
	byte exiting = 0;
	switch (IN_type)
	{
		case 1: // стена на улицу из комнаты на первом этаже
			exiting = (myrand_rand() % 2) == 1 ? 2 : 0; // стена или дверь
			break;
		case 2: // стена на улицу из комнаты на 2+ этаже
			exiting = 0; // только окно
			break;
		case 3: // стена между комнатами
			exiting = myrand_rand() % 3 + 1; // стена, дверь или проход
			break;
	}
	return exiting;
}

// псевдослучайное распределение стен
void generichouse(int* OUT_walls, int IN_type, int len, int wid, int hei)
{
	// x сонаправлен с севером, y - с востоком

	int j = 0; // i - считает по x, j - считает по y
	int nfloor = 1;
	for (int i = 0; i < (len * wid * hei); i++)
	{ // идём по всем ячейкам здания
		if (nfloor == 1)
		{ // если это первый этаж
			if (i % len == 0)
			{ // если это южная стена здания
				if  (IN_type == 2)
				{ // если, в соответствии с типом, эта стена прилегает к другому зданию
					OUT_walls[i] = binrand(3) << 6; //*64
				}
				else
				{
					OUT_walls[i] = binrand(1) << 6; //*64
				}
			}
			else // если стена не южная
			{ // берём информацию о стене из соседней ячейки
				OUT_walls[i] = get2bit(OUT_walls[i - 1], 1) << 6;
			}

			if (j == 0)
			{ // если это западная стена здания
				OUT_walls[i] += binrand(1) << 4;
			}
			else
			{ // если стена не западная
				// берём информацию о стене из соседней ячейки
				OUT_walls[i] += get2bit(OUT_walls[i - len], 0) << 4;
			}

			if (i % len == len - 1)
			{ // если это северная стена здания
				if  (IN_type == 0) // если мы имеем дело с обычным домом
				{
					OUT_walls[i] += binrand(1) << 2;
				}
				else // если дом, который мы генерируем - это блок другого здания
				{				
					OUT_walls[i] += binrand(3) << 2;
				}
			}
			else // если это не северная стена здания
			{
				OUT_walls[i] += binrand(3) << 2;
			}

			if (j == wid - 1)
			{ // если это восточная стена здания
				OUT_walls[i] += binrand(1);
			}
			else
			{ // если это не восточная стена здания
				OUT_walls[i] += binrand(3);
			}
		}
		else
		{ // 2й этаж и выше
			if (i % len == 0)
			{ // если это южная стена
				if  (IN_type <= 1)
				{ // если это крайняя стена
					OUT_walls[i] = binrand(2) << 6;
				}
				else
				{ // если эта стена прилегает к другому зданию
					OUT_walls[i] = binrand(3) << 6;
				}
			}
			else
			{ // если это не южная стена, берём информацию о стене из соседней ячейки
				OUT_walls[i] = get2bit(OUT_walls[i - 1], 1) << 6;
			}

			if (i % len == len - 1)
			{ // если это северная стена
				if (IN_type == 0)
				{// если это крайняя стена
					OUT_walls[i] += binrand(2) << 2;
				}
				else
				{ // если эта стена прилегает к соседнему зданию
					OUT_walls[i] += binrand(3) << 2;
				}
			}
			else
			{ // если это не северная стена здания
				OUT_walls[i] += binrand(3) << 2;
			}

			if (j == 0)
			{ // если это восточная стена
				OUT_walls[i] += binrand(2) << 4;
			}
			else
			{	// если стена не восточная, то берём информацию о стене из соседней ячейки
				OUT_walls[i] += get2bit(OUT_walls[i - len], 0) << 4;
			}

			if (j == wid - 1)
			{ // если это западная стена
				OUT_walls[i] += binrand(2);
			}
			else
			{
				OUT_walls[i] += binrand(3);
			}
		}

		// --------------------
		if (i % len == len - 1)
		{	// если мы дошли до края по X, становимся на начало следующей строки по Y
			j++;
		}
		if (j == wid)
		{ // если мы дошли до края по Y, то начинаем следующий этаж (по Z)
			nfloor++;
			j = 0;
		}
	}
}

// ---------------------лестницы-----------------------------
void genStairs(byte* OUT_stCoords, int* INOUT_Walls, int len, int wid, int hei, int stColls)
{
	// ограничение места действия переменных i, j и accepted
	{
		int i = 0, j;
		bool accepted;
		// пока не расставим все лестницы
		while (i < stColls)
		{
			// располагаем новую лестницу
			OUT_stCoords[i * 2] = myrand_rand() % len;
			OUT_stCoords[i * 2 + 1] = myrand_rand() % (wid - 1);
			accepted = true;

			// сверяемся с создаными лестницами
			for (j = 0; j < i; j++)
			{
				// если лестницы на одной линии
				if (OUT_stCoords[i * 2] == OUT_stCoords[j * 2])
				{	
					// если лестницы перекрывают друг-друга
					if (OUT_stCoords[i * 2 + 1] <= OUT_stCoords[j * 2 + 1] + 1
						&&
						OUT_stCoords[i * 2 + 1] >= OUT_stCoords[j * 2 + 1] - 1)
					{
						// повторить поиск места для этой лестницы
						accepted = false;
					}
				}
			}

			// если новая лестница подходит
			if (accepted)
			{
				// переходим к следующей лестнице
				i++;
			}
		}
	}
	// уже тут переменные i, j и accepted считаются необъявленными

	// располагаем стены вокруг лестниц
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

			// лестница не божет быть у западной стены, т.ч. проверка не нужна
			change2bit(&INOUT_Walls[addr + len], 2, 2);
		}
	}
}

// добавить новый участок пола
void NewFloor(byte* INOUT_flArr, byte IN_flCount, byte x1, byte y1, byte x2, byte y2)
{
	INOUT_flArr[IN_flCount * 4] = x1;
	INOUT_flArr[IN_flCount * 4 + 1] = y1;
	INOUT_flArr[IN_flCount * 4 + 2] = x2;
	INOUT_flArr[IN_flCount * 4 + 3] = y2;
}

// создаём низкополигональный пол
// возвращает количество участков пола
byte gfloor(byte* OUT_flArr, byte* IN_stArr, byte IN_stCount, byte IN_sizeX, byte IN_sizeY)
{
	// количество кусков пола
	byte OUT_flCount = 0;

	// создаём временный массив лестниц
	byte* stairs = new byte[IN_stCount];
	for (byte i = 0; i < IN_stCount; i++)
		stairs[i] = i;

	// в зависимости от длины и ширины здания делим его вдоль или поперёк
	if (IN_sizeX < IN_sizeY)
	{
		// сортируем лестницы в нужном порядке
		// сортировка методом простого выбора
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

		// добавляем все куски пола
		int prew_x = IN_sizeX, prew_y = -1;
		for (byte i = 0; i < IN_stCount; i++)
		{
			// если продолжаем линию
			if (IN_stArr[stairs[i] * 2 + 1] == prew_y)
			{
				// если есть зазор
				if (IN_stArr[stairs[i] * 2] != prew_x)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, IN_stArr[stairs[i] * 2], prew_y + 1);
					prew_x = IN_stArr[stairs[i] * 2] + 1;
				}
			}
			else // новая линия
			{
				// если нужно, заканчиваем предыдущую линию
				if (prew_x < IN_sizeX)
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, IN_sizeX, prew_y + 1);

				// если есть зазор, добавляем линию
				if (IN_stArr[stairs[i] * 2 + 1] > prew_y + 1)
					NewFloor(OUT_flArr, OUT_flCount++, 0, prew_y + 1, IN_sizeX, IN_stArr[stairs[i] * 2 + 1]);

				// если лестница не у самого края добавляем пол
				if (IN_stArr[stairs[i] * 2] != 0)
				{
					NewFloor(OUT_flArr, OUT_flCount++, 0, IN_stArr[stairs[i] * 2 + 1], IN_stArr[stairs[i] * 2], IN_stArr[stairs[i] * 2 + 1] + 1);
				}
				prew_y = IN_stArr[stairs[i] * 2 + 1];
				prew_x = IN_stArr[stairs[i] * 2] + 1;
			}
		}
		// если нужно, заканчиваем предыдущую линию
		if (prew_x < IN_sizeX)
			NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, IN_sizeX, prew_y + 1);

		// если есть зазор, добавляем линию
		if (IN_sizeY > prew_y + 1)
			NewFloor(OUT_flArr, OUT_flCount++, 0, prew_y + 1, IN_sizeX, IN_sizeY);
	}
	else // SIZE_X >= SIZE_Y
	{
		// сортируем лестницы в нужном порядке
		// сортировка методом простого выбора
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

		// добавляем все куски пола
		int prew_y = IN_sizeY, prew_x = -1;
		for (byte i = 0; i < IN_stCount; i++)
		{
			// если продолжаем линию
			if (IN_stArr[stairs[i] * 2] == prew_x)
			{
				// если есть зазор
				if (IN_stArr[stairs[i] * 2 + 1] != prew_y)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, prew_x + 1, IN_stArr[stairs[i] * 2 + 1]);
					prew_y = IN_stArr[stairs[i] * 2 + 1] + 1;
				}
			}
			else // новая линия
			{
				// если нужно, заканчиваем предыдущую линию
				if (prew_y < IN_sizeY)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, prew_x + 1, IN_sizeY);
				}

				// если есть зазор, добавляем линию
				if (IN_stArr[stairs[i] * 2] > prew_x + 1)
				{
					NewFloor(OUT_flArr, OUT_flCount++, prew_x + 1, 0, IN_stArr[stairs[i] * 2], IN_sizeY);
				}

				// если лестница не у самого края добавляем пол
				if (IN_stArr[stairs[i] * 2 + 1] != 0)
				{
					NewFloor(OUT_flArr, OUT_flCount++, IN_stArr[stairs[i] * 2], 0, IN_stArr[stairs[i] * 2] + 1, IN_stArr[stairs[i] * 2 + 1]);
				}
				prew_x = IN_stArr[stairs[i] * 2];
				prew_y = IN_stArr[stairs[i] * 2 + 1] + 1;
			}
		}
		// если нужно, заканчиваем предыдущую линию
		if (prew_y < IN_sizeY)
		{
			NewFloor(OUT_flArr, OUT_flCount++, prew_x, prew_y, prew_x + 1, IN_sizeY);
		}

		// если есть зазор, добавляем линию
		if (IN_sizeX > prew_x + 1)
		{
			NewFloor(OUT_flArr, OUT_flCount++, prew_x + 1, 0, IN_sizeX, IN_sizeY);
		}
	}

	// высвобождаем память временного массива
	delete[] stairs;

	return OUT_flCount;
}

// строим массив блоков
void ghouse(int* OUT_houseParts, int IN_type, byte len, byte wid, int hei, byte stCount, int seed)
{
	myrand_seed = seed;
	int offset = 0;
	int* walls = new int[len * wid * hei];
	byte* stairs = new byte[stCount * 2];
	byte* floors = new byte[stCount * 12 + 4];
	byte flCount;

	// создаём всю информацию о здании
	generichouse(walls, IN_type, len, wid, hei);
	genStairs(stairs, walls, len, wid, hei, stCount);
	flCount = gfloor(floors, stairs, stCount, len, wid);

	// заносим информацию о лестницах
	for (int i = 0; i < stCount * 2; i++)
	{
		OUT_houseParts[i] = stairs[i];
	}
	offset = stCount * 2;

	// заносим информацию о количестве участков пола
	OUT_houseParts[offset] = flCount;
	offset++;

	// заносим информацию о кусках пола
	for (int i = 0; i < flCount * 4; i++)
	{
		OUT_houseParts[offset + i] = floors[i];
	}
	offset += flCount * 4;

	// заносим информацию о стенах здания
	for (int i = 0; i < len * wid * hei; i++)
	{
		OUT_houseParts[offset + i] = walls[i];
	}

	// очищаем память
	delete[] walls;
	delete[] stairs;
	delete[] floors;
}

//---------------------------------------------------------------------------------------------------------
// тут ведётся проверка, видна ли конкретная ячейка дома в данный момент.

void genview(int* house, int* cells, vector* info)
{
	// info[0] - вектор, который содержит размер дома в блоках
	// info[1] - вектор, который содержит размер блока
	// info[2] - вектор, содержащий относительные координаты игрока
	// house - массив, содержащий информацию о ячейках дома
	// cells - массив, содержащий информацию о видимости каждой ячейки (будет заполняться в функции)

	int sx = (int)floor(info[0].x * info[1].x * 0.5); // полуширина дома
	int sy = (int)floor(info[0].y * info[1].y * 0.5); // соответственно ...
	int sz = (int)floor(info[0].z * info[1].z * 0.5); // ..
	int vx, vy, vz;

	if (info[2].x > sx)
	{ // мы видим одну из граней дома
		vx = 1;
	} else if (info[2].x < -sx)
	{ // мы видим противоположную предыдущей грань
		vx = -1;
	}
	else
	{ // не видим ни одну из предыдущих двух
		vx = 0;
	}

	if (info[2].y > sy)
	{ // мы видим одну из граней дома
		vy = 1;
	}
	else if (info[2].y < -sy)
	{ // мы видим противоположную предыдущей грань
		vy = -1;
	}
	else
	{ // не видим ни одну из предыдущих двух
		vy = 0;
	}

	if (info[2].z > sz)
	{ // мы видим крышу дома
		vz = 1;
	}
	else if (info[2].z < -sz)
	{ // мы под землёй
		vz = -1;
	}
	else
	{ // мы ниже крыши, но не под землёй
		vz = 0;
	}

	int x, y, z, lenx = info[0].x, leny = info[0].x, addr;

	if (vx == 0 && vy == 0 && vz == 0)
	{ // если мы внутри дома
		for (z = 0; z < info[0].z; z++)
			for (y = 0; y < leny; y++)
				for (x = 0; x < lenx; x++)
					cells[z * lenx * leny + y * lenx + x] = 2; // тупо делаем "показать всё"
	}
	else
	{
		for (z = 0; z < info[0].z; z++)
			for (y = 0; y < leny; y++)
				for (x = 0; x < lenx; x++)
				{
					addr=z*lenx*leny+y*lenx+x;
					cells[addr] = 0; // по умолчанию ячейка скрыта
					if (vx == 1 && x == lenx - 1) cells[addr] = 2; // но если она с краю - появляем её
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
