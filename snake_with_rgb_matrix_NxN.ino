#include <Adafruit_NeoPixel.h>

#include <Adafruit_GFX.h>
#include <gfxfont.h>

#define PIN 10
#define UP 1
#define LEFT 2
#define RIGHT 3
#define DOWN 4
const int JOYSTICK_XPIN = A0;
const int JOYSTICK_YPIN = A1;

#define ROWS 16
#define COLS 16

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);
uint32_t RED = matrix.Color(255, 0, 0);
uint32_t GREEN = matrix.Color(0, 255, 0);
uint32_t BLUE = matrix.Color(0, 0, 255);
uint32_t CLEAR = matrix.Color(0, 0, 0); 
uint32_t PURPLE = matrix.Color(255, 0, 255); 
uint32_t YELLOW = matrix.Color(255, 255, 0);

struct point {
  int x;
  int y;
};

point player[ROWS * COLS];

int playerDirection;
int playerLength;
point playerHead;

point apple;

int board[ROWS][COLS];

unsigned long lastClockTick;
int gameRate;
int numApplesEaten = 0;

void setup() { 
  matrix.begin();  
  matrix.setBrightness(15);
  matrix.show();
  randomSeed(analogRead(0));

  defineBoard();  
  startGame();    
}

void defineBoard() {  
  for(int i = 0; i < COLS; i++) {
    board[0][i] = 1;    
    board[ROWS - 1][i] = 1;
  }  
  generateRandomBoard();
}

void generateRandomBoard() {
  for(int i = 1; i < ROWS - 1; i++) {
    for(int j = 0; j < COLS; j++) {
      if(j == 0 || j == (COLS - 1)) {
        board[i][j] = 1;         
      } else {
        board[i][j] = 0;
      }
    }
  }
  // 20 random pixels set, but make sure that it won't cause the player to lose (diagonal pixels)
  for(int i = 0; i < 20; i++) {
    bool found = false;
    int x = random(1, ROWS - 1);
    int y = random(1, COLS - 1);
    while(!found) {          
      if(!boardContainsCoordinates(x, y)
         && !boardContainsCoordinates(x - 1, y - 1)         
         && !boardContainsCoordinates(x + 1, y - 1)
         && !boardContainsCoordinates(x - 1, y + 1) 
         && !boardContainsCoordinates(x + 1, y + 1)){
         found = true;
      } else {
        x = random(1, ROWS - 1);
        y = random(1, COLS - 1);
      }
    }
    board[x][y] = 1;
  }
}

void startGame() {
  matrix.clear();
  resetGameVariables();
  drawBoard();  
  drawPlayer();  
  drawApple();
  matrix.show();  
}

void resetGameVariables() {  
  generateRandomBoard();
  bool found = false;  
  while(!found) {
    // start the player in a random spot
    playerHead.x = random(1, ROWS - 1);
    playerHead.y = random(1, COLS - 1);
    int startDirection = random(0, 2);
    
    if(startDirection == 0) {
      if(playerHead.y < COLS / 2) {
        playerDirection = RIGHT;
      } else {
        playerDirection = LEFT;
      }
    } else {
      if(playerHead.x < ROWS / 2) {
        playerDirection = DOWN;
      } else {
        playerDirection = UP;
      }  
    }

    if(playerHas5Moves()) {
      found = true;
    }
  }

  generateApple();

  playerLength = 1;
  player[0].x = playerHead.x;
  player[0].y = playerHead.y;

  lastClockTick = millis();
  gameRate = 300;  
  numApplesEaten = 0;
}
bool playerHas5Moves() {
  for(int i = 0; i < 5; i++) {
    switch(playerDirection) {
      case RIGHT:
        if(board[playerHead.x][playerHead.y + i] == 1) {
          return false;
        }
        break;
      case LEFT:
        if(board[playerHead.x][playerHead.y - i] == 1) {
          return false;
        }
        break;
      case UP:
        if(board[playerHead.x - i][playerHead.y] == 1) {
          return false;
        }
        break;
      case DOWN:
        if(board[playerHead.x + i][playerHead.y] == 1) {
          return false;
        }
        break;
    }
  }
  return true;
}
void generateApple() {
  bool found = false;
  while(!found) {    
    apple.x = random(1, ROWS - 1);
    apple.y = random(1, COLS - 1);
    if(!playerContainsCoordinates(apple.x, apple.y) && !boardContainsCoordinates(apple.x, apple.y)) {       
      found = true;
    }
  }  
}
bool playerContainsCoordinates(int x, int y) {  
  for(int i = 0; i < playerLength; i++) {
    if(player[i].x == x && player[i].y == y) {      
      return true;
    }
  }
  return false;
}
bool boardContainsCoordinates(int x, int y) {
  return board[x][y] == 1;
}

void drawPlayer() {  
  for(int i = 0; i < playerLength; i++) {        
    matrix.setPixelColor(convertToMatrixPoint(player[i].x, player[i].y), BLUE);
  }  
}
void drawBoard() {
  for(int i = 0; i < ROWS; i++) {
    for(int j = 0; j < COLS; j++) {
      if(board[i][j] == 1) {        
        matrix.setPixelColor(convertToMatrixPoint(i, j), GREEN);     
      } else {
        matrix.setPixelColor(convertToMatrixPoint(i, j), CLEAR);
      }
    }
  }  
  matrix.setPixelColor(convertToMatrixPoint(0, 0), BLUE);   // identify the bottom left pixel 
  // identify the matrix start pixels and direction
  matrix.setPixelColor(0, PURPLE);
  matrix.setPixelColor(1, YELLOW);
}
void drawApple() {
  matrix.setPixelColor(convertToMatrixPoint(apple.x, apple.y), RED);
}

void loop() {  
  float x = analogRead(JOYSTICK_XPIN);
  float y = analogRead(JOYSTICK_YPIN);

  float deltax = abs(510 - x);
  float deltay = abs(505 - y);

  if(deltax > deltay) {
    if(x < 480) {
      playerDirection = DOWN;
    } else if(x > 540) {
      playerDirection = UP;
    }
  } else {
    if(y < 475) {
      playerDirection = RIGHT;
    } else if(y > 535) {
      playerDirection = LEFT;
    }
  }  
      
  if(millis() - lastClockTick > gameRate) {
    advancePlayer();    
    detectCollision();   
    detectAppleEaten();
    updateBoard();
    lastClockTick = millis();
  }
}

void advancePlayer() {
  if(playerDirection == LEFT) {
    playerHead.y -= 1;
  } else if(playerDirection == RIGHT) {    
    playerHead.y += 1;    
  } else if(playerDirection == UP) {    
    playerHead.x -= 1;    
  } else if(playerDirection == DOWN) {        
    playerHead.x += 1;
  }
  // see if this point already exists in the player's matrix
  for(int i = 0; i < playerLength; i++) {
    if(player[i].x == playerHead.x && player[i].y == playerHead.y) {
      gameOver();
    }
  }
  for(int i = playerLength - 1; i > 0; i--) {
    player[i] = player[i - 1];
  }
  player[0].x = playerHead.x;
  player[0].y = playerHead.y;
}

void detectCollision() {
  if(board[playerHead.x][playerHead.y] == 1) {
    gameOver();
  }
}
void detectAppleEaten() {
  if(playerHead.x == apple.x && playerHead.y == apple.y) {
    numApplesEaten++;
    playerLength += 1;
    player[playerLength - 1].x = playerHead.x;
    player[playerLength - 1].y = playerHead.y;
    if(numApplesEaten % 5 == 0 && gameRate > 100) {
      gameRate -= 20;
    }
    generateApple();
  }
}

void updateBoard() {  
  drawBoard();
  drawPlayer();
  drawApple();
  matrix.show();
}
void gameOver() {
  matrix.clear();
  for(int i = 0; i < ROWS; i++) {
    for(int j = 0; j < COLS; j++) {
      matrix.setPixelColor(convertToMatrixPoint(i, j), GREEN);      
    }    
  }
  matrix.show();
  delay(3000);
  startGame();
}

int convertToMatrixPoint(int i, int j) {
  if(i % 2 == 0) {
    return (COLS * i) + (COLS - 1) - j;
  } else {
    return (COLS * i) + j;
  }
}
