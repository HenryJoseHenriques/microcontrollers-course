/*
 * sonar_de_ataque.c
 *
 *  Created on: Jun 13, 2026
 *      Author: Acadêmico
 */

#include "sonar_de_ataque.h"
#ifndef SRC_SONAR_DE_ATAQUE_C_
#define SRC_SONAR_DE_ATAQUE_C_

/*Diretivas, Macros e Globais*/
sonar_attack_t prototype;
#define SERVO_MIN_CCR  250 //0º
#define SERVO_MED_CCR 750 //90º
#define SERVO_MAX_CCR 1250 //180°
#define FIRST_QUADRANT 1
#define SECOND_QUADRANT 2
#define BACK 1
#define FORTH -1
#define DISTANCE_BETWEEN_COMPONENTS 10.0f
#define LIMIT_RANGE 10
#define TICK_EXPIRED(lastTick, interval) ((HAL_GetTick() - (lastTick) >= (interval)) ? ((lastTick) = HAL_GetTick(), 1) : 0)

#define ANGULO_PARA_CCR(ang) \
    (SERVO_MIN_CCR + \
    (((uint32_t)(ang)) * \
    (SERVO_MAX_CCR - SERVO_MIN_CCR) / 180U))

#define CCR_PARA_ANGULO(ccr) \
    ((((uint32_t)(ccr)) - SERVO_MIN_CCR) * \
    180U / \
    (SERVO_MAX_CCR - SERVO_MIN_CCR))

#define DEG_TO_RAD(x) ((x) * 3.14159265359f / 180.0f)
#define RAD_TO_DEG(x) ((x) * 180.0f / 3.14159265359f)

static uint8_t alertState = 0; //alternar o estado do buzzer e do LED
static uint32_t lastAlertUpdate = 0; //verifica a passagem de tempo entre o alternar de um estado e de outro do  buzzer e do LED
static uint32_t last_measure = 0; //
static uint32_t last_print = 0;

///*Métodos utilitários*/

static float CalculateTargetAngle(void)
{
    float a = prototype.target.Distance;                  // distancia medida pelo sonar
    float beta = DEG_TO_RAD((float)prototype.target.Angle); // angulo do sonar em rad
    float c = DISTANCE_BETWEEN_COMPONENTS;                // distancia entre sonar e laser

    // Coordenadas do alvo no referencial do sonar
    float x = a * cosf(beta);
    float y = a * sinf(beta);

    // Corrige para o referencial do laser
    float thetaLaser = atan2f(y, (x - c));

    float ang = RAD_TO_DEG(thetaLaser);

    // Limita entre 0 e 180
    if (ang < 0.0f) ang = 0.0f;
    if (ang > 180.0f) ang = 180.0f;

    return ang;
}


static void US_Delay(uint16_t time) {
	__HAL_TIM_SET_COUNTER(prototype.scanSonar.htim, 0);
	while (__HAL_TIM_GET_COUNTER(prototype.scanSonar.htim) < time);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) { //Função de retorno, captura o exato momento em que um evento elétrico ocorre, como a subida ou descida de um pino
	prototype.scanSonar.htim = htim;
	if (prototype.scanSonar.htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) // if the interrupt source is channel1
			{
		if (prototype.scanSonar.IsFirstCaptured == 0) // if the first value is not captured
				{
			prototype.scanSonar.ICVal1 = HAL_TIM_ReadCapturedValue(htim,
					TIM_CHANNEL_1); // read the first value
			prototype.scanSonar.IsFirstCaptured = 1; // set the first captured as true
			// Now change the polarity to falling edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
					TIM_INPUTCHANNELPOLARITY_FALLING);
		}

		else if (prototype.scanSonar.IsFirstCaptured == 1) // if the first is already captured
				{
			prototype.scanSonar.ICVal2 = HAL_TIM_ReadCapturedValue(
					prototype.scanSonar.htim, prototype.scanSonar.channel); // read second value
			__HAL_TIM_SET_COUNTER(prototype.scanSonar.htim, 0); // reset the counter

			if (prototype.scanSonar.ICVal2 > prototype.scanSonar.ICVal1) {
				prototype.scanSonar.Difference = prototype.scanSonar.ICVal2
						- prototype.scanSonar.ICVal1;
			}

			else if (prototype.scanSonar.ICVal1 > prototype.scanSonar.ICVal2) {
				prototype.scanSonar.Difference = (0xffff
						- prototype.scanSonar.ICVal1)
						+ prototype.scanSonar.ICVal2;
			}

			prototype.scanSonar.Distance = prototype.scanSonar.Difference * .034
					/ 2;
			prototype.scanSonar.IsFirstCaptured = 0; // set it back to false

			// set polarity to rising edge
			__HAL_TIM_SET_CAPTUREPOLARITY(prototype.scanSonar.htim,
					prototype.scanSonar.channel,
					TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(prototype.scanSonar.htim, TIM_IT_CC1);
		}
	}
}

/*Métodos da aplicação*/
void Sonar_Attack_Init(void) {
	//Inicia servos
	HAL_TIM_PWM_Start(prototype.attackServo.htim,
			prototype.attackServo.channel);
	__HAL_TIM_SET_COMPARE(prototype.attackServo.htim,
			prototype.attackServo.channel, SERVO_MIN_CCR);
	prototype.attackServo.currentPWM = SERVO_MIN_CCR;
	prototype.attackServo.direction = BACK;
	prototype.attackServo.quadrant = FIRST_QUADRANT;
	prototype.attackServo.lastUpdate = HAL_GetTick();

	HAL_TIM_PWM_Start(prototype.scanServo.htim, prototype.scanServo.channel);
	__HAL_TIM_SET_COMPARE(prototype.scanServo.htim, prototype.scanServo.channel,
			SERVO_MIN_CCR);
	prototype.scanServo.currentPWM = SERVO_MIN_CCR;
	prototype.scanServo.direction = BACK;
	prototype.scanServo.quadrant = FIRST_QUADRANT;
	prototype.scanServo.lastUpdate = HAL_GetTick();

	//Inicia ultrassonico
	HAL_TIM_Base_Start(prototype.scanSonar.htim);
	HAL_TIM_IC_Start_IT(prototype.scanSonar.htim, prototype.scanSonar.channel);
	prototype.scanSonar.ICVal1 = 0;
	prototype.scanSonar.ICVal2 = 0;
	prototype.scanSonar.Difference = 0;
	prototype.scanSonar.IsFirstCaptured = 0;
	prototype.scanSonar.Distance = 0;


	//Inicia sistema de alerta
	HAL_GPIO_WritePin(prototype.alertLed.GPIOx, prototype.alertLed.GPIO_Pin,
			GPIO_PIN_RESET);
	HAL_GPIO_WritePin(prototype.alertBuzzer.GPIOx,
			prototype.alertBuzzer.GPIO_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(prototype.attackLaser.GPIOx,
			prototype.attackLaser.GPIO_Pin, GPIO_PIN_RESET);

	//Alvo
	prototype.target.state = TARGET_SEARCHING;
	prototype.target.entryAngle = 0;
	prototype.target.exitAngle = 0;
	prototype.target.minDistance = 0xFFFF;
	prototype.target.minDistanceAngle = 0;
	prototype.target.Distance = 0;
	prototype.target.Angle = 0;
}

void Sonar_Attack_Scan(void) {
	if (TICK_EXPIRED(prototype.scanServo.lastUpdate, 3)) {
		__HAL_TIM_SET_COMPARE(prototype.scanServo.htim,
				prototype.scanServo.channel, prototype.scanServo.currentPWM);

		prototype.scanServo.currentPWM += prototype.scanServo.direction;
	}
	if (prototype.scanServo.currentPWM >= SERVO_MAX_CCR) {
		prototype.scanServo.currentPWM = SERVO_MAX_CCR;
		prototype.scanServo.direction = FORTH;

		prototype.scanServo.scanFinished = 1;
	}

	if (prototype.scanServo.currentPWM <= SERVO_MIN_CCR) {
		prototype.scanServo.currentPWM = SERVO_MIN_CCR;
		prototype.scanServo.direction = BACK;

		prototype.scanServo.scanFinished = 1;
	}
	prototype.scanServo.quadrant =
			(prototype.scanServo.currentPWM < SERVO_MED_CCR) ?
					FIRST_QUADRANT : SECOND_QUADRANT;
}

void Sonar_Attack_Neutralize(void)
{
    if (prototype.target.state != TARGET_LOCKED)
        return;

    float correctedAngle = CalculateTargetAngle();

    prototype.attackServo.currentPWM = ANGULO_PARA_CCR((uint16_t)correctedAngle);

    __HAL_TIM_SET_COMPARE(
        prototype.attackServo.htim,
        prototype.attackServo.channel,
        prototype.attackServo.currentPWM
    );

    HAL_GPIO_WritePin(
        prototype.attackLaser.GPIOx,
        prototype.attackLaser.GPIO_Pin,
        GPIO_PIN_SET
    );
}


void Sonar_Attack_HCSR04_Read(void) {
	if (TICK_EXPIRED(last_measure, 60)) {
		HAL_GPIO_WritePin(prototype.scanSonar.TrigPort,
				prototype.scanSonar.TrigPin, GPIO_PIN_SET); // pull the TRIG pin HIGH
		US_Delay(10);  // wait for 10 us
		HAL_GPIO_WritePin(prototype.scanSonar.TrigPort,
				prototype.scanSonar.TrigPin, GPIO_PIN_RESET); // pull the TRIG pin low
		__HAL_TIM_ENABLE_IT(prototype.scanSonar.htim, TIM_IT_CC1);
	}
}


//void Sonar_Attack_Target(void)
//{
//    uint16_t distance = prototype.scanSonar.Distance;
//    uint16_t angle = CCR_PARA_ANGULO(prototype.scanServo.currentPWM);
//
//    switch(prototype.target.state)
//    {
//        case TARGET_SEARCHING:
//
//            prototype.target.minDistance = 0xFFFF;
//            prototype.target.minDistanceAngle = 0;
//            prototype.scanServo.scanFinished = 0;
//            prototype.target.state = TARGET_TRACKING;
//
//            break;
//
//        case TARGET_TRACKING:
//
//            if(distance > 0 && distance <= LIMIT_RANGE)
//            {
//                if(distance < prototype.target.minDistance)
//                {
//                    prototype.target.minDistance = distance;
//                    prototype.target.minDistanceAngle = angle;
//                }
//            }
//
//            if(prototype.scanServo.scanFinished)
//            {
//                prototype.scanServo.scanFinished = 0;
//
//                if(prototype.target.minDistance != 0xFFFF)
//                {
//                    prototype.target.Distance = prototype.target.minDistance;
//                    prototype.target.Angle = prototype.target.minDistanceAngle;
//
//                    prototype.target.state = TARGET_LOCKED;
//                }
//                else
//                {
//                    prototype.target.state = TARGET_SEARCHING;
//                }
//            }
//
//            break;
//
//        case TARGET_LOCKED:
//
//            break;
//    }
//}

void Sonar_Attack_Target(void)
{
    uint16_t distance = prototype.scanSonar.Distance;
    uint16_t angle = CCR_PARA_ANGULO(prototype.scanServo.currentPWM);

    switch(prototype.target.state)
    {
        //-------------------------------------------------
        case TARGET_SEARCHING:

            if(distance > 0 && distance <= LIMIT_RANGE)
            {
                prototype.target.Distance = distance;
                prototype.target.Angle = angle;

                prototype.target.stableCount = 1;
                prototype.target.lostCount = 0;

                prototype.target.state = TARGET_CONFIRMING;
            }

            break;

        //-------------------------------------------------
        case TARGET_CONFIRMING:

            if(distance > 0 && distance <= LIMIT_RANGE)
            {
                /* filtro simples */

                prototype.target.Distance =
                        (prototype.target.Distance * 3 + distance) / 4;

                prototype.target.Angle =
                        (prototype.target.Angle * 3 + angle) / 4;

                prototype.target.stableCount++;

                if(prototype.target.stableCount >= 3)
                {
                    prototype.target.state = TARGET_LOCKED;
                }
            }
            else
            {
                prototype.target.state = TARGET_SEARCHING;
            }

            break;

        //-------------------------------------------------
        case TARGET_LOCKED:

            if(distance > 0 && distance <= LIMIT_RANGE)
            {
                prototype.target.Distance =
                        (prototype.target.Distance * 7 + distance) / 8;

                prototype.target.Angle =
                        (prototype.target.Angle * 7 + angle) / 8;

                prototype.target.lostCount = 0;
            }
            else
            {
                prototype.target.lostCount++;

                if(prototype.target.lostCount >= 5)
                {
                    prototype.target.state = TARGET_SEARCHING;
                }
            }

            break;
    }
}

void Sonar_Attack_Monitor(void) {
	uint16_t distanceSonar = prototype.scanSonar.Distance;
	uint16_t angleSonar = CCR_PARA_ANGULO(prototype.scanServo.currentPWM);
	float correctedAngle = CalculateTargetAngle();
	if (TICK_EXPIRED(last_print, 3)) {
		if(prototype.target.state == TARGET_LOCKED){
			printf("State=%d | Dist=%.2f | TargetDist=%u | TargetAng=%u\r\n | Corrigido=%.2f\r\n",
						       prototype.target.state,
						       prototype.scanSonar.Distance,
						       prototype.target.Distance,
						       prototype.target.Angle, correctedAngle);
		}else{
			printf("\nDistancia do Sonar %u cm\r\nAngulo do Sonar: %u",distanceSonar, angleSonar);
		}
	}
}


//void Sonar_Attack_Loop(void)
//{
//	Sonar_Attack_HCSR04_Read();
//    switch(prototype.target.state)
//    {
//        case TARGET_SEARCHING:
//            Sonar_Attack_Scan();
//            Sonar_Attack_Target();
//
//            HAL_GPIO_WritePin(prototype.attackLaser.GPIOx, prototype.attackLaser.GPIO_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(prototype.alertBuzzer.GPIOx, prototype.alertBuzzer.GPIO_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(prototype.alertLed.GPIOx, prototype.alertLed.GPIO_Pin, GPIO_PIN_RESET);
//            break;
//
//        case TARGET_TRACKING:
//            Sonar_Attack_Scan();
//            Sonar_Attack_Target();
//            break;
//
//        case TARGET_LOCKED:
//            //Sonar_Attack_Neutralize();
//
//            if (TICK_EXPIRED(lastAlertUpdate, 300))
//            {
//                float correctedAngle = CalculateTargetAngle();
//
//                prototype.attackServo.currentPWM = ANGULO_PARA_CCR((uint16_t)correctedAngle);
//
//                __HAL_TIM_SET_COMPARE(
//                    prototype.attackServo.htim,
//                    prototype.attackServo.channel,
//                    prototype.attackServo.currentPWM
//                );
//
//                HAL_GPIO_WritePin(
//                    prototype.attackLaser.GPIOx,
//                    prototype.attackLaser.GPIO_Pin,
//                    GPIO_PIN_SET
//                );
//
//                alertState = !alertState;
//                HAL_GPIO_WritePin(prototype.alertBuzzer.GPIOx, prototype.alertBuzzer.GPIO_Pin, alertState);
//                HAL_GPIO_WritePin(prototype.alertLed.GPIOx, prototype.alertLed.GPIO_Pin, alertState);
//            }
//
//            if (prototype.scanSonar.Distance > LIMIT_RANGE)
//            {
//                HAL_GPIO_WritePin(prototype.attackLaser.GPIOx, prototype.attackLaser.GPIO_Pin, GPIO_PIN_RESET);
//                HAL_GPIO_WritePin(prototype.alertBuzzer.GPIOx, prototype.alertBuzzer.GPIO_Pin, GPIO_PIN_RESET);
//                HAL_GPIO_WritePin(prototype.alertLed.GPIOx, prototype.alertLed.GPIO_Pin, GPIO_PIN_RESET);
//
//                prototype.target.state = TARGET_SEARCHING;
//                prototype.target.entryAngle = 0;
//                prototype.target.exitAngle = 0;
//                prototype.target.minDistance = 0xFFFF;
//                prototype.target.minDistanceAngle = 0;
//                prototype.target.Distance = 0;
//                prototype.target.Angle = 0;
//            }
//            break;
//
//        default:
//            prototype.target.state = TARGET_SEARCHING;
//            break;
//    }
//
//    Sonar_Attack_Monitor();
//}

void Sonar_Attack_Loop(void)
{
    Sonar_Attack_HCSR04_Read();

    Sonar_Attack_Scan();

    Sonar_Attack_Target();

    switch(prototype.target.state)
    {
        case TARGET_SEARCHING:

            HAL_GPIO_WritePin(prototype.attackLaser.GPIOx,
                              prototype.attackLaser.GPIO_Pin,
                              GPIO_PIN_RESET);

            HAL_GPIO_WritePin(prototype.alertLed.GPIOx,
                              prototype.alertLed.GPIO_Pin,
                              GPIO_PIN_RESET);

            HAL_GPIO_WritePin(prototype.alertBuzzer.GPIOx,
                              prototype.alertBuzzer.GPIO_Pin,
                              GPIO_PIN_RESET);

            break;

        case TARGET_CONFIRMING:

            /* ainda procurando confirmação */

            break;

        case TARGET_LOCKED:

            Sonar_Attack_Neutralize();

            if(TICK_EXPIRED(lastAlertUpdate,300))
            {
                alertState = !alertState;

                HAL_GPIO_WritePin(
                        prototype.alertLed.GPIOx,
                        prototype.alertLed.GPIO_Pin,
                        alertState);

                HAL_GPIO_WritePin(
                        prototype.alertBuzzer.GPIOx,
                        prototype.alertBuzzer.GPIO_Pin,
                        alertState);
            }

            break;
    }

    Sonar_Attack_Monitor();
}

#endif /* SRC_SONAR_DE_ATAQUE_C_ */
