#include "DxLib.h"
#include "stdio.h"
#include "math.h"
#include "time.h"
#include "Keyboard.h"


#define SHOT_NUM 200
#define ENEMY_NUM 7
#define X_SIZE 700
#define Y_SIZE 768
#define WinX_SIZE 1024
#define WinY_SIZE 768
#define ENEMY_HP 10


#define GAMEOVER 1
#define CONTINUE 0
#define GAMELOOP 0
#define BACK_TITLE 1
#define GAMEEND 2

#define GAMESTART 0
#define TUTORIAL 1
#define GAMEEXIT 2

#define DEF_SET_XY -100

#define P_MOVE_SPEED 8

#define E_SHOT_SPEED 10
#define E_SHOT_NUM 5
#define E_SHOT_BET 50
#define E_MAX_SHOT_COUNT 10

#define WHITE GetColor(255,255,255)
#define BLACK GetColor(0,0,0)
#define RED GetColor(255,0,0)
#define MAGENTA GetColor(229, 0, 134)
#define BLUE GetColor(0,0,255)
#define GREEN GetColor(0,255,0)

#define MASSAGE1 750, 100
#define MASSAGE2 750, 116
#define MASSAGE3 750, 132
#define MASSAGE4 750, 148
typedef struct shot{
	double x = -100;
	double y = -100;
	int flag = 0;
	double radian = 0;
}SHOT;

typedef struct player{
	double x = X_SIZE / 2;
	double y = 600;
	int handle;
	int shot_handle;
	int shift_mode = FALSE;
	int stop_flag = FALSE;
	double stop_time;
	int timetmp;
	int keytmp;
	SHOT shot[SHOT_NUM];
}PLAYER;

typedef struct enemy{
	double x = DEF_SET_XY;
	double y = DEF_SET_XY;
	int flag = FALSE;
	int hp = ENEMY_HP;
	int shot_mode = 0; //0:move, 1:shot
	int shot_time;
	int shot_count = 0;
	SHOT shot[SHOT_NUM];
}ENEMY;

typedef struct result{
	int del_em = 0;
	int score;
	int start_game_time;
}RESULT;

typedef struct menu{
	int x, y;       // ���W�i�[�p�ϐ�
	char name[128]; // ���ږ��i�[�p�ϐ�
}MENU_CONT;

typedef struct title_menu{
	int x, y;
	char name[128];
}MENU_TITLE;

void select_title_menu(PLAYER *player, ENEMY enemy[], RESULT *result);
void set_data(PLAYER *player, ENEMY enemy[], RESULT *result);
void main_proc(PLAYER *player, ENEMY enemy[], RESULT *result);
void shot_judge(int shot_flag, PLAYER *player, ENEMY enemy[], RESULT *result);
void enemy_proc(int enemy_flag, ENEMY enemy[]);
void enemy_shot_judge(int shot_flag, PLAYER player, ENEMY enemy[]);
void player_move_proc(PLAYER *player);
void draw_proc(PLAYER player, ENEMY enemy[], RESULT result);
int gameover_judge_proc(PLAYER *player, ENEMY enemy[], RESULT *result);
int gameover_judge(PLAYER player, ENEMY enemy[]);
int cont_ask(RESULT result);
int title_proc(void);
void tutorial_proc(void);


int FontHandle;
int BigHandle;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	int graoh_judge;
	ChangeWindowMode(TRUE); //�E�B���h�E���[�h�ɐݒ�(640, 480)
	graoh_judge = SetGraphMode(1024, 768, 32);
	DxLib_Init();   // DX���C�u��������������
	SetDrawScreen(DX_SCREEN_BACK); //�`���𗠉�ʂɐݒ�

	PLAYER player;
	ENEMY enemy[ENEMY_NUM];
	RESULT result;

	FontHandle = CreateFontToHandle(NULL, 40, 3);
	BigHandle = CreateFontToHandle(NULL, 80, 10);

	select_title_menu(&player, enemy, &result);

	DxLib_End();    // DX���C�u�����I������
	return 0;
}

void select_title_menu(PLAYER *player, ENEMY enemy[], RESULT *result)
{
	while (1){
		switch (title_proc()){
		case GAMESTART:
			set_data(player, enemy, result);
			main_proc(player, enemy, result);
			break;
		case TUTORIAL:
			tutorial_proc();
			break;
		case GAMEEXIT:
			DxLib_End();    // DX���C�u�����I������
			exit(1);
		}
	}
}

void set_data(PLAYER *player, ENEMY enemy[], RESULT *result)
{
	int i, j;

	player->x = X_SIZE / 2;
	player->y = Y_SIZE / 2 + 300;
	player->shift_mode = player->stop_flag = FALSE;
	player->stop_time = 5000;
	player->keytmp;
	for (i = 0; i < SHOT_NUM; i++){
		player->shot[i].x = player->shot[i].y = DEF_SET_XY;
		player->shot[i].flag = FALSE;
	}

	for (i = 0; i < ENEMY_NUM; i++) {
		enemy[i].x = enemy[i].y = DEF_SET_XY;
		enemy[i].flag = enemy[i].shot_mode = FALSE;
		enemy[i].shot_count = 0;
		enemy[i].hp = ENEMY_HP;
		for (j = 0; j < SHOT_NUM; j++){
			enemy[i].shot[j].x = enemy[i].shot[j].y = DEF_SET_XY;
			enemy[i].shot[j].flag = FALSE;
		}
	}

	result->del_em = 0;
	result->score = 0;
	result->start_game_time = GetNowCount();
}

void main_proc(PLAYER *player, ENEMY enemy[], RESULT *result)
{
	int enemy_judge_time = GetNowCount();

	while (1) {
		if (ProcessMessage() != 0)
			break;

		ClearDrawScreen(); //��ʂ�����

		player_move_proc(player);

		if (CheckHitKey(KEY_INPUT_X) == TRUE)
			shot_judge(TRUE, player, enemy, result);
		else
			shot_judge(FALSE, player, enemy, result);

		if (player->stop_flag == FALSE) {
			if (GetNowCount() - enemy_judge_time > 200) {
				if (rand() % 2 == 0)
					enemy_proc(TRUE, enemy);
				else
					enemy_proc(FALSE, enemy);
				enemy_judge_time = GetNowCount();
			}
			else
				enemy_proc(FALSE, enemy);
			enemy_shot_judge(FALSE, *player, enemy);
		}

		draw_proc(*player, enemy, *result);

		ScreenFlip(); //����ʂ�\��ʂɔ��f

		if (gameover_judge_proc(player, enemy, result) == BACK_TITLE)
			break;
	}
}

void shot_judge(int shot_flag, PLAYER *player, ENEMY enemy[], RESULT *result)
{
	int i, j;

	for (i = 0; i < SHOT_NUM; i++) {
		if (player->shot[i].flag == TRUE) {
			player->shot[i].y -= 10;
			for (j = 0; j < ENEMY_NUM; j++) {
				if (player->shot[i].x + 10> enemy[j].x - 30 && player->shot[i].x - 10 < enemy[j].x + 30) {
					if (player->shot[i].y + 10 > enemy[j].y - 30 && player->shot[i].y - 10 < enemy[j].y + 30) {
						enemy[j].hp -= 1;
						player->shot[i].flag = FALSE; /* �e���� */
						if (enemy[j].hp < 0) {
							enemy[j].flag = 0;
							enemy[j].hp = ENEMY_HP;
							enemy[j].shot_mode = 0;
							enemy[j].x = enemy[j].y = -DEF_SET_XY;
							enemy[j].shot_count = 0;
							result->del_em++;
							result->score = GetNowCount() - result->start_game_time;
						}
					}
				}
			}
			if (player->shot[i].y <= 0) {
				player->shot[i].flag = 0;
				player->shot[i].x = player->shot[i].y = -100;
			}
		}
		if (player->shot[i].flag == 0 && shot_flag == 1) {
			player->shot[i].flag = 1;
			player->shot[i].x = player->x;
			player->shot[i].y = player->y;
			shot_flag = 0;
		}
	}
}

void enemy_proc(int enemy_flag, ENEMY enemy[])
{
	int i, val;

	for (i = 0; i < ENEMY_NUM; i++) {
		if (enemy[i].flag == 1 && enemy[i].shot_mode == 0) {
			enemy[i].y += 5;
			if (enemy[i].y >= 50) {
				enemy[i].shot_mode = 1;
				enemy[i].shot_time = GetNowCount();
			}
		}
	}

	if (enemy_flag == 1) {
		for (i = 0; i < ENEMY_NUM; i++) {
			val = rand() % ENEMY_NUM;
			if (enemy[val].flag == 0) {
				enemy_flag = 0;
				enemy[val].flag = 1;
				enemy[val].x = val * 100 + 50;
				enemy[val].y = -30;
				break;
			}
		}
	}
}

void enemy_shot_judge(int shot_flag, PLAYER player, ENEMY enemy[])
{
	int i, j;
	int flag = 0;

	for (i = 0; i < ENEMY_NUM; i++) {
		flag = TRUE;
		for (j = 0; j < SHOT_NUM; j++) {
			//�����e�����W�X�V
			if (enemy[i].shot[j].flag == TRUE) {
				enemy[i].shot[j].x += E_SHOT_SPEED * cos(enemy[i].shot[j].radian);
				enemy[i].shot[j].y += E_SHOT_SPEED * sin(enemy[i].shot[j].radian);
				if (enemy[i].shot[j].x > X_SIZE || enemy[i].shot[j].y > Y_SIZE) {
					enemy[i].shot[j].x = enemy[i].shot[j].y = -100;
					enemy[i].shot[j].flag = FALSE;
				}
			}

			//�V�K�e���� ���e����100
			if (enemy[i].shot_mode == TRUE && GetNowCount() - enemy[i].shot_time > E_SHOT_BET && enemy[i].shot[j].flag == FALSE){
				enemy[i].shot_time = GetNowCount();
				if (enemy[i].shot_count < E_SHOT_NUM && flag == TRUE) {
					flag = FALSE;
					enemy[i].shot[j].flag = TRUE;
					enemy[i].shot[j].radian = atan2(player.y - enemy[i].y, player.x - enemy[i].x);
					enemy[i].shot[j].x = enemy[i].x;
					enemy[i].shot[j].y = enemy[i].y;
				}
				enemy[i].shot_count++;
				if (enemy[i].shot_count > E_MAX_SHOT_COUNT)
					enemy[i].shot_count = 0;
			}
		}
	}
}

void player_move_proc(PLAYER *player)
{
	int x_move, y_move;
	int move_speed = P_MOVE_SPEED;

	x_move = y_move = 0;

	if (CheckHitKey(KEY_INPUT_LEFT) == 1 && player->x >= 0)
		x_move = -1;
	if (CheckHitKey(KEY_INPUT_UP) == 1 && player->y >= 0)
		y_move = -1;
	if (CheckHitKey(KEY_INPUT_RIGHT) == 1 && player->x <= X_SIZE)
		x_move = 1;
	if (CheckHitKey(KEY_INPUT_DOWN) == 1 && player->y <= Y_SIZE)
		y_move = 1;
	if (CheckHitKey(KEY_INPUT_LSHIFT) == TRUE){
		move_speed /= 2;
		player->shift_mode = TRUE;
	}
	else
		player->shift_mode = FALSE;
	if (CheckHitKey(KEY_INPUT_SPACE) == TRUE && player->stop_time > 0){
		player->stop_flag = TRUE;
		player->keytmp++;
		if (player->keytmp == 1)
			player->timetmp = GetNowCount();
		player->stop_time -= GetNowCount() - player->timetmp;
		player->timetmp = GetNowCount();
	}
	else{
		player->stop_flag = FALSE;
		player->keytmp = 0;
	}

	if (x_move != 0 && y_move != 0) {
		player->x += move_speed * (sqrt(2.0) / 2) * x_move;
		player->y += move_speed * (sqrt(2.0) / 2) * y_move;
	}
	else{
		player->x += move_speed * x_move;
		player->y += move_speed * y_move;
	}
}

void draw_proc(PLAYER player, ENEMY enemy[], RESULT result)
{
	int i, j;

	DrawBox(0, 0, X_SIZE, Y_SIZE, WHITE, TRUE);    // �l�p�`��`��

	/* �v���C���[�̕`�� */
	//DrawGraph(player.x, player.y, player.handle, TRUE);
	DrawOval(player.x, player.y, 30, 30, RED, TRUE);
	if (player.shift_mode == TRUE)
		DrawOval(player.x, player.y, 10, 10, BLUE, TRUE);

	for (i = 0; i < ENEMY_NUM; i++)  {
		/* �G�̕`�� */
		if (enemy[i].flag == TRUE){
			//DrawGraph(enemy[i].x, enemy[i].y, e_status.handle, TRUE);
			DrawOval(enemy[i].x, enemy[i].y, 30, 30, BLUE, TRUE);
		}
		/* �G�̒e�ۂ̕`�� */
		for (j = 0; j < SHOT_NUM; j++){
			if (enemy[i].shot[j].flag == TRUE){
				//DrawGraph(enemy[i].shot[j].x, enemy[i].shot[j].y, e_status.shot_handle, TRUE);
				DrawOval(enemy[i].shot[j].x, enemy[i].shot[j].y, 10, 10, GREEN, TRUE);
			}
		}
	}

	/* �v���C���[�̒e�ۂ̕`�� */
	for (i = 0; i < SHOT_NUM; i++){
		if (player.shot[i].flag == TRUE){
			//DrawGraph(player.shot[i].x, player.shot[i].y, player.shot_handle, TRUE);
			DrawOval(player.shot[i].x, player.shot[i].y, 10, 10, MAGENTA, TRUE);
		}
	}

	//DrawBox(0, 0, X_SIZE, Y_SIZE, WHITE, TRUE);    // �l�p�`��`��
	if (player.stop_flag == TRUE)
		DrawFormatStringToHandle(X_SIZE / 2 - 99, Y_SIZE / 2 + 1, BLUE, BigHandle, "%.2lf", player.stop_time / 1000);
	DrawFormatString(MASSAGE4, BLUE, "�^�C���X�g�b�v�c�� : %.2lf", player.stop_time / 1000);

	DrawFormatString(MASSAGE2, BLUE, "�X�R�A : %d", result.score);
	DrawFormatString(MASSAGE1, BLUE, "�|������ : %d", result.del_em);
}

int gameover_judge_proc(PLAYER *player, ENEMY enemy[], RESULT *result)
/* �Q�[���I�[�o�[�y�уR���e�B�j���[���̏��������� */
/* �^�C�g����ʂ֋A�邩��Ԃ� */
{
	if (gameover_judge(*player, enemy) == GAMEOVER){
		DrawStringToHandle(X_SIZE / 2 - 199, Y_SIZE / 2 - 149, "GAME OVER !!", BLUE, BigHandle);
		DrawStringToHandle(X_SIZE / 2 - 200, Y_SIZE / 2 - 150, "GAME OVER !!", RED, BigHandle);
		ScreenFlip(); //����ʂ�\��ʂɔ��f
		WaitKey();
		switch (cont_ask(*result)){
		case CONTINUE:
			set_data(player, enemy, result);
			break;
		case BACK_TITLE:
			return BACK_TITLE;
		case GAMEEND:
			DxLib_End();    // DX���C�u�����I������
			exit(1);
		}
	}
	return GAMELOOP;
}

int gameover_judge(PLAYER player, ENEMY enemy[])
{
	int i, j;

	for (i = 0; i < ENEMY_NUM; i++){
		for (j = 0; j < SHOT_NUM; j++){
			if (player.x + 10 > enemy[i].shot[j].x - 10 && player.x - 10 < enemy[i].shot[j].x + 10){
				if (player.y + 10 > enemy[i].shot[j].y - 10 && player.y - 10 < enemy[i].shot[j].y + 10)
					return GAMEOVER;
			}
		}
	}
	return FALSE;
}

int cont_ask(RESULT result)
{
	MENU_CONT menu[3] = {
		{ X_SIZE / 2 - 130, Y_SIZE / 2 + 50, ">�R���e�B�j���[" },
		{ X_SIZE / 2 - 130, Y_SIZE / 2 + 100, " �^�C�g���֖߂�" },
		{ X_SIZE / 2 - 130, Y_SIZE / 2 + 150, " �I��" },
	};
	int SelectNum = 0; // ���݂̑I��ԍ�
	int i;

	do{
		Keyboard_Update();
		if (Keyboard_Get(KEY_INPUT_DOWN) == 1 || Keyboard_Get(KEY_INPUT_UP) == 1){
			if (Keyboard_Get(KEY_INPUT_DOWN) == 1)
				SelectNum = (SelectNum + 1) % 3;
			if (Keyboard_Get(KEY_INPUT_UP) == 1)
				SelectNum = (SelectNum - 1 + 3) % 3;
			for (i = 0; i < 3; i++){
				if (i == SelectNum)
					menu[i].name[0] = '>';
				else
					menu[i].name[0] = ' ';
			}
		}

		// �`��t�F�[�Y
		DrawBox(X_SIZE / 2 - 151, Y_SIZE / 2 + 29, X_SIZE / 2 + 221, Y_SIZE / 2 + 221, BLUE, TRUE);    // �l�p�`��`��
		DrawBox(X_SIZE / 2 - 150, Y_SIZE / 2 + 30, X_SIZE / 2 + 220, Y_SIZE / 2 + 220, RED, TRUE);    // �l�p�`��`��
		for (int i = 0; i < 3; i++){ // ���j���[���ڂ�`��
			DrawFormatStringToHandle(menu[i].x, menu[i].y, WHITE, FontHandle, menu[i].name);
		}
		DrawFormatStringToHandle(X_SIZE / 2 - 150, Y_SIZE / 2 - 50, BLUE, FontHandle, "�X�R�A : %d", result.score);

		ScreenFlip();

	} while (Keyboard_Get(KEY_INPUT_Z) != 1);
	return SelectNum;
}

int title_proc(void)
{
	MENU_TITLE menu[3] = {
		{ WinX_SIZE / 2 - 120, WinY_SIZE / 2, ">�͂��߂�" },
		{ WinX_SIZE / 2 - 120, WinY_SIZE / 2 + 50, " �Q�[���̐���" },
		{ WinX_SIZE / 2 - 120, WinY_SIZE / 2 + 100, " �I��" }
	};
	int SelectNum = 0; // ���݂̑I��ԍ�
	int key_sta[2] = { 0, 0 };
	int i;

	do{
		ClearDrawScreen();
		Keyboard_Update();
		if (Keyboard_Get(KEY_INPUT_DOWN) == 1 || Keyboard_Get(KEY_INPUT_UP) == 1){
			if (Keyboard_Get(KEY_INPUT_DOWN) == 1)
				SelectNum = (SelectNum + 1) % 3;
			if (Keyboard_Get(KEY_INPUT_UP) == 1)
				SelectNum = (SelectNum - 1 + 3) % 3;
			for (i = 0; i < 3; i++){
				if (i == SelectNum)
					menu[i].name[0] = '>';
				else
					menu[i].name[0] = ' ';
			}
		}

		// �`��t�F�[�Y
		DrawBox(WinX_SIZE / 2 - 140, WinY_SIZE / 2 - 20, WinX_SIZE / 2 + 160, WinY_SIZE / 2 + 160, BLUE, FALSE);    // �l�p�`��`��
		for (int i = 0; i < 5; i++)
			DrawFormatStringToHandle(menu[i].x, menu[i].y, WHITE, FontHandle, menu[i].name);
		DrawStringToHandle(56, 201, "Simple Shooting Game !!", BLUE, BigHandle);
		DrawStringToHandle(55, 200, "Simple Shooting Game !!", RED, BigHandle);
		DrawString(600, 600, "�����L�[ : �J�[�\���ړ�", WHITE);
		DrawString(600, 620, "Z�L�[    : ����", WHITE);

		ScreenFlip();


	} while (Keyboard_Get(KEY_INPUT_Z) != 1);

	return SelectNum;
}

void tutorial_proc(void)
{
	ClearDrawScreen();
	DrawStringToHandle(50, 100, "�����L�[ : �ړ�", WHITE, FontHandle);
	DrawStringToHandle(50, 150, "X�L�[    : �ˌ�, �߂�", WHITE, FontHandle);
	DrawStringToHandle(50, 200, "Z�L�[    : ����", WHITE, FontHandle);
	DrawStringToHandle(50, 250, "�X�y�[�X�L�[ : ���Ԃ��~�߂�i1�Q�[�����v5�b�ԁj", WHITE, FontHandle);
	DrawStringToHandle(50, 300, "���V�t�g�L�[ : �ړ����x����", WHITE, FontHandle);
	DrawStringToHandle(50, 400, "�����������܂łɓG���ǂꂾ���|���邩", WHITE, FontHandle);
	DrawStringToHandle(50, 450, "�X�R�A�������V���[�e�B���O�Q�[���ł�", WHITE, FontHandle);
	ScreenFlip();
	WaitKey();
}

