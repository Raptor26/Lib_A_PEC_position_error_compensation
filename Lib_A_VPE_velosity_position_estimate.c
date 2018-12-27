/**
 * @file   	%<%NAME%>%.%<%EXTENSION%>%
 * @author 	Mickle Isaev
 * @version
 * @date 	%<%DATE%>%, %<%TIME%>%
 * @brief
 */


/*#### |Begin| --> Секция - "Include" ########################################*/
#include "Lib_A_VPE_velosity_position_estimate.h"
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
	vpe_all_data_s *p_s,
	__VPE_FPT__ vect_NED_a[],
	__VPE_FPT__ vect_ECEF_a[],
	__VPE_FPT__ lat,
	__VPE_FPT__ lon);

static void
VPE_UpdateVelosityEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ *acc_NED_a);

static void
VPE_UpdatePositionEstimate(
	vpe_all_data_s *p_s);

static void
VPE_CorrectVelosityEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ velosityMeasurements_a[]);

static void
VPE_CorrectPositionEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ posByGNSS_ECEF_a[]);
/*#### |End  | <-- Секция - "Прототипы локальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание глобальных функций" ####################*/

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    Функция выполняет инициализацию структуры типа "vpe_all_data_s"
 *           по исходным данным, находящимся в структуре типа "pec_all_data_init_s"
 *
 * @param[out] 	*p_s:  		Указатель на инициализируемую структуру
 * @param[in]  	*pInit_s:   Указатель на структуру с исходными параметрами
 * 
 * @return  None
 */
void
VPE_Init(
	vpe_all_data_s *p_s,
	pec_all_data_init_s *pInit_s)
{
	/* Объявление структуры для инициализации структур интегрирования
	 * вектора скорости и вектора местоположения */
	ninteg_trapz_InitStruct_s 	  init_s;
	NINTEG_Trapz_StructInit		(&init_s);

	/* Отключение аккумулирования интегрированной величины */
	init_s.accumulate_flag = NINTEG_DISABLE;

	/* Установка периода интегрирования */
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
 * @brief    Функция выполняет обновление оценки вектора скорости в СК
 *           сопровождающего трехгранника (NED) и оценку вектора
 *           местоположения в СК, связанной с Землей (ECFE)
 *
 * @param[in,out]	*p_s:	Указатель на структуру в которой содержатся
 * 							необходимые для обновления параметров вектора
 * 							скорости и вектора местоположения данные
 * @param[in]	accBySens_NED_a[3]:	Указатель на нулевой элемент массива
 * 									вектора линейных ускорений в СК
 * 									сопровождающего	трехгранника (NED)
 * @param[in]   velByGNSS_NED_a[3]:	Указатель на нулевой элемент массива
 * 									измерения вектора скорости от GNSS модуля
 * 									в системе координат сопровождающего
 * 									трехгранника (NED)
 * @param[in]   posByGNSS_ECEF_a[3]:	Указатель на нулевой элемент массива
 * 										измерения вектора местоположения от
 * 										GNSS модуля в системе координат,
 * 										связанной с Землёй (ECEF)
 * 
 * @return  None
 */
void
VPE_GetVelosityPositionEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ accBySens_NED_a[],
	__VPE_FPT__ velByGNSS_NED_a[],
	__VPE_FPT__ posByGNSS_ECEF_a[])
{
	/* Обновление оценки скорости по показаниям вектора ускорений */
	VPE_UpdateVelosityEstimate(
		p_s,
		accBySens_NED_a);

	/* Если готовы показания вектора скорости от GNSS модуля */
	if (p_s->velosityInNED_s.velMeasReadyByGNSS_flag == 1u)
	{
		/* Коррекция вектора скорости */
		VPE_CorrectVelosityEstimate(
			p_s,
			velByGNSS_NED_a);

		/* Сброс флага */
		p_s->velosityInNED_s.velMeasReadyByGNSS_flag = 0u;
	}

	/* Обновление оценки местоположения по показаниям вектора скорости */
	VPE_UpdatePositionEstimate(
		p_s);

	/* Если готовы измерения вектора местоположения от GNSS  модуля */
	if (p_s->positionInECEF_s.posMeasReadyByGNSS_flag == 1u)
	{
		/* Коррекция вектора местоположения */
		VPE_CorrectPositionEstimate(
			p_s,
			posByGNSS_ECEF_a);

		/* Сброс флага */
		p_s->positionInECEF_s.posMeasReadyByGNSS_flag = 0u;
	}
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief    Функция устанавливает флаг готовности измерений вектора
 *           скорости от GNSS модуля
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * 						
 * @return  None
 */
void
VPE_API_SetReadyVelMeasByGNSS_flag(
	vpe_all_data_s *p_s)
{
	p_s->velosityInNED_s.velMeasReadyByGNSS_flag = 1u;
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief    Функция устанавливает флаг готовности измерений вектора
 *           местоположения от GNSS
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * 						
 * @return  None
 */
void
VPE_API_SetRedyPosMeasByGNSS_flag(
	vpe_all_data_s *p_s)
{
	p_s->positionInECEF_s.posMeasReadyByGNSS_flag = 1u;
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief    Функция выполняет копирование крайнего значения оценки
 *           вектора скорости в массив "velEstimate"
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * @param[out] 	velEstimate[3]:	Указатель на нулевой элемент массива в который
 * 								будет записано крайнее значение оценки вектора
 * 								скорости в СК сопровождающего трехгранника (NED)
 * 								
 * @return  None
 */
void
VPE_API_GetVelEstimate_NED(
	vpe_all_data_s *p_s,
	__VPE_FPT__ velEstimate[])
{
	velEstimate[VPE_NED_X] = p_s->velosityInNED_s.vel_a[VPE_NED_X];
	velEstimate[VPE_NED_Y] = p_s->velosityInNED_s.vel_a[VPE_NED_Y];
	velEstimate[VPE_NED_Z] = p_s->velosityInNED_s.vel_a[VPE_NED_Z];
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief    Функция выполняет копирование крайнего значения оценки
 *           вектора местоположения в массив "posEstimate"
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * @param[out] 	posEstimate[3]:	Указатель на нулевой элемент массива в который
 * 								будет записано крайнее значение оценки вектора
 * 								местоположения в связанной с Землей СК (ECEF)
 * 												
 * @return  None
 */
void
VPE_API_GetPosEstimate_ECEF(
	vpe_all_data_s *p_s,
	__VPE_FPT__ posEstimate[])
{
	posEstimate[VPE_ECEF_X] = p_s->positionInECEF_s.pos_a[VPE_ECEF_X];
	posEstimate[VPE_ECEF_Y] = p_s->positionInECEF_s.pos_a[VPE_ECEF_Y];
	posEstimate[VPE_ECEF_Z] = p_s->positionInECEF_s.pos_a[VPE_ECEF_Z];
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief   Функция обновляет текущее значение широты и долготы.
 * 			Необходимо для работы функции VPE_NED2ECEF()
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * @param[in]    lat:    Текущая широта
 * @param[in]    lon:    Текущая долгота
 * 
 * @return  None
 */
void
VPE_API_UpdateLatitudeAndLongitude(
	vpe_all_data_s *p_s,
	__VPE_FPT__ lat,
	__VPE_FPT__ lon)
{
	p_s->lat = lat;
	p_s->lon = lon;
}
/*#### |End  | <-- Секция - "Описание глобальных функций" ####################*/


/*#### |Begin| --> Секция - "Описание локальных функций" #####################*/

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    Функция выполняет обновление оценки вектора скорости по
 *           измерениям линейных ускорений
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * @param[in]	acc_NED_a[3]:	Указатель на нулевой элемент массива
 * 								вектора линейных ускорений в СК
 * 								сопровождающего трехгранника (NED)
 * 								
 * @return  None
 */
static void
VPE_UpdateVelosityEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ acc_NED_a[])
{
	/* Интегрирование вектора ускорений для получения вектора оценки скорости */
	size_t i = 0;
	for (i = 0; i < VPE_ECEF_AXIS_NUMB; i++)
	{
		p_s->velosityInNED_s.vel_a[i] +=
			NINTEG_Trapz(
				&p_s->velosityInNED_s.NINTEG_acc2Vel_s_a[i],
				(__NUNTEG_FPT__) acc_NED_a[i]);
	}
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    Функция выполняет обновление оценки вектора местоположения по
 *           оценки вектора скорости
 *
 * @param[out]	*p_s:	Указатель на структуру в которой содержатся
 * 						необходимые для обновления параметров вектора
 * 						скорости и вектора местоположения данные
 * 						
 * @return  None
 */
static void
VPE_UpdatePositionEstimate(
	vpe_all_data_s *p_s)
{
	__VPE_FPT__ velosityInECEF_a[VPE_ECEF_AXIS_NUMB];

	/* Выполнение проекции вектора скорости из NED в ECEF */
	VPE_NED2ECEF(
		p_s,
		p_s->velosityInNED_s.vel_a,
		velosityInECEF_a,
		p_s->lat,
		p_s->lon);

	/* Выполнение интегрирования вектора скорости в ECEF СК для
	 * получения  оценки местоположения в ECEF СК */
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
 * @brief    Функция выполняет коррекцию оценки вектора скорости с помощью
 *           измерений вектора скорости от GNSS модуля
 *
 * @param[in,out]	*p_s:	Указатель на структуру в которой содержатся
 * 							необходимые для обновления параметров вектора
 * 						 	скорости и вектора местоположения данные
 * @param[in]   velByGNSS_NED_a[3]:	Указатель на нулевой элемент массива
 * 									измерения вектора скорости от GNSS модуля
 * 									в системе координат сопровождающего
 * 									трехгранника (NED)
 * 									
 * @return  None
 */
static void
VPE_CorrectVelosityEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ velByGNSS_NED_a[])
{
	VPE_CorrectVector(
		p_s->velosityInNED_s.vel_a,
		velByGNSS_NED_a,
		p_s->velosityInNED_s.compFilt_val,
		p_s->velosityInNED_s.vel_a);
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      25-дек-2018
 *
 * @brief    Функция выполняет коррекцию оценки вектора местоположения с помощью
 *           измерений вектора местоположения от GNSS модуля
 *
 * @param[in,out]	*p_s:	Указатель на структуру в которой содержатся
 * 							необходимые для обновления параметров вектора
 * 						 	скорости и вектора местоположения данные
 * @param[in]   posByGNSS_ECEF_a[3]:	Указатель на нулевой элемент массива
 * 										измерения вектора местоположения от
 * 										GNSS модуля в системе координат,
 * 										связанной с Землёй (ECEF)
 * 										
 * @return  None
 */
static void
VPE_CorrectPositionEstimate(
	vpe_all_data_s *p_s,
	__VPE_FPT__ posByGNSS_ECEF_a[])
{
	VPE_CorrectVector(
		p_s->positionInECEF_s.pos_a,
		posByGNSS_ECEF_a,
		p_s->positionInECEF_s.compFilt_val,
		p_s->positionInECEF_s.pos_a);
}

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief    Функция реализует комплементарный фильтр для 3-х мерного вектора
 *
 * @param[in]	dataEstimate_a[3]:    	Указатель на нулевой элемент массива
 * 										вектора оценки параметра
 * @param[in]   dataMeasurements_a[3]:  Указатель на нулевой элемент массива
 * 										вектора измерений параметра
 * @param[in]   filtCoeff:	Коэффициент комплементарного фильтра. Должен быть в промежутке от 0 до 1.
 * 				@attention 	Если коэффициент равен 0, то вектор оценки не учитывается.
 * 				            Если коэффициент равен 1, то вектор измерений не учитывается.
 * @param[out]  filtered_a[3]: 	Указатель на нулевой элемент массива в
 * 								который будет записано скорректированное значение вектора
 * 								
 * @return  None
 */
static void
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

/*-------------------------------------------------------------------------*//**
 * @author    Mickle Isaev
 * @date      27-дек-2018
 *
 * @brief    Функция выполняет проекцию вектора из СК препровождающего
 *           трехгранника (NED) в систему координат, связанную с Землей (ECEF)
 *
 * @param[in]	*p_s:	Указатель на структуру в которой содержатся
 * 							необходимые для обновления параметров вектора
 * 						 	скорости и вектора местоположения данные
 * @param[in]  	vect_NED_a[3]:	Указатель на нулевой элемент массива
 * 								в котором содержится вектор, проекцию
 * 								которого необходимо выполнить в ECEF СК
 * @param[out] 	vect_ECEF_a[3]:    	Указатель на нулевой элемент массива в
 * 									который будет записана проекция вектора
 * 									vect_NED_a в ECEF СК
 * @param[in]  	lat:	Текущая широта
 * @param[in]   lon:	Текущая долгота
 * 
 * @return  None
 */
static void
VPE_NED2ECEF(
	vpe_all_data_s *p_s,
	__VPE_FPT__ vect_NED_a[],
	__VPE_FPT__ vect_ECEF_a[],
	__VPE_FPT__ lat,
	__VPE_FPT__ lon)
{
	__VPE_FPT__ temp =
		p_s->trigFnc_s.pFncCos(lat) * (-vect_NED_a[VPE_NED_Z])
		- (p_s->trigFnc_s.pFncSin(lat) * vect_NED_a[VPE_NED_X]);

	vect_ECEF_a[VPE_ECEF_X] =
		(p_s->trigFnc_s.pFncCos(lon) * temp )
		- (p_s->trigFnc_s.pFncSin(lon) * vect_NED_a[VPE_NED_Y]);

	vect_ECEF_a[VPE_ECEF_Y] =
		(p_s->trigFnc_s.pFncSin(lon) * temp )
		- (p_s->trigFnc_s.pFncCos(lon) * vect_NED_a[VPE_NED_Y]);

	vect_ECEF_a[VPE_ECEF_Z] =
		p_s->trigFnc_s.pFncSin(lat) * (-vect_NED_a[VPE_NED_Z])
		+  p_s->trigFnc_s.pFncCos(lat) * vect_NED_a[VPE_NED_X];
}
/*#### |End  | <-- Секция - "Описание локальных функций" #####################*/


/*############################################################################*/
/*############################ END OF FILE  ##################################*/
/*############################################################################*/
