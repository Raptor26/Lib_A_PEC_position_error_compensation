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
#include "Lib_A_FILT_filters.c/Lib_A_FILT_filters.h"
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

typedef struct
{
	/**
	 * @brief	Структура для численного интегрирования линейных ускорений методом трапеций
	 */
	ninteg_trapz_s 	NINTEG_acc2Vel_s_a[VPE_NED_AXIS_NUMB];
	FILT_comp_filt_s compFilt_s_a[VPE_NED_AXIS_NUMB];
	__VPE_FPT__ compFilt_val;

	/**
	 * @brief	Оценка скорости в СК сопровождающего трехгранника (NED)
	 */
	__VPE_FPT__  	vel_a[3];
} vpe_velosity_ned_s;

typedef struct
{
	ninteg_trapz_s 	NINTEG_vel2Pos_s_a[VPE_ECEF_AXIS_NUMB];
	FILT_comp_filt_s compFilt_s_a[VPE_ECEF_AXIS_NUMB];
	__VPE_FPT__ compFilt_val;

	/**
	 * @brief	Оценка местопложения в связанной с Землей СК (ECEF)
	 */
	__VPE_FPT__  	pos_a[3];
} vpe_position_ecef_s;

typedef struct
{
	/**
	 * @brief	Структура, содержащая указатели на тригонометрические функции
	 */
	vpe_trigonometric_fnc_pointers_s trigFnc_s;
	vpe_velosity_ned_s 		velosityInNED_s;
	vpe_position_ecef_s 	positionInECEF_s;

	/**
	 * @brief	Долгота местоположения
	 */
	__VPE_FPT__ lat;

	/**
	 * @brief	Широта местоположения
	 */
	__VPE_FPT__ lon;
} pec_all_data_s;

/**
 * @brief	Структура для инициализации "pec_all_data_s"
 */
typedef struct
{
	__VPE_FPT__ velosityCorrectCoeff;
	__VPE_FPT__ positionCorrectCoeff;
	__VPE_FPT__ integratePeriodInSec;
	__VPE_FPT__ lat;
	__VPE_FPT__ lon;

	__VPE_FPT__ (*pFncSin)(
		__VPE_FPT__ val);
	__VPE_FPT__ (*pFncCos)(
		__VPE_FPT__ val);
} pec_all_data_init_s;
/*#### |End  | <-- Секция - "Определение типов" ##############################*/


/*#### |Begin| --> Секция - "Определение глобальных переменных" ##############*/
/*#### |End  | <-- Секция - "Определение глобальных переменных" ##############*/


/*#### |Begin| --> Секция - "Прототипы глобальных функций" ###################*/
extern void
VPE_Init(
	pec_all_data_s *p_s,
	pec_all_data_init_s *pInit_s);

extern void
VPE_GetVelosityPositionEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ accelerationInNED_a[],
	__VPE_FPT__ velosityMeasurementsNED_a[],
	__VPE_FPT__ positionMeasurementsECEF_a[]);
/*#### |End  | <-- Секция - "Прототипы глобальных функций" ###################*/


/*#### |Begin| --> Секция - "Определение макросов" ###########################*/
/*#### |End  | <-- Секция - "Определение макросов" ###########################*/

#endif	/* LIB_A_VPE_VELOSITY_POSITION_ESTIMATE_H_ */

/*############################################################################*/
/*################################ END OF FILE ###############################*/
/*############################################################################*/
