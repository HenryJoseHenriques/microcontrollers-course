#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

#define DIGITS 4
#define LIM_MIN 99
#define LIM_SEG 59
#define TICK_EXPIRED(lastTick, interval) \
    ((HAL_GetTick() - (lastTick) >= (interval)) ? ((lastTick) = HAL_GetTick(), 1) : 0)
#define PART_ONE(n) (n % 10)
#define PART_TEN(n) ((n / 10) % 10)

/*----Máquina de Estados------*/
typedef enum
{
    STATE_NONE,
    STATE_START,
    STATE_SET,
    STATE_DEFAULT,
    STATE_INCREASE,
    STATE_TRADE_DIGIT,
    STATE_STOP_RETURN,

    STATE_INIT_STOPWATCH,
    STATE_WAITING_STOPWATCH,
    STATE_START_STOPWATCH,
    STATE_STOP_RETURN_STOPWATCH
} statesTimer;

typedef enum
{
    BUTTON_NONE,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3
} button;

statesTimer next_state, state = STATE_NONE;
button event = BUTTON_NONE;
/*----FIM Máquina de Estados------*/

/*----Variáveis do debouncing------*/
uint32_t last_timer_tick = 0;
uint32_t last_display_tick = 0;
uint32_t button_S1_tick = 0;
uint32_t button_S2_tick = 0;
uint32_t button_S3_tick = 0;
uint32_t blink_tick = 0;
/*----FIM Variáveis do debouncing------*/

/*----Variáveis do display  de sete segmentos e quatro digitos------*/
const uint8_t segments[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};
uint8_t min, seg, set_min = 99, set_seg = 59;
uint8_t current_digit = 0;
uint8_t selected_digit_idx = 0; // 0 a 3
uint8_t blink_state = 1;        // 1 = visível, 0 = invisível
// uint8_t timer_done = 0;
uint8_t number[DIGITS];
/*----FIM Variáveis do display  de sete segmentos e quatro digitos------*/

/*----Cabeçalhos------*/
void waiting_action(void);
void start_timer(void);
void set_timer(void);
void default_timer(void);
void increase_digit(void);
void trade_digit(void);
void stop_return(void);
void split_number(void);
void enable_digit(uint8_t digit);
void refresh_display(void);

void mode_stopwatch(void);
void start_stopwatch(void);
void stop_stopwatch(void);

statesTimer get_last_state(statesTimer atual, button event);
button read_button(void);
void init_stopwatch(void);
void (*exec_this[])(void) = {
    [STATE_NONE] = waiting_action,
    [STATE_START] = start_timer,
    [STATE_SET] = set_timer,
    [STATE_DEFAULT] = default_timer,
    [STATE_INCREASE] = increase_digit,
    [STATE_TRADE_DIGIT] = trade_digit,
    [STATE_STOP_RETURN] = stop_return,
    [STATE_INIT_STOPWATCH] = init_stopwatch,
    [STATE_WAITING_STOPWATCH] = mode_stopwatch,
    [STATE_START_STOPWATCH] = start_stopwatch,
    [STATE_STOP_RETURN_STOPWATCH] = stop_stopwatch};
/*----FIM Cabeçalhos------*/

int main(void)
{

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    default_timer();
    split_number();
    refresh_display();
    while (1)
    {
        // Parte 1: verifica a entrada
        event = read_button();

        // Parte 2: Atualiza o state se necessário
        state = get_last_state(state, event);

        // Parte 3: executa a logica repetitiva
        exec_this[state]();

        split_number();
        refresh_display();
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
/*----Funções máquinas de estados----*/
statesTimer get_last_state(statesTimer atual, button event)
{
    switch (atual)
    {
        case STATE_NONE:
            switch (event)
            {
                case BUTTON_1: return STATE_START;
                case BUTTON_2: return STATE_SET;
                case BUTTON_3: return STATE_INIT_STOPWATCH;
                default:       return STATE_NONE;
            }

        case STATE_START:
            switch (event)
            {
                case BUTTON_1: return STATE_STOP_RETURN;
                case BUTTON_2: return STATE_SET;
                case BUTTON_3: return STATE_DEFAULT;
                default:       return STATE_START;
            }

        case STATE_SET:
            switch (event)
            {
                case BUTTON_1: return STATE_NONE;
                case BUTTON_2: return STATE_INCREASE;
                case BUTTON_3: return STATE_TRADE_DIGIT;
                default:       return STATE_SET;
            }

        case STATE_STOP_RETURN:
            switch (event)
            {
                case BUTTON_1: return STATE_START;
                case BUTTON_2: return STATE_DEFAULT;
                default:       return STATE_STOP_RETURN;
            }

        case STATE_INCREASE:
            return STATE_SET;

        case STATE_TRADE_DIGIT:
            return STATE_SET;

        case STATE_DEFAULT:
            return STATE_NONE;

        /* CRONÔMETRO */

        case STATE_INIT_STOPWATCH:
            return STATE_WAITING_STOPWATCH;

        case STATE_WAITING_STOPWATCH:
            switch (event)
            {
                case BUTTON_1: return STATE_START_STOPWATCH;
                case BUTTON_3: return STATE_DEFAULT;
                default:       return STATE_WAITING_STOPWATCH;
            }

        case STATE_START_STOPWATCH:
            if (event == BUTTON_1)
                return STATE_STOP_RETURN_STOPWATCH;
            return STATE_START_STOPWATCH;

        case STATE_STOP_RETURN_STOPWATCH:
            switch (event)
            {
                case BUTTON_1: return STATE_START_STOPWATCH;
                case BUTTON_2: return STATE_WAITING_STOPWATCH;
                default:       return STATE_STOP_RETURN_STOPWATCH;
            }

        default:
            return atual;
    }
}


button read_button()
{
    // Variáveis estáticas para lembrar como o botão estava na última vez que a função rodou
    static GPIO_PinState last_S1 = GPIO_PIN_SET;
    static GPIO_PinState last_S2 = GPIO_PIN_SET;
    static GPIO_PinState last_S3 = GPIO_PIN_SET;

    button pressed = BUTTON_NONE;

    GPIO_PinState current_S1 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    if (current_S1 == GPIO_PIN_RESET && last_S1 == GPIO_PIN_SET)
    {
        if (TICK_EXPIRED(button_S1_tick, 50))
        {
            pressed = BUTTON_1;
        }
    }
    last_S1 = current_S1;

    GPIO_PinState current_S2 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14);
    if (current_S2 == GPIO_PIN_RESET && last_S2 == GPIO_PIN_SET)
    {
        if (TICK_EXPIRED(button_S2_tick, 50))
        {
            pressed = BUTTON_2;
        }
    }
    last_S2 = current_S2;

    GPIO_PinState current_S3 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
    if (current_S3 == GPIO_PIN_RESET && last_S3 == GPIO_PIN_SET)
    {
        if (TICK_EXPIRED(button_S3_tick, 50))
        {
            pressed = BUTTON_3;
        }
    }
    last_S3 = current_S3;

    return pressed;
}
/*----FIM funções máquinas de estados----*/

/*----Funções do temporizador----*/
void waiting_action()
{
    // enable_set = 0;
    return;
}

void start_timer(void)
{
    if (TICK_EXPIRED(last_timer_tick, 1000))
    {
        if (seg > 0)
        {
            seg--;
        }
        else
        {
            if (min > 0)
            {
                seg = LIM_SEG;
                min--;
            }
            else
            {
                // TIMER CHEGOU A 00:00
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
            }
        }
    }
}

void set_timer(void)
{
    return;
}

void default_timer(void)
{
    min = set_min;
    seg = set_seg;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
}

void increase_digit(void)
{
    uint8_t s1 = PART_ONE(seg), s10 = PART_TEN(seg);
    uint8_t m1 = PART_ONE(min), m10 = PART_TEN(min);
    switch (selected_digit_idx)
    {
    case 0:
        s1 = (s1 + 1) % 10;
        break;
    case 1:
        s10 = (s10 + 1) % 6;
        break; // Segundos dezena: 0 a 5
    case 2:
        m1 = (m1 + 1) % 10;
        break;
    case 3:
        m10 = (m10 + 1) % 10;
        break; // Minutos dezena: 0 a 9 (até 99)
    }
    set_seg = (s10 * 10) + s1;
    set_min = (m10 * 10) + m1;
    min = set_min;
    seg = set_seg;
    // next_state = STATE_SET;
}

void trade_digit(void)
{
    selected_digit_idx = (selected_digit_idx + 1) % 4;
    // next_state = STATE_SET;
}

void stop_return(void)
{
    return;
}
/*----FIM Funções do temporizador----*/

/*----Funções utilitárias do display----*/

void split_number(void)
{
    number[3] = PART_TEN(min);
    number[2] = PART_ONE(min);
    number[1] = PART_TEN(seg);
    number[0] = PART_ONE(seg);
}

void enable_digit(uint8_t digit)
{
    GPIOB->BSRR = (GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) << 16;
    switch (digit)
    {
    case 0:
        GPIOB->BSRR = GPIO_PIN_12;
        break;
    case 1:
        GPIOB->BSRR = GPIO_PIN_13;
        break;
    case 2:
        GPIOB->BSRR = GPIO_PIN_14;
        break;
    case 3:
        GPIOB->BSRR = GPIO_PIN_15;
        break;
    }
}

void refresh_display(void)
{
    if (TICK_EXPIRED(last_display_tick, 2))
    {
        // Lógica do Blink: Inverte o state a cada 250ms
        // if (HAL_GetTick() - blink_tick >= 250) {
        //     blink_tick = HAL_GetTick();
        //     blink_state = !blink_state;
        // }
        if (TICK_EXPIRED(blink_tick, 250))
        {
            blink_state = !blink_state;
        }

        // Desliga segmentos para eventitar ghosting
        GPIOA->BSRR = 0x00FF << 16;

        // Se estiver no modo SET e for o dígito selecionado, verifica o blink_state
        // if (enable_set && current_digit == selected_digit_idx && !blink_state) {
        if (state == STATE_SET && current_digit == selected_digit_idx && !blink_state)
        {
            // Não liga o dígito (fica apagado para efeito de blink)
            GPIOB->BSRR = (GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) << 16;
        }
        else
        {
            GPIOA->BSRR = segments[number[current_digit]];
            enable_digit(current_digit);
        }

        current_digit = (current_digit + 1) % DIGITS;
    }
}
/*----FIM Funções utilitárias do display----*/

/*----Funções Cronometro----*/
void init_stopwatch(void)
{
    min = 0;
    seg = 0;
    return;
}

void mode_stopwatch(void)
{
    return;
}

void start_stopwatch(void)
{
    if (min == 99 && seg == 59)
        return;

    if (TICK_EXPIRED(last_timer_tick, 1000))
    {
        seg++;
        if (seg > 59)
        {
            seg = 0;
            min++;
        }
    }
}

void stop_stopwatch(void)
{
    return;
}
/*----FIM funções Cronometro----*/
