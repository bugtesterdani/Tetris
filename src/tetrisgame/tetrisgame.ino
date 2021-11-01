/***********************************************************
    * tetris -- Programm to Play the Old Game Tetris       *
    *                                                      *
    * Author:  Daniel, Glasner                             *
    * Build:   2021-10-31 (Rev. 1)                         *
    *                                                      *
    * Purpose:  Learning C Code Programming for the        *
    *           CircuitMess Board. In this case the Tetris *
    *           Game.                                      *
    *                                                      *
    * Usage:                                               *
    *      Run this Game at a CircuitMess Ringo Phone      *
    *                                                      *
    * Bugs:                                                *
    *      There is no real Bug, but the Pause and Menu    *
    *      should be shown. Also the Highscore Table.      *
    *                                                      *
    *      Actual reaching the Pause:                      *
    *         - Pressing B (One Time when Playing)         *
    *      Actual exit the Pause:                          *
    *         - Pressing B again (One Time when Pausing)   *
    *      Actual restart the Game:                        *
    *         - Pressing A (One Time when Pausing)         *
    ********************************************************/
	
// Changes:
// 	- 2021-11-01: DG
// 		- Tetris Background Music added
//		- New Blocks added (inserted all 7 Blocks of Tetris)
//			(https://www.russlandjournal.de/wp-content/uploads/2014/07/tetris-steine.jpg)

#include <Arduino.h>
#include "MAKERphone.h"
#include <FastLED/FastLED.h>
#include <utility/Buttons/Buttons.h>
#include <utility/soundLib/MPWavLib.h>

#include "Sprite.h"

#define BACKGROUND      TFT_BLACK
#define TEXTCOLOR_W     TFT_WHITE
#define TEXTCOLOR_G     TFT_GREEN
#define TEXTCOLOR_Y     TFT_YELLOW
#define TEXTCOLOR_B     TFT_BLUE
#define TEXTCOLOR_S     TFT_BLACK

#define BLOCKSIZE       2
#define BLOCKRED        0xf800
#define BLOCKYELLOW     0xffe0
#define BLOCKBLUE       0x001f
#define BLOCKGREEN      0x07e0
#define BLOCKORANGE     0xfbc0
#define BLOCKPURPLE     0xf81f
#define BLOCKLIGHTBLUE  0x079f
#define BLOCKINVISIBLE  0x0120

#define BLOCK_x_MIN_px  28
#define BLOCK_x_MAX_px  128
#define BLOCK_y_MIN_px  36
#define BLOCK_y_MAX_px  76

#define MUSIC_MENU_START  0
#define MUSIC_GAME_START  1
#define MUSIC_STOP        2

#define CounterLevelAStart    10
#define CounterLevelBStart    700
#define CounterLevelCStart    400
#define CounterLevelAMinimum  50
#define CounterLevelBMinimum  30
#define CounterLevelCMinimum  10
#define CounterDown           5
#define CounterSteps          2

#define BLOCK_MINNUM    0
#define BLOCK_MAXNUM    18

#define BLOCK1_START    0
#define BLOCK1_SIZE     4
#define BLOCK1_END      3

#define BLOCK2_START    4
#define BLOCK2_SIZE     4
#define BLOCK2_END      7

#define BLOCK3_START    8
#define BLOCK3_SIZE     2
#define BLOCK3_END      9

#define BLOCK4_START    10
#define BLOCK4_SIZE     1
#define BLOCK4_END      10

#define BLOCK5_START    11
#define BLOCK5_SIZE     2
#define BLOCK5_END      12

#define BLOCK6_START    13
#define BLOCK6_SIZE     2
#define BLOCK6_END      14

#define BLOCK7_START    15
#define BLOCK7_SIZE     4
#define BLOCK7_END      18

#define GamePlayArray_X_MAX   25
#define GamePlayArray_Y_MAX   10

MAKERphone mp;
Oscillator *osc;

const char *bgMusicPath = "/Tetris/Tetris_Theme.wav";
const char *highscoresPath = "/Tetris/highscores.sav";
const char *folderPath = "/Tetris";
uint16_t HighScores[4];

uint16_t GamePlay[GamePlayArray_Y_MAX][GamePlayArray_X_MAX];
uint8_t x;
uint8_t y;

uint16_t CounterBlockFalling_Counter;
uint16_t CounterBlockFalling_Limit;
uint8_t Level;
uint32_t Score;
uint16_t BlocksLeftLevel;
uint8_t SelectedLevel;
bool PauseGame_State;

Sprite UsageBlock;
Sprite NextBlock;
Sprite Block1[BLOCK1_SIZE];
Sprite Block2[BLOCK2_SIZE];
Sprite Block3[BLOCK3_SIZE];
Sprite Block4[BLOCK4_SIZE];
Sprite Block5[BLOCK5_SIZE];
Sprite Block6[BLOCK6_SIZE];
Sprite Block7[BLOCK7_SIZE];
Sprite BackgroundImageGame;
Sprite BackgroundImageMenu;
uint8_t actualBlockUsed;
uint8_t nextBlock;
uint8_t LinesRemoved;

MPTrack *bgMusic;

void setup()
{
  MPInitialize();
  StartGame();
}

void loop()
{
  if (mp.buttons.released(BTN_B))
  {
    mp.buttons.update();
    PauseGame_State ^= true;
    if (PauseGame_State)
    {
      bgMusic->pause();
    }
    else
    {
      bgMusic->resume();
    }
  }
  if (PauseGame_State)
  {
    if (mp.buttons.released(BTN_A))
    {
      mp.buttons.update();
      StartGame();
    }
    mp.update();
    return;
  }
  
  if (CheckGameOver())
  {
    return;
  }

  mp.display.fillScreen(BACKGROUND);
  mp.display.drawIcon(BackgroundImageGame.Data, 0, 0,
    BackgroundImageGame.w, BackgroundImageGame.h, 1, TFT_TRANSPARENT);

  CheckButtonPressed();
  CheckJoystick();
  FallBlockCounterBased();
  PrintBlocks();
  printNextBlock();
  PrintScore();
  printLines();
  printLevel();
  
  mp.update();
}

///////////////////////////////////////////////////////////
/////// Setup
///////////////////////////////////////////////////////////

void MPInitialize()
{
  Serial.begin(115200);
  mp.begin();
  mp.display.fillScreen(BACKGROUND);
  InitializeSprite();
  bgMusic = new MPTrack(bgMusicPath);
  bgMusic->setVolume(40);
  addTrack(bgMusic);
  bgMusic->setRepeat(1);
  bgMusic->rewind();
  bgMusic->play();
  osc = new Oscillator(SINE);
  addOscillator(osc);
  osc->setVolume(60);
  LoadRankingFile();
  BootAnimation();
}

void InitializeSprite()
{
  // Red Blocks
  Block1[0].Data = image_data_Block1_1;
  Block1[0].w = 6;
  Block1[0].h = 4;
  Block1[1].Data = image_data_Block1_2;
  Block1[1].w = 4;
  Block1[1].h = 6;
  Block1[2].Data = image_data_Block1_3;
  Block1[2].w = 6;
  Block1[2].h = 4;
  Block1[3].Data = image_data_Block1_4;
  Block1[3].w = 4;
  Block1[3].h = 6;

  // Yellow Blocks
  Block2[0].Data = image_data_Block2_1;
  Block2[0].w = 6;
  Block2[0].h = 4;
  Block2[1].Data = image_data_Block2_2;
  Block2[1].w = 4;
  Block2[1].h = 6;
  Block2[2].Data = image_data_Block2_3;
  Block2[2].w = 6;
  Block2[2].h = 4;
  Block2[3].Data = image_data_Block2_4;
  Block2[3].w = 4;
  Block2[3].h = 6;

  // Blue Blocks
  Block3[0].Data = image_data_Block3_1;
  Block3[0].w = 2;
  Block3[0].h = 8;
  Block3[1].Data = image_data_Block3_2;
  Block3[1].w = 2;
  Block3[1].h = 8;
  
  // Green Blocks
  Block4[0].Data = image_data_Block4_1;
  Block4[0].w = 4;
  Block4[0].h = 4;
  
  // Orange Blocks
  Block5[0].Data = image_data_Block5_1;
  Block5[0].w = 6;
  Block5[0].h = 4;
  Block5[1].Data = image_data_Block5_2;
  Block5[1].w = 4;
  Block5[1].h = 6;
  
  // Purple Blocks
  Block6[0].Data = image_data_Block6_1;
  Block6[0].w = 6;
  Block6[0].h = 4;
  Block6[1].Data = image_data_Block6_2;
  Block6[1].w = 4;
  Block6[1].h = 6;
  
  // Light Blue Blocks
  Block7[0].Data = image_data_Block7_1;
  Block7[0].w = 6;
  Block7[0].h = 4;
  Block7[1].Data = image_data_Block7_2;
  Block7[1].w = 4;
  Block7[1].h = 6;
  Block7[2].Data = image_data_Block7_3;
  Block7[2].w = 6;
  Block7[2].h = 4;
  Block7[3].Data = image_data_Block7_4;
  Block7[3].w = 4;
  Block7[3].h = 6;

  BackgroundImageGame.Data = image_data_Tetris_Background;
  BackgroundImageGame.w = 160;
  BackgroundImageGame.h = 128;
}

void LoadRankingFile()
{
  if (!SD.exists(folderPath))
    SD.mkdir(folderPath);
  File file = SD.open(highscoresPath);
  JsonArray &highscores = mp.jb.parseArray(file);
  file.close();
  if (highscores.size() == 0)
  {
    JsonArray &highscores = mp.jb.createArray();
    JsonObject &test = mp.jb.createObject();
    test["Name"] = "ABC";
    test["Score"] = 0;
    test["Rank"] = 1;
    highscores.add(test);
    File file = SD.open(highscoresPath, "w");
    highscores.prettyPrintTo(file);
    file.close();
    mp.readFile(highscoresPath);
  }
}

void BootAnimation()
{
  uint8_t stSize = 1;
  for (uint8_t i = 60; i >= 30; i -= 12)
  {
    mp.display.fillScreen(BACKGROUND);
    mp.display.setCursor(i, 45, 1);
    mp.display.setTextColor(TEXTCOLOR_W);
    mp.display.setTextSize(stSize);
    stSize++;
    mp.display.println(F("TETRIS"));
    while (!mp.update());
    FastLED.showColor(CRGB::Green);
    delay(500);
  }
  mp.display.fillScreen(BACKGROUND);
  mp.display.setCursor(6, 45, 1);
  mp.display.setTextColor(TEXTCOLOR_G);
  mp.display.setTextSize(4);
  mp.display.print(F("TE"));
  mp.display.setTextColor(TEXTCOLOR_Y);
  mp.display.print(F("TR"));
  mp.display.setTextColor(TEXTCOLOR_B);
  mp.display.println(F("IS"));
  while (!mp.update());
  delay(1000);
}

void StartGame()
{
  ResetVariables();
  NewBlock();
}

void ResetVariables()
{
  for (int i = 0; i < GamePlayArray_Y_MAX; i++)
  {
    for (int j = 0; j < GamePlayArray_X_MAX; j++)
    {
      GamePlay[i][j] = BLOCKINVISIBLE;
    }
  }
  x = 0;
  y = 0;
  actualBlockUsed = 0;
  CounterBlockFalling_Counter = 0;
  CounterBlockFalling_Limit = CounterLevelAStart;
  Score = 0;
  nextBlock = random(BLOCK_MINNUM, BLOCK_MAXNUM);
  LinesRemoved = 0;
  PauseGame_State = false;
}

void NewBlock()
{
  actualBlockUsed = nextBlock;
  nextBlock = random(BLOCK_MINNUM, BLOCK_MAXNUM);
  if (actualBlockUsed >= BLOCK1_START && actualBlockUsed <= BLOCK1_END)
  {
    UsageBlock = Block1[actualBlockUsed - BLOCK1_START];
  }
  else if (actualBlockUsed >= BLOCK2_START && actualBlockUsed <= BLOCK2_END)
  {
    UsageBlock = Block2[actualBlockUsed - BLOCK2_START];
  }
  else if (actualBlockUsed >= BLOCK3_START && actualBlockUsed <= BLOCK3_END)
  {
    UsageBlock = Block3[actualBlockUsed - BLOCK3_START];
  }
  else if (actualBlockUsed >= BLOCK4_START && actualBlockUsed <= BLOCK4_END)
  {
    UsageBlock = Block4[actualBlockUsed - BLOCK4_START];
  }
  else if (actualBlockUsed >= BLOCK5_START && actualBlockUsed <= BLOCK5_END)
  {
    UsageBlock = Block5[actualBlockUsed - BLOCK5_START];
  }
  else if (actualBlockUsed >= BLOCK6_START && actualBlockUsed <= BLOCK6_END)
  {
    UsageBlock = Block6[actualBlockUsed - BLOCK6_START];
  }
  else if (actualBlockUsed >= BLOCK7_START && actualBlockUsed <= BLOCK7_END)
  {
    UsageBlock = Block7[actualBlockUsed - BLOCK7_START];
  }
  if (nextBlock >= BLOCK1_START && nextBlock <= BLOCK1_END)
  {
    NextBlock = Block1[nextBlock - BLOCK1_START];
  }
  else if (nextBlock >= BLOCK2_START && nextBlock <= BLOCK2_END)
  {
    NextBlock = Block2[nextBlock - BLOCK2_START];
  }
  else if (nextBlock >= BLOCK3_START && nextBlock <= BLOCK3_END)
  {
    NextBlock = Block3[nextBlock - BLOCK3_START];
  }
  else if (nextBlock >= BLOCK4_START && nextBlock <= BLOCK4_END)
  {
    NextBlock = Block4[nextBlock - BLOCK4_START];
  }
  else if (nextBlock >= BLOCK5_START && nextBlock <= BLOCK5_END)
  {
    NextBlock = Block5[nextBlock - BLOCK5_START];
  }
  else if (nextBlock >= BLOCK6_START && nextBlock <= BLOCK6_END)
  {
    NextBlock = Block6[nextBlock - BLOCK6_START];
  }
  else if (nextBlock >= BLOCK7_START && nextBlock <= BLOCK7_END)
  {
    NextBlock = Block7[nextBlock - BLOCK7_START];
  }
  x = BLOCK_x_MIN_px;
  y = (((BLOCK_y_MAX_px - BLOCK_y_MIN_px) / 2) + BLOCK_y_MIN_px);
}

///////////////////////////////////////////////////////////
/////// Loop
///////////////////////////////////////////////////////////

void CheckButtonPressed()
{
  if (mp.buttons.pressed(BTN_A))
  {
    mp.buttons.update();
    rotateFallingBlock();
  }
}

void rotateFallingBlock()
{
  if (actualBlockUsed >= BLOCK1_START && actualBlockUsed <= BLOCK1_END)
  {
    rotateFallingBlock(Block1, BLOCK1_START, BLOCK1_END);
  }
  else if (actualBlockUsed >= BLOCK2_START && actualBlockUsed <= BLOCK2_END)
  {
    rotateFallingBlock(Block2, BLOCK2_START, BLOCK2_END);
  }
  else if (actualBlockUsed >= BLOCK3_START && actualBlockUsed <= BLOCK3_END)
  {
    rotateFallingBlock(Block3, BLOCK3_START, BLOCK3_END);
  }
  else if (actualBlockUsed >= BLOCK4_START && actualBlockUsed <= BLOCK4_END)
  {
    rotateFallingBlock(Block4, BLOCK4_START, BLOCK4_END);
  }
  else if (actualBlockUsed >= BLOCK5_START && actualBlockUsed <= BLOCK5_END)
  {
    rotateFallingBlock(Block5, BLOCK5_START, BLOCK5_END);
  }
  else if (actualBlockUsed >= BLOCK6_START && actualBlockUsed <= BLOCK6_END)
  {
    rotateFallingBlock(Block6, BLOCK6_START, BLOCK6_END);
  }
  else if (actualBlockUsed >= BLOCK7_START && actualBlockUsed <= BLOCK7_END)
  {
    rotateFallingBlock(Block7, BLOCK7_START, BLOCK7_END);
  }
}

void rotateFallingBlock(Sprite Block[], uint8_t BLOCKStart, uint8_t BLOCKEnd)
{
  uint8_t BlockUsedBackup = actualBlockUsed;
  
  actualBlockUsed++;
  if (actualBlockUsed > BLOCKEnd)
  {
    actualBlockUsed = BLOCKStart;
  }
  UsageBlock = Block[actualBlockUsed - BLOCKStart];

  if (!CheckBlockWall(y) || !CheckBlockTopBottom(x) || !CheckBlockGame())
  {
    actualBlockUsed = BlockUsedBackup;
    UsageBlock = Block[actualBlockUsed - BLOCKStart];
  }
}

bool CheckBlockWall(uint8_t _y)
{
  if (_y + (BLOCKSIZE * UsageBlock.w) > BLOCK_y_MAX_px)
  {
    return false;
  }
  if (_y < BLOCK_y_MIN_px)
  {
    return false;
  }
  return true;
}

bool CheckBlockTopBottom(uint8_t _x)
{
  if (_x + (BLOCKSIZE * UsageBlock.h) > BLOCK_x_MAX_px)
  {
    return false;
  }
  if (_x < BLOCK_x_MIN_px)
  {
    return false;
  }
  return true;
}

bool CheckBlockGame()
{
  uint8_t xArray = (x - BLOCK_x_MIN_px) / (2 * BLOCKSIZE);
  uint8_t yArray = (y - BLOCK_y_MIN_px) / (2 * BLOCKSIZE);

  for (int i = 0; i < UsageBlock.h / 2; i++)
  {
    for (int j = 0; j < UsageBlock.w / 2; j++)
    {
      if (UsageBlock.Data[(i * 2 * (UsageBlock.w / 2) + j) * 2] != BLOCKINVISIBLE)
      {
        if (GamePlay[j + yArray][i + xArray] != BLOCKINVISIBLE)
        {
          return false;
        }
      }
    }
  }
  return true;
}

void CheckJoystick()
{
  if (mp.buttons.repeat(BTN_LEFT, 1))
  {
    MoveBlock(BLOCKSIZE, 0, 0);
  }
  else if (mp.buttons.repeat(BTN_RIGHT, 1))
  {
    MoveBlock(0, BLOCKSIZE, 0);
  }
  else if (mp.buttons.repeat(BTN_DOWN, 1))
  {
    Score += 5;
    if (MoveBlock(0, 0, BLOCKSIZE))
    {
      SaveBlock();
      NewBlock();
    }
  }
}

bool MoveBlock(uint8_t _y_Left, uint8_t _y_Right, uint8_t _x_Down)
{
  uint8_t x_Backup = x;
  uint8_t y_Backup = y;
  uint8_t y_ = y;
  uint8_t x_ = x;
  y_ -= _y_Left;
  y_ += _y_Right;
  x_ += _x_Down;

  if (CheckBlockWall(y_) && CheckBlockTopBottom(x_))
  {
    x = x_;
    y = y_;
    if (!CheckBlockGame())
    {
      x = x_Backup;
      y = y_Backup;
      return true;
    }
  }

  if (EndeErreicht())
  {
    return true;
  }

  return false;
}

bool EndeErreicht()
{
  if ((x + (UsageBlock.h * BLOCKSIZE)) >= BLOCK_x_MAX_px)
  {
    return true;
  }
  return false;
}

void SaveBlock()
{
  uint8_t xArray = (x - BLOCK_x_MIN_px) / (2 * BLOCKSIZE);
  uint8_t yArray = (y - BLOCK_y_MIN_px) / (2 * BLOCKSIZE);

  for (int i = 0; i < UsageBlock.h / 2; i++)
  {
    for (int j = 0; j < UsageBlock.w / 2; j++)
    {
      if (UsageBlock.Data[(i * 2 * (UsageBlock.w / 2) + j) * 2] != BLOCKINVISIBLE)
      {
        GamePlay[j + yArray][i + xArray] = UsageBlock.Data[(i * 2 * (UsageBlock.w / 2) + j) * 2];
      }
    }
  }
  CheckRows();
}

void FallBlockCounterBased()
{
  if (CounterBlockFalling_Counter >= CounterBlockFalling_Limit)
  {
    if (MoveBlock(0, 0, BLOCKSIZE))
    {
      SaveBlock();
      NewBlock();
    }
    CounterBlockFalling_Counter = 0;
  }
  CounterBlockFalling_Counter += 1;
}

void PrintBlocks()
{
  for (int i = 0; i < GamePlayArray_Y_MAX; i++)
  {
    for (int j = 0; j < GamePlayArray_X_MAX; j++)
    {
      if (GamePlay[i][j] != BLOCKINVISIBLE)
      {
        uint8_t _x = BLOCK_x_MIN_px + (j * 2 * BLOCKSIZE);
        uint8_t _y = BLOCK_y_MIN_px + (i * 2 * BLOCKSIZE);
        mp.display.fillRect(_y, _x, 2 * BLOCKSIZE, 2 * BLOCKSIZE, GamePlay[i][j]);
      }
    }
  }

  for (int xArray = 0; xArray < UsageBlock.h; xArray++)
  {
    for (int yArray = 0; yArray < UsageBlock.w; yArray++)
    {
      uint16_t color = UsageBlock.Data[(xArray * UsageBlock.w) + yArray];
      if (color != BLOCKINVISIBLE)
      {
        mp.display.fillRect(y + (yArray * BLOCKSIZE), x + (xArray * BLOCKSIZE), BLOCKSIZE, BLOCKSIZE, color);
      }
    }
  }
}

void PrintScore()
{
  bool printScore = true;
  mp.display.setTextColor(TEXTCOLOR_W);
  if (Score < 1000)
  {
    mp.display.setTextSize(2);
    if (Score < 10)
    {
      mp.display.setCursor(125, 22, 1);
    }
    else if (Score < 100)
    {
      mp.display.setCursor(118, 22, 1);
    }
    else
    {
      mp.display.setCursor(111, 22, 1);
    }
  }
  else
  {
    mp.display.setTextSize(1);
    if (Score < 10000)
    {
      mp.display.setCursor(116, 25, 1);
    }
    else if (Score < 100000)
    {
      mp.display.setCursor(113, 25, 1);
    }
    else if (Score < 1000000)
    {
      mp.display.setCursor(110, 25, 1);
    }
    else if (Score < 10000000)
    {
      mp.display.setCursor(107, 25, 1);
    }
    else
    {
      mp.display.setTextSize(2);
      mp.display.setCursor(116, 20, 1);
      printScore = false;
    }
  }
  if (printScore)
  {
    mp.display.println(Score);
  }
  else
  {
    mp.display.println(F("oo"));
  }
}

void printNextBlock()
{
  // x = 98 - 119
  uint8_t _x_NB = (((119 - 98 - (NextBlock.h * BLOCKSIZE)) / 2) + 98);
  // y = 123 - 151
  uint8_t _y_NB = (((151 - 123 - (NextBlock.w * BLOCKSIZE)) / 2) + 123);
  for (int xArray = 0; xArray < NextBlock.h; xArray++)
  {
    for (int yArray = 0; yArray < NextBlock.w; yArray++)
    {
      uint16_t color = NextBlock.Data[(xArray * NextBlock.w) + yArray];
      if (color != BLOCKINVISIBLE)
      {
        mp.display.fillRect(_y_NB + (yArray * BLOCKSIZE), _x_NB + (xArray * BLOCKSIZE), BLOCKSIZE, BLOCKSIZE, color);
      }
    }
  }
}

void printLines()
{
  mp.display.setTextColor(TEXTCOLOR_W);
  mp.display.setTextSize(1);
  if (LinesRemoved < 10)
  {
    mp.display.setCursor(127, 81, 1);
  }
  else if (LinesRemoved < 100)
  {
    mp.display.setCursor(124, 81, 1);
  }
  else if (LinesRemoved < 1000)
  {
    mp.display.setCursor(121, 81, 1);
  }
  else if (LinesRemoved < 10000)
  {
    mp.display.setCursor(118, 81, 1);
  }
  else if (LinesRemoved < 100000)
  {
    mp.display.setCursor(115, 81, 1);
  }
  else
  {
    mp.display.setCursor(112, 81, 1);
  }
  mp.display.println(LinesRemoved);
}

void printLevel()
{
  mp.display.setTextColor(TEXTCOLOR_W);
  mp.display.setTextSize(1);
  if (Level < 10)
  {
    mp.display.setCursor(127, 56, 1);
  }
  else if (Level < 100)
  {
    mp.display.setCursor(124, 56, 1);
  }
  else
  {
    mp.display.setCursor(121, 56, 1);
  }
  mp.display.println(0);
}

void CheckRows()
{
  for (int i = GamePlayArray_X_MAX - 1; i > 0; i--)
  {
    for (int j = 0; j < GamePlayArray_Y_MAX; j++)
    {
      if (GamePlay[j][i] == BLOCKINVISIBLE)
      {
        j = GamePlayArray_Y_MAX;
      }
      else if (j == (GamePlayArray_Y_MAX - 1))
      {
        MoveGameArrayDown(i);
        Score += 1000;
        LinesRemoved += 1;
        j = 0;
      }
    }
  }
}

void MoveGameArrayDown(int from)
{
  for (int i = 0; i < GamePlayArray_Y_MAX; i++)
  {
    for (int j = from; j > 0; j--)
    {
      GamePlay[i][j] = GamePlay[i][j - 1];
    }
    GamePlay[i][0] = BLOCKINVISIBLE;
  }
}

bool CheckGameOver()
{
  for (int i = 0; i < GamePlayArray_Y_MAX; i++)
  {
    if (GamePlay[i][0] != BLOCKINVISIBLE)
    {
      PrintGameOver();
      return true;
    }
  }
  return false;
}

void PrintGameOver()
{
  mp.display.setCursor(10, 30, 1);
  mp.display.setTextColor(TEXTCOLOR_W);
  mp.display.setTextSize(4);
  mp.display.println(F("Game"));
  mp.display.setCursor(10, 70, 1);
  mp.display.println(F("Over"));
  mp.update();
}