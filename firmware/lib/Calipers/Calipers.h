#ifndef __CALIPER_INCLUDED__
#define __CALIPER_INCLUDED__
#include "Arduino.h"

/*

Arduino Library for reading cheap chinese digital vernier calipers
by Michael Egger


Based on code from Ahmed Ragab
https://www.instructables.com/Hacked-Digital-Vernier-Caliper-Using-Arduino/

Protocol description:
http://www.shumatech.com/support/chinese_scales.htm

More:
https://github.com/dave-herbert/DialIndicator

*/
#define CALIPER_RX_BUF_SIZE 24

struct Calipers
{
public:
    void begin(int datapin, int clockpin);
    bool available();
    bool is_on();
    void print();
    float get_mm();
    float get_inch();

private:
    static void clk_ISR(void *data);
    int pin_clk;
    int pin_data;

    long last_rx_millis;
    volatile uint32_t rx_buf[2];
    volatile uint8_t rx_bit_idx;
    volatile uint8_t rx_buf_idx;
    volatile boolean rx_done;

    uint32_t raw_data[2];
    uint32_t raw_data_last;

    bool mm_in = 0; // 0 for mm, 1 for inch
};

// ------------------------------------------------------------------------------------------------
// Is the caliper turned on (is the clock running)
bool Calipers::is_on() {
    if (millis() < 3000) return true;
    
    return (millis() - last_rx_millis < 500);
}



// ------------------------------------------------------------------------------------------------
// Get value in inches
float Calipers::get_inch()
{
    signed int m = raw_data[1];
    float f = ((float)m / 20480.);
    return f;
}

// ------------------------------------------------------------------------------------------------
// Get value in mm
float Calipers::get_mm()
{
    return get_inch() * 25.4;
}


// ------------------------------------------------------------------------------------------------
// Init Caliper class, set pins and attach gpio interrupt to clock pin
void Calipers::begin(int datapin, int clockpin)
{
    pin_clk = clockpin;
    pin_data = datapin;
    pinMode(pin_clk, INPUT);
    pinMode(pin_data, INPUT);
    // attachInterrupt(digitalPinToInterrupt(pin_clk), &clk_ISR(this), FALLING);

    // Code below inspired by:
    // https://esp32.com/viewtopic.php?t=25929#

    gpio_config_t io_conf{
        .pin_bit_mask = 1ULL << pin_clk,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE};
    //.intr_type = GPIO_INTR_NEGEDGE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    (void)gpio_install_isr_service(0); // ignore errors as it could be already installed
    ESP_ERROR_CHECK(gpio_isr_handler_add((gpio_num_t)pin_clk, clk_ISR, this));
}

// ------------------------------------------------------------------------------------------------
// Print caliper data to Serial monitor
void Calipers::print()
{
    uint32_t mask;
    //    if (raw_data_last == raw_data) return;
    for (int buf = 0; buf < 2; buf++)
    {
        for (int byte = 0; byte < 3; byte++)
        {
            for (int bit = 0; bit < 8; bit++)
            {
                mask = (1 << ((byte * 8) + bit));
                if (raw_data[buf] & mask)
                {
                    Serial.print('1');
                }
                else
                {
                    Serial.print('0');
                }
            }
            Serial.print(' ');
        }
        Serial.print(" --- ");
    }

    signed int m = raw_data[1];
    float f = ((float)m / 20480.);

    Serial.printf(" - Rx-isx: %d - lower %d , upper %d - %f", rx_bit_idx, raw_data[0], raw_data[1], f);
    Serial.println();

    raw_data_last = raw_data[0];
}

// ------------------------------------------------------------------------------------------------
// calculate distance from raw data, TRUE if value has changed, FALSE if it hasn't
// This function has to be called as often as possible, from the main loop for example, if you don't wan't to miss anything

bool Calipers::available()
{

    // TODO return false if there

    if (!rx_done)
        return false;

    noInterrupts();
    raw_data[0] = rx_buf[0];
    raw_data[1] = rx_buf[1];
    interrupts();
    rx_done = false;

    return (raw_data[0] != raw_data_last);
}

// ------------------------------------------------------------------------------------------------
// Interrupt Service Routine on falling edge of Clock pin

void IRAM_ATTR Calipers::clk_ISR(void *data)
{
    Calipers *c = (Calipers *)data;

    if (millis() - c->last_rx_millis > 5)
    {
        c->rx_bit_idx = 0;
        c->rx_buf_idx = 0;
        c->rx_buf[0] = 0;
        c->rx_buf[1] = 0;
        c->rx_done = false;
    }

    if (c->rx_bit_idx < CALIPER_RX_BUF_SIZE)
    {
        char n = 0;
        for (int i = 0; i < 4; i++) // four times oversampling to overcome glitches
        {
            if (!digitalRead(c->pin_data))
            {
                n++;
            }
        }
        if (n > 2)
        {
            c->rx_buf[c->rx_buf_idx] |= (1 << c->rx_bit_idx);
        }
        c->rx_bit_idx++;
    }
    else
    {
        // log_e("Too many bits.");
    }

    if (c->rx_bit_idx >= CALIPER_RX_BUF_SIZE)
    {
        if (c->rx_buf_idx == 0)
        {
            c->rx_buf_idx = 1;
            c->rx_bit_idx = 0;
        }
        else
        {
            c->rx_done = true;
        }
    }

    c->last_rx_millis = millis();
}

#endif