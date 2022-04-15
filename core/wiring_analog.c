/*
 * Copyright (c) 2021, Meco Jianting Man <jiantingman@foxmail.com>
 *
 * SPDX-License-Identifier: LGPL-v2.1
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-10     Meco Man     first version
 */

#include <Arduino.h>
#include "wiring_private.h"
#include <rtdevice.h>

#define DBG_TAG    "Arduino"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

static uint8_t _analog_read_resolution = 10; /* default: Arduino UNO is 10 bits */
static uint8_t _analog_write_resolution = 8; /* default: Arduino UNO is 8 bits */
static uint32_t _analog_pwm_hz = 500; /* default: Arduino UNO's PWM is around 500Hz */

#define PWM_PERIOD_NS (1000*1000*1000/*ns*//_analog_pwm_hz)

/* 2^x */
static uint32_t _pow2(uint8_t x)
{
    return (uint32_t)pow(2.0, (double)x);
}

void analogReference(uint8_t mode)
{
    LOG_E("analogReference() has not been implemented yet!");
}

void analogReadResolution(uint8_t bits)
{
    _analog_read_resolution = bits;
    LOG_E("analogReadResolution() has not been implemented yet!");
}

void analogWriteResolution(uint8_t bits)
{
    _analog_write_resolution = bits;
}

void analogWriteFrequency(uint32_t frequency)
{
    _analog_pwm_hz = frequency;
}

void analogWrite(uint8_t pin, int val)
{
#ifdef RT_USING_PWM
    struct rt_device_pwm *pwm_dev;
    rt_uint32_t rt_val;

    pwm_dev = (struct rt_device_pwm*)rt_device_find(pin_map_table[pin].device_name);
    if(pwm_dev != RT_NULL && pwm_dev->parent.type == RT_Device_Class_PWM)
    {
        rt_val = map(val, 0, _pow2(_analog_write_resolution)-1, 0, PWM_PERIOD_NS);
        rt_pwm_disable(pwm_dev, pin_map_table[pin].channel);
        rt_pwm_set(pwm_dev, pin_map_table[pin].channel, PWM_PERIOD_NS, rt_val);
        rt_pwm_enable(pwm_dev, pin_map_table[pin].channel);
        return;
    }
#endif /* RT_USING_PWM */

#ifdef RT_USING_DAC
    rt_dac_device_t dac_dev;
    rt_uint32_t rt_val;

    dac_dev = (rt_dac_device_t)rt_device_find(pin_map_table[pin].device_name);
    if(dac_dev != RT_NULL && dac_dev->parent.type == RT_Device_Class_DAC)
    {
        /* TODO */
        return;
    }
#endif

    /* This pin doesn't support PWM or DAC */
    if (val < _pow2(_analog_write_resolution)/2)
    {
        digitalWrite(pin, LOW);
    }
    else
    {
        digitalWrite(pin, HIGH);
    }
}

int analogRead(uint8_t pin)
{
#ifdef RT_USING_ADC
    rt_adc_device_t adc_dev;

    adc_dev = (rt_adc_device_t)rt_device_find(pin_map_table[pin].device_name);
    if(adc_dev != RT_NULL)
    {
        rt_adc_enable(adc_dev, pin_map_table[pin].channel);
        return rt_adc_read(adc_dev, pin_map_table[pin].channel);
    }
    else
#endif /* RT_USING_ADC */
    {
        LOG_E("This pin %d doesn't support ADC", pin);
        return 0;
    }
}
