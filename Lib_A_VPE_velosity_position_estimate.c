/**
 * @file   	%<%NAME%>%.%<%EXTENSION%>%
 * @author 	Mickle Isaev
 * @version
 * @date 	%<%DATE%>%, %<%TIME%>%
 * @brief
 */


/*#### |Begin| --> Секция - "Include" ########################################*/
#include <Lib_A_PEC_position_error_compensation/Lib_A_VPE_velosity_position_estimate.h>
/*#### |End  | <-- Секция - "Include" ########################################*/


/*#### |Begin| --> Секция - "Глобальные переменные" ##########################*/
/*#### |End  | <-- Секция - "Глобальные переменные" ##########################*/


/*#### |Begin| --> Секция - "Локальные переменные" ###########################*/
/*#### |End  | <-- Секция - "Локальные переменные" ###########################*/


/*#### |Begin| --> Секция - "Прототипы локальных функций" ####################*/
static void
VPE_CorrectVector(
	__VPE_FPT__ dataEstimate_a[],
	__VPE_FPT__ dataMeasurements_a[],
	__VPE_FPT__ filtCoeff,
	__VPE_FPT__ filtered_a[]);

static void
VPE_NED2ECEF(
	pec_all_data_s *p_s,
	__VPE_FPT__ vectInNED_a[],
	__VPE_FPT__ VectInECEF_a[],
	__VPE_FPT__ lat,
	__VPE_FPT__ lon);

static void
VPE_UpdateVelosityEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ *pAccelerationInNED);

static void
VPE_UpdatePositionEstimate(
	pec_all_data_s *p_s);

static void
VPE_CorrectVelosityEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ velosityMeasurements_a[]);

static void
VPE_CorrectPositionEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ positionMeasurements_a[]);
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
VPE_Init(
	pec_all_data_s *p_s,
	pec_all_data_init_s *pInit_s)
{
	ninteg_trapz_InitStruct_s 	  init_s;
	NINTEG_Trapz_StructInit		(&init_s);

	init_s.accumulate_flag = NINTEG_DISABLE;
	init_s.integratePeriod = pInit_s->integratePeriodInSec;

	size_t i = 0;
	for (i = 0; i < VPE_ECEF_AXIS_NUMB; i++)
	{
		/* Инициализация структуры для интегрирования ускорения */
		NINTEG_Trapz_Init(
			&p_s->velosityInNED_s.NINTEG_acc2Vel_s_a[i],
			&init_s);

		/* Инициализация структуры для интегрирования скорости */
		NINTEG_Trapz_Init(
			&p_s->positionInECEF_s.NINTEG_vel2Pos_s_a[i],
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
VPE_GetVelosityPositionEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ accelerationInNED_a[],
	__VPE_FPT__ velosityMeasurementsNED_a[],
	__VPE_FPT__ positionMeasurementsECEF_a[])
{
	/* Проверка готовности измерений */
	size_t velMeasReady_flag = 1,
		   posMeasReady_flag = 1;
	size_t i = 0;
	for (i = 0; i < VPE_ECEF_AXIS_NUMB; i++)
	{
		if (velosityMeasurementsNED_a[i] == (__VPE_FPT__) 0.0)
		{
			velMeasReady_flag = 0;
			break;
		}

		if (positionMeasurementsECEF_a[i] == (__VPE_FPT__) 0.0)
		{
			posMeasReady_flag = 0;
			break;
		}
	}

	/* Обновление оценки скорости по показаниям ускорений */
	VPE_UpdateVelosityEstimate(
		p_s,
		accelerationInNED_a);

	/* Если измерения скорости готовы */
	if (velMeasReady_flag == 1)
	{
		/* Коррекция вектора скорости */
		VPE_CorrectVelosityEstimate(
			p_s,
			velosityMeasurementsNED_a);
	}

	/* Обновление оценки местоположения по показаниям скорости */
	VPE_UpdatePositionEstimate(
		p_s);

	if (posMeasReady_flag == 1)
	{
		VPE_CorrectPositionEstimate(
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
VPE_UpdateVelosityEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ *pAccelerationInNED)
{
	/* Интегрирование ускорений по трем осям для получения оценки скорости */
	size_t i = 0;
	for (i = 0; i < VPE_ECEF_AXIS_NUMB; i++)
	{
		p_s->velosityInNED_s.vel_a[i] +=
			NINTEG_Trapz(
				&p_s->velosityInNED_s.NINTEG_acc2Vel_s_a[i],
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
VPE_UpdatePositionEstimate(
	pec_all_data_s *p_s)
{
	/* Выполнение проекции вектора скорости из NED в ECEF */
	__VPE_FPT__ velosityInECEF_a[VPE_ECEF_AXIS_NUMB];
	VPE_NED2ECEF(
		p_s,
		p_s->velosityInNED_s.vel_a,
		velosityInECEF_a,
		p_s->lat,
		p_s->lon);

	/* Выполнение интегрирование вектора скорости в ECEF системе координат */
	size_t i = 0;
	for (i = 0; i < VPE_ECEF_AXIS_NUMB; i++)
	{
		p_s->positionInECEF_s.pos_a[i] +=
			NINTEG_Trapz(
				&p_s->positionInECEF_s.NINTEG_vel2Pos_s_a[i],
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
VPE_CorrectVelosityEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ velosityMeasurements_a[])
{
	VPE_CorrectVector(
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
VPE_CorrectPositionEstimate(
	pec_all_data_s *p_s,
	__VPE_FPT__ positionMeasurements_a[])
{
	VPE_CorrectVector(
		p_s->positionInECEF_s.pos_a,
		positionMeasurements_a,
		p_s->positionInECEF_s.compFilt_val,
		p_s->positionInECEF_s.pos_a);
}
/*#### |End  | <-- Секция - "Описание глобальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание локальных функций" #####################*/
void
VPE_CorrectVector(
	__VPE_FPT__ dataEstimate_a[],
	__VPE_FPT__ dataMeasurements_a[],
	__VPE_FPT__ filtCoeff,
	__VPE_FPT__ filtered_a[])
{
	size_t i = 0;
	for (i = 0; i < VPE_ECEF_AXIS_NUMB; i++)
	{
		filtered_a[i] =
			(dataEstimate_a[i] * filtCoeff)
			+ ((__VPE_FPT__)1.0 - filtCoeff) * dataMeasurements_a[i];
	}
}

void
VPE_NED2ECEF(
	pec_all_data_s *p_s,
	__VPE_FPT__ vectInNED_a[],
	__VPE_FPT__ VectInECEF_a[],
	__VPE_FPT__ lat,
	__VPE_FPT__ lon)
{
	__VPE_FPT__ temp =
		p_s->trigFnc_s.pFncCos(lat) * (-vectInNED_a[VPE_NED_Z])
		- (p_s->trigFnc_s.pFncSin(lat) * vectInNED_a[VPE_NED_X]);

	VectInECEF_a[VPE_ECEF_X] =
		(p_s->trigFnc_s.pFncCos(lon) * temp )
		- (p_s->trigFnc_s.pFncSin(lon) * vectInNED_a[VPE_NED_Y]);

	VectInECEF_a[VPE_ECEF_Y] =
		(p_s->trigFnc_s.pFncSin(lon) * temp )
		- (p_s->trigFnc_s.pFncCos(lon) * vectInNED_a[VPE_NED_Y]);

	VectInECEF_a[VPE_ECEF_Z] =
		p_s->trigFnc_s.pFncSin(lat) * (-vectInNED_a[VPE_NED_Z])
		+  p_s->trigFnc_s.pFncCos(lat) * vectInNED_a[VPE_NED_X];
}
/*#### |End  | <-- Секция - "Описание локальных функций" #####################*/


/*############################################################################*/
/*############################ END OF FILE  ##################################*/
/*############################################################################*/
