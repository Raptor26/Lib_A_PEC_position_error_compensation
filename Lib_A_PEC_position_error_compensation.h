/** 
 * @file   	%<%NAME%>%.%<%EXTENSION%>%
 * @author 	Mickle Isaev
 * @version	
 * @date 	%<%DATE%>%, %<%TIME%>%
 * @brief
 */


#ifndef LIB_A_PEC_POSITION_ERROR_COMPENSATION_H_
#define LIB_A_PEC_POSITION_ERROR_COMPENSATION_H_


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
#if !defined (__PEC_FPT__)
#error 'Please, set PEC_FPT float or double'
#endif
/*#### |End  | <-- Секция - "Определение констант" ###########################*/


/*#### |Begin| --> Секция - "Определение типов" ##############################*/
typedef enum
{
	PEC_ECEF_X = 0,
	PEC_ECEF_Y,
	PEC_ECEF_Z,

	PEC_ECEF_AXIS_NUMB,
} pec_ecef_coordinate_system_e;

typedef enum
{
	PEC_NED_X = 0,
	PEC_NED_Y,
	PEC_NED_Z,

	PEC_NED_AXIS_NUMB,
} pec_ned_coordinate_system_e;

typedef struct
{
	__PEC_FPT__ (*pFncSin)(
			__PEC_FPT__ val);

	__PEC_FPT__ (*pFncCos)(
			__PEC_FPT__ val);

} pec_trigonometric_fnc_pointers_s;

typedef struct
{
	ninteg_trapz_s 	acc2VelIntegrate_s_a[PEC_NED_AXIS_NUMB];

	FILT_comp_filt_s compFilt_s_a[PEC_NED_AXIS_NUMB];

	/* Оценка скорости в NED системе координат */
	__PEC_FPT__  	vel_a[3];
} pec_velosity_ned_s;

typedef struct
{
	ninteg_trapz_s 	vel2PosIntegrate_s_a[PEC_ECEF_AXIS_NUMB];
	FILT_comp_filt_s compFilt_s_a[PEC_ECEF_AXIS_NUMB];

	/* Оценка местоположения в ECEF системе координат */
	__PEC_FPT__  	pos_a[3];
} pec_position_ecef_s;

typedef struct
{
	pec_trigonometric_fnc_pointers_s trigFnc_s;
	pec_velosity_ned_s 		velosityInNED_s;
	pec_position_ecef_s 	positionInECEF_s;
	__PEC_FPT__ lat;
	__PEC_FPT__ lon;
} pec_all_data_s;

typedef struct
{
	__PEC_FPT__ velosityCorrectCoeff;
	__PEC_FPT__ positionCorrectCoeff;
} pec_all_data_init_s;
/*#### |End  | <-- Секция - "Определение типов" ##############################*/


/*#### |Begin| --> Секция - "Определение глобальных переменных" ##############*/
/*#### |End  | <-- Секция - "Определение глобальных переменных" ##############*/


/*#### |Begin| --> Секция - "Прототипы глобальных функций" ###################*/
/*#### |End  | <-- Секция - "Прототипы глобальных функций" ###################*/


/*#### |Begin| --> Секция - "Определение макросов" ###########################*/
/*#### |End  | <-- Секция - "Определение макросов" ###########################*/

#endif	/* LIB_A_PEC_POSITION_ERROR_COMPENSATION_H_ */

/*############################################################################*/
/*################################ END OF FILE ###############################*/
/*############################################################################*/
