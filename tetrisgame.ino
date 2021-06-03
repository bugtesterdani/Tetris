#include <Arduino.h>
#include "MAKERphone.h"
#include <FastLED/FastLED.h>
#include <utility/Buttons/Buttons.h>
#include <utility/soundLib/MPWavLib.h>

#include "Sprite.h"

#define DEBUGGING       TRUE
#define BACKGROUND      TFT_BLACK
#define TEXTCOLOR_W     TFT_WHITE
#define TEXTCOLOR_G     TFT_GREEN
#define TEXTCOLOR_Y     TFT_YELLOW
#define TEXTCOLOR_B     TFT_BLUE
#define SPRITE_R        TFT_RED
#define BLOCKSIZE       2
#define BLOCKRED        0xf800
#define BLOCKYELLOW     0x1234
#define BLOCKINVISIBLE  0x0120

MAKERphone mp;

Oscillator *osc;
const char *highscoresPath = "/Tetris/highscores.sav";

uint16_t GamePlay[10][25];
uint8_t x;
uint8_t y;
uint8_t actualSprite;
uint32_t BlockUsed[4][4];

Sprite UsageBlock;
Sprite Block1_1;
Sprite Block1_2;
Sprite Block1_3;
Sprite Block1_4;
Sprite BackgroundImage;

void setup()
{
  #if DEBUGGING
  Serial.begin(115200);
  Serial.println("Initialized");
  #endif
  mp.begin();
  mp.display.fillScreen(BACKGROUND);
  osc = new Oscillator(SINE);
  addOscillator(osc);
  osc->setVolume(60);
  File file = SD.open(highscoresPath);
  JsonArray &highscores = mp.jb.parseArray(file);
  file.close();
  if (!SD.exists("/Tetris"))
    SD.mkdir("/Tetris");
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
  initSprites();
  startAnimation();
  restart();
}

void restart()
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 25; j++)
    {
      GamePlay[i][j] = BLOCKINVISIBLE;
    }
  }
}

void loop()
{
  mp.display.fillScreen(BACKGROUND);
  mp.display.drawIcon(BackgroundImage.Data, 0, 0, BackgroundImage.w, BackgroundImage.h, 1, TFT_TRANSPARENT);

  // Check the rotate Button pressed (Button A)
  if (mp.buttons.pressed(BTN_A))
  {
    rotateFallingBlock();
  }

  // Move the actual Block
  moveFallingBlock();
  mp.display.drawIcon(UsageBlock.Data, y, x, UsageBlock.w, UsageBlock.h, BLOCKSIZE, TFT_TRANSPARENT);

  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 25; j++)
    {
      if (GamePlay[i][j] != BLOCKINVISIBLE)
      {
        uint8_t x1_array = 38 + (i * 2 * BLOCKSIZE);
        uint8_t y1_array = 36 + (j * 2 * BLOCKSIZE);
        mp.display.fillRect(x1_array, y1_array, 2 * BLOCKSIZE, 2 * BLOCKSIZE, GamePlay[i][j]);
      }
    }
  }
  
  mp.update();
}

void moveFallingBlock()
{
  if (mp.buttons.repeat(BTN_LEFT, 1))
  {
    if ((y - (2 * BLOCKSIZE)) > 36)
    {
      y -= (2 * BLOCKSIZE);
    }
  }
  else if (mp.buttons.repeat(BTN_RIGHT, 1))
  {
    if ((y + (2 * BLOCKSIZE) + UsageBlock.w) < 78)
    {
      y += (2 * BLOCKSIZE);
    }
  }
  else if (mp.buttons.repeat(BTN_DOWN, 1))
  {
    MoveDown();
  }
}

void MoveDown()
{
  if ((x + (2 * BLOCKSIZE) + UsageBlock.w) < 128)
  {
    uint8_t startBlock = ((y - 36) / (2 * BLOCKSIZE)) - 1;
    /* Abort cause error / glitch / ... */
    if (startBlock > 10)
    {
      mp.display.fillScreen(TFT_GREEN);
      exit(0);
    }
    boolean for_success = true;
    for (int j = 0; j < (UsageBlock.w / 2); j++)
    {
      uint8_t maxindex = 0;
      for (int i = 24; i >= 0; i--)
      {
        if (GamePlay[startBlock][i] != BLOCKINVISIBLE)
        {
          maxindex = i;
          break;
        }
      }
      uint8_t x_min = 128 - (maxindex * 2 * BLOCKSIZE);
      uint8_t x_maxBlock = 0;
      for (int i = 0; i < (UsageBlock.h / 2); i++)
      {
        if (UsageBlock.Data[(24 / (UsageBlock.h / 2)) + (i * 2)] != BLOCKINVISIBLE)
        {
          x_maxBlock = UsageBlock.h - i;
          break;
        }
      }
      x_maxBlock *= (2 * BLOCKSIZE);
      if ((x + x_maxBlock) > x_min)
      {
        for_success = false;
      }
    }
    if (for_success)
    {
      x += (2 * BLOCKSIZE);
      return;
    }
  }
  SaveBlock();
  CheckRows();
  NewBlock();
}

void SaveBlock()
{
  uint8_t x1_array = ((x - 38) / (2 * BLOCKSIZE)) - 1;
  uint8_t y1_array = ((y - 36) / (2 * BLOCKSIZE)) - 1;
  for (int i = 0; i < (UsageBlock.h / 2); i++)
  {
    for (int j = 0; j < (UsageBlock.w / 2); j++)
    {
      if (UsageBlock.Data[((i * UsageBlock.w) + (j * 2))] != BLOCKINVISIBLE)
      {
        GamePlay[(y1_array + j)][(x1_array + i)] = UsageBlock.Data[((i * UsageBlock.w) + (j * 2))];
      }
    }
  }
}

void CheckRows()
{
  for (int i = 24; i >= 0; i--)
  {
    for (int j = 0; j < 10; j++)
    {
      if (GamePlay[j][i] == BLOCKINVISIBLE)
      {
        break;
      }
      
      if (j == 9)
      {
        for (int k = 0; k < 10; k++)
        {
          for (int l = i; l < 24; l++)
          {
            GamePlay[k][l] = GamePlay[k][l + 1];
          }
        }
      }
    }
  }
}

void NewBlock()
{
  switch(random(1, 5))
  {
    case 1:
      UsageBlock = Block1_1;
      break;
    case 2:
      UsageBlock = Block1_2;
      break;
    case 3:
      UsageBlock = Block1_3;
      break;
    case 4:
      UsageBlock = Block1_4;
      break;
    default:
      break;
  }
  y = (36 + 21 - (2 * BLOCKSIZE));
  x = 38;
}

void rotateFallingBlock()
{
  if (UsageBlock.Data == Block1_1.Data)
  {
    UsageBlock = Block1_2;
  }
  else if (UsageBlock.Data == Block1_2.Data)
  {
    UsageBlock = Block1_3;
  }
  else if (UsageBlock.Data == Block1_3.Data)
  {
    UsageBlock = Block1_4;
  }
  else if (UsageBlock.Data == Block1_4.Data)
  {
    UsageBlock = Block1_1;
  }
}

void startAnimation()
{
  uint8_t stSize = 1;
  for (uint8_t i = 60; i >= 30; i-=12)
  {
    mp.display.fillScreen(BACKGROUND);
    mp.display.setCursor(i, 45, 1);
    mp.display.setTextColor(TEXTCOLOR_W);
    mp.display.setTextSize(stSize);
    stSize++;
    mp.display.println(F("TETRIS"));
    while(!mp.update());
    FastLED.showColor(CRGB::Green);
    delay(500);
  }
  mp.display.fillScreen(BACKGROUND);
  mp.display.setCursor(6,45,1);
  mp.display.setTextColor(TEXTCOLOR_G);
  mp.display.setTextSize(4);
  mp.display.print(F("TE"));
  mp.display.setTextColor(TEXTCOLOR_Y);
  mp.display.print(F("TR"));
  mp.display.setTextColor(TEXTCOLOR_B);
  mp.display.println(F("IS"));
  while (!mp.update());
  UsageBlock = Block1_1;
  y = (36 + 21 - (2 * BLOCKSIZE));
  x = 38;
  delay(1000);
  mp.display.drawIcon(UsageBlock.Data, y, x, UsageBlock.w, UsageBlock.h, BLOCKSIZE, TFT_TRANSPARENT);
  while (!mp.update());
  delay(1000);
  Serial.println("Done displaying 1");
}

void initSprites()
{
  // Red Blocks
  Block1_1.Data = image_data_Block1_1;
  Block1_1.w = 6;
  Block1_1.h = 4;
  Block1_2.Data = image_data_Block1_2;
  Block1_2.w = 4;
  Block1_2.h = 6;
  Block1_3.Data = image_data_Block1_3;
  Block1_3.w = 6;
  Block1_3.h = 4;
  Block1_4.Data = image_data_Block1_4;
  Block1_4.w = 4;
  Block1_4.h = 6;

  BackgroundImage.Data = image_data_Tetris_Background;
  BackgroundImage.w = 160;
  BackgroundImage.h = 128;
}
