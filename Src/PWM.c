/*-*****************************************************************************

ArmmiteF4 MMBasic

 PWM.c

 Handles the PWM and SERVO commands

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "main.h"
//TM_PWM_TIM_t TIM4_Data, TIM3_Data;

static unsigned int f1 = 0xFFFFFFFF, f2 = 0xFFFFFFFF, f3 = 0xFFFFFFFF; // a place to save the last used frequency
char oc1, oc2, oc3, oc4, oc5, oc6, oc7, oc8, oc9;
char s[20];
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim9;
static int dcy[3][4];

// flags which are true is the OCx module is opened

// the PWM and SERVO commands use the same function
void cmd_pwm(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
	int i, j, channel, f;
	MMFLOAT a, duty;
	int prescale = 0, counts;

	getargs(&cmdline, 15, ",");
	if ((argc & 0x01) == 0 || argc < 3)
		error("Invalid syntax");

	channel = getint(argv[0], 1, 3) - 1;

	if (checkstring(argv[2], "STOP")) {
		PWMClose(channel);
		return;
	}

	if (cmdtoken == GetCommandValue("PWM")) {
		f = getint(argv[2], 1, 20000000);
		do {
			prescale++;
			counts = round(
					(MMFLOAT) SystemCoreClock / (channel==2 ? (MMFLOAT) 1.0 : (MMFLOAT) 2.0) / (MMFLOAT) prescale
							/ (MMFLOAT) f);
		} while (counts > 65535);
		counts--;
		prescale--;
		j = 4;
		for (i = 0; i < 4; i++, j += 2) {
			if (argc < j)
				dcy[channel][i] = -1;
			else {
				duty = getnumber(argv[j]);
				if (duty < 0.0 || duty > 100.0)
					error("Number out of bounds");
				dcy[channel][i] = duty * 100.0;
				if (duty == 100.0)
					dcy[channel][i] = 10100;
				dcy[channel][i] = (counts * dcy[channel][i]) / 10000;
			}
		}
	} else {
		// Command must be SERVO
		f = getinteger(argv[2]);
		if (f >= 20) { //must be a frequency
			if (f > 1000)
				error("% out of bounds", f);
			j = 4;
		} else {
			f = 50;
			j = 2;
		}
		do {
			prescale++;
			counts = round(
					(MMFLOAT) SystemCoreClock / (MMFLOAT) 2.0 / (MMFLOAT) prescale
							/ (MMFLOAT) f);
		} while (counts > 65535);
		counts--;
		prescale--;

		for (i = 0; i < 4; i++, j += 2) {
			if (argc < j)
				dcy[channel][i] = -1;
			else {
				MMFLOAT ontime = getnumber(argv[j]);
				if (ontime < 0.01 || ontime > 18.9)
					error("Number out of bounds");
				a = ((MMFLOAT) 1.0) / ((MMFLOAT) f) * ((MMFLOAT) 1000.0);
				duty = ontime / a * ((MMFLOAT) 100.0);
				if (duty > 99.9)
					error("Number out of bounds");
				dcy[channel][i] = duty * 100.0;
				dcy[channel][i] = (counts * dcy[channel][i]) / 10000;
			}
		}
	}
	if (channel == 0 && f != f1) { //reset the clock if needed
		htim3.Instance = TIM3;
		htim3.Init.Prescaler = prescale;
		htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim3.Init.Period = counts;
		htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
			error("HAL_TIM_Base_Init(&htim3)");
		}

		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
			error("HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig)");
		}

		if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
			error("HAL_TIM_PWM_Init(&htim3)");
		}

		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
				!= HAL_OK) {
			error(
					"HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)");
		}
	} else if (channel == 1 && f != f2) {
		htim4.Instance = TIM4;
		htim4.Init.Prescaler = prescale;
		htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim4.Init.Period = counts;
		htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
			error("HAL_TIM_Base_Init(&htim4)");
		}

		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
			error("HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig)");
		}

		if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) {
			error("HAL_TIM_PWM_Init(&htim4)");
		}

		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	} else if (channel == 2 && f != f3) {
		htim9.Instance = TIM9;
		htim9.Init.Prescaler = prescale;
		htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim9.Init.Period = counts;
		htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if (HAL_TIM_Base_Init(&htim9) != HAL_OK) {
			error("HAL_TIM_Base_Init(&htim9)");
		}

		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK) {
			error("HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig)");
		}

		if (HAL_TIM_PWM_Init(&htim9) != HAL_OK) {
			error("HAL_TIM_PWM_Init(&htim9)");
		}

		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	}

	// this is channel 1
	if (channel == 0) {
		// this is output 1A
		if (!oc1) {
			//        	PIntComma(dcy[0][0]);PRet();
			ExtCfg(PWM_CH1_PIN, EXT_COM_RESERVED, 0);
			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = dcy[0][0];
			sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
			sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
			if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1)
					!= HAL_OK) {
				error("HAL_TIM_PWM_ConfigChannel");
			}
			GPIO_InitStruct.Pin = PWM_1A_Pin;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			/* Start channel 1 */
			if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK) {
				/* PWM Generation Error */
				error("HAL_TIM_PWM_Start");
			}
			f1 = f;
			oc1 = true;
			oc2 = false;
			oc3 = false;
			oc4 = false;
		} else
			htim3.Instance->CCR1 = dcy[0][0];

		// this is output 1B
		if (dcy[0][1] >= 0) {
			if (!oc2 || f1 != f) {
				ExtCfg(PWM_CH2_PIN, EXT_COM_RESERVED, 0);
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.Pulse = dcy[0][1];
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2)
						!= HAL_OK) {
					error("HAL_TIM_PWM_ConfigChannel");
				}
				GPIO_InitStruct.Pin = PWM_1B_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
				/* Start channel 1 */
				if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK) {
					/* PWM Generation Error */
					error("HAL_TIM_PWM_Start");
				}
				oc2 = true;
			} else
				htim3.Instance->CCR2 = dcy[0][1];
		}

		// this is output 1C
		if (dcy[0][2] >= 0) {
			if (!oc3) {
				ExtCfg(PWM_CH3_PIN, EXT_COM_RESERVED, 0);
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.Pulse = dcy[0][2];
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3)
						!= HAL_OK) {
					error("HAL_TIM_PWM_ConfigChannel");
				}
				GPIO_InitStruct.Pin = PWM_1C_F_CS_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
				/* Start channel 1 */
				if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK) {
					/* PWM Generation Error */
					error("HAL_TIM_PWM_Start");
				}
				oc3 = true;
			} else
				htim3.Instance->CCR3 = dcy[0][2];
		}
		// now set the timer for channel 2
	} else if (channel == 1) {
#ifndef RGT

		// this is channel 2A
		if (!oc5 || f2 != f) {
			ExtCfg(PWM_CH5_PIN, EXT_COM_RESERVED, 0);
			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = dcy[1][0];
			sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
			sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
			if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1)
					!= HAL_OK) {
				error("HAL_TIM_PWM_ConfigChannel");
			}
			GPIO_InitStruct.Pin = PWM_2A_Pin;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
			HAL_GPIO_Init(PWM_2A_GPIO_Port, &GPIO_InitStruct);
			/* Start channel 1 */
			if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) != HAL_OK) {
				/* PWM Generation Error */
				error("HAL_TIM_PWM_Start");
			}
			f2 = f;
			oc6 = oc7 = false;
			oc5 = true;
		} else
			htim4.Instance->CCR1 = dcy[1][0];

		// this is output 2B
		if (dcy[1][1] >= 0) {
			if (!oc6) {
				ExtCfg(PWM_CH6_PIN, EXT_COM_RESERVED, 0);
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.Pulse = dcy[1][1];
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3)
						!= HAL_OK) {
					error("HAL_TIM_PWM_ConfigChannel");
				}
				GPIO_InitStruct.Pin = PWM_2B_NRF_IRQ_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
				/* Start channel 1 */
				if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3) != HAL_OK) {
					/* PWM Generation Error */
					error("HAL_TIM_PWM_Start");
				}
				oc6 = true;
			} else
				htim4.Instance->CCR3 = dcy[1][1];
		}
		// this is output 2C
		if (dcy[1][2] >= 0) {
			if (!oc7) {
				ExtCfg(PWM_CH7_PIN, EXT_COM_RESERVED, 0);
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.Pulse = dcy[1][2];
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4)
						!= HAL_OK) {
					error("HAL_TIM_PWM_ConfigChannel");
				}
				GPIO_InitStruct.Pin = PWM_2C_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
				/* Start channel 1 */
				if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4) != HAL_OK) {
					/* PWM Generation Error */
					error("HAL_TIM_PWM_Start");
				}
				oc7 = true;
			} else
				htim4.Instance->CCR4 = dcy[1][2];
		}
#endif
		// now set the timer for channel 2
	}
#ifndef RGT
	else if (channel == 2) {
		//this is channel 3A
		if (!oc8 || f3 != f) {
			ExtCfg(PWM_CH8_PIN, EXT_COM_RESERVED, 0);
			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = dcy[2][0];
			sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
			sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
			if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1)
					!= HAL_OK) {
				error("HAL_TIM_PWM_ConfigChannel");
			}
			GPIO_InitStruct.Pin = PWM_3A_Pin;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
			/* Start channel 1 */
			if (HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1) != HAL_OK) {
				/* PWM Generation Error */
				error("HAL_TIM_PWM_Start");
			}
			f3 = f;
			oc9 = false;
			oc8 = true;
		} else
			htim9.Instance->CCR1 = dcy[2][0];

		// this is output 3B
		if (dcy[2][1] >= 0) {
			if (!oc9) {
				ExtCfg(PWM_CH9_PIN, EXT_COM_RESERVED, 0);
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.Pulse = dcy[2][1];
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_2)
						!= HAL_OK) {
					error("HAL_TIM_PWM_ConfigChannel");
				}
				GPIO_InitStruct.Pin = PWM_3B_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
				HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
				/* Start channel 1 */
				if (HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2) != HAL_OK) {
					/* PWM Generation Error */
					error("HAL_TIM_PWM_Start");
				}
				oc9 = true;
			} else
				htim9.Instance->CCR2 = dcy[2][1];
		}

	}
#endif
}

// close the PWM output
void PWMClose(int channel) {
	switch (channel) {
	case 0:
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
		HAL_TIM_Base_DeInit(&htim3);
		if (oc1)
			ExtCfg(PWM_CH1_PIN, EXT_NOT_CONFIG, 0);
		if (oc2)
			ExtCfg(PWM_CH2_PIN, EXT_NOT_CONFIG, 0);
		if (oc3)
			ExtCfg(PWM_CH3_PIN, EXT_NOT_CONFIG, 0);
		oc1 = oc2 = oc3 = oc4 = oc5 = oc6 = oc7 = oc8 = oc9 = false;
		f1 = 0;
		break;
	case 1:
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
		HAL_TIM_Base_DeInit(&htim4);
#ifndef RGT
		if (oc5)
			ExtCfg(PWM_CH5_PIN, EXT_NOT_CONFIG, 0);
#endif
		if (oc6)
			ExtCfg(PWM_CH6_PIN, EXT_NOT_CONFIG, 0);
		if (oc7)
			ExtCfg(PWM_CH7_PIN, EXT_NOT_CONFIG, 0);
		oc5 = oc6 = oc7 = false;
		f2 = 0;
		break;
#ifndef RGT
	case 2:
		HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2);
		HAL_TIM_Base_DeInit(&htim9);
		if (oc8)
			ExtCfg(PWM_CH8_PIN, EXT_NOT_CONFIG, 0);
		if (oc9)
			ExtCfg(PWM_CH9_PIN, EXT_NOT_CONFIG, 0);
		oc8 = oc9 = false;
		f3 = 0;
		break;
#endif
	}
}


