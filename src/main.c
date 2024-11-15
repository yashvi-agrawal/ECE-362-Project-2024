/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Feb 7, 2024
  * @brief   ECE 362 Lab 7 student template
  ******************************************************************************
*/

/*******************************************************************************/

// Fill out your username!  Even though we're not using an autotest, 
// it should be a habit to fill out your username in this field now.
const char* username = "agraw192";

/*******************************************************************************/ 

#include "stm32f0xx.h"
#include <stdint.h>


int msg_index = 0;
uint16_t msg[8] = {0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700};
extern const char font[];
extern char keymap;
char *keymap_arr = &keymap;
extern uint8_t col;
char falling_key = '7'; 
const char arrow_chars[4] = {'7', '5', '9', '0'};

void internal_clock();

// Uncomment only one of the following to test each step
// #define STEP1
// #define STEP2
// #define STEP3
// #define STEP4
#define STEP 6 
#define SHELL

void init_usart5() {
    // TODO
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    GPIOC->MODER &= ~(0x3000000); //configure pin PC12 to be routed to USART5_TX
    GPIOC->MODER |= 0x2000000;
    GPIOC->AFR[1] |= 0x20000;

    GPIOD->MODER &= ~(0xF0); //configure pin PD2 to be routed to USART5_RX
    GPIOD->MODER |= 0x20;
    GPIOD->AFR[0] |= 0x200;

    //Configure USART5:
    USART5->CR1 &= ~USART_CR1_UE;

    USART5->CR1 &= ~USART_CR1_M0;
    USART5->CR1 &= ~USART_CR1_M1;

    USART5->CR2 &= ~USART_CR2_STOP_0;
    USART5->CR2 &= ~USART_CR2_STOP_1;

    USART5->CR1 &= ~USART_CR1_PCE;
    USART5->CR1 &= ~USART_CR1_OVER8;

    USART5->BRR = 0x1A1;

    USART5->CR1 |= USART_CR1_TE;
    USART5->CR1 |= USART_CR1_RE;
    USART5->CR1 |= USART_CR1_UE;

    while(!(USART5->ISR & USART_ISR_TEACK) | !(USART5->ISR & USART_ISR_REACK));

}

#ifdef STEP1
int main(void){
    internal_clock();
    init_usart5();
    for(;;) {
        while (!(USART5->ISR & USART_ISR_RXNE)) { }
        char c = USART5->RDR;
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = c;
    }
}
#endif

#ifdef STEP2
#include <stdio.h>

// TODO Resolve the echo and carriage-return problem

int __io_putchar(int c) {
    // TODO
    if(c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    while (!(USART5->ISR & USART_ISR_RXNE));
    char c = USART5->RDR;
    // TODO
    if(c == '\r'){
        c = '\n';
    }
    __io_putchar(c);
    return c;
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP3
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c) {
    // TODO Copy from your STEP2
    if(c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO
    return line_buffer_getchar();
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#include <stdio.h>
#include "fifo.h"
#include "tty.h"

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART3_8_IRQn);
    USART5->CR3 |= USART_CR3_DMAR; 

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;  // First make sure DMA is turned off

    // The DMA channel 2 configuration goes here
    DMA2_Channel2->CMAR = &serfifo;
    DMA2_Channel2->CPAR = &(USART5->RDR);
    DMA2_Channel2->CNDTR |= FIFOSIZE;
    DMA2_Channel2->CCR &= ~(DMA_CCR_DIR | DMA_CCR_HTIE | DMA_CCR_TCIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_1|DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_1|DMA_CCR_PSIZE_0);
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;
    DMA2_Channel2->CCR |= DMA_CCR_EN;
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar() {
    // TODO
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
    }
    return fifo_remove(&input_fifo);
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if(c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

// TODO Copy the content for the USART5 ISR here
void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

// TODO Remember to look up for the proper name of the ISR function
#ifdef STEP5

int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();

    setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n"); // After, will type TWO instead of ONE
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef SHELL
#include "commands.h"
#include <stdio.h>

#include "fifo.h"
#include "tty.h"
#include "lcd.h"

int inc = 0;
int randomIndex = 0; 
int randomLR;

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART3_8_IRQn);
    USART5->CR3 |= USART_CR3_DMAR; 

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;  // First make sure DMA is turned off

    // The DMA channel 2 configuration goes here
    DMA2_Channel2->CMAR = &serfifo;
    DMA2_Channel2->CPAR = &(USART5->RDR);
    DMA2_Channel2->CNDTR |= FIFOSIZE;
    DMA2_Channel2->CCR &= ~(DMA_CCR_DIR | DMA_CCR_HTIE | DMA_CCR_TCIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_1|DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_1|DMA_CCR_PSIZE_0);
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;
    DMA2_Channel2->CCR |= DMA_CCR_EN;
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar() {
    // TODO
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
    }
    return fifo_remove(&input_fifo);
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if(c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

void my_command_shell(void)
{
  char line[100];
  int len = strlen(line);
  puts("This is the STM32 command shell.");
  for(;;) {
      printf("> ");
      fgets(line, 99, stdin);
      line[99] = '\0';
      len = strlen(line);
      if ((line[len-1]) == '\n')
          line[len-1] = '\0';
      parse_command(line);
  }
}

void init_spi1_slow ()
{
    //PB3 (SCK), PB4 (MISO), and PB5 (MOSI)
    //AFR 0

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    GPIOB->MODER &= ~0xFC0;
    GPIOB->MODER |= 0xA80;
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFRL3 | GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5);

    SPI1->CR1 &= ~SPI_CR1_SPE;

    SPI1->CR1 |= (SPI_CR1_BR | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI);
    SPI1->CR2 |= 0x700;
    SPI1->CR2 &= ~SPI_CR2_DS_3;
    SPI1->CR2 |= SPI_CR2_FRXTH;

    SPI1->CR1 |= SPI_CR1_SPE;

}

void enable_sdcard()
{
    GPIOB->BRR |= GPIO_BRR_BR_2;
}

void disable_sdcard()
{
    GPIOB->BSRR |= GPIO_BSRR_BS_2;
}

void init_sdcard_io()
{
    init_spi1_slow();
    GPIOB->MODER |= 0x10;
    disable_sdcard();

}

void sdcard_io_high_speed()
{
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= 0x8;
    SPI1->CR1 &= ~(SPI_CR1_BR_2|SPI_CR1_BR_1);
    SPI1->CR1 |= SPI_CR1_SPE;
}

void init_lcd_spi()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER &= ~(0x30C30000);
    GPIOB->MODER |= (0x10410000);

    init_spi1_slow();
    sdcard_io_high_speed();

}


void initialLCD()
{
    LCD_DrawFillRectangle(0,0,320,320,WHITE);
    LCD_DrawLine(120,0,120,320,BLACK);
    


    // srand(time(NULL));
    // void (*arrowFunctions[])() = {drawUP, drawDOWN, drawLEFT, drawRIGHT};


    // for(int i = 0; i < 320; i++) //up arrow
    // {
    //     if(170-i <= 0) break;
        
    //     LCD_DrawLine(180, 170-i, 180, 220-i, BLACK);
    //     LCD_DrawLine(180, 220-i, 150, 200-i, BLACK);
    //     LCD_DrawLine(180, 220-i, 210, 200-i, BLACK); 

    //     LCD_DrawLine(180, 220-i, 150, 200-i, WHITE);
    //     LCD_DrawLine(180, 220-i, 210, 200-i, WHITE);  
    //     LCD_DrawLine(180, 170-i, 180, 220-i, WHITE);

    //     // LCD_DrawFillRectangle(0,0,320,320,WHITE);
    //     // LCD_DrawLine(120,0,120,320,BLACK);
        
    // }




}

// screen is 240 x 320

void drawUP(int lr, int bw) // lr = left (0) or right (1) side arrow, inc = increment down by inc, bw = black or white
{
    if (lr == 0){ // left
        LCD_DrawLine(180, 320 - inc, 180, 270 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(180, 320 - inc, 155, 295 - inc, (bw == 0) ? BLACK : WHITE); // up arrow
        LCD_DrawLine(180, 320 - inc, 205, 295 - inc, (bw == 0) ? BLACK : WHITE);
    } else { // right
        LCD_DrawLine(60, 320 - inc, 60, 270 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(60, 320 - inc, 35, 295 - inc, (bw == 0) ? BLACK : WHITE); // up arrow
        LCD_DrawLine(60, 320 - inc, 85, 295 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawDOWN(int lr, int bw)
{
    if (lr == 0) {
        LCD_DrawLine(180, 320 - inc, 180, 270 - inc, (bw == 0) ? BLACK : WHITE); // line
        LCD_DrawLine(180, 270 - inc, 155, 295 - inc, (bw == 0) ? BLACK : WHITE); // down arrow
        LCD_DrawLine(180, 270 - inc, 205, 295 - inc, (bw == 0) ? BLACK : WHITE);
    } else {
        LCD_DrawLine(60, 320 - inc, 60, 270 - inc, (bw == 0) ? BLACK : WHITE); // line
        LCD_DrawLine(60, 270 - inc, 35, 295 - inc, (bw == 0) ? BLACK : WHITE); // down arrow
        LCD_DrawLine(60, 270 - inc, 85, 295 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawLEFT(int lr, int bw)
{
    if (lr == 0) {
        LCD_DrawLine(155, 220 - inc, 205, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(205, 220 - inc, 185, 190 - inc, (bw == 0) ? BLACK : WHITE); // left arrow
        LCD_DrawLine(205, 220 - inc, 185, 250 - inc, (bw == 0) ? BLACK : WHITE);
    } else {
        LCD_DrawLine(35, 220 - inc, 85, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(85, 220 - inc, 65, 190 - inc, (bw == 0) ? BLACK : WHITE); // left arrow
        LCD_DrawLine(85, 220 - inc, 65, 250 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawRIGHT(int lr, int bw)
{
    if (lr == 0) {
        LCD_DrawLine(155, 220 - inc, 205, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(155, 220 - inc, 175, 190 - inc, (bw == 0) ? BLACK : WHITE); // right arrow
        LCD_DrawLine(155, 220 - inc, 175, 250 - inc, (bw == 0) ? BLACK : WHITE);
    } else {
        LCD_DrawLine(35,220-inc,85,220-inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(35,220-inc,55,190-inc, (bw == 0) ? BLACK : WHITE); //right arrow
        LCD_DrawLine(35,220-inc,55,250-inc, (bw == 0) ? BLACK : WHITE);    
    }
}

void TIM7_IRQHandler()
{
    TIM7 -> SR &= ~TIM_SR_UIF;
    void (*arrowFunctions[])(int, int) = {drawUP, drawDOWN, drawLEFT, drawRIGHT};

    (*arrowFunctions[randomIndex])(randomLR, 1);

    inc++;

    if (inc == 150)
    {
        inc = 0;
        return;
    }
    if ((inc-1) == 0) {
        randomIndex = rand() % 4;
        randomLR = rand() % 2;
        falling_key = arrow_chars[randomIndex]; 
        (*arrowFunctions[randomIndex])(randomLR, 0);
    }
    else
    {
        (*arrowFunctions[randomIndex])(randomLR, 0);
    }
}

void setup_tim7()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 -> PSC = 1000000 - 1;
    TIM7 -> ARR = 24 - 1;
    TIM7 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM7_IRQn;
    TIM7 -> CR1 |= TIM_CR1_CEN;
}

#endif

#ifdef STEP 6

void init_spi2(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~(GPIO_MODER_MODER15_0 | GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0);
    GPIOB->MODER |= GPIO_MODER_MODER15_1 | GPIO_MODER_MODER12_1 | GPIO_MODER_MODER13_1;
    GPIOB->AFR[1] &= ~0xf0ff0000;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 &= ~SPI_CR1_SPE;
    SPI2->CR1 |= SPI_CR1_BR; 
    SPI2->CR2 = SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
    SPI2->CR1 |= SPI_CR1_MSTR; 
    SPI2->CR2 |= SPI_CR2_SSOE; 
    SPI2->CR2 |= SPI_CR2_NSSP;  
    SPI2->CR2 |= SPI_CR2_TXDMAEN; 
    SPI2->CR1 |= SPI_CR1_SPE; 
}

void spi2_setup_dma(void)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5->CMAR = (uint32_t)(&msg); 
    DMA1_Channel5->CPAR = (uint32_t)(&(SPI2->DR));
    DMA1_Channel5->CNDTR = 8;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;
    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR &= ~DMA_CCR_MSIZE_1;
    DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel5->CCR &= ~DMA_CCR_PSIZE_1;
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;
}

void spi2_enable_dma(void)
{
    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

void enable_ports()
{
  GPIOC->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7); 
  GPIOC->MODER |= (GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);

  //inputs for port C
  GPIOC->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3); 

  // pull down for port C
  GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_1; 
  GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR0_0; 
  GPIOC->PUPDR |= GPIO_PUPDR_PUPDR1_1;
  GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR1_0;
  GPIOC->PUPDR |= GPIO_PUPDR_PUPDR2_1;
  GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR2_0;
  GPIOC->PUPDR |= GPIO_PUPDR_PUPDR3_1;
  GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR3_0;
}

int c = 0; 
void drive_column(int c)
{
    GPIOC->BSRR = (1 << (7 + 16) | 1 << (6 + 16) | 1 << (5 + 16) | 1 << (4 + 16)); 
    GPIOC->BSRR = 1 << ((0b11 & c) + 4);  
}

int read_rows()
{
    return GPIOC->IDR &= (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  
}

char rows_to_key(int rows)
{
  if (rows & 1)
  {
    rows = 0; 
  }
  else if (rows & 2)
  {
    rows = 1; 
  }
  else if (rows & 4)
  {
    rows = 2; 
  }
  else if (rows & 8)
  {
    rows = 3; 
  }
  int col_val = col*4 + rows; 
  return keymap_arr[col_val]; 

}

char handle_input() {
    for (int col = 0; col < 4; col++) {
        drive_column(col); 
        int rows = read_rows(); 
        if (rows) 
        {
            return rows_to_key(rows);
        }
    }
    return '\0'; 
}

int score = 0; 
void update_score(falling_key) {
    char user_input = handle_input();  

    // Check if the pressed key matches the falling key
    if (user_input == falling_key) 
    {
        score++;  
    } 
    else 
    {
        score--;  
    }

    if (score < 0)
    {
        score = 0;
    }
   
    if (score > 999) 
    {
        score = 999;
    }

    msg[5] |= font['0' + (score / 100) % 10]; 
    msg[6] |= font['0' + (score / 10) % 10];  
    msg[7] |= font['0' + score % 10];          
}

void TIM14_IRQHandler()
{
  TIM14->SR &= ~TIM_SR_UIF;  
  handle_input(); 
  update_score(falling_key); 
}

void setup_tim14()
{ 
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN; 
    //set prescaler and arr
    TIM14 -> PSC = 1000000 - 1;
    TIM14 -> ARR = 24 - 1;
    TIM14->DIER |= TIM_DIER_UIE; 
    NVIC->ISER[0] = 1 << TIM14_IRQn;  
    TIM14->CR1 |= TIM_CR1_CEN;
}


#endif 


int main() {
    internal_clock();
    msg[0] |= font['S'];
    msg[1] |= font['C'];
    msg[2] |= font['O'];
    msg[3] |= font['R'];
    msg[4] |= font['E'];
    msg[5] |= font['0'];
    msg[6] |= font['0'];
    msg[7] |= font['0'];
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    // command_shell();
    LCD_Setup();
    srand(time(0));

    initialLCD();
    setup_tim7();


    
    // workerLCD();

}
