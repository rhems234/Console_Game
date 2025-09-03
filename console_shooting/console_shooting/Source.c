#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <conio.h>
#include <time.h>

void Start_Menu();
void EndGame();

#define SIZE 10000

#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 60

#define TITLE_WIDTH 11
#define TITLE_HEIGHT 5

#define HELP_WIDTH 15
#define HELP_HEIGHT 5

#define MAP_WIDTH 40
#define MAP_HEIGHT 60

#define END_WIDTH 48
#define END_HEIGHT 5

#define SPACE 0x20

int screen_index;
int total_score = 0;
int heart = 4;
int gameover = 0;
time_t startTime = 0;
double playTime = 0;

int prevY = 0;

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
	SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
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
				bullets[i].y >= 60 || bullets[i].y < 0)
			{
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

void updateEnemy(int playerX, int playerY) {

	for (int i = 0; i < MAX_ENEMY; i++) {
		if (enemy[i].active) {
			enemy[i].y++;
			
			// 플레이어를 지나쳤을 시 라이프 감소
			if (enemy[i].y >= 60) {
				enemy[i].active = 0;
				heart--;

			}

			// 적 충돌 시 라이프 감소
			if (enemy[i].x == playerX && enemy[i].y == playerY) {
				enemy[i].active = 0;
				heart--;

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
		if (bullets[i].active) { // 총알 활성화 상태
			for (int j = 0; j < MAX_ENEMY; j++) {
				if(enemy[j].active) {
					if (bullets[i].y - enemy[j].y <= 1 &&
						bullets[i].x >= enemy[j].x &&
						bullets[i].x < enemy[j].x + 1)
					{
						bullets[i].active = 0;
						enemy[j].active = 0;

						total_score += 100;

						break;
					}
				}
			}
		}
	}
}
// 적 출현

// 게임 맵
int offsetX = (SCREEN_WIDTH - MAP_WIDTH) / 2;
int offsetY = 0;

void GameMap() {
	char map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

	// 맵 초기화
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (j == 0 || j == MAP_WIDTH - 1) {
				map[i][j] = 1;
			}
			else  {
				map[i][j] = 0;
			}
		}
	}

	// 맵 출력
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (map[i][j] == 1) {
				render(offsetX + j, offsetY + i, "■");
			}
		}
	}

}

// 게임 종료 후
void EndGame() {
	system("cls");

	char text[END_HEIGHT][END_WIDTH] = {
		{1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
		{1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0},
		{1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
		{1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1}
	};

	for (int i = 0; i < END_HEIGHT; i++) {
		move_main(10, 13 + i);
		for (int j = 0; j < END_WIDTH; j++) {
			if (text[i][j] == 1) {
				printf("■");
			}
			else {
				printf("  ");
			}
		}
		printf("\n");
	}

	move_main(50, 25);
	printf("점수 : %d\n", total_score);

	move_main(50, 27);
	printf("플레이 시간 : %.f초\n", playTime);

	move_main(35, 30);
	printf("스페이스바를 누르면 시작 화면으로 돌아갑니다...");

	while (1) {

		if (GetAsyncKeyState(VK_SPACE) & 0x0001) {
			heart = 4;
			total_score = 0;
			Start_Menu();
			return;
		}

	}

}

// 점수, 목숨, 가이드
void player_info() {
	// printf는 루프 종료로 인한 출력으로 인해 sprintf_s를 통한 render 방식으로 출력
	char score[32];
	char heartStr[16];
	char key_guide[64];
	char attack_guide[64];
	char pause_guide[64];

	// 점수 표시
	sprintf_s(score, sizeof(score), "점수 : %-5d", total_score);
	render(1, 1, score);

	// 가이드
	sprintf_s(key_guide, sizeof(key_guide), "조작 : ←, →");
	sprintf_s(attack_guide, sizeof(attack_guide), "공격 : Space bar");
	sprintf_s(pause_guide, sizeof(pause_guide), "일시정지 : ESC");

	render(100, 54, key_guide);
	render(100, 56, attack_guide);
	render(100, 58, pause_guide);

	// 목숨 표시
	switch (heart) {
	case 4:
		sprintf_s(heartStr, sizeof(heartStr), "♥ ♥ ♥ ♥");
		break;
	case 3:
		sprintf_s(heartStr, sizeof(heartStr), "♥ ♥ ♥");
		break;
	case 2:
		sprintf_s(heartStr, sizeof(heartStr), "♥ ♥");
		break;
	case 1:
		sprintf_s(heartStr, sizeof(heartStr), "♥");
		break;
	case 0:
		sprintf_s(heartStr, sizeof(heartStr), "  ");
		gameover = 1;
		break;
	default:
		sprintf_s(heartStr, sizeof(heartStr), "  ");
		break;
	}

	render(1, 3, heartStr);

	// 플레이 타임
	char timeStr[32];
	time_t endTime = time(NULL);
	playTime = difftime(endTime, startTime);

	sprintf_s(timeStr, sizeof(timeStr), "플레이 시간 : %.f초", playTime);

	render(1, 58, timeStr);

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

	move_main(50, 20);
	printf("조작 : ←, →\n");
	move_main(50, 21);
	printf("공격 : Space bar\n");
	move_main(50, 22);
	printf("일시정지 : ESC\n");
	move_main(50, 25);
	printf("메인 메뉴로 돌아갈려면 BackSpace 키를 누르세요...");

	while (1) {
		if (GetAsyncKeyState(VK_BACK) & 0x0001) {
			Start_Menu();
			return;
		}
		Sleep(10);
	}

}

// 1번 게임시작
void Game_Start(int startX, int startY) {

	int x = startX;
	int y = startY;

	DWORD lastFiretime = 0;
	int fireDelay = 300;

	int paused = 0;
	startTime = time(NULL);

	initialize();
	initbullets();
	initEnemies();

	while (1) {

		char text[16];

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			paused = !paused;
			Sleep(100);
		}

		if (paused) {

			sprintf_s(text, sizeof(text), "PAUSE");
			render(110, 0, text);

			flip();
			Sleep(100);
			continue;
		}

		// 플레이어 이동
		if (GetAsyncKeyState(VK_LEFT) & 0x0001) {
			if (x > MAP_WIDTH + 2) {
				x -= 2;
			}
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x0001) {
			if (x < (MAP_WIDTH - 2) * 2) {
				x += 2;
			}
		}

		// 플레이어 총알 발사
		if ((GetAsyncKeyState(VK_SPACE) & 0x8000)) {
			
			DWORD currentTime = GetTickCount64();

			if (currentTime - lastFiretime >= fireDelay) {
				fireBullet(x, y);
				lastFiretime = currentTime;
			}
		}

		enemySpawnCounter++;
		if (enemySpawnCounter >= ENEMY_SPAWN_INTERVAL) {
			enemySpawnCounter = 0;

			int enemyX = (rand() % ((MAP_WIDTH-2)/2)) * 2;
			spawnEnemy(offsetX + enemyX, 0);
		}

		updateBullets();
		updateEnemy(x, y);
		checkBulletsCollision();

		clear();

		GameMap();
		renderBullets();
		renderEnemy();

		render(x, y, "■");

		player_info();

		if (heart <= 0) {
			break;
		}

		flip();
		
		Sleep(30);

	}

	release();

	EndGame();
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
		printf("Start Game(1)");

		// 도움말
		move_main(50, 25);
		printf("Help(2)");

		// 종료
		move_main(50, 30);
		printf("Exit(3)");

		int y = 0;

		while (1) {

			move_main(47, 20 + prevY);
			printf("  ");

			if (GetAsyncKeyState(VK_UP) & 0x0001) {
				if (y > 0) {
					y -= 5;
				}
			}

			if (GetAsyncKeyState(VK_DOWN) & 0x0001) {
				if (y < 10) {
					y += 5;
				}
			}

			move_main(47, 20 + y);
			printf("▶");

			prevY = y;

			if (GetAsyncKeyState(VK_SPACE) & 0x0001) {

				if (y == 0) {
					system("cls");
					Game_Start(offsetX + MAP_WIDTH / 2, offsetY + MAP_HEIGHT - 5);
					break;
				}

				if (y == 5) {
					Help();
					break;
				}
				if (y == 10) {
					move_main(47, 35);
					printf("게임을 종료합니다. ");
					exit(0);
					break;
				}
			}

			Sleep(100);
		}

}

int main()
{
 // 콘솔 슈팅 게임 - 메인 화면

	system("mode con:cols=120 lines=60");

	while (1) {
		Start_Menu();
	}

	return 0;
}