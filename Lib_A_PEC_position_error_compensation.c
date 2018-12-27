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
PEC_CorrectVector(
	__PEC_FPT__ dataEstimate_a[],
	__PEC_FPT__ dataMeasurements_a[],
	__PEC_FPT__ filtCoeff,
	__PEC_FPT__ filtered_a[]);

static void
PEC_NED2ECEF(
	pec_all_data_s *p_s,
	__PEC_FPT__ vectInNED_a[],
	__PEC_FPT__ VectInECEF_a[],
	__PEC_FPT__ lat,
	__PEC_FPT__ lon);

static void
PEC_UpdateVelosityEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ *pAccelerationInNED);

static void
PEC_UpdatePositionEstimate(
	pec_all_data_s *p_s);

static void
PEC_CorrectVelosityEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ velosityMeasurements_a[]);

static void
PEC_CorrectPositionEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ positionMeasurements_a[]);
/*#### |End  | <-- Секция - "Прототипы локальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание глобальных функций" ####################*/

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    { function_description }
 *
 * @param    p_s        P s
 * @param    pInit_s    Инициализировать с
 */
void
PEC_Init(
	pec_all_data_s *p_s,
	pec_all_data_init_s *pInit_s)
{
	ninteg_trapz_InitStruct_s 	  init_s;
	NINTEG_Trapz_StructInit		(&init_s);

	init_s.accumulate_flag = NINTEG_DISABLE;
	init_s.integratePeriod = pInit_s->integratePeriodInSec;

	size_t i = 0;
	for (i = 0; i < PEC_ECEF_AXIS_NUMB; i++)
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

	/* Присваивание значений коэффициентов для комплементарного фильтра */
	p_s->velosityInNED_s.compFilt_val =
		pInit_s->velosityCorrectCoeff;
	p_s->positionInECEF_s.compFilt_val =
		pInit_s->positionCorrectCoeff;

	/* Начальные значения широты и долготы */
	p_s->lat = pInit_s->lat;
	p_s->lon = pInit_s->lon;

	/* Указатели на функции нахождения синуса и косинуса */
	p_s->trigFnc_s.pFncCos = pInit_s->pFncCos;
	p_s->trigFnc_s.pFncSin = pInit_s->pFncSin;
}


/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    { function_description }
 *
 * @param    p_s                           P s
 * @param    accelerationInNED_a           Ускорение в нед
 * @param    velosityMeasurementsNED_a     Измерения скорости нед
 * @param    positionMeasurementsECEF_a    Измерения положения ecef a
 */
void
PEC_GetVelosityPositionEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ accelerationInNED_a[],
	__PEC_FPT__ velosityMeasurementsNED_a[],
	__PEC_FPT__ positionMeasurementsECEF_a[])
{
	/* Проверка готовности измерений */
	size_t velMeasReady_flag = 1,
		   posMeasReady_flag = 1;
	size_t i = 0;
	for (i = 0; i < PEC_ECEF_AXIS_NUMB; i++)
	{
		if (velosityMeasurementsNED_a[i] == (__PEC_FPT__) 0.0)
		{
			velMeasReady_flag = 0;
			break;
		}

		if (positionMeasurementsECEF_a[i] == (__PEC_FPT__) 0.0)
		{
			posMeasReady_flag = 0;
			break;
		}
	}

	/* Обновление оценки скорости по показаниям ускорений */
	PEC_UpdateVelosityEstimate(
		p_s,
		accelerationInNED_a);

	/* Если измерения скорости готовы */
	if (velMeasReady_flag == 1)
	{
		/* Коррекция вектора скорости */
		PEC_CorrectVelosityEstimate(
			p_s,
			velosityMeasurementsNED_a);
	}

	/* Обновление оценки местоположения по показаниям скорости */
	PEC_UpdatePositionEstimate(
		p_s);

	if (posMeasReady_flag == 1)
	{
		PEC_CorrectPositionEstimate(
			p_s,
			positionMeasurementsECEF_a);
	}
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    { function_description }
 *
 * @param    p_s                   P s
 * @param    pAccelerationInNED    Ускорение в нед
 */
void
PEC_UpdateVelosityEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ *pAccelerationInNED)
{
	/* Интегрирование ускорений по трем осям для получения оценки скорости */
	size_t i = 0;
	for (i = 0; i < PEC_ECEF_AXIS_NUMB; i++)
	{
		p_s->velosityInNED_s.vel_a[i] +=
			NINTEG_Trapz(
				&p_s->velosityInNED_s.acc2VelIntegrate_s_a[i],
				(__NUNTEG_FPT__) * pAccelerationInNED++);
	}
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    { function_description }
 *
 * @param    p_s    P s
 */
void
PEC_UpdatePositionEstimate(
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
	for (i = 0; i < PEC_ECEF_AXIS_NUMB; i++)
	{
		p_s->positionInECEF_s.pos_a[i] +=
			NINTEG_Trapz(
				&p_s->positionInECEF_s.vel2PosIntegrate_s_a[i],
				(__NUNTEG_FPT__) velosityInECEF_a[i]);
	}
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    { function_description }
 *
 * @param    p_s                       P s
 * @param    velosityMeasurements_a    Измерения скорости
 */
void
PEC_CorrectVelosityEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ velosityMeasurements_a[])
{
	PEC_CorrectVector(
		p_s->velosityInNED_s.vel_a,
		velosityMeasurements_a,
		p_s->velosityInNED_s.compFilt_val,
		p_s->velosityInNED_s.vel_a);
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    { function_description }
 *
 * @param    p_s                       P s
 * @param    positionMeasurements_a    Измерения положения а
 */
void
PEC_CorrectPositionEstimate(
	pec_all_data_s *p_s,
	__PEC_FPT__ positionMeasurements_a[])
{
	PEC_CorrectVector(
		p_s->positionInECEF_s.pos_a,
		positionMeasurements_a,
		p_s->positionInECEF_s.compFilt_val,
		p_s->positionInECEF_s.pos_a);
}
/*#### |End  | <-- Секция - "Описание глобальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание локальных функций" #####################*/
void
PEC_CorrectVector(
	__PEC_FPT__ dataEstimate_a[],
	__PEC_FPT__ dataMeasurements_a[],
	__PEC_FPT__ filtCoeff,
	__PEC_FPT__ filtered_a[])
{
	size_t i = 0;
	for (i = 0; i < PEC_ECEF_AXIS_NUMB; i++)
	{
		filtered_a[i] =
			(dataEstimate_a[i] * filtCoeff)
			+ ((__PEC_FPT__)1.0 - filtCoeff) * dataMeasurements_a[i];
	}
}

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
