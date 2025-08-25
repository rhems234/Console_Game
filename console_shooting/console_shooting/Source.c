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

int screen_index;

HANDLE screen[2];
// 기본 세팅
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
	COORD pos = { x , y, };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
// 기본 세팅

// 플레이어 총알 --------------------------------------------------------
#define MAXBULLET 5
typedef struct {
	int x;
	int y;
	int active;
	int dx;
	int dy;
} Bullet;

Bullet bullets[MAXBULLET];

void initbullets() {
	for (int i = 0; i < MAXBULLET; i++) {
		bullets[i].active = 0;
	}
}

void fireBullet(int playerX, int playerY) {
	for (int i = 0; i < MAXBULLET; i++) {
		if (!bullets[i].active) {
			bullets[i].x = playerX;
			bullets[i].y = playerY;
			bullets[i].active = 1;
			bullets[i].dx = 0;
			bullets[i].dy = -1;
			break;
		}
	}
}

void updateBullets() {
	for (int i = 0; i < MAXBULLET; i++) {
		if (bullets[i].active) {
			bullets[i].x += bullets[i].dx;
			bullets[i].y += bullets[i].dy;

			if (bullets[i].x >= 120 || bullets[i].x < 0 || 
				bullets[i].y >= 60 || bullets[i].y < 0) {
				bullets[i].active = 0;
			}
		}
	}
}

void renderBullets() {
	for (int i = 0; i < MAXBULLET; i++) {
		if (bullets[i].active) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
			render(bullets[i].x, bullets[i].y, "▲");
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
		}
	}
}
// 플레이어 총알 --------------------------------------------------------

// 적 출현
int enemySpawnCounter = 0;
#define ENEMY_SPAWN_INTERVAL 30
#define MAX_ENEMY 10
struct {
	int x;
	int y;
	int active;
} enemy[MAX_ENEMY];

void initEnemies() {
	for (int i = 0; i < MAX_ENEMY; i++) {
		enemy[i].active = 0;
	}
}

void spawnEnemy(int x, int y) {
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (!enemy[i].active) {
			enemy[i].x = x;
			enemy[i].y = y;
			enemy[i].active = 1;
			break;
		}
	}
}

void updateEnemy() {
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (enemy[i].active) {
			enemy[i].y++;

			if (enemy[i].y >= 120) {
				enemy[i].active = 0;
			}
		}
	}
}

void renderEnemy() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);

	for (int i = 0; i < MAX_ENEMY; i++) {
		if (enemy[i].active) {
			render(enemy[i].x, enemy[i].y, "■");
		}
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void checkBulletsCollision() {
	for (int i = 0; i < MAXBULLET; i++) {
		if (bullets[i].active) {
			for (int j = 0; j < MAX_ENEMY; j++) {
				if (enemy[i].active && bullets[i].x == enemy[j].x &&
					bullets[i].y == enemy[i].y) {
					bullets[i].active = 0;
					enemy[i].active = 0;
					break;
				}
			}
		}
	}
}

// 적 출현

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


// 1번 게임시작
void Game_Start(int startX, int startY) {

	int x = startX;
	int y = startY;

	initbullets();
	initEnemies();
	initialize();

	while (1) {

		// 게임종료
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}

		// 플레이어 이동
		if (GetAsyncKeyState(VK_LEFT) & 0x0001) {
			if(x > 0)
			x -= 2;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x0001) {
			x += 2;
		}

		// 플레이어 총알 발사
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			fireBullet(x, y);
		}

		enemySpawnCounter++;
		if (enemySpawnCounter >= ENEMY_SPAWN_INTERVAL) {
			enemySpawnCounter = 0;

			int enemyX = rand() % 60;
			spawnEnemy(enemyX, 0);
		}

		updateBullets();
		updateEnemy();
		checkBulletsCollision();

		flip();

		clear();

		renderBullets();
		renderEnemy();

		render(x, y, "★");

		Sleep(20);

	}
	release();
}

void Start_Menu() {
	system("cls");

	char map[TITLE_HEIGHT][TITLE_WIDTH] = {
		{1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1},
		{1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
		{1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0},
		{0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0},
		{1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0}
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
		system("cls");
		Game_Start(56 ,50);
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

	system("mode con:cols=120 lines=60");

	Start_Menu();


	return 0;
}