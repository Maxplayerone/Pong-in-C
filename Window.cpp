#include "SDL.h"
#include <stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<time.h>

#define WIDTH 640
#define HEIGHT 480


SDL_Window* window;
SDL_Renderer* renderer;

bool Initialize(void);
void Update(float);
void Shutdown(void);

typedef struct Ball {
	int x;
	int y;
	int xSpeed;
	int ySpeed;
	int size;
}Ball;

const int BALL_SIZE = 10;
Ball ball;

Ball MakeBall(int);
int CoinFlip(void);
void RenderBall(const Ball*);
void UpdateBall(Ball*, float);
bool served = false;

typedef struct Player {
	int score;
	int yPos;
}Player;

Player MakePlayer(void);
void UpdatePlayers(float);
void RenderPlayers(void);

Player player1;
Player player2;

const int PLAYER_MARGIN = 10;
const int PLAYER_WIDTH = 20;
const int PLAYER_HEIGHT = 75;
const int PLAYER_MOVE_SPEED = 250;

void UpdateScore(int, int);

int main(int argc, char* argv[]) {
	srand(time(0));
	atexit(Shutdown);

	if(!Initialize()){
		exit(1);
	}

	bool quit = false;
	SDL_Event event;
	Uint32 lastTick = SDL_GetTicks();

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}
		Uint32 currentTick = SDL_GetTicks();
		Uint32 diff = currentTick - lastTick;
		float elapsed = diff / 1000.0f;
		Update(elapsed);
		lastTick = currentTick;
	}
	
}

bool Initialize(void) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Could load sdl");
		return false;
	}
	//tworzenie okna
	window = SDL_CreateWindow(
		"Pong",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WIDTH,
		HEIGHT,
		SDL_WINDOW_SHOWN
	);
	if (!window)
		return false;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!window)
		return false;

	ball = MakeBall(BALL_SIZE);
	player1 = MakePlayer();
	player2 = MakePlayer();
	return true;
}

void Update(float elapsed) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	UpdateBall(&ball, elapsed);
	RenderBall(&ball);
	
	UpdatePlayers(elapsed);
	RenderPlayers();

	SDL_RenderPresent(renderer);
}

void Shutdown(void) {
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();
}

Ball MakeBall(int size){
	const float SPEED = 120.0f;
	Ball ball;
	ball.x = WIDTH / 2 - size / 2;
	ball.y = HEIGHT / 2 - size / 2;
	ball.xSpeed = CoinFlip() * SPEED;
	ball.ySpeed = CoinFlip() * SPEED;
	ball.size = size;

	return ball;
}
//1 is true and 0 is false
int CoinFlip(void) {
	return rand() % 2 == 1 ? true : false;
}

void RenderBall(const Ball* ball) {
	int size = ball->size;
	int halfSize = size / 2;
	SDL_Rect rect;
	rect.x = ball->x - halfSize;
	rect.y = ball->y - halfSize;
	rect.w = size;
	rect.h = size;

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &rect);
}

void UpdateBall(Ball* ball, float elapsed) {
	if (!served) {
		ball->x = WIDTH / 2;
		ball->y = HEIGHT / 2;
		return;
	}
	ball->x += ball->xSpeed * elapsed;
	ball->y += ball->ySpeed * elapsed;

	if (ball->x < BALL_SIZE / 2)
		ball->xSpeed = fabs(ball->xSpeed);
	if (ball->x > WIDTH - BALL_SIZE / 2)
		ball->xSpeed = -fabs(ball->xSpeed);
	if (ball->y < BALL_SIZE / 2)
		ball->ySpeed = fabs(ball->ySpeed);
	if (ball->y > HEIGHT - BALL_SIZE / 2)
		ball->ySpeed = -fabs(ball->ySpeed);
}

Player MakePlayer(void) {
	Player player;
	player.yPos = HEIGHT / 2;
	return player;
}

void UpdatePlayers(float elapsed) {
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

	//checking the window barrier
	if (player1.yPos < PLAYER_HEIGHT / 2)
		player1.yPos = PLAYER_HEIGHT / 2;
	if (player1.yPos >= HEIGHT - PLAYER_HEIGHT / 2)
		player1.yPos = HEIGHT - PLAYER_HEIGHT / 2;
	if (player2.yPos < PLAYER_HEIGHT / 2)
		player2.yPos = PLAYER_HEIGHT / 2;
	if (player2.yPos >= HEIGHT - PLAYER_HEIGHT / 2)
		player2.yPos = HEIGHT - PLAYER_HEIGHT / 2;

	//moving both players
	if (keyboardState[SDL_SCANCODE_SPACE])
		served = true;
	if (keyboardState[SDL_SCANCODE_W])
		player1.yPos -= PLAYER_MOVE_SPEED * elapsed;
	if (keyboardState[SDL_SCANCODE_S])
		player1.yPos += PLAYER_MOVE_SPEED * elapsed;

	if (keyboardState[SDL_SCANCODE_UP])
		player2.yPos -= PLAYER_MOVE_SPEED * elapsed;
	if (keyboardState[SDL_SCANCODE_DOWN])
		player2.yPos += PLAYER_MOVE_SPEED * elapsed;

	//checking for collsion between the ball and the players
	SDL_Rect ballRect;
	ballRect.x = ball.x - ball.size / 2;
	ballRect.y = ball.y - ball.size / 2;
	ballRect.w = ball.size;
	ballRect.h = ball.size;

	SDL_Rect player1Rect;
	player1Rect.x = PLAYER_MARGIN;
	player1Rect.y = (int)player1.yPos - PLAYER_HEIGHT / 2;
	player1Rect.w = PLAYER_WIDTH;
	player1Rect.h = PLAYER_HEIGHT;

	if (SDL_HasIntersection(&ballRect, &player1Rect)) {
		ball.xSpeed = fabs(ball.xSpeed);
		UpdateScore(1, 100);
	}

	SDL_Rect player2Rect;
	player2Rect.x = WIDTH - (PLAYER_MARGIN + PLAYER_WIDTH);
	player2Rect.y = (int)player2.yPos - PLAYER_HEIGHT / 2;
	player2Rect.w = PLAYER_WIDTH;
	player2Rect.h = PLAYER_HEIGHT;

	if (SDL_HasIntersection(&ballRect, &player2Rect)) {
		ball.xSpeed = -fabs(ball.xSpeed);
		UpdateScore(2, 100);
	}
}

void RenderPlayers(void) {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_Rect player1Rect;
	player1Rect.x = PLAYER_MARGIN;
	player1Rect.y = (int)player1.yPos - PLAYER_HEIGHT / 2;
	player1Rect.w = PLAYER_WIDTH;
	player1Rect.h = PLAYER_HEIGHT;
	SDL_RenderFillRect(renderer, &player1Rect);

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_Rect player2Rect;
	player2Rect.x = WIDTH - (PLAYER_MARGIN + PLAYER_WIDTH);
	player2Rect.y = (int)player2.yPos - PLAYER_HEIGHT / 2;
	player2Rect.w = PLAYER_WIDTH;
	player2Rect.h = PLAYER_HEIGHT;
	SDL_RenderFillRect(renderer, &player2Rect);
}

void UpdateScore(int player, int points) {
	served = true;
	/*
	if (player == 1)
		player1.score += points;
	if (player == 2)
		player2.score += points;

	char* fmt = "Player1 %d | Player2 %d";
	int len = snprintf(NULL, 0, fmt, player1.score, player2.score);
	char buf[100];
	snprintf(buf, len + 1, fmt, player1.score, player2.score);
	SDL_SetWindowTitle(window, buf);
	*/
}
