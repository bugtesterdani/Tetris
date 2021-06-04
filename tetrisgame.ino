#include <Arduino.h>
#include "MAKERphone.h"
#include <FastLED/FastLED.h>
#include <utility/Buttons/Buttons.h>
#include <utility/soundLib/MPWavLib.h>

#include "Sprite.h"

#define TRUE            1 == 1
#define FALSE           1 == 0

#define DEBUGGING       FALSE
#define SUBDEBUGGING    FALSE
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

#define Box_x_min       38
#define Box_y_min       36
#define Box_x_max       128
#define Box_y_max       74

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
#elif SUBDEBUGGING
  Serial.begin(115200);
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
#if DEBUGGING
  Serial.println("Restart done");
#endif
}

void loop()
{
#if DEBUGGING
  Serial.println("Loop entered");
#endif
  mp.display.fillScreen(BACKGROUND);
  mp.display.drawIcon(BackgroundImage.Data, 0, 0, BackgroundImage.w, BackgroundImage.h, 1, TFT_TRANSPARENT);
#if DEBUGGING
  Serial.println("Drawn black");
#endif
  // Check the rotate Button pressed (Button A)
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

  // Move the actual Block
  moveFallingBlock();
  mp.display.drawIcon(UsageBlock.Data, y, x, UsageBlock.w, UsageBlock.h, BLOCKSIZE, TFT_TRANSPARENT);
#if DEBUGGING
  Serial.println("Falling Block done");
#endif
  for (int i = 0; i < 10; i++)
  {
    int counter_for_j = 0;
    for (int j = 24; j >= 0; j--)
    {
//#if DEBUGGING
//      Serial.print("GameArray: ");
//      Serial.println(GamePlay[i][j]);
//#endif
      if (GamePlay[i][j] != BLOCKINVISIBLE)
      {
        uint8_t x1_array = Box_x_min + ((24 - counter_for_j) * 2 * BLOCKSIZE);
        uint8_t y1_array = Box_y_min + (i * 2 * BLOCKSIZE);
#if DEBUGGING
        Serial.print("Draw: ");
        Serial.print(x1_array);
        Serial.print(" x ");
        Serial.println(y1_array);
#endif
        mp.display.fillRect(y1_array, x1_array, 2 * BLOCKSIZE, 2 * BLOCKSIZE, GamePlay[i][j]);
      }
      counter_for_j++;
    }
  }
#if DEBUGGING
  Serial.println("Blocks drawn");
#endif
  
  mp.update();
}

void moveFallingBlock()
{
  if (mp.buttons.repeat(BTN_LEFT, 1))
  {
    if ((y - (2 * BLOCKSIZE)) > Box_y_min)
    {
      y -= (2 * BLOCKSIZE);
    }
  }
  else if (mp.buttons.repeat(BTN_RIGHT, 1))
  {
    if ((y + (2 * BLOCKSIZE) + UsageBlock.w) < Box_y_max)
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
#if DEBUGGING
  Serial.println("Move Down");
#endif
  if ((x + (2 * BLOCKSIZE) + UsageBlock.h) < Box_x_max)
  {
    uint8_t startBlockArray_y = ((y - Box_y_min) / (2 * BLOCKSIZE));
    uint8_t startBlockArray_x = ((x - Box_x_min) / (2 * BLOCKSIZE));
    if (startBlockArray_y >= 10
        || startBlockArray_x >= 25)
    {
      mp.display.fillScreen(TFT_GREEN);
      mp.display.setCursor(6,45,1);
      mp.display.setTextColor(TFT_BLACK);
      mp.display.setTextSize(3);
      mp.display.println(F("GLITCHED"));
      mp.update();
#if DEBUGGING
      Serial.print("EXIT: y: ");
      Serial.print(startBlockArray_y);
      Serial.print(" x: ");
      Serial.println(startBlockArray_x);
#endif
      delay(5000);
      exit(0);
    }
    boolean for_success = true;
#if DEBUGGING
    Serial.println("For loop");
    Serial.print("BlockArray: x: ");
    Serial.print(startBlockArray_x);
    Serial.print(" y: ");
    Serial.println(startBlockArray_y);
#endif
    for (int i = 0; i < (UsageBlock.w / 2); i++)
    {
      for (int j = 0; j < (24 / (UsageBlock.w * 2)); j++)
      {
#if DEBUGGING
        Serial.print("Checking: ");
        Serial.println((UsageBlock.w * 2) * j);
#endif
        if (UsageBlock.Data[((UsageBlock.w * 2) * j)] != BLOCKINVISIBLE)
        {
          uint8_t checkArray_x = startBlockArray_x + j + 1;
          uint8_t checkArray_y = startBlockArray_y + i + 1;
          if (checkArray_x >= 25)
          {
            checkArray_x = 24;
          }
          if (checkArray_y >= 10)
          {
            checkArray_y = 9;
          }
          if (GamePlay[checkArray_y][checkArray_x] != BLOCKINVISIBLE)
          {
#if DEBUGGING
            Serial.println("Found Block");
#endif
            for_success = false;
            break;
          }
        }
      }
      if (!for_success)
      {
        break;
      }
    }
    if (for_success)
    {
      x += (2 * BLOCKSIZE);
      return;
    }
  }
#if DEBUGGING
  Serial.println("SB");
#endif
  SaveBlock();
#if DEBUGGING
  Serial.println("CR");
#endif
  CheckRows();
#if DEBUGGING
  Serial.println("NB");
#endif
  NewBlock();
}

void SaveBlock()
{
  uint8_t y1_array = ((y - Box_y_min) / (2 * BLOCKSIZE));
  uint8_t x1_array = ((x - Box_x_min) / (2 * BLOCKSIZE));
#if SUBDEBUGGING or DEBUGGING
  bool done = false;
#endif
#if DEBUGGING
  Serial.print("ARRAY: x: ");
  Serial.print(x1_array);
  Serial.print(" y: ");
  Serial.println(y1_array);
#endif
  for (int i = 0; i < (UsageBlock.h / 2); i++)
  {
    for (int j = 0; j < (UsageBlock.w / 2); j++)
    {
#if DEBUGGING
      Serial.print(i);
      Serial.println(j);
#endif
      if (UsageBlock.Data[(((24 / (UsageBlock.h / 2)) * i) + (j * 2))] != BLOCKINVISIBLE)
      {
#if DEBUGGING or SUBDEBUGGING
        Serial.print("ADDED ");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(j);
        Serial.print(" ");
        Serial.println(UsageBlock.Data[(((24 / (UsageBlock.h / 2)) * i) + (j * 2))]);
        done = true;
#endif
        GamePlay[(y1_array + j)][(x1_array + i)] = UsageBlock.Data[(((24 / (UsageBlock.h / 2)) * i) + (j * 2))];
      }
#if DEBUGGING
      else
      {
        Serial.println((((24 / (UsageBlock.h / 2)) * i) + (j * 2)));
      }
#endif
    }
  }
#if DEBUGGING
  if (!done)
  {
    Serial.println("NOTHING ENTERED");
  }
#endif
}

void CheckRows()
{
  for (int i = 0; i < 25; i++)
  {
    for (int j = 0; j < 10; j++)
    {
#if SUBDEBUGGING
      Serial.print("GP ");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(j);
      Serial.print(" ");
      Serial.println(GamePlay[j][i]);
#endif
      if (GamePlay[j][i] == BLOCKINVISIBLE)
      {
        break;
      }
      
      if (j == 9)
      {
#if SUBDEBUGGING
        Serial.print("DELETELINE ");
        Serial.println(i);
#endif
        for (int k = 0; k < 10; k++)
        {
          for (int l = i; l > 0; l--)
          {
            GamePlay[k][l] = GamePlay[k][l - 1];
          }
        }
      }
    }
  }
}

void NewBlock()
{
  uint8_t rndnum = random(1, 5);
  switch(rndnum)
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
  y = (Box_y_min + 21 - (2 * BLOCKSIZE));
  x = Box_x_min;
#if DEBUGGING
  Serial.print("RNDNUM : ");
  Serial.print(rndnum);
  Serial.print(" Coordinates NB: x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.println(y);
#endif
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
#if DEBUGGING
  Serial.println("Done displaying 1");
#endif
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
