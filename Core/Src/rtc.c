/***********************************************************************************
 * @file        rtc.c                                                              *
 * @author      Lachie Keane                                                       *
 * @addtogroup  RTC                                                                *
 * @brief       Implements RTC integration.                                        *
 ***********************************************************************************/

#include "rtc.h"
#include "stdbool.h"

/**
 * @brief  Initialises the RTC and sets the time
 *
 * @param  ts Pointer struct containing the current time
 *
 * @return @c NULL
 **/
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

    // Delay 5 cycles to ensure peripheral is enabled
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");

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
    RTC->WPR = 0xFF;   // Can be any value other than the keys

    // Disable backup access
    PWR->CR &= ~PWR_CR_DBP;
}

/**
 * @brief  Sets the time
 *
 * @param  ts Pointer struct containing the current time
 *
 * @return @c NULL
 **/
void RTC_setTime(ts *ts) {

    uint8_t ht = ts->hours / 10;
    uint8_t hu = ts->hours % 10;
    uint8_t mnt = ts->mins / 10;
    uint8_t mnu = ts->mins % 10;
    uint8_t st = ts->secs / 10;
    uint8_t su = ts->secs % 10;

    uint32_t reg = 0;
    reg |= ht << RTC_TR_HT_Pos;
    reg |= hu << RTC_TR_HU_Pos;
    reg |= mnt << RTC_TR_MNT_Pos;
    reg |= mnu << RTC_TR_MNU_Pos;
    reg |= st << RTC_TR_ST_Pos;
    reg |= su << RTC_TR_SU_Pos;
    RTC->TR = reg;

    uint8_t yt = (ts->year - 2000) / 10;
    uint8_t yu = (ts->year - 2000) % 10;
    uint8_t dayOfWk = ts->dayOfWk;
    uint8_t mt = ts->month / 10;
    uint8_t mu = ts->month % 10;
    uint8_t dt = ts->date / 10;
    uint8_t du = ts->date % 10;

    reg = 0;
    reg |= yt << RTC_DR_YT_Pos;
    reg |= yu << RTC_DR_YU_Pos;
    reg |= dayOfWk << RTC_DR_WDU_Pos;
    reg |= mt << RTC_DR_MT_Pos;
    reg |= mu << RTC_DR_MU_Pos;
    reg |= dt << RTC_DR_DT_Pos;
    reg |= du << RTC_DR_DU_Pos;
    RTC->DR = reg;
}

/**
 * @brief  Gets the time
 *
 * @param  ts Pointer struct where the time will be stored
 *
 * @return @c NULL
 **/
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

    ts->year = 2000 + yt * 10 + yu;
    ts->dayOfWk = dayOfWk;
    ts->month = mt * 10 + mu;
    ts->date = dt * 10 + du;

    RTC_checkDst(ts);
}

/**
 * @brief  Checks if it is currently daylight savings or not and adjusts the time accordingly. However, if the hour is zero and daylight
 *         savings has ended, the RTC won't be able to subtract one from the hour. In this case, it will just wait until the next time 
 *         this function is called to try again.
 *
 * @param  ts Pointer struct containing the current time
 *
 * @return @c NULL
 **/
void RTC_checkDst(ts *ts) {
    // Daylight savings starts on the first Sunday of October at 2am and ends on the first Sunday of April at 3am
    bool isDst = (ts->month > 10)
        || (ts->month == 10 && (ts->dayOfWk == Sunday || ts->date > 7) && ts->hours >= 2)
        || (ts->month < 4)
        || (ts->month == 4 && (ts->dayOfWk < Sunday && ts->date < 7) && ts->hours < 3);

    ts->isDst = isDst;

    // Check if the RTC is already set to DST
    bool rtcSetToDst = (RTC->CR & RTC_CR_BKP) ? true : false;

    if (rtcSetToDst == ts->isDst) {
        //return;
    }

    // If DST is ending but we can't subtract an hour because the current hour is 0
    if (!ts->isDst && ts->hours == 0) {
        return;
    }

    // Enable write access to backup domain
    PWR->CR |= PWR_CR_DBP;

    // Unlock write protection
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    if (ts->isDst) {
        RTC->CR |= RTC_CR_ADD1H;
        RTC->CR |= RTC_CR_BKP;
    }
    else {
        RTC->CR |= RTC_CR_SUB1H;
        RTC->CR &= ~RTC_CR_BKP;
    }

    // Enable write protection
    RTC->WPR = 0xFF;   // Can be any value other than the keys

    // Disable backup access
    PWR->CR &= ~PWR_CR_DBP;

    // Get updated time
    RTC_getTime(ts);
}