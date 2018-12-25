/**
 * @file   	%<%NAME%>%.%<%EXTENSION%>%
 * @author 	Mickle Isaev
 * @version
 * @date 	%<%DATE%>%, %<%TIME%>%
 * @brief
 */


/*#### |Begin| --> Секция - "Include" ########################################*/
#include "Lib_A_PEC_position_error_compensation.h"
/*#### |End  | <-- Секция - "Include" ########################################*/


/*#### |Begin| --> Секция - "Глобальные переменные" ##########################*/
/*#### |End  | <-- Секция - "Глобальные переменные" ##########################*/


/*#### |Begin| --> Секция - "Локальные переменные" ###########################*/
/*#### |End  | <-- Секция - "Локальные переменные" ###########################*/


/*#### |Begin| --> Секция - "Прототипы локальных функций" ####################*/
static void
PEC_NED2ECEF(
	pec_all_data_s *p_s,
	__PEC_FPT__ vectInNED_a[],
	__PEC_FPT__ VectInECEF_a[],
	__PEC_FPT__ lat,
	__PEC_FPT__ lon);
/*#### |End  | <-- Секция - "Прототипы локальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание глобальных функций" ####################*/
void
PEC_Init(
	pec_all_data_s *p_s,
	pec_all_data_init_s *pInit_s,
	__PEC_FPT__ integratePeriodInSec,
	__PEC_FPT__ lat,
	__PEC_FPT__ lon)
{
	ninteg_trapz_InitStruct_s 	  init_s;
	NINTEG_Trapz_StructInit		(&init_s);

	init_s.accumulate_flag = NINTEG_DISABLE;
	init_s.integratePeriod = (__NUNTEG_FPT__) integratePeriodInSec;

	size_t i = 0;
	for (i = 0; i <= PEC_ECEF_AXIS_NUMB; i++)
	{
		/* Инициализация структуры для интегрирования ускорения */
		NINTEG_Trapz_Init(
			&p_s->velosityInNED_s.acc2VelIntegrate_s_a[i],
			&init_s);

		/* Инициализация структуры для интегрирования скорости */
		NINTEG_Trapz_Init(
			&p_s->positionInECEF_s.vel2PosIntegrate_s_a[i],
			&init_s);
	}

	p_s->lat = lat;
	p_s->lon = lon;
}

void
PEC_GetVelosityEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ *pAccelerationInNED)
{
	/* Интегрирование ускорений по трем осям для получения оценки скорости */
	size_t i = 0;
	for (i = 0; i <= PEC_ECEF_AXIS_NUMB; i++)
	{
		p_s->velosityInNED_s.vel_a[i] +=
			NINTEG_Trapz(
				&p_s->velosityInNED_s.acc2VelIntegrate_s_a[i],
				(__NUNTEG_FPT__) * pAccelerationInNED++);
	}
}

void
PEC_Get_PositionEstimate(
	pec_all_data_s *p_s)
{
	/* Выполнение проекции вектора скорости из NED в ECEF */
	__PEC_FPT__ velosityInECEF_a[PEC_ECEF_AXIS_NUMB];
	PEC_NED2ECEF(
		p_s,
		p_s->velosityInNED_s.vel_a,
		velosityInECEF_a,
		p_s->lat,
		p_s->lon);

	/* Выполнение интегрирование вектора скорости в ECEF системе координат */
	size_t i = 0;
	for (i = 0; i <= PEC_ECEF_AXIS_NUMB; i++)
	{
		p_s->positionInECEF_s.pos_a[i] +=
			NINTEG_Trapz(
				&p_s->positionInECEF_s.vel2PosIntegrate_s_a[i],
				(__NUNTEG_FPT__) velosityInECEF_a[i]);
	}
}

void
PEC_CorrectVelosity(
	pec_all_data_s *p_s,
	__PEC_FPT__ velosityMeasurements_a[])
{
	
}
/*#### |End  | <-- Секция - "Описание глобальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание локальных функций" #####################*/
void
PEC_NED2ECEF(
	pec_all_data_s *p_s,
	__PEC_FPT__ vectInNED_a[],
	__PEC_FPT__ VectInECEF_a[],
	__PEC_FPT__ lat,
	__PEC_FPT__ lon)
{
	__PEC_FPT__ temp =
		p_s->trigFnc_s.pFncCos(lat) * (-vectInNED_a[PEC_NED_Z])
		- (p_s->trigFnc_s.pFncSin(lat) * vectInNED_a[PEC_NED_X]);

	VectInECEF_a[PEC_ECEF_X] =
		(p_s->trigFnc_s.pFncCos(lon) * temp )
		- (p_s->trigFnc_s.pFncSin(lon) * vectInNED_a[PEC_NED_Y]);

	VectInECEF_a[PEC_ECEF_Y] =
		(p_s->trigFnc_s.pFncSin(lon) * temp )
		- (p_s->trigFnc_s.pFncCos(lon) * vectInNED_a[PEC_NED_Y]);

	VectInECEF_a[PEC_ECEF_Z] =
		p_s->trigFnc_s.pFncSin(lat) * (-vectInNED_a[PEC_NED_Z])
		+  p_s->trigFnc_s.pFncCos(lat) * vectInNED_a[PEC_NED_X];

}
/*#### |End  | <-- Секция - "Описание локальных функций" #####################*/


/*############################################################################*/
/*############################ END OF FILE  ##################################*/
/*############################################################################*/
