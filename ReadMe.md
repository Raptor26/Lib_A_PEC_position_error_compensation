# VPE_velosity_position_estimate #

Пример работы с библиотекой на языке СИ:

```C
/* Подключение библиотеки */
#include "Lib_A_VPE_velosity_position_estimate/Lib_A_VPE_velosity_position_estimate.h"

/* Подключение библиотеки в которой содержатся функции синуса и косинуса */
#include "math.h"

/* Объявление структуры для оценки вектора скорости и вектора местоположения */
vpe_all_data_s VPE_velPosEstim_s;

/* В данном массиве содержится вектор линейных ускорений в системе координат 
 * сопровождающего трехгранника */
__VPE_FPT__ accBySens_NED_a[3];

__VPE_FPT__ accBySens_BODY_a[3];

/* В данном массиве содержится вектор скорости, полученный от GNSS модуля 
 * в системе координат сопровождающего трехгранника */
__VPE_FPT__ velByGNSS_NED_a[3];

/* В данном массиве содержится вектор местоположения, полученный от GNSS 
 * модуля в системе координат ECEF */
__VPE_FPT__ posByGNSS_ECEF_a[3];

/* Если готовы измерения вектора скорости, полученные от GNSS модуля готовы, то флаг должен быть равен 1 */
size_t velMeasReadyByGNSS_IRQflag;

/* Если готовы измерения вектора местоположения, полученные от GNSS модуля готовы, то флаг должен быть равен 1 */
size_t posMeasReadyByGNSS_IRQflag;

int main(void)
{
    ...
    /* ============================================================= */
    /* Объявление структуры инициализации библиотеки */
    vpe_all_data_init_s VPE_init_s;

    /* Инициализация структуры значениями по умолчанию */
    VPE_StructInit(&VPE_init_s);

    VPE_init_s.velosityCorrectCoeff = (__VPE_FPT__) 0.85;
    VPE_init_s.positionCorrectCoeff = (__VPE_FPT__) 0.85;
    VPE_init_s.integratePeriodInSec = (__VPE_FPT__) 0.001;

    /* Долгота в момент взлета РБЛА */
    VPE_init_s.lat = (__VPE_FPT__) latByGNSSInRad;
    VPE_init_s.lon = (__VPE_FPT__) lonByGNSSInRad;

    /* Указатель на функцию нахождения синуса */
    VPE_init_s.pFncSin = sin;

    /* Указатель на функцию нахождения косинуса */
    VPE_init_s.pFncCos = cos;

    /* Вызов функции инициализации */
    VPE_Init(
        &VPE_velPosEstim_s, 
        &VPE_init_s);
    /* ============================================================= */

    ...

    /* Начало бесконечного цикла */
    while(1)
    {
        ...
        /* ============================================================= */
        /* Функция VPE_GetVelosityPositionEstimate() вызывается только в том 
         * случае, когда готовы показания линейных ускорений в системе 
         * координат сопровождающего трехгранника */

        /* Если готовы новые измерения вектора скорости */
        if (velMeasReadyByGNSS_IRQflag == 1u)
        {
            /* Установка флага готовности измерений в структуре 
             * "VPE_velPosEstim_s" */
            VPE_API_SetReadyVelMeasByGNSS_flag(
                &VPE_velPosEstim_s);

            /* Сброс флага готовности измерений */
            velMeasReadyByGNSS_IRQflag = 0;
        }

        /* Если готовы новые измерения вектора местоположения */
        if (posMeasReadyByGNSS_IRQflag == 1u)
        {
            /* Установка флага готовности измерений в структуре 
             * "VPE_velPosEstim_s" */
            VPE_API_SetRedyPosMeasByGNSS_flag(
                &VPE_velPosEstim_s);

            /* Сброс флага готовности измерений */
            posMeasReadyByGNSS_IRQflag = 0;
        }

        /* Вызов функции для обновления оценки вектора скорости 
         * и вектора местоположения */
        VPE_GetVelosityPositionEstimate(
            &VPE_velPosEstim_s, 
            accBySens_NED_a,
            velByGNSS_NED_a, 
            posByGNSS_ECEF_a);
        /* ============================================================= */

        /* ============================================================= */
        /* Для получения оценки вектора скорости */
        __VPE_FPT__ velEstim_NED_a[3];
        VPE_API_GetVelEstimate_NED(
            &VPE_velPosEstim_s,
            velEstim_NED_a);
        /* Теперь в массив velEstim_NED_a записано крайнее значение оценки вектора скорости */
        /* ============================================================= */

        /* ============================================================= */
        /* Для получения оценки вектора местоположения */
        __VPE_FPT__ posEstim_NED_a[3];
        VPE_API_GetVelEstimate_NED(
            &VPE_velPosEstim_s,
            posEstim_NED_a);
        /* Теперь в массив posEstim_NED_a записано крайнее значение оценки вектора местоположения */
        /* ============================================================= */
        ...
    }
}
```