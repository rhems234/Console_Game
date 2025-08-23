#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <conio.h>

#define SIZE 10000
#define TITLE_WIDTH 11
#define TITLE_HEIGHT 5
#define HELP_WIDTH 15
#define HELP_HEIGHT 5

#define SPACE 0x20
#define UP 72
#define LEFT 75
#define RIGHT 77
#define DOWN 80

int screen_index;

HANDLE screen[2];

void initialize() {
	CONSOLE_CURSOR_INFO cursor;

	// 화면 버퍼를 2개 생성합니다.
	screen[0] = CreateConsoleScreenBuffer
	(
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		CONSOLE_TEXTMODE_BUFFER, NULL
	);

	screen[1] = CreateConsoleScreenBuffer
	(
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		CONSOLE_TEXTMODE_BUFFER, NULL
	);

	cursor.dwSize = 1;
	cursor.bVisible = FALSE;

	SetConsoleCursorInfo(screen[0], &cursor);
	SetConsoleCursorInfo(screen[1], &cursor);

}

void flip() {
	SetConsoleActiveScreenBuffer(screen[screen_index]);

	screen_index = !screen_index;

	// 1 <- 0
	// 0 <- 1
}

void clear() {
	COORD position = { 0, 0 };

	DWORD dword;

	CONSOLE_SCREEN_BUFFER_INFO console;

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(handle, &console);

	int width = console.srWindow.Right - console.srWindow.Left + 1;
	int height = console.srWindow.Bottom - console.srWindow.Top + 1;

	FillConsoleOutputCharacter(screen[screen_index], ' ', width * height, position, &dword);

}

void release() {

	CloseHandle(screen[0]);
	CloseHandle(screen[1]);
}

void render(int x, int y, const char* text) {
	DWORD dword;
	COORD position = { x , y };

	SetConsoleCursorPosition(screen[screen_index], position);
	WriteFile(screen[screen_index], text, strlen(text), &dword, NULL);
}

void move_main(int x, int y) {
	COORD pos = { x , y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// 2번 도움말
void Help() {
	system("cls");

	char map[HELP_HEIGHT][HELP_WIDTH] = {
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1},
		{1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1},
		{1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1},
		{1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0},
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0}
	};

	for (int i = 0; i < HELP_HEIGHT; i++) {
		move_main(50, 10 + i);
		for (int j = 0; j < HELP_WIDTH; j++) {
			if (map[i][j] == 1) {
				printf("■");
			}
			else {
				printf("  ");
			}
		}
		printf("\n");
	}

	move_main(42, 20);
	printf("도도도도도도도도도도도도도도도도도도도도도도\n");
	move_main(42, 21);
	printf("움움움움움움움움움움움움움움움움움움움움움움\n");
	move_main(42, 22);
	printf("말말말말말말말말말말말말말말말말말말말말말말\n");

}

void init_game() {
	system("cls");
}

// 1번 게임시작
void Game_Start() {
	//init_game();

	int x = 0;
	int y = 0;

	initialize();

	while (1) {
		char ch = _getch();

		if (ch == 0 || ch == -32) {
			ch = _getch();
		}

		if (ch == 27) {
			break;
		}

		if (GetAsyncKeyState(VK_UP) & 0x0001) {
			y--;
		}
		if (GetAsyncKeyState(VK_LEFT) & 0x0001) {
			x -= 2;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x0001) {
			x += 2;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x0001) {
			y++;
		}
		flip();

		clear();

		render(x, y, "★");

	}
	release();
}

void Start_Menu() {
	system("cls");

	char map[TITLE_HEIGHT][TITLE_WIDTH] = {
		{1, 1, 1, 0, 1, 0, 1, 0,1, 1, 1},
		{1, 0, 0, 0, 1, 0, 1, 0,0, 1, 0},
		{1, 1, 1, 0, 1, 1, 1, 0,0, 1, 0},
		{0, 0, 1, 0, 1, 0, 1, 0,0, 1, 0},
		{1, 1, 1, 0, 1, 0, 1, 0,0, 1, 0}
	};

	// 메인 타이틀
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);

	for (int i = 0; i < TITLE_HEIGHT; i++) {
		move_main(50, 10 + i);
		for (int j = 0; j < TITLE_WIDTH; j++) {
			if (map[i][j] == 1) {
				printf("■");
			}
			else {
				printf("  ");
			}
		}
		printf("\n");
	}

	// 시작
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); // 녹색
	move_main(50, 20);
	printf("▶ Start Game(1)");

	// 도움말
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); // 녹색
	move_main(50, 25);
	printf("▶ Help(2)");

	// 종료
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); // 기본색
	move_main(50, 30);
	printf("▶ Exit(3)\n");

	int number = 0;

	move_main(50, 35);
	printf("번호를 입력해주세요 : ");
	scanf_s("%d", &number);

	if (number == 1) {
		// 게임시작
		Game_Start();
	}
	else if (number == 2) {
		// 도움말
		Help();
	}
	else if (number == 3) {
		// 종료
		printf("exit");
		
	}
}

int main()
{
 // 콘솔 슈팅 게임 - 메인 화면

	Start_Menu();


	return 0;
}