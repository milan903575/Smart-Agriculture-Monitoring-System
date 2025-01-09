#include <LPC17xx.h>
#include <stdio.h>
#include <AN_LCD.h>

#define Ref_Vtg 5.000            // Reference voltage (5V)
#define Full_Scale 0xFFF         // 12-bit ADC Full Scale (0xFFF = 4095)

extern unsigned long int temp1, temp2, LED;

unsigned long int temp1 = 0, temp2 = 0, LED = 0;

// Pin configuration for LCD control (RS, EN, Data lines)
#define RS_CTRL 0x00000100       // PO.8 (RS Pin for LCD)
#define EN_CTRL 0x00000200       // PO.9 (EN Pin for LCD)
#define DT_CTRL 0x00000FF0       // PO.4 to PO.7 (Data lines for LCD)

// Function to generate a delay using Timer0
void delay() {
    LPC_TIM0->TCR = 0x02;         // Reset Timer
    LPC_TIM0->EMR = 0x20;         // Match to stop on EMR
    LPC_TIM0->MCR = 0x04;         // Generate interrupt on match
    LPC_TIM0->TCR = 0x01;         // Start Timer
    
    // Wait for the EMR flag to set, indicating timer has finished
    while (!(LPC_TIM0->EMR & 0x01)); 

    return;
}

int main(void) {
    unsigned int adc_temp;       // To store ADC conversion result
    unsigned int i, j;           // Loop counters
    float in_vtg;                // Input voltage from ADC
    char vtg[14], dval[14];      // Buffer for voltage and data
    char Msg3[] = {"Moisture:"}; // String to display moisture information

    // System Initialization
    SystemInit();
    SystemCoreClockUpdate();

    // Configure ADC pin (P0.23) for A/D conversion
    LPC_PINCON->PINSEL1 = (1 << 14) | (0 << 16); // P0.23 as ADC0

    // Configure GPIO for Buzzer and LED control
    LPC_PINCON->PINSEL0 = 0x0;    // Set pin function for GPIO
    LPC_GPIO1->FIODIR = 0x01000FF0; // Configure GPIO as output
    LPC_SC->PCONP |= (1 << 12);   // Enable ADC peripheral power

    LED = 0x0;  // Initialize LED state

    while(1) {
        // Configure ADC for conversion
        LPC_ADC->ADCR = (1 << 0) | (1 << 21) | (1 << 24); // ADC0, start conversion, enable ADC
        
        // Wait for ADC conversion to complete
        for(i = 0; i < 2000; i++);  // Delay for conversion
        while (((adc_temp = LPC_ADC->ADGDR) & 0x80000000) == 0);  // Wait till conversion is complete

        // Extract ADC value (12 bits)
        adc_temp >>= 4;
        adc_temp &= 0x00000FFF;

        // Calculate the input voltage corresponding to ADC value
        in_vtg = ((float)adc_temp * (float)Ref_Vtg) / ((float)Full_Scale);

        // Control LED/Buzzer based on input voltage range
        if (in_vtg <= 1.5) {
            LPC_GPIO1->FIOSET = 3 << 10;   // Set GPIO pins for LED/Buzzer
            LPC_GPIO1->FIOSET = 1 << 24;   // Turn on Buzzer
            LPC_TIM0->PR = 1000;           // Set Timer0 pre-scaler
            LPC_TIM0->MRO = 3000;          // Set Timer0 match register
            delay();                       // Wait for a delay
            LPC_GPIO1->FIOCLR = 3 << 10;   // Clear GPIO pins (turn off LED)
            LPC_GPIO1->FIOCLR = 1 << 24;   // Turn off Buzzer
        }
        else if (in_vtg <= 3) {
            LPC_GPIO1->FIOSET = (3 << 6) | (3 << 8); // Set GPIO pins for LED/Buzzer
            LPC_TIM0->PR = 2000;           // Set Timer0 pre-scaler
            LPC_TIM0->MRO = 3000;          // Set Timer0 match register
            delay();                       // Wait for a delay
            LPC_GPIO1->FIOCLR = (3 << 6) | (3 << 8); // Clear GPIO pins (turn off LED)
        }
        else {
            LPC_GPIO1->FIOSET = 3 << 4;    // Set GPIO pins for LED
            LPC_GPIO1->FIOSET = 1 << 24;   // Turn on Buzzer
            LPC_TIM0->PR = 3000;           // Set Timer0 pre-scaler
            LPC_TIM0->MRO = 3000;          // Set Timer0 match register
            delay();                       // Wait for a delay
            LPC_GPIO1->FIOCLR = 1 << 24;   // Turn off Buzzer
            LPC_GPIO1->FIOCLR = 3 << 4;    // Turn off LED
        }
    }

    return 0;
}