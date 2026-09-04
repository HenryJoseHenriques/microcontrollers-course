/*
 * sonar_de_ataque.h
 *
 *  Created on: Jun 13, 2026
 *      Author: AcadÃªmico
 */

#ifndef INC_SONAR_DE_ATAQUE_H_
#define INC_SONAR_DE_ATAQUE_H_
#include "main.h" //inclui as configuraÃ§Ãµes padrÃµes do stm32IDECube e tivos nativos
#include "math.h" //inclui funÃ§Ãµes matemÃ¡tica, necessÃ¡rias para calculo trigonomÃ©trico do alvo
#include <stdio.h> //inclui funÃ§Ãµes de entrada e saÃ­da de dados padrÃµes da linguagem C.

/*Estados do sistema de rastreamento*/
//typedef enum
//{
//    TARGET_SEARCHING, //procurando alvo
//    TARGET_DETECTED, //alvo detectado
//    TARGET_LOCKED //alvo neutralizado (laser apontando)
//} target_state_t;

typedef enum
{
    TARGET_SEARCHING,
    TARGET_TRACKING,
    TARGET_LOCKED
} target_state_t;

typedef struct {
	TIM_HandleTypeDef *htim; //timer, deve ser atrelado ao timer configurado no arquivo .ioc
	uint32_t channel; //canal, deve ser atrelado ao canal configurado no timer do arquivo .ioc
	uint16_t currentPWM; //Atual CCR, usando para fazer o servo girar, deve ser convertido para graus em caso de calculo trogonomÃ©trico
	int8_t direction; //direÃ§Ã£o (1 indo e -1 voltando)
	int8_t quadrant; // quadrante, para corrigir o laser
	uint32_t lastUpdate; //tempo de resposta mÃ­nimo, serve para evitar que anda no tempo correto
} SG90_t; //Nome do modelo do sensor ultrassÃ´nico

typedef struct {
	TIM_HandleTypeDef *htim; //timer, deve ser atrelado ao timer configurado no arquivo .ioc
	uint32_t channel; //canal, deve ser atrelado ao canal configurado no timer do arquivo .ioc
	GPIO_TypeDef *TrigPort; //Gatilho de disparo, o microcontrador enviar um pulso num intervalo de 10us
	uint16_t TrigPin; //Pino do Trigger
	uint32_t ICVal1; //Captura de velocidade da ida do pulso
	uint32_t ICVal2; //Captura a velocidade de volta a partir do momento em que captura um obstÃ¡culo
	uint32_t Difference; //Diferencia de velocidade entre a ida e a volta
	uint8_t IsFirstCaptured; //Diz se Ã© a primeira captura de pulso da Ida, para poder acionara a volta
	float Distance; //DistÃ¢ncia, em cm
}HCSR04_t;//Nome do modelo do servo motor utilizado

typedef struct{
	GPIO_TypeDef *GPIOx; //Tipo do GPIO
	uint16_t GPIO_Pin; //Pino
}alert_t; //aciona o sistema de alerta em caso de detecÃ§Ã£o de intrusos (buzzer, led e laser)

//typedef struct
//{
//    uint16_t entryDistance; //DistÃ¢ncia inicial de quando o alvo foi localizado enquanto o sonar fazia um meio ciclo
//    uint16_t exitDistance; //DistÃ¢ncia em que o alvo parou de ser localizado enquanto o sonar fazia um meio ciclo
//    uint16_t entryAngle; //Angulo inicial de quando o alvo comeÃ§ou a ser localizado
//    uint16_t exitAngle; ////Angulo final de quando o alvo parou de ser localizado
//
//    uint16_t Distance; //DistÃ¢ncia do alvo (distÃ¢ncia mÃ©dia)
//    uint16_t Angle; //Algo do alvo (angulo mÃ©dio)
//
//    target_state_t state; //Estado de captura do alvo, utilizado na mÃ¡quina de estados.
//
//} target_t;

typedef struct
{
    uint16_t entryAngle;
    uint16_t exitAngle;

    uint16_t minDistance;
    uint16_t minDistanceAngle;

    uint16_t Distance;
    uint16_t Angle;

    target_state_t state;

} target_t;

typedef struct{
	 SG90_t scanServo; //Servo motor responsÃ¡vel fazer o sensor ultrassÃ´nico capturar alvo dentro de um certo intervalo angular
	 SG90_t attackServo; //Servo responsÃ¡vel por posionar o laser em direÃ§Ã£o ao alvo dectado pelo sensor ultrassÃ´nico.
	 HCSR04_t scanSonar; //Sensor ultrassÃ´nico responsÃ¡vel por detectar alvo numa certa distÃ¢ncia
	 alert_t attackLaser; //Laser que aponta para o alvo, Ã© acionado quando o servo de attack se posionar em direÃ§Ã£o ao alvo
	 alert_t alertBuzzer; //Emite um bipe intermitente para alertar que um alvo foi detectado
	 alert_t alertLed; //Pisca para alertar que um alvo foi detectado
	 target_t target; //InformaÃ§Ãµes do alvo
}sonar_attack_t; //Nome geral do projeto

extern sonar_attack_t prototype; //VariÃ¡vel que Ã© usada em outro arquivo, mas que precisa exitir nesse para poder ser chamada no main

void Sonar_Attack_Init(void); //Inicia todas as variaveis dos componentes para o estado inicial
void Sonar_Attack_Scan(void); //ResponsÃ¡vel por fazer o servo de ataque girar
void Sonar_Attack_Neutralize(void); //Acionar os sistemas de alertar do prototipo (buzzer, led e laser)
void Sonar_Attack_HCSR04_Read(void); //Sonar, o principal, faz varreduras procurando algum alvo
void Sonar_Attack_Target(void); //MÃ¡quina de estados responsÃ¡val por determinar o que deve ser feito enquanto um alvo Ã© ou nÃ£o localizado
void Sonar_Attack_Monitor(void); //Envia informaÃ§Ãµes para o monitor serial

void test_ultrassonico(void);

#endif /* INC_SONAR_DE_ATAQUE_H_ */
