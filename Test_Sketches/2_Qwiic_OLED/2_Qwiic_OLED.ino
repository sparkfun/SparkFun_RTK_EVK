/*
  SparkFun RTK Control Test Sketch: Qwiic OLED

  License: MIT. Please see LICENSE.md for more details

  Pins 12 and 15 are strapping pins
  If you are running this code on a WROVER breakout:
    Uncomment the #define DELAY_10s
    Disconnect the Qwiic OLED when uploading the code
    Connect it during the 10s delay.

  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : STAT LED
  D3  : Serial RX (CH340 TX)
  D4  : SD CS
  D5  : Unused - via 74HC4066 switch
  D12 : SDA2 - Qwiic OLED - via 74HC4066 switch
  D13 : Serial1 TX - LARA_TXDI
  D14 : Serial1 RX - LARA RXDO
  D15 : SCL2 - Qwiic OLED - via 74HC4066 switch
  D16 : N/C
  D17 : N/C
  D18 : SPI SCK
  D19 : SPI POCI
  D21 : I2C SDA
  D22 : I2C SCL
  D23 : SPI PICO
  D25 : GNSS Time Pulse
  D26 : LARA Power On
  D27 : Ethernet Chip Select
  D32 : PWREN
  D33 : Ethernet Interrupt
  A34 : LARA Network Indicator
  A35 : Board Detect (3.0V)
  A36 : SD Card Detect
*/

//#define DELAY_10s // Uncomment this line if you are using a WROVER breakout with no strapping pin isolation

const int SD_CS = 4; // Chip select for the microSD card
const int ETHERNET_CS = 27; // Chip select for the WizNet 5500
const int PWREN = 32; // 3V3_SW and SDIO Enable
const int STAT_LED = 2;
const int SCL_2 = 15;
const int SDA_2 = 12;

#include <SparkFun_Qwiic_OLED.h> //http://librarymanager/All#SparkFun_Qwiic_OLED
Qwiic1in3OLED myOLED; // 128x64

TwoWire I2C_2 = TwoWire(1);

int width;
int height;

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(ETHERNET_CS, OUTPUT);
  digitalWrite(ETHERNET_CS, HIGH);
  pinMode(PWREN, OUTPUT);
  digitalWrite(PWREN, HIGH);

  delay(1000);

  Serial.begin(115200);
  Serial.println("SparkFun RTK - Test Sketch");

#ifdef DELAY_10s
  Serial.print("Test starts in 10 seconds");
  for (int i = 0; i < 10; i++)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
#endif

  I2C_2.begin((int)SDA_2, (int)SCL_2, (uint32_t)400000);

  // Initalize the OLED device and related graphics system
  if (myOLED.begin(I2C_2) == false)
  {
      Serial.println("OLED begin failed. Freezing...");
      while (true)
          ;
  }

  width = myOLED.getWidth();
  height = myOLED.getHeight();
}

void loop()
{
    pixelExample();
    lineExample();
    shapeExample();
}

void pixelExample()
{
    myOLED.erase();
    for (int i = 0; i < 512; i++)
    {
        myOLED.pixel(random(width), random(height));
        myOLED.display();
        delay(10);
    }
}

void lineExample()
{
    int middleX = width / 2;
    int middleY = height / 2;
    int xEnd, yEnd;
    int lineWidth = min(middleX, middleY);

    myOLED.erase();
    int deg;

    for (int i = 0; i < 3; i++)
    {

        for (deg = 0; deg < 360; deg += 15)
        {

            xEnd = lineWidth * cos(deg * PI / 180.0);
            yEnd = lineWidth * sin(deg * PI / 180.0);

            myOLED.line(middleX, middleY, middleX + xEnd, middleY + yEnd);
            myOLED.display();
            delay(10);
        }

        for (deg = 0; deg < 360; deg += 15)
        {

            xEnd = lineWidth * cos(deg * PI / 180.0);
            yEnd = lineWidth * sin(deg * PI / 180.0);

            myOLED.line(middleX, middleY, middleX + xEnd, middleY + yEnd, 0);
            myOLED.display();
            delay(10);
        }
    }
}

void shapeExample()
{
    // Silly pong demo. It takes a lot of work to fake pong...
    int paddleW = 3;  // Paddle width
    int paddleH = 15; // Paddle height

    // Paddle 0 (left) position coordinates
    int paddle0_Y = (height / 2) - (paddleH / 2);
    int paddle0_X = 2;

    // Paddle 1 (right) position coordinates
    int paddle1_Y = (height / 2) - (paddleH / 2);
    int paddle1_X = width - 3 - paddleW;
    int ball_rad = 4; // Ball radius

    // Ball position coordinates
    int ball_X = paddle0_X + paddleW + ball_rad;
    int ball_Y = random(1 + ball_rad, height - ball_rad); // paddle0_Y + ball_rad;
    int ballVelocityX = 1;                                // Ball left/right velocity
    int ballVelocityY = 1;                                // Ball up/down velocity
    int paddle0Velocity = -1;                             // Paddle 0 velocity
    int paddle1Velocity = 1;                              // Paddle 1 velocity

    // Draw the Pong Field
    myOLED.erase();

    // Draw an outline of the screen:
    myOLED.rectangle(0, 0, width - 1, height);

    // Draw the center line
    myOLED.rectangleFill(width / 2 - 1, 0, 2, height);

    bool firstLoop = true;

    while ((ball_X - ball_rad > 1) && (ball_X + ball_rad < width - 2))
    {

        if (!firstLoop)
        {

            // Erase the old ball. In XOR mode, so just draw old values again!
            // Draw the Paddles:
            myOLED.rectangleFill(paddle0_X, paddle0_Y, paddleW, paddleH);
            myOLED.rectangleFill(paddle1_X, paddle1_Y, paddleW, paddleH);
            // Draw the ball: - use rect - xor and circle fails b/c of circle algorithm overdraws
            myOLED.rectangleFill(ball_X, ball_Y, ball_rad, ball_rad);
        }
        // Increment ball's position
        ball_X += ballVelocityX;
        ball_Y += ballVelocityY;

        // Check if the ball is colliding with the left paddle
        if (ball_X - ball_rad < paddle0_X + paddleW)
        {

            // Check if ball is within paddle's height
            if ((ball_Y > paddle0_Y) && (ball_Y < paddle0_Y + paddleH))
            {

                ball_X++;                       // Move ball over one to the right
                ballVelocityX = -ballVelocityX; // Change velocity
            }
        }

        // Check if the ball hit the right paddle
        if (ball_X + ball_rad > paddle1_X)
        {

            // Check if ball is within paddle's height
            if ((ball_Y > paddle1_Y) && (ball_Y < paddle1_Y + paddleH))
            {

                ball_X--;                       // Move ball over one to the left
                ballVelocityX = -ballVelocityX; // change velocity
            }
        }

        // Check if the ball hit the top or bottom
        if ((ball_Y <= 1) || (ball_Y >= (height - ball_rad - 1)))
        {

            // Change up/down velocity direction
            ballVelocityY = -ballVelocityY;
        }

        // Move the paddles up and down
        paddle0_Y += paddle0Velocity;
        paddle1_Y += paddle1Velocity;

        // Change paddle 0's direction if it hit top/bottom
        if ((paddle0_Y <= 1) || (paddle0_Y > height - 2 - paddleH))
            paddle0Velocity = -paddle0Velocity;

        // Change paddle 1's direction if it hit top/bottom
        if ((paddle1_Y <= 1) || (paddle1_Y > height - 2 - paddleH))
            paddle1Velocity = -paddle1Velocity;

        // Draw the Paddles:
        myOLED.rectangleFill(paddle0_X, paddle0_Y, paddleW, paddleH);
        myOLED.rectangleFill(paddle1_X, paddle1_Y, paddleW, paddleH);

        // Draw the ball:
        myOLED.rectangleFill(ball_X, ball_Y, ball_rad, ball_rad);

        // Actually draw everything on the screen:
        myOLED.display();

        // Once the first loop is done, switch to XOR mode. So we just update our
        // moving parts
        if (firstLoop)
        {
            firstLoop = false;
            myOLED.setDrawMode(grROPXOR);
        }

        delay(25); // Delay for visibility
    }
    delay(1000);
    myOLED.setDrawMode(grROPCopy);
}
