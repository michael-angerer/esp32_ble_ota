/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#include <esp_err.h>
#include "driver/gpio.h"

#ifndef VALITURUS_BUTTON_H_
#define VALITURUS_BUTTON_H_

#ifdef __cplusplus
extern "C"
{
#endif
   typedef void (*onButtonPress)(void *args);

   /*
      used to init button with routine
   */
   esp_err_t valiturus_button_init(onButtonPress routine);
#ifdef __cplusplus
}
#endif

#endif /* VALITURUS_BUTTON_H_ */