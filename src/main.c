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
const char *username = "agraw192";

/*******************************************************************************/

#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();

// Uncomment only one of the following to test each step
// #define STEP1
// #define STEP2
// #define STEP3
// #define STEP4
#define SHELL

void init_usart5()
{
    // TODO
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    GPIOC->MODER &= ~(0x3000000); // configure pin PC12 to be routed to USART5_TX
    GPIOC->MODER |= 0x2000000;
    GPIOC->AFR[1] |= 0x20000;

    GPIOD->MODER &= ~(0xF0); // configure pin PD2 to be routed to USART5_RX
    GPIOD->MODER |= 0x20;
    GPIOD->AFR[0] |= 0x200;

    // Configure USART5:
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

    while (!(USART5->ISR & USART_ISR_TEACK) | !(USART5->ISR & USART_ISR_REACK))
        ;
}

#ifdef STEP1
int main(void)
{
    internal_clock();
    init_usart5();
    for (;;)
    {
        while (!(USART5->ISR & USART_ISR_RXNE))
        {
        }
        char c = USART5->RDR;
        while (!(USART5->ISR & USART_ISR_TXE))
        {
        }
        USART5->TDR = c;
    }
}
#endif

#ifdef STEP2
#include <stdio.h>

// TODO Resolve the echo and carriage-return problem

int __io_putchar(int c)
{
    // TODO
    if (c == '\n')
    {
        while (!(USART5->ISR & USART_ISR_TXE))
            ;
        USART5->TDR = '\r';
    }
    while (!(USART5->ISR & USART_ISR_TXE))
        ;
    USART5->TDR = c;
    return c;
}

int __io_getchar(void)
{
    while (!(USART5->ISR & USART_ISR_RXNE))
        ;
    char c = USART5->RDR;
    // TODO
    if (c == '\r')
    {
        c = '\n';
    }
    __io_putchar(c);
    return c;
}

int main()
{
    internal_clock();
    init_usart5();
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for (;;)
    {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP3
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c)
{
    // TODO Copy from your STEP2
    if (c == '\n')
    {
        while (!(USART5->ISR & USART_ISR_TXE))
            ;
        USART5->TDR = '\r';
    }
    while (!(USART5->ISR & USART_ISR_TXE))
        ;
    USART5->TDR = c;
    return c;
}

int __io_getchar(void)
{
    // TODO
    return line_buffer_getchar();
}

int main()
{
    internal_clock();
    init_usart5();
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for (;;)
    {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP4

#include <stdio.h>
#include "fifo.h"
#include "tty.h"

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void)
{
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART3_8_IRQn);
    USART5->CR3 |= USART_CR3_DMAR;

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN; // First make sure DMA is turned off

    // The DMA channel 2 configuration goes here
    DMA2_Channel2->CMAR = &serfifo;
    DMA2_Channel2->CPAR = &(USART5->RDR);
    DMA2_Channel2->CNDTR |= FIFOSIZE;
    DMA2_Channel2->CCR &= ~(DMA_CCR_DIR | DMA_CCR_HTIE | DMA_CCR_TCIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_1 | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_1 | DMA_CCR_PSIZE_0);
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;
    DMA2_Channel2->CCR |= DMA_CCR_EN;
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar()
{
    // TODO
    while (fifo_newline(&input_fifo) == 0)
    {
        asm volatile("wfi"); // wait for an interrupt
    }
    return fifo_remove(&input_fifo);
}

int __io_putchar(int c)
{
    // TODO copy from STEP2
    if (c == '\n')
    {
        while (!(USART5->ISR & USART_ISR_TXE))
            ;
        USART5->TDR = '\r';
    }
    while (!(USART5->ISR & USART_ISR_TXE))
        ;
    USART5->TDR = c;
    return c;
}

int __io_getchar(void)
{
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

// TODO Copy the content for the USART5 ISR here
void USART3_8_IRQHandler(void)
{
    while (DMA2_Channel2->CNDTR != sizeof serfifo - seroffset)
    {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

// TODO Remember to look up for the proper name of the ISR function

int main()
{
    internal_clock();
    init_usart5();
    enable_tty_interrupt();

    setbuf(stdin, 0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n"); // After, will type TWO instead of ONE
    for (;;)
    {
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
int randomIndex;

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void USART3_8_IRQHandler(void)
{
    while (DMA2_Channel2->CNDTR != sizeof serfifo - seroffset)
    {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

void enable_tty_interrupt(void)
{
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART3_8_IRQn);
    USART5->CR3 |= USART_CR3_DMAR;

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN; // First make sure DMA is turned off

    // The DMA channel 2 configuration goes here
    DMA2_Channel2->CMAR = &serfifo;
    DMA2_Channel2->CPAR = &(USART5->RDR);
    DMA2_Channel2->CNDTR |= FIFOSIZE;
    DMA2_Channel2->CCR &= ~(DMA_CCR_DIR | DMA_CCR_HTIE | DMA_CCR_TCIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_1 | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_1 | DMA_CCR_PSIZE_0);
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;
    DMA2_Channel2->CCR |= DMA_CCR_EN;
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar()
{
    // TODO
    while (fifo_newline(&input_fifo) == 0)
    {
        asm volatile("wfi"); // wait for an interrupt
    }
    return fifo_remove(&input_fifo);
}

int __io_putchar(int c)
{
    // TODO copy from STEP2
    if (c == '\n')
    {
        while (!(USART5->ISR & USART_ISR_TXE))
            ;
        USART5->TDR = '\r';
    }
    while (!(USART5->ISR & USART_ISR_TXE))
        ;
    USART5->TDR = c;
    return c;
}

int __io_getchar(void)
{
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

void my_command_shell(void)
{
    char line[100];
    int len = strlen(line);
    puts("This is the STM32 command shell.");
    for (;;)
    {
        printf("> ");
        fgets(line, 99, stdin);
        line[99] = '\0';
        len = strlen(line);
        if ((line[len - 1]) == '\n')
            line[len - 1] = '\0';
        parse_command(line);
    }
}

void init_spi1_slow()
{
    // PB3 (SCK), PB4 (MISO), and PB5 (MOSI)
    // AFR 0

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
    SPI1->CR1 &= ~(SPI_CR1_BR_2 | SPI_CR1_BR_1);
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
//===========================================================================
// init_wavetable()
// Write the pattern for a complete cycle of a sine wave into the
// wavetable[] array.
//===========================================================================
void init_wavetable(void)
{
    for (int i = 0; i < N; i++)
    // wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

//============================================================================
// set_freq()
//============================================================================
void set_freq(int chan, float f)
{
    if (chan == 0)
    {
        if (f == 0.0)
        {
            step0 = 0;
            offset0 = 0;
        }
        else
            step0 = (f * N / RATE) * (1 << 16);
    }
}

void setup_dac(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x300;
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC1->CR &= ~DAC_CR_TSEL1;
    DAC1->CR |= DAC_CR_TEN1;
    DAC1->CR |= DAC_CR_EN1;
}

//============================================================================
// Timer 6 ISR
//============================================================================
// Write the Timer 6 ISR here.  Be sure to give it the right name.
void TIM6_IRQHandler()
{
    TIM6->SR &= ~TIM_SR_UIF; // Remember to acknowledge the interrupt here!
    offset0 += step0;

    if (offset0 >= (N << 16))
    {
        offset0 -= (N << 16);
    }

    int samp = wavetable[offset0 >> 16];
    samp *= volume;
    samp = samp >> 17;
    samp += 2048;
    DAC->DHR12R1 = samp;
}
//============================================================================
// init_tim6()
//============================================================================
void init_tim6(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = (480000 / RATE) - 1;
    TIM6->ARR = (100) - 1;
    TIM6->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] = 1 << TIM6_IRQn;
    TIM6->CR1 |= TIM_CR1_CEN;
    TIM6->CR2 &= ~TIM_CR2_MMS_0;
    TIM6->CR2 |= TIM_CR2_MMS_1;
    TIM6->CR2 &= ~TIM_CR2_MMS_2;
}

int main()
{
    internal_clock();
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    // command_shell();
    LCD_Setup();
    srand(time(0));

    initialLCD();
    setup_tim7();

    // workerLCD();
}

void initialLCD()
{
    LCD_DrawFillRectangle(0, 0, 320, 320, WHITE);
    LCD_DrawLine(120, 0, 120, 320, BLACK);

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

void drawUP(int lr, int bw) // lr = left (0) or right (1) side arrow, inc = increment down by inc, bw = black or white
{
    if (lr == 0)
    { // left
        LCD_DrawLine(180, 170 - inc, 180, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(180, 220 - inc, 150, 200 - inc, (bw == 0) ? BLACK : WHITE); // up arrow
        LCD_DrawLine(180, 220 - inc, 210, 200 - inc, (bw == 0) ? BLACK : WHITE);
    }
    else
    { // right
        LCD_DrawLine(180, 170 - inc, 180, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(180, 220 - inc, 150, 200 - inc, (bw == 0) ? BLACK : WHITE); // up arrow
        LCD_DrawLine(180, 220 - inc, 210, 200 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawDOWN(int lr, int bw)
{
    if (lr == 0)
    {
        LCD_DrawLine(180, 170 - inc, 180, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(180, 170 - inc, 150, 190 - inc, (bw == 0) ? BLACK : WHITE); // down arrow
        LCD_DrawLine(180, 170 - inc, 210, 190 - inc, (bw == 0) ? BLACK : WHITE);
    }
    else
    {
        LCD_DrawLine(180, 170 - inc, 180, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(180, 170 - inc, 150, 190 - inc, (bw == 0) ? BLACK : WHITE); // down arrow
        LCD_DrawLine(180, 170 - inc, 210, 190 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawLEFT(int lr, int bw)
{
    if (lr == 0)
    {
        LCD_DrawLine(155, 220 - inc, 205, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(205, 220 - inc, 185, 190 - inc, (bw == 0) ? BLACK : WHITE); // left arrow
        LCD_DrawLine(205, 220 - inc, 185, 250 - inc, (bw == 0) ? BLACK : WHITE);
    }
    else
    {
        LCD_DrawLine(155, 220 - inc, 205, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(205, 220 - inc, 185, 190 - inc, (bw == 0) ? BLACK : WHITE); // left arrow
        LCD_DrawLine(205, 220 - inc, 185, 250 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

void drawRIGHT(int lr, int bw)
{
    if (lr == 0)
    {
        LCD_DrawLine(155, 220 - inc, 205, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(155, 220 - inc, 175, 190 - inc, (bw == 0) ? BLACK : WHITE); // right arrow
        LCD_DrawLine(155, 220 - inc, 175, 250 - inc, (bw == 0) ? BLACK : WHITE);
    }
    else
    {
        LCD_DrawLine(155, 220 - inc, 205, 220 - inc, (bw == 0) ? BLACK : WHITE);
        LCD_DrawLine(155, 220 - inc, 175, 190 - inc, (bw == 0) ? BLACK : WHITE); // right arrow
        LCD_DrawLine(155, 220 - inc, 175, 250 - inc, (bw == 0) ? BLACK : WHITE);
    }
}

// void drawALL(int lr, int bw)
// {
//     drawUP(lr, bw);
//     drawDOWN(lr, bw);
//     drawLEFT(lr, bw);
//     drawRIGHT(lr, bw);
// }

void TIM7_IRQHandler()
{
    TIM7->SR &= ~TIM_SR_UIF;
    void (*arrowFunctions[])(int, int) = {drawLEFT, drawDOWN, drawUP, drawRIGHT};

    // LCD_DrawFillRectangle(150,0,210,320,WHITE); //erasure left
    // drawALL(0,1);
    (*arrowFunctions[randomIndex])(0, 1);
    inc++;

    if (inc == 150)
    {
        inc = 0;
        return;
    }

    if (inc == 0)
    {
        randomIndex = rand() % 4;
        (*arrowFunctions[randomIndex])(0, 0);
        inc++;
    }
    else
    {
        (*arrowFunctions[randomIndex])(0, 0);
    }
}

void setup_tim7()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 1000000 - 1;
    TIM7->ARR = 24 - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] = 1 << TIM7_IRQn;
    TIM7->CR1 |= TIM_CR1_CEN;
}

#endif
