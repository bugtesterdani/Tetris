#include <Arduino.h>
#include "MAKERphone.h"
#include <FastLED/FastLED.h>
#include <utility/Buttons/Buttons.h>
#include <utility/soundLib/MPWavLib.h>

#include "Sprite.h"

#define TRUE            1 == 1
#define FALSE           1 == 0

#define DEBUGGING       FALSE

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

MAKERphone mp;
Oscillator *osc;
const char *highscoresPath = "/Tetris/highscores.sav";
const char *folderPath = "/Tetris";
const JsonArray *highscores;

uint16_t GamePlay[10][25];
uint8_t x;
uint8_t y;

uint16_t CounterBlockFalling;

Sprite UsageBlock;
Sprite Block1[4];
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
  mp.display.fillScreen(BACKGROUND);
  mp.display.drawIcon(BackgroundImageGame.Data, 0, 0,
    BackgroundImageGame.w, BackgroundImageGame.h, 1, TFT_TRANSPARENT);
  
  CheckButtonsPressed();
  CheckJoystick();
  FallBlockCounterBased();

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
    
  }
  else if (mp.buttons.repeat(BTN_RIGHT, 1))
  {
    
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

void rotateFallingBlock()
{
  
}

void MoveFallingBlock()
{
  
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

  mp.display.fillScreen(BACKGROUND);
  StartMusic(MUSIC_MENU_START);
  while (!mp.update());

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
    case 0: // Start Menu Music
      StartMusic(MUSIC_STOP);
      break;
    case 1: // Start Game Music
      StartMusic(MUSIC_STOP);
      break;
    case 2: // Stop Music
      break;
    default:  // Do nothing.
      break;
  }
}

void ShowMM()
{
  boolean Loop = true;
  while (Loop)
  {
    Loop = false;
  }
}
