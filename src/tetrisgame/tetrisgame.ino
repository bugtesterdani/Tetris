#include <Arduino.h>
#include "MAKERphone.h"
#include <FastLED/FastLED.h>
#include <utility/Buttons/Buttons.h>
#include <utility/soundLib/MPWavLib.h>

#include "Sprite.h"

#define TRUE            1 == 1
#define FALSE           1 == 0
#define SIZEOF(a)       sizeof(a)/sizeof(*a)

#define DEBUGGING       TRUE

#define BACKGROUND      TFT_BLACK
#define TEXTCOLOR_W     TFT_WHITE
#define TEXTCOLOR_G     TFT_GREEN
#define TEXTCOLOR_Y     TFT_YELLOW
#define TEXTCOLOR_B     TFT_BLUE

#define BLOCKSIZE       2
#define BLOCKRED        0xf800
#define BLOCKYELLOW     0x1234
#define BLOCKINVISIBLE  0x0120

#define BLOCK_x_MIN_px  36
#define BLOCK_x_MAX_px  128
#define BLOCK_y_MIN_px  36
#define BLOCK_y_MAX_px  74

#define MUSIC_MENU_START  0
#define MUSIC_GAME_START  1
#define MUSIC_STOP        2

#define MaxCounterFalling 10000

#define BLOCK_MINNUM    0
#define BLOCK_MAXNUM    3
#define BLOCK1_START    0
#define BLOCK1_SIZE     4
#define BLOCK1_END      3

#define GamePlayArray_X_MAX   25
#define GamePlayArray_Y_MAX   10

MAKERphone mp;
Oscillator *osc;
const char *highscoresPath = "/Tetris/highscores.sav";
const char *folderPath = "/Tetris";
uint16_t HighScores[4];

uint16_t GamePlay[GamePlayArray_Y_MAX][GamePlayArray_X_MAX];
uint8_t x;
uint8_t y;

uint16_t CounterBlockFalling;
uint8_t Level;
uint16_t Highscore;
uint16_t BlocksLeftLevel;

Sprite UsageBlock;
Sprite Block1[BLOCK1_SIZE];
Sprite BackgroundImageGame;
Sprite BackgroundImageMenu;
uint8_t actualBlockUsed;

void setup()
{
  SerialInitialize();
#if DEBUGGING
  Serial.println("Initialize");
#endif
  MPInitialize();
  BootupProcess();
}

void loop()
{
#if DEBUGGING
  Serial.println("Loop entered");
#endif
  if (mp.buttons.released(BTN_B))
  {
    mp.buttons.update();
    if (!PauseGame())
    {
      ShowMM();
      ResetVariables();
    }
  }
  
  mp.display.fillScreen(BACKGROUND);
  mp.display.drawIcon(BackgroundImageGame.Data, 0, 0,
    BackgroundImageGame.w, BackgroundImageGame.h, 1, TFT_TRANSPARENT);
  
  CheckButtonsPressed();
  CheckJoystick();
  FallBlockCounterBased();
  Serial.println("ALL DONE");
//  PrintResults();
  PrintBlocks();

#if DEBUGGING
  Serial.println("Loop done");
#endif
  mp.update();
}




void CheckButtonsPressed()
{
  if (mp.buttons.pressed(BTN_A))
  {
#if DEBUGGING
    Serial.println("Doing rotation");
#endif
    rotateFallingBlock();
#if DEBUGGING
    Serial.println("Rotation done");
#endif
  }
}

void CheckJoystick()
{
  if (mp.buttons.repeat(BTN_LEFT, 1))
  {
    if (CheckBlockWall((y - BLOCKSIZE), UsageBlock) == 2)
    {
      if (CheckBlock((y - BLOCKSIZE), x, UsageBlock))
      {
        y -= BLOCKSIZE;
      }
    }
  }
  else if (mp.buttons.repeat(BTN_RIGHT, 1))
  {
    if (CheckBlockWall((y + BLOCKSIZE), UsageBlock) == 2)
    {
      if (CheckBlock((y + BLOCKSIZE), x, UsageBlock))
      {
        y += BLOCKSIZE;
      }
    }
  }
  else if (mp.buttons.repeat(BTN_DOWN, 1))
  {
    MoveFallingBlock();
  }
}

void FallBlockCounterBased()
{
  if (CounterBlockFalling >= MaxCounterFalling)
  {
    CounterBlockFalling = 0;
    MoveFallingBlock();
  }
  CounterBlockFalling++;
}

void rotateFBSub(Sprite Block[], uint8_t BLOCKStart, uint8_t BLOCKEnd)
{
  CounterBlockFalling++;
  if (CounterBlockFalling > BLOCKEnd)
  {
    CounterBlockFalling = BLOCKStart;
  }
  Sprite tmpBlock = Block[CounterBlockFalling - BLOCKStart];

  switch (CheckBlockWall(y, tmpBlock))
  {
    case 0:
      if (CheckBlock(BLOCK_y_MIN_px, x, tmpBlock))
      {
        y = BLOCK_y_MIN_px;
        UsageBlock = tmpBlock;
      }
      else
      {
        CounterBlockFalling--;
        if (CounterBlockFalling < BLOCKStart)
        {
          CounterBlockFalling = BLOCKEnd;
        }
      }
      break;
    case 1:
      if (CheckBlock(BLOCK_y_MAX_px - (tmpBlock.w * BLOCKSIZE), x, tmpBlock))
      {
        y = BLOCK_y_MAX_px - (tmpBlock.w * BLOCKSIZE);
        UsageBlock = tmpBlock;
      }
      else
      {
        CounterBlockFalling--;
        if (CounterBlockFalling < BLOCKStart)
        {
          CounterBlockFalling = BLOCKEnd;
        }
      }
      break;
    case 2:
      if (CheckBlock(y, x, tmpBlock))
      {
        UsageBlock = tmpBlock;
      }
      else
      {
        CounterBlockFalling--;
        if (CounterBlockFalling < BLOCKStart)
        {
          CounterBlockFalling = BLOCKEnd;
        }
      }
      break;
    default:
      CounterBlockFalling--;
      if (CounterBlockFalling < BLOCKStart)
      {
        CounterBlockFalling = BLOCKEnd;
      }
      break;
  }
}

void rotateFallingBlock()
{
  if (CounterBlockFalling >= BLOCK1_START && CounterBlockFalling <= BLOCK1_END)
  {
    rotateFBSub(Block1, BLOCK1_START, BLOCK1_END);
  }
}

boolean CheckBlock(uint8_t _y, uint8_t _x, Sprite BlockChecking)
{
  uint8_t _x_ = ((_x - BLOCK_x_MIN_px) / BLOCKSIZE);
  uint8_t _y_ = ((_y - BLOCK_y_MIN_px) / BLOCKSIZE);
  for (int i = 0; i < (BlockChecking.h / BLOCKSIZE); i++)
  {
    for (int j = 0; j < (BlockChecking.w / BLOCKSIZE); j++)
    {
      if (GamePlay[_x_ + i][_y_ + j] != BLOCKINVISIBLE)
      {
        if (UsageBlock.Data[((i * BLOCKSIZE) * UsageBlock.w) + (j * BLOCKSIZE)] != BLOCKINVISIBLE)
        {
          return false;
        }
      }
    }
  }
  return true;
}

uint8_t CheckBlockWall(uint8_t _y, Sprite BlockChecking)
{
  if (_y < BLOCK_y_MIN_px)
  {
    return 0;
  }
  else if ((_y + (BlockChecking.w * BLOCKSIZE)) >= BLOCK_y_MAX_px)
  {
    return 1;
  }
  else
  {
    return 2;
  }
}

void MoveFallingBlock()
{
  if (CheckBlock(y, x + BLOCKSIZE, UsageBlock))
  {
    x = x + BLOCKSIZE;
  }
  else
  {
    SaveBlock();
    NewBlock();
  }
}

void SaveBlock()
{
  
}

void NewBlock()
{
  uint8_t rndnum = random(BLOCK_MINNUM,BLOCK_MAXNUM);
  if (rndnum >= BLOCK1_START && rndnum <= BLOCK1_END)
  {
    UsageBlock = Block1[rndnum - BLOCK1_START];
  }
  x = BLOCK_x_MIN_px;
  y = (((BLOCK_y_MAX_px - BLOCK_y_MIN_px) / 2) + BLOCK_y_MIN_px);
}

boolean PauseGame()
{
  while(1)
  {
    mp.display.fillScreen(TFT_BLACK);
    mp.display.drawIcon(BackgroundImageGame.Data, 0, 0,
      BackgroundImageGame.w, BackgroundImageGame.h, 1, TFT_TRANSPARENT);
    mp.display.setTextColor(TEXTCOLOR_W);
    mp.display.setCursor(0, mp.display.height() / 2 - 30);
    mp.display.setTextFont(2);
    mp.display.setTextSize(2);
    mp.display.printCenter("Paused");
    mp.display.setCursor(4, 110);
    mp.display.setTextSize(1);
		mp.display.printCenter("A: resume         B: quit");
    mp.update();
    if (mp.buttons.released(BTN_A))
    {
      mp.buttons.update();
      return true;
    }
    if (mp.buttons.released(BTN_B))
    {
      mp.buttons.update();
      return false;
    }
  }
}

void PrintBlocks()
{
  Serial.println("PB started");
  for (int i = 0; i < GamePlayArray_Y_MAX - 1; i++)
  {
    for (int j = GamePlayArray_X_MAX - 1; j >= 0; j--)
    {
      if (GamePlay[i][j] != BLOCKINVISIBLE)
      {
        uint8_t _x = BLOCK_x_MIN_px + (j * BLOCKSIZE);
        uint8_t _y = BLOCK_y_MIN_px + (i * BLOCKSIZE);
        mp.display.fillRect(_y, _x, BLOCKSIZE, BLOCKSIZE, GamePlay[i][j]);
      }
    }
  }
  Serial.println("Part1 done");
  uint8_t counter = 0;
  for (int xArray = ((x - BLOCK_x_MIN_px) / BLOCKSIZE); xArray < ((SIZEOF(UsageBlock.Data) / BLOCKSIZE) / UsageBlock.h); xArray++)
  {
    for (int yArray = ((BLOCK_y_MAX_px - UsageBlock.w) / BLOCKSIZE); yArray < ((SIZEOF(UsageBlock.Data) / BLOCKSIZE) / UsageBlock.w); yArray++)
    {
      if (UsageBlock.Data[counter] != BLOCKINVISIBLE)
      {
        mp.display.fillRect(y + (yArray * BLOCKSIZE), x + (xArray * BLOCKSIZE), BLOCKSIZE, BLOCKSIZE, UsageBlock.Data[counter]);
      }
      counter += BLOCKSIZE;
    }
  }
  Serial.println("DONE");
}






void SerialInitialize()
{
#if DEBUGGING
  Serial.begin(115200);
#endif
}

void MPInitialize()
{
  mp.begin();
  DisplayInitialize();
  SoundInitialize();
  LoadRankingFile();
}

void DisplayInitialize()
{
  mp.display.fillScreen(BACKGROUND);
  SpriteDefinitions();
}

void SoundInitialize()
{
  osc = new Oscillator(SINE);
  addOscillator(osc);
  osc->setVolume(60);
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

void SpriteDefinitions()
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

  BackgroundImageGame.Data = image_data_Tetris_Background;
  BackgroundImageGame.w = 160;
  BackgroundImageGame.h = 128;
}

void BootupProcess()
{
  Animation();
  ResetVariables();
}

void Animation()
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

  ShowMM();
}

void ResetVariables()
{
#if DEBUGGING
  Serial.println("Started restart");
#endif
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 25; j++)
    {
      GamePlay[i][j] = BLOCKINVISIBLE;
    }
  }
  x = 0;
  y = 0;
  actualBlockUsed = 0;
#if DEBUGGING
  Serial.println("Restart done");
#endif
}

void StartMusic(uint8_t WHAT_TODO)
{
  switch (WHAT_TODO)
  {
    case MUSIC_MENU_START: // Start Menu Music
      StartMusic(MUSIC_STOP);
      break;
    case MUSIC_GAME_START: // Start Game Music
      StartMusic(MUSIC_STOP);
      break;
    case MUSIC_STOP: // Stop Music
      break;
    default:  // Do nothing.
      break;
  }
}

void ShowMM()
{
  boolean Loop = true;

  StartMusic(MUSIC_MENU_START);

  mp.display.fillScreen(BACKGROUND);
  mp.display.drawIcon(BackgroundImageMenu.Data, 0, 0,
    BackgroundImageMenu.w, BackgroundImageMenu.h, 1, TFT_TRANSPARENT);
  mp.update();

  while (Loop)
  {
    if (StartGame())
    {
      Loop = false;
      StartMusic(MUSIC_GAME_START);
    }
    ShowHighscores();
    if (ExitGame())
    {
      return;
    }
  }
  StartMusic(MUSIC_GAME_START);
}

boolean StartGame()
{
  NewBlock();
  return true;
}

void ShowHighscores()
{
}

boolean ExitGame()
{
  return false;
}
