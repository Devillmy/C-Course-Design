#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<graphics.h>
#include<Windows.h>
#include<conio.h>
#include<math.h>

#pragma comment(lib, "winmm.lib")

#define WIDTH 1024
#define HEIGHT 768
#define DISTANCE(x1, y1, x2, y2) (sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)))

#undef UNICODE
#undef _UNICODE


//button按钮
struct button
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
    char info[20] = "NULL";
	bool isMouseOn = false;
	bool isMouseClick = false;
	bool useable = true;
};

//猫
struct Cat
{
	bool life;
	int x;
	int y;
	int r;
	int v;
	bool strong;
};

//变量
struct Global
{
	int strong;
	int speed;
	int diff_t;
	int temp;
	int gamechoose; //简单为1 正常为2 困难为3
	int rankchoose;
	int operatchoose; //键盘为1 鼠标为0
	int eat;
	int NUM;
};

//老鼠
struct Mouse
{
	bool life;
	int x;
	int y;
	int r;
	int vx;
	int vy;
	int distance[2];		//记录某个老鼠，距离它最近的老鼠的距离，以及这个老鼠的下标
};

//速度鱼
struct SpeedFish
{
	bool life;
	int x;
	int y;
	int r;
	int num;
};

//强壮鱼
struct StrongFish
{
	bool life;
	int x;
	int y;
	int r;
	int num;
};

Cat cat = { 0, 0, 0, 20, 3, 0};
Mouse mouse[25];
DWORD* pBuffer;
IMAGE start[41];
Global global = { 0, 0, 0, 0, 1, 1, 1, 0, 30 };
SpeedFish speedfish = {0, 2999, 2999, 30};
StrongFish strongfish = { 0, 2999, 2999, 30 };


//HighScore结构，绘制高分榜及存储成绩
struct HighScore 
{
	char name[100];
	int score;
	struct HighScore* next;
};

//读取分数文件
struct HighScore* readHighscore()
{
	struct HighScore* temp, * p, * q, * head;
	FILE* fp = NULL;
	temp = (struct HighScore*)malloc(sizeof(struct HighScore));
	head = (struct HighScore*)malloc(sizeof(struct HighScore));
	p = q = head;
	if (global.rankchoose == 1)
	{
		if (fopen_s(&fp, "data//easyhighscore.txt", "r")) printf("error");
		else
		{
			while (fread_s(temp, sizeof(struct HighScore), sizeof(struct HighScore), 1, fp))
			{
				q = (struct HighScore*)malloc(sizeof(struct HighScore));
				strcpy_s(q->name, strlen(temp->name) + 1, temp->name);
				q->score = temp->score;
				p->next = q;
				p = q;
			}
			p->next = NULL;
		}
	}

	if (global.rankchoose == 2)
	{
		if (fopen_s(&fp, "data//normalhighscore.txt", "r")) printf("error");
		else
		{
			while (fread_s(temp, sizeof(struct HighScore), sizeof(struct HighScore), 1, fp))
			{
				q = (struct HighScore*)malloc(sizeof(struct HighScore));
				strcpy_s(q->name, strlen(temp->name) + 1, temp->name);
				q->score = temp->score;
				p->next = q;
				p = q;
			}
			p->next = NULL;
		}
	}

	if (global.rankchoose == 3)
	{
		if (fopen_s(&fp, "data//hardhighscore.txt", "r")) printf("error");
		else
		{
			while (fread_s(temp, sizeof(struct HighScore), sizeof(struct HighScore), 1, fp))
			{
				q = (struct HighScore*)malloc(sizeof(struct HighScore));
				strcpy_s(q->name, strlen(temp->name) + 1, temp->name);
				q->score = temp->score;
				p->next = q;
				p = q;
			}
			p->next = NULL;
		}
	}
	
	fclose(fp);
	free(temp);
	return head;
}

void drawButton(button btn);
void checkMouseOn(button& btn, MOUSEMSG msg);
void checkMouseClick(button& btn, MOUSEMSG msg);
void loadlogo();
void getstart();
void show_help();
void move(Cat* cat);
void rule(Cat* cat, struct HighScore* newuser);
void transparentimage(int x, int y, IMAGE* srcing, IMAGE* masking);
void draw();
void mouseset();
void fishset();
void set();
void delay(DWORD ms);
void endgame(int eaten, button back);
void UpdateScore(struct HighScore* head);
void drawhighscore(struct HighScore* head);
void addHighScore(struct HighScore us);
void gamechoose();
void gameoperating();
void showrank();
void storagegame();
void loadgame();

int main()
{
	//游戏状态，0为准备，1为开始，2为结束
	int begin = 0;
	int mu = 1;
	char str[50];
	initgraph(WIDTH, HEIGHT, SHOWCONSOLE);
	cleardevice();
	setbkmode(TRANSPARENT);
	srand((unsigned)time(NULL));
	
	//newuser当前游戏者信息
	struct HighScore newuser;
	sprintf_s(newuser.name, "无名氏");
	newuser.score = 0;
	InputBox(newuser.name, 100, "敢问英雄尊姓大名？", "提示消息", "无名氏", 200);
	
	mciSendString("open music\\start.mp3 alias start", NULL, 0, NULL);
	mciSendString("open music\\game.mp3 alias game", NULL, 0, NULL);

	loadlogo();
	getstart();

	//开始按钮
	button start;
	start.x = 260;
	start.y = 280;
	start.width = 128;
	start.height = 48;
	sprintf_s(start.info, "开始");

	//退出按钮
	button end;
	end.x = 260;
	end.y = 700;
	end.width = 128;
	end.height = 48; 
	sprintf_s(end.info, "退出");

	//帮助按钮
	button help;
	help.x = 260;
	help.y = 560;
	help.width = 128;
	help.height = 48;
	help.useable = true;
	sprintf_s(help.info, "帮助");
	//bool show_help = false;//是否在显示帮助

	//排行榜按钮
	button rank;
	rank.x =260;
	rank.y = 420;
	rank.width = 128;
	rank.height = 49;
	sprintf_s(rank.info, "排行榜");

	//返回按钮
	button back;
	back.x = 860;
	back.y = 690;
	back.width = 128;
	back.height = 49;
	sprintf_s(back.info, "返回");
	back.useable = false;

	//音乐按钮
	button musicstart;
	musicstart.x = 860;
	musicstart.y = 20;
	musicstart.width = 128;
	musicstart.height = 49;
	sprintf_s(musicstart.info, "音乐：开");
	musicstart.useable = false;

	button musicend;
	musicend.x = 860;
	musicend.y = 20;
	musicend.width = 128;
	musicend.height = 49;
	sprintf_s(musicend.info, "音乐：关");
	musicend.useable = true;

	//游戏界面返回
	button gameback;
	gameback.x = 435;
	gameback.y = 500;
	gameback.width = 128;
	gameback.height = 49;
	sprintf_s(gameback.info, "返回");
	gameback.useable = false;

	//保存
	button storage;
	storage.x = 435;
	storage.y = 400;
	storage.width = 128;
	storage.height = 49;
	sprintf_s(storage.info, "存档");
	storage.useable = false;

	//鼠标
	MOUSEMSG msg;
	msg = GetMouseMsg();
	
	//死循环
	while (1)
	{
		BeginBatchDraw();

		if (MouseHit())
		{
			msg = GetMouseMsg();

			checkMouseOn(start, msg);
			checkMouseOn(rank, msg);
			checkMouseOn(end, msg);
			checkMouseOn(help, msg);
			checkMouseOn(back, msg);
			checkMouseOn(musicstart, msg);
			checkMouseOn(musicend, msg);

			checkMouseClick(start, msg);
			checkMouseClick(rank, msg);
			checkMouseClick(end, msg);
			checkMouseClick(help, msg);
			checkMouseClick(back, msg);
			checkMouseClick(musicstart, msg);
			checkMouseClick(musicend, msg);
		}

		drawButton(start);
		drawButton(rank);
		drawButton(end);
		drawButton(help);
		drawButton(back);
		drawButton(musicstart);
		drawButton(musicend);

		if (start.isMouseClick)
		{
			begin = 1;
			start.useable = false;
			start.isMouseClick = false;
			end.useable = false;
			help.useable = false;
			rank.useable = false;
			back.useable = false;
			gameback.useable = true;
		}

		if (end.isMouseClick)
		{
			end.isMouseClick = false;
			closegraph();
			exit(0);
		}

		if (help.isMouseClick)
		{
			start.useable = false;
			end.useable = false;
			help.useable = false;
			rank.useable = false;
			back.useable = true;
			show_help();
		}

		if (rank.isMouseClick)
		{
			start.useable = false;
			end.useable = false;
			help.useable = false;
			rank.useable = false;
			rank.isMouseClick = false;
			back.useable = true;
			showrank();
		}

		if (musicstart.isMouseClick)
		{
			mciSendString("stop start", NULL, 0, NULL);
			musicstart.useable = false;
			musicend.useable = true;
		}

		if (musicend.isMouseClick)
		{
			mciSendString("play start repeat", NULL, 0, NULL);
			musicstart.useable = true;
			musicend.useable = false;
		}

		if (begin == 2)
		{
			back.useable = true;
			endgame(global.eat, back);
		}

		if (back.isMouseClick)
		{
			getstart();
			start.useable = true;
			end.useable = true;
			help.useable = true;
			rank.useable = true;
			back.useable = false;
			begin = 0;
		}

		if (begin == 1)
		{
			mciSendString("stop start", NULL, 0, NULL);
			
			if (musicstart.useable == true)
			{
				mciSendString("play game", NULL, 0, NULL);
			}
			else
			{
				mciSendString("stop game", NULL, 0, NULL);
			}

			bool judge = true;
			int gametime = 0;
			int temp = 0;
			global.eat = 0;
			cat.v = 3;

			gamechoose();

			if (cat.life == 0)
			{
				mouseset();
				set();
				gameoperating();
				switch (global.gamechoose)
				{
				case 1: gametime = 90;
					break;
				case 2: gametime = 60;
					break;
				case 3: gametime = 40;
					break;
				default: break;
				}
			}
			else
			{
				gametime = global.diff_t;
			}

			BeginBatchDraw();
			
			time_t start_t;
			time(&start_t);
			

			while (1)
			{
				move(&cat);
				fishset();
				draw();


				//计时并显示
				time_t end_t;
				time(&end_t);
				global.diff_t = gametime - difftime(end_t, start_t);
				
				settextstyle(28, 0, _T("华文行楷"));
				sprintf_s(_T(str), _T("%d"),global.diff_t);
				outtextxy(170, 20, _T("倒计时"));
				settextstyle(28, 0, _T("华文行楷"));
				outtextxy(270, 20, str);
				
				rule(&cat, &newuser);
				 
				//判断暂停
				if (GetAsyncKeyState(VK_SPACE) & 0x0001) 
				{
					//time_t stop_t;
					//stop_t = end_t;

					temp = difftime(end_t, start_t);

					musicstart.x = 435;
					musicstart.y = 300;

					musicend.x = 435;
					musicend.y = 300;
					storage.useable = true;

					while (1)
					{
						setfillcolor(RGB(193, 210, 240));
						fillrectangle(400, 200, 600, 600);
						settextstyle(28, 0, _T("华文行楷"));
						outtextxy(440, 230, _T("游戏暂停"));
						
						if (MouseHit())
						{
							msg = GetMouseMsg();
							checkMouseOn(gameback, msg);
							checkMouseOn(storage, msg);
							checkMouseOn(musicstart, msg);
							checkMouseOn(musicend, msg);

							checkMouseClick(gameback, msg);
							checkMouseClick(storage, msg);
							checkMouseClick(musicstart, msg);
							checkMouseClick(musicend, msg);
						}
						
						drawButton(gameback);
						drawButton(storage);
						drawButton(musicstart);
						drawButton(musicend);

						if (gameback.isMouseClick)
						{
							begin = 0;
							cat = { 0, 0, 0, 20, 3, 0 };
							musicend.useable = true;
							musicstart.useable = false;
							start.useable = true;
							end.useable = true;
							help.useable = true;
							rank.useable = true;
							gameback.useable = false;
							back.useable = false;
							storage.useable = false;
							getstart();
							break;
						}

						if (storage.isMouseClick)
						{
							storagegame();
						}

						if (musicstart.isMouseClick)
						{
							mciSendString("stop game", NULL, 0, NULL);
							musicstart.useable = false;
							musicend.useable = true;
						}

						if (musicend.isMouseClick)
						{
							mciSendString("play game", NULL, 0, NULL);
							musicstart.useable = true;
							musicend.useable = false;
						}

						if ((GetAsyncKeyState(VK_SPACE) & 0x0001))
						{
							start_t = time(NULL) - temp;
							storage.useable = false;
							break;
						}
						FlushBatchDraw();
					}
				}
				if (begin == 0)
				{
					musicstart.x = 860;
					musicstart.y = 20;

					musicend.x = 860;
					musicend.y = 20;

					if (musicstart.useable == true)
					{
						mciSendString("play start", NULL, 0, NULL);
					}
					else
					{
						mciSendString("stop start", NULL, 0, NULL);
					}

					break;
				}

				//判断结束
				if (global.eat % global.NUM == 0 && global.eat != 0 && global.diff_t >= 1.0 && judge)
				{
					mouseset();
					judge = false;
				}

				if (global.eat % global.NUM != 0)
				{
					judge = true;
				}

				if (global.diff_t <= 0.0)
				{
					musicstart.x = 860;
					musicstart.y = 20;

					musicend.x = 860;
					musicend.y = 20;

					if (musicstart.useable == true)
					{
						mciSendString("play start", NULL, 0, NULL);
					}
					else
					{
						mciSendString("stop start", NULL, 0, NULL);
					}

					setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 1);
					setlinecolor(WHITE);
					newuser.score = global.eat;
					addHighScore(newuser);
					cat.life = 0;
					begin = 2;
					break;
				}

				FlushBatchDraw();
				delay(20);
			}
		}
		EndBatchDraw();
	}
	return 0;
}

//绘制button
void drawButton(button btn)
{
	settextcolor(BLACK);
	settextstyle(20, 0, _T("宋体"));
	if (btn.useable)
	{
		if (btn.isMouseOn)
		{
			setfillcolor(0x5555FF);
		}
		else
		{
			setfillcolor(GREEN);
		}
		fillrectangle(btn.x, btn.y, btn.x + btn.width, btn.y + btn.height);
		outtextxy(btn.x + 3, btn.y + btn.height / 2, btn.info);
	}
}

//判断鼠标是否位于button上
void checkMouseOn(button& btn, MOUSEMSG msg)
{
	if (msg.x > btn.x && msg.x < btn.x + btn.width && msg.y > btn.y && msg.y < btn.y + btn.height)
	{
		btn.isMouseOn = true;
	}
	else
	{
		btn.isMouseOn = false;
	}
}

//判断鼠标是否点击按钮
void checkMouseClick(button& btn, MOUSEMSG msg)
{
	if (btn.isMouseOn && msg.uMsg == WM_LBUTTONDOWN && btn.useable)
	{
		btn.isMouseClick = true;
		printf_s("%s被点了一下\n", btn.info);//debug
	}
	else
	{
		btn.isMouseClick = false;
	}
}

//开始动画
void loadlogo()
{
	setbkcolor(0xE8E8E8);					// 设置背景颜色
	cleardevice();

	loadimage(&start[0], "image\\start\\1.jpg");
	loadimage(&start[1], "image\\start\\2.jpg");
	loadimage(&start[2], "image\\start\\3.jpg");
	loadimage(&start[3], "image\\start\\4.jpg");
	loadimage(&start[4], "image\\start\\5.jpg");
	loadimage(&start[5], "image\\start\\6.jpg");
	loadimage(&start[6], "image\\start\\7.jpg");
	loadimage(&start[7], "image\\start\\8.jpg");
	loadimage(&start[8], "image\\start\\9.jpg");
	loadimage(&start[9], "image\\start\\10.jpg");
	loadimage(&start[10], "image\\start\\11.jpg");
	loadimage(&start[11], "image\\start\\12.jpg");
	loadimage(&start[12], "image\\start\\13.jpg");
	loadimage(&start[13], "image\\start\\14.jpg");
	loadimage(&start[14], "image\\start\\15.jpg");
	loadimage(&start[15], "image\\start\\16.jpg");
	loadimage(&start[16], "image\\start\\17.jpg");
	loadimage(&start[17], "image\\start\\18.jpg");
	loadimage(&start[18], "image\\start\\19.jpg");
	loadimage(&start[19], "image\\start\\20.jpg");
	loadimage(&start[20], "image\\start\\21.jpg");
	loadimage(&start[21], "image\\start\\22.jpg");
	loadimage(&start[22], "image\\start\\23.jpg");
	loadimage(&start[23], "image\\start\\24.jpg");
	loadimage(&start[24], "image\\start\\25.jpg");
	loadimage(&start[25], "image\\start\\26.jpg");
	loadimage(&start[26], "image\\start\\27.jpg");
	loadimage(&start[27], "image\\start\\28.jpg");
	loadimage(&start[28], "image\\start\\29.jpg");
	loadimage(&start[29], "image\\start\\30.jpg");
	loadimage(&start[30], "image\\start\\31.jpg");
	loadimage(&start[31], "image\\start\\32.jpg");
	loadimage(&start[32], "image\\start\\load0.jpg");
	loadimage(&start[33], "image\\start\\load1.jpg");
	loadimage(&start[34], "image\\start\\load2.jpg");
	loadimage(&start[35], "image\\start\\load3.jpg");
	loadimage(&start[36], "image\\start\\load4.jpg");
	loadimage(&start[37], "image\\start\\load5.jpg");
	loadimage(&start[38], "image\\start\\load6.jpg");
	loadimage(&start[39], "image\\start\\load7.jpg");
	loadimage(&start[40], "image\\start\\load8.jpg");

	for (int i = 0; i < 41; i++) {
		putimage(280, 240, &start[i]);
		Sleep(100);
	}
}

//开始
void getstart()
{
	//初始背景
	IMAGE background;
	loadimage(&background, "image/start.png");
	int height = background.getheight();
	int width = background.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &background, SRCCOPY);
		}
	}

	settextcolor(RED);
	settextstyle(80, 0, _T("华文行楷"));
	outtextxy((WIDTH / 2) - 340, 120, _T("猫捉老鼠"));
	settextcolor(BLACK);
	settextstyle(20, 0, _T("宋体"));

}

//显示帮助
void show_help()
{
	cleardevice();

	IMAGE background;
	loadimage(&background, "image/help.png");
	int height = background.getheight();
	int width = background.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &background, SRCCOPY);
		}
	}
}

//移动
void move(Cat* cat)
{
	srand((unsigned)time(NULL));
	
	if (global.operatchoose == 1)
	{
		if (cat->strong == false)
		{
			if (GetAsyncKeyState(65) || GetAsyncKeyState(37))
			{
				cat->x -= cat->v;
			}
			if (GetAsyncKeyState(68) || GetAsyncKeyState(39))
			{
				cat->x += cat->v;
			}
			if (GetAsyncKeyState(83) || GetAsyncKeyState(40))
			{
				cat->y += cat->v;
			}
			if (GetAsyncKeyState(87) || GetAsyncKeyState(38))
			{
				cat->y -= cat->v;
			}
		}

		if (cat->strong == true)
		{
			if (GetAsyncKeyState(65) || GetAsyncKeyState(37))
			{
				cat->x -= 15;
			}
			if (GetAsyncKeyState(68) || GetAsyncKeyState(39))
			{
				cat->x += 15;
			}
			if (GetAsyncKeyState(83) || GetAsyncKeyState(40))
			{
				cat->y += 15;
			}
			if (GetAsyncKeyState(87) || GetAsyncKeyState(38))
			{
				cat->y -= 15;
			}
		}
	}

	if (global.operatchoose == 0)
	{
		MOUSEMSG msg;
		msg = GetMouseMsg();
		if (cat->strong == false)
		{
			if (cat->x > msg.x)
			{
				cat->x -= cat->v;
			}
			if (cat->x < msg.x)
			{
				cat->x += cat->v;
			}
			if (cat->y < msg.y)
			{
				cat->y += cat->v;
			}
			if (cat->y > msg.y)
			{
				cat->y -= cat->v;
			}
		}

		if (cat->strong == true)
		{
			if (cat->x > msg.x)
			{
				cat->x -= 10 + cat->v;
			}
			if (cat->x < msg.x)
			{
				cat->x += 10 + cat->v;
			}
			if (cat->y < msg.y)
			{
				cat->y += 10 + cat->v;
			}
			if (cat->y > msg.y)
			{
				cat->y -= 10 + cat->v;
			}
		}
	}
	

	for (int i = 0; i < global.NUM; i++)
	{

		if (mouse[i].life == true)
		{
			mouse[i].x = mouse[i].x + mouse[i].vx;
			mouse[i].y = mouse[i].y + mouse[i].vy;
		}
		else
		{
			mouse[i].x = 2024;
			mouse[i].y = 2024;
		}
	}

}

//规则
void rule(Cat* cat, struct HighScore* newuser)
{
	//游戏结束
	if (cat->life == false)
	{
		HWND hwnd = GetHWnd();
		MessageBox(hwnd, _T("你没了"), _T("游戏结束"), MB_OK | MB_ICONEXCLAMATION);	// 结束
		closegraph();
		exit(0);
	}

	//玩家触碰边界
	if (cat->x >= (WIDTH - cat->r))
	{
		cat->x = WIDTH - cat->r;
	}

	if (cat->x <= 0)
	{
		cat->x = 0;
	}

	if (cat->y >= (HEIGHT - cat->r))
	{
		cat->y = HEIGHT - cat->r;
	}

	if (cat->y <= 0)
	{
		cat->y = 0;
	}

	//老鼠之间距离
	for (int i = 0; i < global.NUM; i++)
	{
		for (int j = 0; j < global.NUM; j++)
		{
			if (i != j)
			{
				int dist;
				dist = (mouse[i].x - mouse[j].x) * (mouse[i].x - mouse[j].x) + (mouse[i].y - mouse[j].y) * (mouse[i].y - mouse[j].y);
				if (dist < mouse[i].distance[0])
				{
					mouse[i].distance[0] = dist;
					mouse[i].distance[1] = j;
				}
			}
		}
	}

	//老鼠触碰边界
	for (int i = 0; i < global.NUM; i++)
	{
		if (mouse[i].life == true)
		{
			if (mouse[i].x <= 0 || mouse[i].x >= WIDTH - mouse[i].r) mouse[i].vx = -mouse[i].vx;
			if (mouse[i].y <= 0 || mouse[i].y >= HEIGHT - mouse[i].r) mouse[i].vy = -mouse[i].vy;

			if (mouse[i].x <= 0) mouse[i].x = 0;
			if (mouse[i].x >= WIDTH - mouse[i].r) mouse[i].x = WIDTH - mouse[i].r;
			if (mouse[i].y <= 0) mouse[i].y = 0;
			if (mouse[i].y >= HEIGHT - mouse[i].r) mouse[i].y = HEIGHT - mouse[i].r;
		}
	}

	//老鼠触碰老鼠
	for (int i = 0; i < global.NUM; i++)
	{
		if (mouse[i].distance[0] <= 4 * (mouse[i].r * mouse[i].r))
		{
			int j = mouse[i].distance[1];
			//交换速度
			int temp;
			temp = mouse[i].vx;
			mouse[i].vx = mouse[j].vx;
			mouse[j].vx = temp;
			temp = mouse[i].vy;
			mouse[i].vy = mouse[j].vy;
			mouse[j].vy = temp;

			mouse[i].distance[0] = 9999999;
			mouse[i].distance[1] = -1;

			mouse[j].distance[0] = 9999999;
			mouse[j].distance[1] = -1;

			if (mouse[i].x >= mouse[j].x)
			{
				mouse[i].x += abs(mouse[i].vx);
			}
		}
	}

	//老鼠触碰鱼 
	for (int i = 0; i < global.NUM; i++)
	{
		if (DISTANCE(strongfish.x, strongfish.y, mouse[i].x, mouse[i].y) < 3 / 5.0 * (strongfish.r + mouse[i].r) || DISTANCE(speedfish.x, speedfish.y, mouse[i].x, mouse[i].y) < 3 / 5.0 * (speedfish.r + mouse[i].r))
		{
			mouse[i].vx = -mouse[i].vx;
			mouse[i].vy = -mouse[i].vy;
		}
	}

	//猫吃老鼠
	for (int i = 0; i < global.NUM; i++)
	{
		if (cat->r >= mouse[i].r)
		{
			if (mouse[i].life == 0) continue;

			if (DISTANCE(cat->x, cat->y, mouse[i].x, mouse[i].y) < 3 / 5.0 * (cat->r + mouse[i].r))
			{
				mouse[i].life = 0;
				//cat->r += (mouse[i].r * mouse[i].r / 2) / cat->r;
				global.eat++;
				PlaySound(_T("music\\eatmusic.wav"), NULL, SND_ASYNC);
			}
		}
	}

	//老鼠躲避猫
	for (int i = 0; i < global.NUM; i++)
	{
		if (DISTANCE(cat->x, cat->y, mouse[i].x, mouse[i].y) <= 2 * (cat->r + mouse[i].r))
		{
			if (abs(mouse[i].x - cat->x) >= abs(mouse[i].y - cat->y))
			{
				if (mouse[i].x >= cat->x)
				{
					mouse[i].x += abs(mouse[i].vx);
				}
				else if (mouse[i].x < cat->x)
				{
					mouse[i].x -= abs(mouse[i].vx);
				}
			}
			else
			{
				if (mouse[i].y >= cat->y)
				{
					mouse[i].y += abs(mouse[i].vy);
				}
				else if (mouse[i].y < cat->y)
				{
					mouse[i].y -= abs(mouse[i].vy);
				}
			}
		}
	}

	//猫吃鱼
	if (cat->r >= speedfish.r && cat->r >= strongfish.r)
	{
		if (speedfish.life == false) {}
		if (strongfish.life == false) {}

		if (DISTANCE(cat->x, cat->y, speedfish.x, speedfish.y) < 3 / 5.0 * (cat->r + speedfish.r))
		{
			speedfish.life = false;
			speedfish.num = 1;
			speedfish.x = 2999;
			speedfish.y = 2999;
			cat->v += 1;
			PlaySound(_T("music\\eatmusic.wav"), NULL, SND_ASYNC);
		}

		if (DISTANCE(cat->x, cat->y, strongfish.x, strongfish.y) < 3 / 5.0 * (cat->r + strongfish.r))
		{
			strongfish.life = false;
			strongfish.num = 1;
			strongfish.x = 2999;
			strongfish.y = 2999;
			cat->strong = true;
			global.temp = global.diff_t;
			PlaySound(_T("music\\eatmusic.wav"), NULL, SND_ASYNC);
		}
	}

	if (cat->strong == true)
	{
		if (global.temp - global.diff_t >= 5)
		{
			cat->strong = false;
			global.temp = 0;
		}
	}

	//计数
	char str[50];
	settextcolor(RED);
	settextstyle(28, 0, _T("华文行楷"));
	sprintf_s(_T(str), _T("%d"), global.eat);
	outtextxy(400, 20, _T("吃掉老鼠数"));
	settextstyle(50, 0, _T("华文行楷"));
	outtextxy(580, 10, str);
	settextstyle(28, 0, _T("华文行楷"));
	sprintf_s(_T(str), _T("%ld"), cat->v);
	outtextxy(700, 20, _T("猫当前速度"));
	settextstyle(50, 0, _T("华文行楷"));
	outtextxy(880, 10, str);
}

//蒙版
void transparentimage(int x, int y, IMAGE* srcing, IMAGE* masking) {
	putimage(x, y, masking, SRCAND);
	putimage(x, y, srcing, SRCPAINT);
}

//画对象
void draw()
{
	IMAGE img0, img1, img2, img3, img4, img5, img6, img7;
	clearcliprgn();

	IMAGE game;
	loadimage(&game, "image\\game.png");
	int height = game.getheight();
	int width = game.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &game, SRCCOPY);
		}
	}

	//画老鼠
	for (int i = 0; i < global.NUM; i++)
	{
		if (mouse[i].life == 0) continue;
		loadimage(&img0, _T("image\\mouse.png"), mouse[i].r + 1, mouse[i].r + 1);
		loadimage(&img1, _T("image\\mouse2.png"), mouse[i].r + 1, mouse[i].r + 1);
		transparentimage(mouse[i].x, mouse[i].y, &img0, &img1);
	}

	//画猫
	if (cat.life == true)
	{
		if (cat.strong == false)
		{
			loadimage(&img2, _T("image\\cat.png"), cat.r + 2, cat.r + 2);
			loadimage(&img3, _T("image\\cat2.png"), cat.r + 2, cat.r + 2);
			transparentimage(cat.x, cat.y, &img2, &img3);
		}

		if (cat.strong == true)
		{
			loadimage(&img2, _T("image\\strongcat.png"), cat.r + 2, cat.r + 2);
			loadimage(&img3, _T("image\\strongcat2.png"), cat.r + 2, cat.r + 2);
			transparentimage(cat.x, cat.y, &img2, &img3);
		}
	}

	//画鱼
	if (strongfish.life == true)
	{
		loadimage(&img4, _T("image\\strongfish.png"), strongfish.r + 2, strongfish.r + 2);
		loadimage(&img5, _T("image\\strongfish1.png"), strongfish.r + 2, strongfish.r + 2);
		transparentimage(strongfish.x, strongfish.y, &img4, &img5);
	}

	if (speedfish.life == true)
	{
		loadimage(&img6, _T("image\\speedfish.png"), speedfish.r + 2, speedfish.r + 2);
		loadimage(&img7, _T("image\\speedfish1.png"), speedfish.r + 2, speedfish.r + 2);
		transparentimage(speedfish.x, speedfish.y, &img6, &img7);
	}

}

//老鼠设置
void mouseset()
{
	srand((unsigned)time(NULL));
	int vx = 0, vy = 0;

	switch (global.gamechoose)
	{
	case 1: global.NUM = 20;
		vx = 1;
		vy = 1;
		break;
	case 2: global.NUM = 10;
		vx = 5;
		vy = 5;
		break;
	case 3: global.NUM = 5;
		vx = 7;
		vy = 7;
		break;
	default:
		break;
	}

	for (int i = 0; i < global.NUM; i++)
	{
		mouse[i].life = 1;
		mouse[i].r = int(rand() % 10 + 35);
		mouse[i].x = rand() % (WIDTH - int(mouse[i].r + 2));
		mouse[i].y = rand() % (HEIGHT - int(mouse[i].r + 2));

		for (int j = 0; j < global.NUM; j++)
		{
			if (i != j)
			{
				//让老鼠不宜重叠
				if ((mouse[i].x >= mouse[j].x - mouse[i].r) && (mouse[i].x <= mouse[j].x + mouse[i].r) && (mouse[i].y >= mouse[j].y - mouse[i].r) && (mouse[i].y <= mouse[j].y + mouse[i].r))
				{
					mouse[i].x = rand() % (WIDTH - int(mouse[i].r + 2));
					mouse[i].y = rand() % (HEIGHT - int(mouse[i].r + 2));
				}
			}
			mouse[i].vx = vx;
			mouse[i].vy = vy;
		}
	}

	for (int i = 0; i < global.NUM; i++)
	{
		mouse[i].distance[0] = 9999999;
		mouse[i].distance[1] = -1;
	}

}

//鱼设置
void fishset()
{
	srand((unsigned)time(NULL));
	speedfish.num = rand() % 10 + 1;
	strongfish.num = rand() % 10 + 1;

	if (speedfish.num % 5 == 0)
	{
		speedfish.life = true;
		speedfish.r = 40;
		speedfish.x = rand() % (WIDTH - int(speedfish.r + 2));
		speedfish.y = rand() % (HEIGHT - int(speedfish.r + 2));
	}

	if (strongfish.num % 7 == 0)
	{
		strongfish.life = true;
		strongfish.r = 40;
		strongfish.x = rand() % (WIDTH - int(speedfish.r + 2));
		strongfish.y = rand() % (HEIGHT - int(speedfish.r + 2));
	}
}

//游戏设置
void set()
{
	cat.life = 1;
	cat.x = int(20.5);
	cat.y = int(20.5);
	cat.r = 70;
	pBuffer = GetImageBuffer(NULL);	// 获取显存指针
}

//绝对延时
void delay(DWORD ms)
{
	static DWORD oldtime = GetTickCount();

	while (GetTickCount() - oldtime < ms)
		Sleep(1);

	oldtime = GetTickCount();
}

//结束游戏
void endgame(int eaten, button back)
{
	char str[50];
	cleardevice();
	IMAGE end;
	loadimage(&end, "image\\end.png");
	int height = end.getheight();
	int width = end.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &end, SRCCOPY);
		}
	}
	settextstyle(60, 0, _T("宋体"));
	settextcolor(RED);
	sprintf_s(str, _T("您的得分：%d"), eaten);
	outtextxy(350, 200, str);
	drawButton(back);
}

//对成绩排序
void UpdateScore(struct HighScore* head)
{
	struct HighScore* p = NULL, * q = NULL, * temp = NULL;
	if (head == NULL || head->next == NULL) return;
	for (p = head; p != NULL; p = p->next)
	{
		for (q = p->next; q != NULL; q = q->next)
		{
			if (p->score < q->score)
			{
				temp = (struct HighScore*)malloc(sizeof(struct HighScore));
				temp->score = p->score;
				p->score = q->score;
				q->score = temp->score;
				strcpy_s(temp->name, strlen(p->name) + 1, p->name);
				strcpy_s(p->name, strlen(q->name) + 1, q->name);
				strcpy_s(q->name, strlen(temp->name) + 1, temp->name);
			}
		}
	}
}

//输出成绩
void drawhighscore(struct HighScore* head)
{
	cleardevice();
	IMAGE background;
	loadimage(&background, "image/rank.png");
	int height = background.getheight();
	int width = background.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &background, SRCCOPY);
		}
	}
	struct HighScore* pointer;
	pointer = head;
	int i = 0;
	settextcolor(BLACK);
	settextstyle(28, 0, _T("宋体"));
	while (pointer != NULL)
	{
		char str[100];
		sprintf_s(str, "%d.    %s    %d", i + 1, pointer->name, pointer->score);
		outtextxy(380, 270 + 50 * i, str);
		i++;
		pointer = pointer->next;
		if (i == 10) break;
	}
}

//添加成绩
void addHighScore(struct HighScore us)
{
	FILE* fp = NULL;
	if (global.gamechoose == 1)
	{
		if (fopen_s(&fp, "data\\easyhighscore.txt", "a+")) printf("error");
		else
		{
			fwrite(&us, sizeof(struct HighScore), 1, fp);
		}
	}

	if (global.gamechoose == 2)
	{
		if (fopen_s(&fp, "data\\normalhighscore.txt", "a+")) printf("error");
		else
		{
			fwrite(&us, sizeof(struct HighScore), 1, fp);
		}
	}

	if (global.gamechoose == 3)
	{
		if (fopen_s(&fp, "data\\hardhighscore.txt", "a+")) printf("error");
		else
		{
			fwrite(&us, sizeof(struct HighScore), 1, fp);
		}
	}

	fclose(fp);
}

//选择
void gamechoose()
{
	cleardevice();
	IMAGE background;
	loadimage(&background, "image/start.png");
	int height = background.getheight();
	int width = background.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &background, SRCCOPY);
		}
	}

	settextcolor(RED);
	settextstyle(80, 0, _T("华文行楷"));
	outtextxy((WIDTH / 2) - 340, 120, _T("选择游戏难度"));
	settextcolor(BLACK);
	settextstyle(20, 0, _T("宋体"));
	FlushBatchDraw();

	MOUSEMSG msg;
	msg = GetMouseMsg();

	button easy;
	easy.x = 260;
	easy.y = 280;
	easy.width = 128;
	easy.height = 48;
	sprintf_s(easy.info, "简单");
	easy.useable = true;

	button normal;
	normal.x = 260;
	normal.y = 400;
	normal.width = 128;
	normal.height = 48;
	sprintf_s(normal.info, "中等");
	normal.useable = true;

	button hard;
	hard.x = 260;
	hard.y = 520;
	hard.width = 128;
	hard.height = 48;
	sprintf_s(hard.info, "困难");
	hard.useable = true;
	
	button load;
	load.x = 260;
	load.y = 640;
	load.width = 128;
	load.height = 48;
	sprintf_s(load.info, "读档");
	load.useable = true;

	while (1)
	{
		if (MouseHit())
		{
			msg = GetMouseMsg();

			checkMouseOn(easy, msg);
			checkMouseOn(normal, msg);
			checkMouseOn(hard, msg); 
			checkMouseOn(load, msg);

			checkMouseClick(easy, msg);
			checkMouseClick(normal, msg);
			checkMouseClick(hard, msg);
			checkMouseClick(load, msg);

		}

		drawButton(easy);
		drawButton(normal);
		drawButton(hard);
		drawButton(load);

		if (easy.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			load.useable = false;
			global.gamechoose = 1;
			return;
		}

		if (normal.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			load.useable = false;
			global.gamechoose = 2;
			return;
		}

		if (hard.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			load.useable = false;
			global.gamechoose = 3;
			return;
		}

		if (load.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			load.useable = false;
			loadgame();
			return;
		}

		FlushBatchDraw();
	}
} 

//操作选择
void gameoperating()
{
	cleardevice();
	IMAGE background;
	loadimage(&background, "image/start.png");
	int height = background.getheight();
	int width = background.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &background, SRCCOPY);
		}
	}

	settextcolor(RED);
	settextstyle(80, 0, _T("华文行楷"));
	outtextxy((WIDTH / 2) - 340, 120, _T("选择操作方式"));
	settextcolor(BLACK);
	settextstyle(20, 0, _T("宋体"));
	FlushBatchDraw();

	MOUSEMSG msg;
	msg = GetMouseMsg();

	button keyboard;
	keyboard.x = 260;
	keyboard.y = 300;
	keyboard.width = 128;
	keyboard.height = 48;
	sprintf_s(keyboard.info, "键盘操纵");
	keyboard.useable = true;

	button mou;
	mou.x = 260;
	mou.y = 480;
	mou.width = 128;
	mou.height = 48;
	sprintf_s(mou.info, "鼠标操纵");
	mou.useable = true;

	while (1)
	{
		if (MouseHit())
		{
			msg = GetMouseMsg();

			checkMouseOn(keyboard, msg);
			checkMouseOn(mou, msg);

			checkMouseClick(keyboard, msg);
			checkMouseClick(mou, msg);

		}

		drawButton(keyboard);
		drawButton(mou);

		if (keyboard.isMouseClick)
		{
			keyboard.useable = false;
			mou.useable = false;
			global.operatchoose = 1;
			return;
		}

		if (mou.isMouseClick)
		{
			keyboard.useable = false;
			mou.useable = false;
			global.operatchoose = 0;
			return;
		}
		FlushBatchDraw();
	}
}

//显示排行榜
void showrank()
{
	cleardevice();
	IMAGE background;
	loadimage(&background, "image/start.png");
	int height = background.getheight();
	int width = background.getwidth();
	for (int i = 0; i <= HEIGHT / height; i++)
	{
		for (int j = 0; j <= WIDTH / width; j++)
		{
			putimage(j * width, i * height, &background, SRCCOPY);
		}
	}

	settextcolor(RED);
	settextstyle(80, 0, _T("华文行楷"));
	outtextxy((WIDTH / 2) - 340, 120, _T("选择排行榜难度"));
	settextcolor(BLACK);
	settextstyle(20, 0, _T("宋体"));
	FlushBatchDraw();

	MOUSEMSG msg;
	msg = GetMouseMsg();

	button easy;
	easy.x = 260;
	easy.y = 280;
	easy.width = 128;
	easy.height = 48;
	sprintf_s(easy.info, "简单");

	button normal;
	normal.x = 260;
	normal.y = 400;
	normal.width = 128;
	normal.height = 48;
	sprintf_s(normal.info, "中等");

	button hard;
	hard.x = 260;
	hard.y = 520;
	hard.width = 128;
	hard.height = 48;
	sprintf_s(hard.info, "困难");

	while (1)
	{
		if (MouseHit())
		{
			msg = GetMouseMsg();

			checkMouseOn(easy, msg);
			checkMouseOn(normal, msg);
			checkMouseOn(hard, msg);

			checkMouseClick(easy, msg);
			checkMouseClick(normal, msg);
			checkMouseClick(hard, msg);

		}

		drawButton(easy);
		drawButton(normal);
		drawButton(hard);

		if (easy.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			global.rankchoose = 1;
			break;
		}

		if (normal.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			global.rankchoose = 2;
			break;
		}

		if (hard.isMouseClick)
		{
			easy.useable = false;
			normal.useable = false;
			hard.useable = false;
			global.rankchoose = 3;
			break;
		}
		FlushBatchDraw();
	}

	struct HighScore* head;
	head = readHighscore();
	head = head->next;
	UpdateScore(head);
	drawhighscore(head);
}

//存档
void storagegame()
{
	FILE* fp;
	if (fopen_s(&fp, "data\\game.txt", "w")) printf("error");
	else
	{
		fwrite(&cat, sizeof(struct Cat), 1, fp);
		fwrite(mouse, sizeof(struct Mouse), 25, fp);
		fwrite(&global, sizeof(struct Global), 1, fp);
		fwrite(&speedfish, sizeof(struct SpeedFish), 1, fp);
		fwrite(&strongfish, sizeof(struct SpeedFish), 1, fp);
	}

	fclose(fp);
}

//读档
void loadgame()
{
	char ch;
	FILE* fp;
	if (fopen_s(&fp, "data\\game.txt", "r")) printf("error");
	else
	{
			fread(mouse, sizeof(struct Mouse), 25, fp);
			fread(&global, sizeof(struct Global), 1, fp);
			fread(&speedfish, sizeof(struct SpeedFish), 1, fp);
			fread(&strongfish, sizeof(struct SpeedFish), 1, fp);
	}

	fclose(fp);
}