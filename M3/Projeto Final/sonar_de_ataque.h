/*
 * sonar_de_ataque.h
 *
 *  Created on: Jun 13, 2026
 *      Author: Acadêmico
 */

#ifndef INC_SONAR_DE_ATAQUE_H_
#define INC_SONAR_DE_ATAQUE_H_
#include "main.h" //inclui as configurações padrões do stm32IDECube e tivos nativos
#include "math.h" //inclui funções matemática, necessárias para calculo trigonométrico do alvo
#include <stdio.h> //inclui funções de entrada e saída de dados padrões da linguagem C.

/*Estados do sistema de rastreamento*/


//typedef enum
//{
//    TARGET_SEARCHING,
//    TARGET_TRACKING,
//    TARGET_LOCKED
//} target_state_t;

typedef enum
{
    TARGET_SEARCHING = 0,
    TARGET_CONFIRMING,
    TARGET_LOCKED
} target_state_t;

typedef struct {
	TIM_HandleTypeDef *htim; //timer, deve ser atrelado ao timer configurado no arquivo .ioc
	uint32_t channel; //canal, deve ser atrelado ao canal configurado no timer do arquivo .ioc
	uint16_t currentPWM; //Atual CCR, usando para fazer o servo girar, deve ser convertido para graus em caso de calculo trogonométrico
	int8_t direction; //direção (1 indo e -1 voltando)
	int8_t quadrant; // quadrante, para corrigir o laser
	uint32_t lastUpdate; //tempo de resposta mínimo, serve para evitar que anda no tempo correto
	uint8_t scanFinished; //Varredura de 180 graus completa
} SG90_t; //Nome do modelo do sensor ultrassônico

typedef struct {
	TIM_HandleTypeDef *htim; //timer, deve ser atrelado ao timer configurado no arquivo .ioc
	uint32_t channel; //canal, deve ser atrelado ao canal configurado no timer do arquivo .ioc
	GPIO_TypeDef *TrigPort; //Gatilho de disparo, o microcontrador enviar um pulso num intervalo de 10us
	uint16_t TrigPin; //Pino do Trigger
	uint32_t ICVal1; //Captura de velocidade da ida do pulso
	uint32_t ICVal2; //Captura a velocidade de volta a partir do momento em que captura um obstáculo
	uint32_t Difference; //Diferencia de velocidade entre a ida e a volta
	uint8_t IsFirstCaptured; //Diz se é a primeira captura de pulso da Ida, para poder acionara a volta
	float Distance; //Distância, em cm
}HCSR04_t;//Nome do modelo do servo motor utilizado

typedef struct{
	GPIO_TypeDef *GPIOx; //Tipo do GPIO
	uint16_t GPIO_Pin; //Pino
}alert_t; //aciona o sistema de alerta em caso de detecção de intrusos (buzzer, led e laser)

typedef struct
{
    uint16_t entryAngle;
    uint16_t exitAngle;

    uint16_t minDistance;
    uint16_t minDistanceAngle;

    uint16_t Distance;
    uint16_t Angle;

    target_state_t state;

    uint8_t stableCount;
    uint8_t lostCount;

    uint16_t filteredDistance;
    uint16_t filteredAngle;
} target_t;

typedef struct{
	 SG90_t scanServo; //Servo motor responsável fazer o sensor ultrassônico capturar alvo dentro de um certo intervalo angular
	 SG90_t attackServo; //Servo responsável por posionar o laser em direção ao alvo dectado pelo sensor ultrassônico.
	 HCSR04_t scanSonar; //Sensor ultrassônico responsável por detectar alvo numa certa distância
	 alert_t attackLaser; //Laser que aponta para o alvo, é acionado quando o servo de attack se posionar em direção ao alvo
	 alert_t alertBuzzer; //Emite um bipe intermitente para alertar que um alvo foi detectado
	 alert_t alertLed; //Pisca para alertar que um alvo foi detectado
	 target_t target; //Informações do alvo
}sonar_attack_t; //Nome geral do projeto

extern sonar_attack_t prototype; //Variável que é usada em outro arquivo, mas que precisa exitir nesse para poder ser chamada no main

void Sonar_Attack_Init(void); //Inicia todas as variaveis dos componentes para o estado inicial
void Sonar_Attack_Scan(void); //Responsável por fazer o servo de ataque girar
void Sonar_Attack_Neutralize(void); //Acionar os sistemas de alertar do prototipo (buzzer, led e laser)
void Sonar_Attack_HCSR04_Read(void); //Sonar, o principal, faz varreduras procurando algum alvo
void Sonar_Attack_Target(void); //Máquina de estados responsával por determinar o que deve ser feito enquanto um alvo é ou não localizado
void Sonar_Attack_Monitor(void); //Envia informações para o monitor serial
void Sonar_Attack_Loop(void);
//void test_ultrassonico(void);

#endif /* INC_SONAR_DE_ATAQUE_H_ */
