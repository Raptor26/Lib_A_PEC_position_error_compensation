/**
 * @file   	%<%NAME%>%.%<%EXTENSION%>%
 * @author 	Mickle Isaev
 * @version
 * @date 	%<%DATE%>%, %<%TIME%>%
 * @brief
 */


#ifndef LIB_A_VPE_VELOSITY_POSITION_ESTIMATE_H_
#define LIB_A_VPE_VELOSITY_POSITION_ESTIMATE_H_


/*#### |Begin| --> Секция - "Include" ########################################*/
/*==== |Begin| --> Секция - "C libraries" ====================================*/
#include <stdint.h>
#include <stdio.h>
#include <math.h>
/*==== |End  | <-- Секция - "C libraries" ====================================*/

/*==== |Begin| --> Секция - "MK peripheral libraries" ========================*/
/*==== |End  | <-- Секция - "MK peripheral libraries" ========================*/

/*==== |Begin| --> Секция - "Extern libraries" ===============================*/
#include "Lib_A_NINTEG_numerical_integration/Lib_A_NINTEG_numerical_integration.h"
/*==== |End  | <-- Секция - "Extern libraries" ===============================*/
/*#### |End  | <-- Секция - "Include" ########################################*/


/*#### |Begin| --> Секция - "Определение констант" ###########################*/
#if !defined (__VPE_FPT__)
#error 'Please, set __VPE_FPT__ float or double'
#endif
/*#### |End  | <-- Секция - "Определение констант" ###########################*/


/*#### |Begin| --> Секция - "Определение типов" ##############################*/
typedef enum
{
	VPE_ECEF_X = 0,
	VPE_ECEF_Y,
	VPE_ECEF_Z,

	VPE_ECEF_AXIS_NUMB,
} pec_ecef_coordinate_system_e;

typedef enum
{
	VPE_NED_X = 0,
	VPE_NED_Y,
	VPE_NED_Z,

	VPE_NED_AXIS_NUMB,
} pec_ned_coordinate_system_e;

/**
 * @brief	Структура, содержащая указатели на тригонометрические функции
 */
typedef struct
{
	/**
	 * @brief	Указатель на функцию нахождения синуса
	 */
	__VPE_FPT__ (*pFncSin)(
		__VPE_FPT__ val);

	/**
	 * @brief	Указатель на функцию нахождения косинуса
	 */
	__VPE_FPT__ (*pFncCos)(
		__VPE_FPT__ val);

} vpe_trigonometric_fnc_pointers_s;

/**
 * @brief	Структура, содержащая необходимые параметры для обновления
 * 			вектора скорости в СК сопровождающего трехгранника (NED)
 */
typedef struct
{
	/**
	 * @brief	Массив структур для численного интегрирования вектора линейных ускорений методом трапеций
	 */
	ninteg_trapz_s 	NINTEG_acc2Vel_s_a[VPE_NED_AXIS_NUMB];

	/**
	 * @brief	Коэффициент комплементарного фильтра для коррекции оценки вектора скорости по информации от GNSS модуля
	 */
	__VPE_FPT__ compFilt_val;

	/**
	 * @brief	Оценка скорости в СК сопровождающего трехгранника (NED)
	 */
	__VPE_FPT__  	vel_a[3];

	/**
	 * @brief	Инструментальные погрешности в векторе скорости (NED)
	 */
	__VPE_FPT__		velBias_a[3];

	__VPE_FPT__ 	velBiasCorrCoeff;

	/**
	 * @brief	Флаг готовности измерений вектора скорости от GNSS модуля
	 *  		в СК сЦопровождающего трехгранника
	 */
	size_t velMeasReadyByGNSS_flag;
} vpe_velosity_ned_s;

/**
 * @brief	Структура, содержащая необходимые параметры для обновления
 * 			вектора местоположения в связанной с Землей СК.
 */
typedef struct
{
	/**
	 * @brief	Массив структур для численного интегрирования вектора скорости методом трапеций
	 */
	ninteg_trapz_s 	NINTEG_vel2Pos_s_a[VPE_ECEF_AXIS_NUMB];

	/**
	 * @brief	Коэффициент комплементарного фильтра для коррекции оценки вектора
	 *        	местоположения по информации от GNSS модуля
	 */
	__VPE_FPT__ compFilt_val;

	/**
	 * @brief	Оценка вектора местоположения в связанной с Землей СК (ECEF)
	 */
	__VPE_FPT__  	pos_a[3];

	__VPE_FPT__		posBias_a[3];

	__VPE_FPT__		posBiasCorrCoeff;

	/**
	 * @brief	Флаг готовности измерений вектора местоположения от GNSS модуля
	 *  		в ECEF СК
	 */
	size_t posMeasReadyByGNSS_flag;
} vpe_position_ecef_s;

/**
 * @brief 	Структура, содержащая необходимые данные для обновления
 *         	вектора скорости и вектора местоположения
 */
typedef struct
{
	/**
	 * @brief	Структура, содержащая указатели на тригонометрические функции
	 */
	vpe_trigonometric_fnc_pointers_s trigFnc_s;

	/**
	 * @brief	Структура данных для оценки вектора скорости
	 */
	vpe_velosity_ned_s 		velosityInNED_s;

	/**
	 * @brief	Структура данных для оценки вектора местоположения
	 */
	vpe_position_ecef_s 	positionInECEF_s;

	/**
	 * @brief	Долгота местоположения
	 */
	__VPE_FPT__ lat;

	/**
	 * @brief	Широта местоположения
	 */
	__VPE_FPT__ lon;

} vpe_all_data_s;

/**
 * @brief	Структура для инициализации "vpe_all_data_s"
 */
typedef struct
{
	/**
	 * @brief	Коэффициент коррекции оценки вектора скорости
	 */
	__VPE_FPT__ velosityCorrectCoeff;

	/**
	 * @brief	Коэффициент коррекции оценки вектора местоположения
	 */
	__VPE_FPT__ positionCorrectCoeff;

	/**
	 * @brief	Период интегрирования вектора линейных ускорений и вектора скорости в секундах
	 */
	__VPE_FPT__ integratePeriodInSec;

	/**
	 * @brief	Начальное значение широты местоположения
	 */
	__VPE_FPT__ lat;

	/**
	 * @brief	Начальное значение долготы местоположения
	 */
	__VPE_FPT__ lon;

	__VPE_FPT__ (*pFncSin)(
		__VPE_FPT__ val);
	__VPE_FPT__ (*pFncCos)(
		__VPE_FPT__ val);

	/* Начальное значение вектора местоположения */
	__VPE_FPT__ pos_ECEF_a[VPE_ECEF_AXIS_NUMB];

	__VPE_FPT__ velBiasCorrCoeff;

	__VPE_FPT__ posBiasCorrCoeff;
} vpe_all_data_init_s;
/*#### |End  | <-- Секция - "Определение типов" ##############################*/


/*#### |Begin| --> Секция - "Определение глобальных переменных" ##############*/
/*#### |End  | <-- Секция - "Определение глобальных переменных" ##############*/


/*#### |Begin| --> Секция - "Прототипы глобальных функций" ###################*/
extern void
VPE_Init(
	vpe_all_data_s *p_s,
	vpe_all_data_init_s *pInit_s);

extern void
VPE_StructInit(
	vpe_all_data_init_s *pInit_s);

extern void
VPE_GetVelosityPositionEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ accBySens_NED_a[],
	__VPE_FPT__ velByGNSS_NED_a[],
	__VPE_FPT__ posByGNSS_ECEF_a[]);

extern void
VPE_API_SetReadyVelMeasByGNSS_flag(
	vpe_all_data_s *p_s);

extern void
VPE_API_SetRedyPosMeasByGNSS_flag(
	vpe_all_data_s *p_s);

extern void
VPE_API_GetVelEstimate_NED(
	vpe_all_data_s *p_s,
	__VPE_FPT__ velEstimate[]);

extern void
VPE_API_GetPosEstimate_ECEF(
	vpe_all_data_s *p_s,
	__VPE_FPT__ posEstimate[]);

extern void
VPE_API_UpdateLatitudeAndLongitude(
	vpe_all_data_s *p_s,
	__VPE_FPT__ lat,
	__VPE_FPT__ lon);
/*#### |End  | <-- Секция - "Прототипы глобальных функций" ###################*/


/*#### |Begin| --> Секция - "Определение макросов" ###########################*/
/*#### |End  | <-- Секция - "Определение макросов" ###########################*/

#endif	/* LIB_A_VPE_VELOSITY_POSITION_ESTIMATE_H_ */

/*############################################################################*/
/*################################ END OF FILE ###############################*/
/*############################################################################*/
