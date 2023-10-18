#include "i2c_lcd.h"

extern I2C_HandleTypeDef hi2c1;

void I2C_LCD_Test(void)
{
#if 0 // 오실로 스코프 파형 확인용 코드
	unsigned char i2c_test[] = {'5', 0};

	while(1)
	{
		while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS,
				i2c_test, 1, 100)!=HAL_OK){
			//HAL_Delay(1);
		}
		HAL_Delay(200);
	}
#endif

#if 0 // LCD 동작 확인용 코드
	uint8_t value=0;
	char test[20];

 	I2C_LCD_Init();

	while(1)
	{
		sprintf(test, "Hello World!!!");
		Move_Cursor(0,0);
		LCD_String(test); // lcd_data() 함수가 0x20 이상의 ASCII값만 LCD에 출력할 수 있기 때문에, 그보다 작은 \n(0x0A)을 여기에 넣으면 출력겨로가가 깨지게 된다.
		Move_Cursor(1,0);
		LCD_Data(value + '0');
		value++;
		if(value>9)value=0;
		HAL_Delay(500); // I2C의 통신 프로토콜을 통해서 정보가 왔다갔다 하고 있기 때문에 그것이 이루어지기 위한 최소한의 시간을 LCD단에서 보장해야 한다.(만약 이 딜레이가 없으면 I2C가 죽어버린다.)
	}
#endif
}

//
void LCD_Command(uint8_t command)
{

	uint8_t high_nibble, low_nibble;
	uint8_t i2c_buffer[4];
	high_nibble = command & 0xf0;
	low_nibble = (command<<4) & 0xf0;
	i2c_buffer[0] = high_nibble | 0x04 | 0x08; //en=1, rs=0, rw=0, backlight=1
	i2c_buffer[1] = high_nibble | 0x00 | 0x08; //en=0, rs=0, rw=0, backlight=1
	i2c_buffer[2] = low_nibble  | 0x04 | 0x08; //en=1, rs=0, rw=0, backlight=1
	i2c_buffer[3] = low_nibble  | 0x00 | 0x08; //en=0, rs=0, rw=0, backlight=1
	while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS,
			i2c_buffer, 4, 100)!=HAL_OK){
		//HAL_Delay(1);
	}
	return;
}

// 1 byte write... 딱 한개의 캐릭터만 찍는 것이다.
void LCD_Data(uint8_t data)
{

	uint8_t high_nibble, low_nibble;
	uint8_t i2c_buffer[4];
	high_nibble = data & 0xf0;
	low_nibble = (data<<4) & 0xf0;
	i2c_buffer[0] = high_nibble | 0x05 | 0x08; //en=1, rs=1, rw=0, backlight=1
	i2c_buffer[1] = high_nibble | 0x01 | 0x08; //en=0, rs=1, rw=0, backlight=1
	i2c_buffer[2] = low_nibble  | 0x05 | 0x08; //en=1, rs=1, rw=0, backlight=1
	i2c_buffer[3] = low_nibble  | 0x01 | 0x08; //en=0, rs=1, rw=0, backlight=1
	while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS,
			i2c_buffer, 4, 100)!=HAL_OK){
		//HAL_Delay(1);
	}
	return;
}

// lcd 초기화
void I2C_LCD_Init(void)
{

	LCD_Command(0x33);
	LCD_Command(0x32);
	LCD_Command(0x28);	//Function Set 4-bit mode
	LCD_Command(DISPLAY_ON);
	LCD_Command(0x06);	//Entry mode set
	LCD_Command(CLEAR_DISPLAY);
	HAL_Delay(2);
}

// null을 만날때 까지 string을 LCD에 출력
void LCD_String(uint8_t *str)
{
	while(*str) // 널문자를 만날 때까지
	{
		LCD_Data(*str++);
	}
}

// 해당 줄,col으로 이동 하는 함수
void Move_Cursor(uint8_t row, uint8_t column)
{
	LCD_Command(0x80 | row<<6 | column);
	return;
}
