/***********************************************************************************
 * @file        rtc.c                                                              *
 * @author      Lachie Keane                                                       *
 * @addtogroup  RTC                                                                *
 * @brief       Implements RTC integration.                                        *
 ***********************************************************************************/

#include "rtc.h"

void RTC_init(ts *ts) {

    /*  Enable RTC
    * 1. Enable access to PWR
    * 2. Enable write access to backup domain
    * 3. Enable LSE and wait for it to be ready
    * 4. Set RTC source to LSE
    * 5. Enable RTC
    */

    // Enable access to PWR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Enable write access to backup domain
    PWR->CR |= PWR_CR_DBP;

    // Enable LSE and wait for it to be ready
    RCC->BDCR |= RCC_BDCR_LSEON;
    while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0);

    // Set RTC source to LSE
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;
    RCC->BDCR &= ~RCC_BDCR_RTCSEL_1;

    // Enable RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;
    
    // Unlock write protection
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /*  Initialise
    * 1. Unlock write protection
    * 2. Enter initialisation mode and wait for the init flag to be set, signalling the RTC is ready
    * 3. Set sync prescaler then async prescaler (manual specifically says in this order)
    * 4. Load initial time and date values in the shadow registers and configure time mode (12h or 24h)
    * 5. Exit initialisation mode
    * 6. Wait for synchronisation
    * 
    * May not be necessary:
    * 7. Enable write protection
    * 8. Disable backup access
    */

    // Enter initialisation mode
    RTC->ISR |= RTC_ISR_INIT;
    while ((RTC->ISR & RTC_ISR_INITF) == 0);

    // Set sync prescaler then async prescaler (manual specifically says in this order)
    RTC->PRER |= 255 << RTC_PRER_PREDIV_S_Pos;
    RTC->PRER |= 127 << RTC_PRER_PREDIV_A_Pos;

    // Load initial time and date values in the shadow registers and configure time mode (12h or 24h)
    RTC->CR &= ~RTC_CR_FMT;     // Set to 24h format (0 is the reset value anyway, but doing this just in case)
    RTC_setTime(ts);

    // Exit initialisation mode
    RTC->ISR &= ~RTC_ISR_INIT;

    // Wait for synchronisation
    while((RTC->ISR & RTC_ISR_INITF) != 0);

    // Enable write protection
    RTC->WPR = 1;   // Can be any value other than the keys

    // Disable backup access
    PWR->CR &= ~PWR_CR_DBP;
}

void RTC_setTime(ts *ts) {

    /*Enable Backup access to config RTC*/
	// PWR->CR |= PWR_CR_DBP;

	// /*Disable RTC registers write protection*/
	// RTC->WPR = 0xCA;
	// RTC->WPR = 0x53;


	// /*Start init mode*/
	// RTC->ISR |= RTC_ISR_INIT;

	// /*Wait until Initializing mode is active*/
	// while((RTC->ISR & RTC_ISR_INITF) == 0);

    uint8_t ht = ts->hours / 10;
    uint8_t hu = ts->hours % 10;
    uint8_t mnt = ts->mins / 10;
    uint8_t mnu = ts->mins % 10;
    uint8_t st = ts->secs / 10;
    uint8_t su = ts->secs % 10;

    RTC->TR = 0;
    RTC->TR |= ht << RTC_TR_HT_Pos;
    RTC->TR |= hu << RTC_TR_HU_Pos;
    RTC->TR |= mnt << RTC_TR_MNT_Pos;
    RTC->TR |= mnu << RTC_TR_MNU_Pos;
    RTC->TR |= st << RTC_TR_ST_Pos;
    RTC->TR |= su << RTC_TR_SU_Pos;

    uint8_t yt = (ts->year - 2000) / 10;
    uint8_t yu = (ts->year - 2000) % 10;
    uint8_t dayOfWk = ts->dayOfWk;
    uint8_t mt = ts->month / 10;
    uint8_t mu = ts->month % 10;
    uint8_t dt = ts->date / 10;
    uint8_t du = ts->date % 10;

    RTC->DR |= yt << RTC_DR_YT_Pos;
    RTC->DR |= yu << RTC_DR_YU_Pos;
    RTC->DR |= dayOfWk << RTC_DR_WDU_Pos;
    RTC->DR |= mt << RTC_DR_MT_Pos;
    RTC->DR |= mu << RTC_DR_MU_Pos;
    RTC->DR |= dt << RTC_DR_DT_Pos;
    RTC->DR |= du << RTC_DR_DU_Pos;

    /*Exit the initialization mode*/
	// RTC->ISR&=~RTC_ISR_INIT;

	// /*Wait for synchronisation*/
	// while((RTC->ISR & RTC_ISR_INITF) != 0);

	// /*Enable RTC registers write protection*/
	// RTC->WPR = 0xFF;
}

void RTC_getTime(ts *ts) {
    while ((RTC->ISR & RTC_ISR_RSF) == 0);

    uint8_t ht = (RTC->TR & RTC_TR_HT) >> RTC_TR_HT_Pos;
    uint8_t hu = (RTC->TR & RTC_TR_HU) >> RTC_TR_HU_Pos;
    uint8_t mnt = (RTC->TR & RTC_TR_MNT) >> RTC_TR_MNT_Pos;
    uint8_t mnu = (RTC->TR & RTC_TR_MNU) >> RTC_TR_MNU_Pos;
    uint8_t st = (RTC->TR & RTC_TR_ST) >> RTC_TR_ST_Pos;
    uint8_t su = (RTC->TR & RTC_TR_SU) >> RTC_TR_SU_Pos;

    ts->hours = ht * 10 + hu;
    ts->mins = mnt * 10 + mnu;
    ts->secs = st * 10 + su;

    uint8_t yt = (RTC->DR & RTC_DR_YT) >> RTC_DR_YT_Pos;
    uint8_t yu = (RTC->DR & RTC_DR_YU) >> RTC_DR_YU_Pos;
    uint8_t dayOfWk = (RTC->DR & RTC_DR_WDU) >> RTC_DR_WDU_Pos;
    uint8_t mt = (RTC->DR & RTC_DR_MT) >> RTC_DR_MT_Pos;
    uint8_t mu = (RTC->DR & RTC_DR_MU) >> RTC_DR_MU_Pos;
    uint8_t dt = (RTC->DR & RTC_DR_DT) >> RTC_DR_DT_Pos;
    uint8_t du = (RTC->DR & RTC_DR_DU) >> RTC_DR_DU_Pos;

    ts->year = yt * 10 + yu;
    ts->dayOfWk = dayOfWk;
    ts->month = mt * 10 + mu;
    ts->date = dt * 10 + du;
}