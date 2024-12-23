
#include "stm32f0xx.h"
#include <stdint.h>
#include "commands.h"
#include <stdio.h>

#include "fifo.h"
#include "tty.h"
#include "lcd.h"



void internal_clock();
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


int incL = 0;
int randomIndex = 0; 
int randomIndexR = 0;
int incR = -1;
int initial = -1;

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

}

// screen is 240 x 320

void drawUP(int inc, int lr, int bw) // lr = left (0) or right (1) side arrow, inc = increment down by inc, bw = black or white
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

void drawDOWN(int inc, int lr, int bw) // lr = left (0) or right (1) side arrow, inc = increment down by inc, bw = black or white
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

void drawLEFT(int inc, int lr, int bw) // lr = left (0) or right (1) side arrow, inc = increment down by inc, bw = black or white
{
    if (lr == 0) {
        LCD_DrawLine(155, 295 - inc, 205, 295 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(180, 320 - inc, 205, 295 - inc, (bw == 0) ? BLACK : WHITE); // left arrow
        LCD_DrawLine(205, 295 - inc, 180, 270 - inc, (bw == 0) ? BLACK : WHITE);
    } else {
        LCD_DrawLine(35, 295 - inc, 85, 295 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(60, 320 - inc, 85, 295 - inc, (bw == 0) ? BLACK : WHITE); // left arrow
        LCD_DrawLine(85, 295 - inc, 60, 270 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawRIGHT(int inc, int lr, int bw) // lr = left (0) or right (1) side arrow, inc = increment down by inc, bw = black or white
{
    if (lr == 0) {
        LCD_DrawLine(155, 295 - inc, 205, 295 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(155, 295 - inc, 180, 320 - inc, (bw == 0) ? BLACK : WHITE); // right arrow
        LCD_DrawLine(155, 295 - inc, 180, 270 - inc, (bw == 0) ? BLACK : WHITE);
    } else {
        LCD_DrawLine(35, 295 - inc, 85, 295 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(35, 295 - inc, 60, 320 - inc, (bw == 0) ? BLACK : WHITE); // right arrow
        LCD_DrawLine(35, 295 - inc, 60, 270 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void TIM7_IRQHandler()
{
    TIM7 -> SR &= ~TIM_SR_UIF;
    void (*arrowFunctions[])(int, int, int) = {drawUP, drawDOWN, drawLEFT, drawRIGHT};

    (*arrowFunctions[randomIndex])(incL,0, 1);
    (*arrowFunctions[randomIndexR])(incR,1, 1);

    incL++;
    incR++;

    if((incL - 1) == 0)
    {
        randomIndex = rand() % 4;
        (*arrowFunctions[randomIndex])(incL,0, 0);
    }
    else if(incL == 270)
    {
        incL = 0;
    }
    else
    {
        (*arrowFunctions[randomIndex])(incL, 0, 0);
    }

    if(incL == 150 && initial == -1)
    {
        initial = 0;
        incR = 1;
    }

    if(initial == 0)
    {
    if((incR - 1) == 0)
        {
            randomIndexR = rand() % 4;
            (*arrowFunctions[randomIndexR])(incR, 1, 0);
        }
    else if(incR == 270)
        {
            incR = 0;
        }
    else
        {
            (*arrowFunctions[randomIndexR])(incR, 1, 0);
        }
    }
    // hi
}

void setup_tim7()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 -> PSC = 1000000 - 1;
    TIM7 -> ARR = 55 - 1;
    TIM7 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM7_IRQn;
    TIM7 -> CR1 |= TIM_CR1_CEN;
}

void heart()
{
    LCD_Clear(WHITE);
        // Triangle 1: Left upper lobe
    LCD_DrawFillTriangle(100, 180, 80, 150, 120, 150, RED);

    // Triangle 2: Right upper lobe
    LCD_DrawFillTriangle(140, 180, 120, 150, 160, 150, RED);

    // Triangle 3: Bottom point
    LCD_DrawFillTriangle(80, 150, 160, 150, 120, 100, RED);

}

void printPress()
{
    
    LCD_DrawLine(160,90,160,70,BLACK);
    LCD_DrawRectangle(160,90,150,80,BLACK);  //p

    LCD_DrawLine(145,90,145,70,BLACK);
    LCD_DrawRectangle(145,90,135,80,BLACK); //r
    LCD_DrawLine(145,80,135,70,BLACK);

    LCD_DrawLine(130,90,130,70,BLACK);
    LCD_DrawLine(130,90,120,90,BLACK); //e
    LCD_DrawLine(130,80,120,80,BLACK);
    LCD_DrawLine(130,70,120,70,BLACK);

    LCD_DrawLine(115,90,115,80,BLACK);
    LCD_DrawLine(115,90,105,90,BLACK); //s
    LCD_DrawLine(115,80,105,80,BLACK);
    LCD_DrawLine(115,70,105,70,BLACK);
    LCD_DrawLine(105,80,105,70,BLACK);

    LCD_DrawLine(115-15,90,115-15,80,BLACK);
    LCD_DrawLine(115-15,90,105-15,90,BLACK); //s
    LCD_DrawLine(115-15,80,105-15,80,BLACK);
    LCD_DrawLine(115-15,70,105-15,70,BLACK);
    LCD_DrawLine(105-15,80,105-15,70,BLACK);

    LCD_DrawLine(120,65,120,45, BLACK);
    LCD_DrawLine(120,65,125,55, BLACK); //1
    LCD_DrawLine(125,45,115,45, BLACK);



}





int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    // command_shell();
    LCD_Setup();
    heart();
    printPress();
    // srand(time(0));

    // initialLCD();
    // setup_tim7();


}



