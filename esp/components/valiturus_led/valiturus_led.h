/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include <esp_err.h>

#ifndef VALITURUS_LED_H_
#define VALITURUS_LED_H_

#ifdef __cplusplus
extern "C"
{
#endif

   esp_err_t valiturus_led_init();

   esp_err_t valiturus_led_set_level(bool level);

#ifdef __cplusplus
}
#endif

#endif /* VALITURUS_LED_H_ */