/*-*****************************************************************************

ArmmiteF4 MMBasic

Audio.c

Handles the PLAY command.

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#include <stdio.h>
#include <stdbool.h>                                // Pascal
#include <stdint.h>                                 // Pascal

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
void *ReAllocMemory(void *addr, size_t msize){
    void *newaddr=GetMemory(msize);
    if(addr!=NULL){
        memcpy(newaddr,addr,MemSize(addr));
        FreeMemory(addr);
    }
	return newaddr;
}
#define DR_WAV_IMPLEMENTATION
#define DRWAV_MALLOC(sz)                   GetMemory((sz))
#define DRWAV_FREE(p)                      FreeMemory((p))
#define DRWAV_REALLOC(p, sz)               ReAllocMemory((p), (sz))
#include "dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#define DRFLAC_MALLOC(sz)                   GetMemory((sz))
#define DRFLAC_FREE(p)                      FreeMemory((p))
#define DRFLAC_REALLOC(p, sz)               ReAllocMemory((p), (sz))
#include "DRFLAC.h"
#define hdac1 hdac
extern 	volatile unsigned int SDtimer;
char *buffer = NULL;
extern int InitSDCard(void);
extern const int ErrorMap[21];
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern DAC_HandleTypeDef hdac1;

/********************************************************************************************************************************************
commands and functions
 each function is responsible for decoding a command
 all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a basic function) so, if you want to search for the
 function responsible for the NAME command look for cmd_name

 There are 4 items of information that are setup before the command is run.
 All these are globals.

 int cmdtoken	This is the token number of the command (some commands can handle multiple
				statement types and this helps them differentiate)

 char *cmdline	This is the command line terminated with a zero char and trimmed of leading
				spaces.  It may exist anywhere in memory (or even ROM).

 char *nextstmt	This is a pointer to the next statement to be executed.  The only thing a
				command can do with it is save it or change it to some other location.

 char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

 The only actions a command can do to change the program flow is to change nextstmt or
 execute longjmp(mark, 1) if it wants to abort the program.

 ********************************************************************************************************************************************/


// define the PWM output frequency for making a tone
volatile unsigned char PWM_count = 0;
volatile unsigned int PhaseM_left, PhaseM_right;

volatile e_CurrentlyPlaying CurrentlyPlaying = P_NOTHING;
volatile int v_left, v_right, vol_left = 100, vol_right = 100;
char *wav_buf;                                                      // pointer to the buffer for received wav data
volatile int wav_filesize;                                                   // head and tail of the ring buffer for com1
volatile int tickspersample;
char *WAVInterrupt = NULL;
int WAVcomplete;
int WAV_fnbr;
int PWM_FREQ=80000;
volatile int swingbuf = 0,nextbuf = 0, playreadcomplete = 0;
char *sbuff1, *sbuff2;
uint16_t *ibuff1, *ibuff2;
#ifndef STM32F4Version
	volatile int swingbufe = 0,nextbufe = 0, playreadcompletee = 0;
	char *sbuff1e, *sbuff2e;
	uint16_t *ibuff1e, *ibuff2e;
	volatile int ppose = 0;                                                       // playing position for PLAY WAV
	volatile unsigned int bcounte[3] = {0, 0, 0};
#endif
char *pbuffp;
union map1
{
    unsigned short channels[2];
    unsigned int value;
} flacvalue;
uint16_t *flacbuff;
volatile int ppos = 0;                                                       // playing position for PLAY WAV
int sinemin, sinemax, sineavg, nchannels;
volatile unsigned int bcount[3] = {0, 0, 0};
volatile int last_left, last_right, current_left, current_right;


drwav* mywav=NULL;
drflac* myflac;
volatile unsigned int PhaseAC_left, PhaseAC_right;
#define PSpeedDiv SystemCoreClock

const unsigned short SineTable[4096] = {
		2000,2003,2006,2009,2012,2015,2017,2020,2023,2026,2029,2032,2035,2038,2041,2044,2047,2050,2052,2055,2058,2061,2064,2067,2070,2073,2076,2079,2082,2084,2087,2090,
		2093,2096,2099,2102,2105,2108,2111,2114,2117,2119,2122,2125,2128,2131,2134,2137,2140,2143,2146,2148,2151,2154,2157,2160,2163,2166,2169,2172,2175,2178,2180,2183,
		2186,2189,2192,2195,2198,2201,2204,2207,2209,2212,2215,2218,2221,2224,2227,2230,2233,2235,2238,2241,2244,2247,2250,2253,2256,2259,2261,2264,2267,2270,2273,2276,
		2279,2282,2285,2287,2290,2293,2296,2299,2302,2305,2308,2310,2313,2316,2319,2322,2325,2328,2331,2333,2336,2339,2342,2345,2348,2351,2354,2356,2359,2362,2365,2368,
		2371,2374,2376,2379,2382,2385,2388,2391,2394,2396,2399,2402,2405,2408,2411,2413,2416,2419,2422,2425,2428,2430,2433,2436,2439,2442,2445,2448,2450,2453,2456,2459,
		2462,2464,2467,2470,2473,2476,2479,2481,2484,2487,2490,2493,2496,2498,2501,2504,2507,2510,2512,2515,2518,2521,2524,2526,2529,2532,2535,2538,2540,2543,2546,2549,
		2552,2554,2557,2560,2563,2565,2568,2571,2574,2577,2579,2582,2585,2588,2590,2593,2596,2599,2602,2604,2607,2610,2613,2615,2618,2621,2624,2626,2629,2632,2635,2637,
		2640,2643,2646,2648,2651,2654,2657,2659,2662,2665,2667,2670,2673,2676,2678,2681,2684,2687,2689,2692,2695,2697,2700,2703,2706,2708,2711,2714,2716,2719,2722,2724,
		2727,2730,2732,2735,2738,2741,2743,2746,2749,2751,2754,2757,2759,2762,2765,2767,2770,2773,2775,2778,2781,2783,2786,2789,2791,2794,2797,2799,2802,2804,2807,2810,
		2812,2815,2818,2820,2823,2826,2828,2831,2833,2836,2839,2841,2844,2846,2849,2852,2854,2857,2859,2862,2865,2867,2870,2872,2875,2878,2880,2883,2885,2888,2891,2893,
		2896,2898,2901,2903,2906,2908,2911,2914,2916,2919,2921,2924,2926,2929,2931,2934,2937,2939,2942,2944,2947,2949,2952,2954,2957,2959,2962,2964,2967,2969,2972,2974,
		2977,2979,2982,2984,2987,2989,2992,2994,2997,2999,3002,3004,3007,3009,3012,3014,3016,3019,3021,3024,3026,3029,3031,3034,3036,3039,3041,3043,3046,3048,3051,3053,
		3056,3058,3060,3063,3065,3068,3070,3072,3075,3077,3080,3082,3084,3087,3089,3092,3094,3096,3099,3101,3104,3106,3108,3111,3113,3115,3118,3120,3122,3125,3127,3129,
		3132,3134,3137,3139,3141,3144,3146,3148,3150,3153,3155,3157,3160,3162,3164,3167,3169,3171,3174,3176,3178,3180,3183,3185,3187,3190,3192,3194,3196,3199,3201,3203,
		3205,3208,3210,3212,3214,3217,3219,3221,3223,3226,3228,3230,3232,3234,3237,3239,3241,3243,3245,3248,3250,3252,3254,3256,3259,3261,3263,3265,3267,3269,3272,3274,
		3276,3278,3280,3282,3285,3287,3289,3291,3293,3295,3297,3300,3302,3304,3306,3308,3310,3312,3314,3316,3319,3321,3323,3325,3327,3329,3331,3333,3335,3337,3339,3341,
		3344,3346,3348,3350,3352,3354,3356,3358,3360,3362,3364,3366,3368,3370,3372,3374,3376,3378,3380,3382,3384,3386,3388,3390,3392,3394,3396,3398,3400,3402,3404,3406,
		3408,3410,3412,3414,3416,3418,3419,3421,3423,3425,3427,3429,3431,3433,3435,3437,3439,3441,3442,3444,3446,3448,3450,3452,3454,3456,3458,3459,3461,3463,3465,3467,
		3469,3471,3472,3474,3476,3478,3480,3482,3483,3485,3487,3489,3491,3492,3494,3496,3498,3500,3501,3503,3505,3507,3509,3510,3512,3514,3516,3517,3519,3521,3523,3524,
		3526,3528,3530,3531,3533,3535,3536,3538,3540,3542,3543,3545,3547,3548,3550,3552,3553,3555,3557,3558,3560,3562,3563,3565,3567,3568,3570,3572,3573,3575,3577,3578,
		3580,3581,3583,3585,3586,3588,3589,3591,3593,3594,3596,3597,3599,3601,3602,3604,3605,3607,3608,3610,3611,3613,3615,3616,3618,3619,3621,3622,3624,3625,3627,3628,
		3630,3631,3633,3634,3636,3637,3639,3640,3642,3643,3644,3646,3647,3649,3650,3652,3653,3655,3656,3657,3659,3660,3662,3663,3665,3666,3667,3669,3670,3672,3673,3674,
		3676,3677,3678,3680,3681,3682,3684,3685,3687,3688,3689,3691,3692,3693,3694,3696,3697,3698,3700,3701,3702,3704,3705,3706,3707,3709,3710,3711,3713,3714,3715,3716,
		3718,3719,3720,3721,3723,3724,3725,3726,3727,3729,3730,3731,3732,3733,3735,3736,3737,3738,3739,3741,3742,3743,3744,3745,3746,3747,3749,3750,3751,3752,3753,3754,
		3755,3756,3758,3759,3760,3761,3762,3763,3764,3765,3766,3767,3768,3770,3771,3772,3773,3774,3775,3776,3777,3778,3779,3780,3781,3782,3783,3784,3785,3786,3787,3788,
		3789,3790,3791,3792,3793,3794,3795,3796,3797,3798,3799,3799,3800,3801,3802,3803,3804,3805,3806,3807,3808,3809,3810,3810,3811,3812,3813,3814,3815,3816,3816,3817,
		3818,3819,3820,3821,3822,3822,3823,3824,3825,3826,3826,3827,3828,3829,3830,3830,3831,3832,3833,3833,3834,3835,3836,3837,3837,3838,3839,3839,3840,3841,3842,3842,
		3843,3844,3844,3845,3846,3847,3847,3848,3849,3849,3850,3851,3851,3852,3853,3853,3854,3854,3855,3856,3856,3857,3858,3858,3859,3859,3860,3861,3861,3862,3862,3863,
		3863,3864,3865,3865,3866,3866,3867,3867,3868,3868,3869,3869,3870,3871,3871,3872,3872,3873,3873,3874,3874,3874,3875,3875,3876,3876,3877,3877,3878,3878,3879,3879,
		3879,3880,3880,3881,3881,3882,3882,3882,3883,3883,3883,3884,3884,3885,3885,3885,3886,3886,3886,3887,3887,3887,3888,3888,3888,3889,3889,3889,3890,3890,3890,3891,
		3891,3891,3891,3892,3892,3892,3892,3893,3893,3893,3893,3894,3894,3894,3894,3895,3895,3895,3895,3895,3896,3896,3896,3896,3896,3897,3897,3897,3897,3897,3897,3898,
		3898,3898,3898,3898,3898,3898,3898,3899,3899,3899,3899,3899,3899,3899,3899,3899,3899,3899,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,
		3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3900,3899,3899,3899,3899,3899,3899,3899,3899,3899,3899,3899,3898,3898,3898,3898,3898,3898,
		3898,3898,3897,3897,3897,3897,3897,3897,3896,3896,3896,3896,3896,3895,3895,3895,3895,3895,3894,3894,3894,3894,3893,3893,3893,3893,3892,3892,3892,3892,3891,3891,
		3891,3891,3890,3890,3890,3889,3889,3889,3888,3888,3888,3887,3887,3887,3886,3886,3886,3885,3885,3885,3884,3884,3883,3883,3883,3882,3882,3882,3881,3881,3880,3880,
		3879,3879,3879,3878,3878,3877,3877,3876,3876,3875,3875,3874,3874,3874,3873,3873,3872,3872,3871,3871,3870,3869,3869,3868,3868,3867,3867,3866,3866,3865,3865,3864,
		3863,3863,3862,3862,3861,3861,3860,3859,3859,3858,3858,3857,3856,3856,3855,3854,3854,3853,3853,3852,3851,3851,3850,3849,3849,3848,3847,3847,3846,3845,3844,3844,
		3843,3842,3842,3841,3840,3839,3839,3838,3837,3837,3836,3835,3834,3833,3833,3832,3831,3830,3830,3829,3828,3827,3826,3826,3825,3824,3823,3822,3822,3821,3820,3819,
		3818,3817,3816,3816,3815,3814,3813,3812,3811,3810,3810,3809,3808,3807,3806,3805,3804,3803,3802,3801,3800,3799,3799,3798,3797,3796,3795,3794,3793,3792,3791,3790,
		3789,3788,3787,3786,3785,3784,3783,3782,3781,3780,3779,3778,3777,3776,3775,3774,3773,3772,3771,3770,3768,3767,3766,3765,3764,3763,3762,3761,3760,3759,3758,3756,
		3755,3754,3753,3752,3751,3750,3749,3747,3746,3745,3744,3743,3742,3741,3739,3738,3737,3736,3735,3733,3732,3731,3730,3729,3727,3726,3725,3724,3723,3721,3720,3719,
		3718,3716,3715,3714,3713,3711,3710,3709,3707,3706,3705,3704,3702,3701,3700,3698,3697,3696,3694,3693,3692,3691,3689,3688,3687,3685,3684,3682,3681,3680,3678,3677,
		3676,3674,3673,3672,3670,3669,3667,3666,3665,3663,3662,3660,3659,3657,3656,3655,3653,3652,3650,3649,3647,3646,3644,3643,3642,3640,3639,3637,3636,3634,3633,3631,
		3630,3628,3627,3625,3624,3622,3621,3619,3618,3616,3615,3613,3611,3610,3608,3607,3605,3604,3602,3601,3599,3597,3596,3594,3593,3591,3589,3588,3586,3585,3583,3581,
		3580,3578,3577,3575,3573,3572,3570,3568,3567,3565,3563,3562,3560,3558,3557,3555,3553,3552,3550,3548,3547,3545,3543,3542,3540,3538,3536,3535,3533,3531,3530,3528,
		3526,3524,3523,3521,3519,3517,3516,3514,3512,3510,3509,3507,3505,3503,3501,3500,3498,3496,3494,3492,3491,3489,3487,3485,3483,3482,3480,3478,3476,3474,3472,3471,
		3469,3467,3465,3463,3461,3459,3458,3456,3454,3452,3450,3448,3446,3444,3442,3441,3439,3437,3435,3433,3431,3429,3427,3425,3423,3421,3419,3418,3416,3414,3412,3410,
		3408,3406,3404,3402,3400,3398,3396,3394,3392,3390,3388,3386,3384,3382,3380,3378,3376,3374,3372,3370,3368,3366,3364,3362,3360,3358,3356,3354,3352,3350,3348,3346,
		3344,3341,3339,3337,3335,3333,3331,3329,3327,3325,3323,3321,3319,3316,3314,3312,3310,3308,3306,3304,3302,3300,3297,3295,3293,3291,3289,3287,3285,3282,3280,3278,
		3276,3274,3272,3269,3267,3265,3263,3261,3259,3256,3254,3252,3250,3248,3245,3243,3241,3239,3237,3234,3232,3230,3228,3226,3223,3221,3219,3217,3214,3212,3210,3208,
		3205,3203,3201,3199,3196,3194,3192,3190,3187,3185,3183,3180,3178,3176,3174,3171,3169,3167,3164,3162,3160,3157,3155,3153,3150,3148,3146,3144,3141,3139,3137,3134,
		3132,3129,3127,3125,3122,3120,3118,3115,3113,3111,3108,3106,3104,3101,3099,3096,3094,3092,3089,3087,3084,3082,3080,3077,3075,3072,3070,3068,3065,3063,3060,3058,
		3056,3053,3051,3048,3046,3043,3041,3039,3036,3034,3031,3029,3026,3024,3021,3019,3016,3014,3012,3009,3007,3004,3002,2999,2997,2994,2992,2989,2987,2984,2982,2979,
		2977,2974,2972,2969,2967,2964,2962,2959,2957,2954,2952,2949,2947,2944,2942,2939,2937,2934,2931,2929,2926,2924,2921,2919,2916,2914,2911,2908,2906,2903,2901,2898,
		2896,2893,2891,2888,2885,2883,2880,2878,2875,2872,2870,2867,2865,2862,2859,2857,2854,2852,2849,2846,2844,2841,2839,2836,2833,2831,2828,2826,2823,2820,2818,2815,
		2812,2810,2807,2804,2802,2799,2797,2794,2791,2789,2786,2783,2781,2778,2775,2773,2770,2767,2765,2762,2759,2757,2754,2751,2749,2746,2743,2741,2738,2735,2732,2730,
		2727,2724,2722,2719,2716,2714,2711,2708,2706,2703,2700,2697,2695,2692,2689,2687,2684,2681,2678,2676,2673,2670,2667,2665,2662,2659,2657,2654,2651,2648,2646,2643,
		2640,2637,2635,2632,2629,2626,2624,2621,2618,2615,2613,2610,2607,2604,2602,2599,2596,2593,2590,2588,2585,2582,2579,2577,2574,2571,2568,2565,2563,2560,2557,2554,
		2552,2549,2546,2543,2540,2538,2535,2532,2529,2526,2524,2521,2518,2515,2512,2510,2507,2504,2501,2498,2496,2493,2490,2487,2484,2481,2479,2476,2473,2470,2467,2464,
		2462,2459,2456,2453,2450,2448,2445,2442,2439,2436,2433,2430,2428,2425,2422,2419,2416,2413,2411,2408,2405,2402,2399,2396,2394,2391,2388,2385,2382,2379,2376,2374,
		2371,2368,2365,2362,2359,2356,2354,2351,2348,2345,2342,2339,2336,2333,2331,2328,2325,2322,2319,2316,2313,2310,2308,2305,2302,2299,2296,2293,2290,2287,2285,2282,
		2279,2276,2273,2270,2267,2264,2261,2259,2256,2253,2250,2247,2244,2241,2238,2235,2233,2230,2227,2224,2221,2218,2215,2212,2209,2207,2204,2201,2198,2195,2192,2189,
		2186,2183,2180,2178,2175,2172,2169,2166,2163,2160,2157,2154,2151,2148,2146,2143,2140,2137,2134,2131,2128,2125,2122,2119,2117,2114,2111,2108,2105,2102,2099,2096,
		2093,2090,2087,2084,2082,2079,2076,2073,2070,2067,2064,2061,2058,2055,2052,2050,2047,2044,2041,2038,2035,2032,2029,2026,2023,2020,2017,2015,2012,2009,2006,2003,
		2000,1997,1994,1991,1988,1985,1983,1980,1977,1974,1971,1968,1965,1962,1959,1956,1953,1950,1948,1945,1942,1939,1936,1933,1930,1927,1924,1921,1918,1916,1913,1910,
		1907,1904,1901,1898,1895,1892,1889,1886,1883,1881,1878,1875,1872,1869,1866,1863,1860,1857,1854,1852,1849,1846,1843,1840,1837,1834,1831,1828,1825,1822,1820,1817,
		1814,1811,1808,1805,1802,1799,1796,1793,1791,1788,1785,1782,1779,1776,1773,1770,1767,1765,1762,1759,1756,1753,1750,1747,1744,1741,1739,1736,1733,1730,1727,1724,
		1721,1718,1715,1713,1710,1707,1704,1701,1698,1695,1692,1690,1687,1684,1681,1678,1675,1672,1669,1667,1664,1661,1658,1655,1652,1649,1646,1644,1641,1638,1635,1632,
		1629,1626,1624,1621,1618,1615,1612,1609,1606,1604,1601,1598,1595,1592,1589,1587,1584,1581,1578,1575,1572,1570,1567,1564,1561,1558,1555,1552,1550,1547,1544,1541,
		1538,1536,1533,1530,1527,1524,1521,1519,1516,1513,1510,1507,1504,1502,1499,1496,1493,1490,1488,1485,1482,1479,1476,1474,1471,1468,1465,1462,1460,1457,1454,1451,
		1448,1446,1443,1440,1437,1435,1432,1429,1426,1423,1421,1418,1415,1412,1410,1407,1404,1401,1398,1396,1393,1390,1387,1385,1382,1379,1376,1374,1371,1368,1365,1363,
		1360,1357,1354,1352,1349,1346,1343,1341,1338,1335,1333,1330,1327,1324,1322,1319,1316,1313,1311,1308,1305,1303,1300,1297,1294,1292,1289,1286,1284,1281,1278,1276,
		1273,1270,1268,1265,1262,1259,1257,1254,1251,1249,1246,1243,1241,1238,1235,1233,1230,1227,1225,1222,1219,1217,1214,1211,1209,1206,1203,1201,1198,1196,1193,1190,
		1188,1185,1182,1180,1177,1174,1172,1169,1167,1164,1161,1159,1156,1154,1151,1148,1146,1143,1141,1138,1135,1133,1130,1128,1125,1122,1120,1117,1115,1112,1109,1107,
		1104,1102,1099,1097,1094,1092,1089,1086,1084,1081,1079,1076,1074,1071,1069,1066,1063,1061,1058,1056,1053,1051,1048,1046,1043,1041,1038,1036,1033,1031,1028,1026,
		1023,1021,1018,1016,1013,1011,1008,1006,1003,1001,998,996,993,991,988,986,984,981,979,976,974,971,969,966,964,961,959,957,954,952,949,947,
		944,942,940,937,935,932,930,928,925,923,920,918,916,913,911,908,906,904,901,899,896,894,892,889,887,885,882,880,878,875,873,871,
		868,866,863,861,859,856,854,852,850,847,845,843,840,838,836,833,831,829,826,824,822,820,817,815,813,810,808,806,804,801,799,797,
		795,792,790,788,786,783,781,779,777,774,772,770,768,766,763,761,759,757,755,752,750,748,746,744,741,739,737,735,733,731,728,726,
		724,722,720,718,715,713,711,709,707,705,703,700,698,696,694,692,690,688,686,684,681,679,677,675,673,671,669,667,665,663,661,659,
		656,654,652,650,648,646,644,642,640,638,636,634,632,630,628,626,624,622,620,618,616,614,612,610,608,606,604,602,600,598,596,594,
		592,590,588,586,584,582,581,579,577,575,573,571,569,567,565,563,561,559,558,556,554,552,550,548,546,544,542,541,539,537,535,533,
		531,529,528,526,524,522,520,518,517,515,513,511,509,508,506,504,502,500,499,497,495,493,491,490,488,486,484,483,481,479,477,476,
		474,472,470,469,467,465,464,462,460,458,457,455,453,452,450,448,447,445,443,442,440,438,437,435,433,432,430,428,427,425,423,422,
		420,419,417,415,414,412,411,409,407,406,404,403,401,399,398,396,395,393,392,390,389,387,385,384,382,381,379,378,376,375,373,372,
		370,369,367,366,364,363,361,360,358,357,356,354,353,351,350,348,347,345,344,343,341,340,338,337,335,334,333,331,330,328,327,326,
		324,323,322,320,319,318,316,315,313,312,311,309,308,307,306,304,303,302,300,299,298,296,295,294,293,291,290,289,287,286,285,284,
		282,281,280,279,277,276,275,274,273,271,270,269,268,267,265,264,263,262,261,259,258,257,256,255,254,253,251,250,249,248,247,246,
		245,244,242,241,240,239,238,237,236,235,234,233,232,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,
		211,210,209,208,207,206,205,204,203,202,201,201,200,199,198,197,196,195,194,193,192,191,190,190,189,188,187,186,185,184,184,183,
		182,181,180,179,178,178,177,176,175,174,174,173,172,171,170,170,169,168,167,167,166,165,164,163,163,162,161,161,160,159,158,158,
		157,156,156,155,154,153,153,152,151,151,150,149,149,148,147,147,146,146,145,144,144,143,142,142,141,141,140,139,139,138,138,137,
		137,136,135,135,134,134,133,133,132,132,131,131,130,129,129,128,128,127,127,126,126,126,125,125,124,124,123,123,122,122,121,121,
		121,120,120,119,119,118,118,118,117,117,117,116,116,115,115,115,114,114,114,113,113,113,112,112,112,111,111,111,110,110,110,109,
		109,109,109,108,108,108,108,107,107,107,107,106,106,106,106,105,105,105,105,105,104,104,104,104,104,103,103,103,103,103,103,102,
		102,102,102,102,102,102,102,101,101,101,101,101,101,101,101,101,101,101,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
		100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,101,101,101,101,101,101,101,101,101,101,101,102,102,102,102,102,102,
		102,102,103,103,103,103,103,103,104,104,104,104,104,105,105,105,105,105,106,106,106,106,107,107,107,107,108,108,108,108,109,109,
		109,109,110,110,110,111,111,111,112,112,112,113,113,113,114,114,114,115,115,115,116,116,117,117,117,118,118,118,119,119,120,120,
		121,121,121,122,122,123,123,124,124,125,125,126,126,126,127,127,128,128,129,129,130,131,131,132,132,133,133,134,134,135,135,136,
		137,137,138,138,139,139,140,141,141,142,142,143,144,144,145,146,146,147,147,148,149,149,150,151,151,152,153,153,154,155,156,156,
		157,158,158,159,160,161,161,162,163,163,164,165,166,167,167,168,169,170,170,171,172,173,174,174,175,176,177,178,178,179,180,181,
		182,183,184,184,185,186,187,188,189,190,190,191,192,193,194,195,196,197,198,199,200,201,201,202,203,204,205,206,207,208,209,210,
		211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,232,233,234,235,236,237,238,239,240,241,242,244,
		245,246,247,248,249,250,251,253,254,255,256,257,258,259,261,262,263,264,265,267,268,269,270,271,273,274,275,276,277,279,280,281,
		282,284,285,286,287,289,290,291,293,294,295,296,298,299,300,302,303,304,306,307,308,309,311,312,313,315,316,318,319,320,322,323,
		324,326,327,328,330,331,333,334,335,337,338,340,341,343,344,345,347,348,350,351,353,354,356,357,358,360,361,363,364,366,367,369,
		370,372,373,375,376,378,379,381,382,384,385,387,389,390,392,393,395,396,398,399,401,403,404,406,407,409,411,412,414,415,417,419,
		420,422,423,425,427,428,430,432,433,435,437,438,440,442,443,445,447,448,450,452,453,455,457,458,460,462,464,465,467,469,470,472,
		474,476,477,479,481,483,484,486,488,490,491,493,495,497,499,500,502,504,506,508,509,511,513,515,517,518,520,522,524,526,528,529,
		531,533,535,537,539,541,542,544,546,548,550,552,554,556,558,559,561,563,565,567,569,571,573,575,577,579,581,582,584,586,588,590,
		592,594,596,598,600,602,604,606,608,610,612,614,616,618,620,622,624,626,628,630,632,634,636,638,640,642,644,646,648,650,652,654,
		656,659,661,663,665,667,669,671,673,675,677,679,681,684,686,688,690,692,694,696,698,700,703,705,707,709,711,713,715,718,720,722,
		724,726,728,731,733,735,737,739,741,744,746,748,750,752,755,757,759,761,763,766,768,770,772,774,777,779,781,783,786,788,790,792,
		795,797,799,801,804,806,808,810,813,815,817,820,822,824,826,829,831,833,836,838,840,843,845,847,850,852,854,856,859,861,863,866,
		868,871,873,875,878,880,882,885,887,889,892,894,896,899,901,904,906,908,911,913,916,918,920,923,925,928,930,932,935,937,940,942,
		944,947,949,952,954,957,959,961,964,966,969,971,974,976,979,981,984,986,988,991,993,996,998,1001,1003,1006,1008,1011,1013,1016,1018,1021,
		1023,1026,1028,1031,1033,1036,1038,1041,1043,1046,1048,1051,1053,1056,1058,1061,1063,1066,1069,1071,1074,1076,1079,1081,1084,1086,1089,1092,1094,1097,1099,1102,
		1104,1107,1109,1112,1115,1117,1120,1122,1125,1128,1130,1133,1135,1138,1141,1143,1146,1148,1151,1154,1156,1159,1161,1164,1167,1169,1172,1174,1177,1180,1182,1185,
		1188,1190,1193,1196,1198,1201,1203,1206,1209,1211,1214,1217,1219,1222,1225,1227,1230,1233,1235,1238,1241,1243,1246,1249,1251,1254,1257,1259,1262,1265,1268,1270,
		1273,1276,1278,1281,1284,1286,1289,1292,1294,1297,1300,1303,1305,1308,1311,1313,1316,1319,1322,1324,1327,1330,1333,1335,1338,1341,1343,1346,1349,1352,1354,1357,
		1360,1363,1365,1368,1371,1374,1376,1379,1382,1385,1387,1390,1393,1396,1398,1401,1404,1407,1410,1412,1415,1418,1421,1423,1426,1429,1432,1435,1437,1440,1443,1446,
		1448,1451,1454,1457,1460,1462,1465,1468,1471,1474,1476,1479,1482,1485,1488,1490,1493,1496,1499,1502,1504,1507,1510,1513,1516,1519,1521,1524,1527,1530,1533,1536,
		1538,1541,1544,1547,1550,1552,1555,1558,1561,1564,1567,1570,1572,1575,1578,1581,1584,1587,1589,1592,1595,1598,1601,1604,1606,1609,1612,1615,1618,1621,1624,1626,
		1629,1632,1635,1638,1641,1644,1646,1649,1652,1655,1658,1661,1664,1667,1669,1672,1675,1678,1681,1684,1687,1690,1692,1695,1698,1701,1704,1707,1710,1713,1715,1718,
		1721,1724,1727,1730,1733,1736,1739,1741,1744,1747,1750,1753,1756,1759,1762,1765,1767,1770,1773,1776,1779,1782,1785,1788,1791,1793,1796,1799,1802,1805,1808,1811,
		1814,1817,1820,1822,1825,1828,1831,1834,1837,1840,1843,1846,1849,1852,1854,1857,1860,1863,1866,1869,1872,1875,1878,1881,1883,1886,1889,1892,1895,1898,1901,1904,
		1907,1910,1913,1916,1918,1921,1924,1927,1930,1933,1936,1939,1942,1945,1948,1950,1953,1956,1959,1962,1965,1968,1971,1974,1977,1980,1983,1985,1988,1991,1994,1997
};
size_t onRead(void  *userdata,  char  *pBufferOut,   size_t bytesToRead){
    unsigned int nbr;
    FSerror=f_read(FileTable[WAV_fnbr].fptr,pBufferOut, bytesToRead, &nbr);
    if(FSerror)nbr=0;
    ErrorCheck(WAV_fnbr);
    SDtimer=1000;
    return nbr;
}
drwav_bool32 onSeek(void  *userdata,  int offset,  drwav_seek_origin origin){
    if(origin==drwav_seek_origin_start) FSerror=f_lseek(FileTable[WAV_fnbr].fptr,offset);
    else FSerror=f_lseek(FileTable[WAV_fnbr].fptr,FileTable[WAV_fnbr].fptr->fptr+offset);
    SDtimer=1000;
    return 1;
}
void ConfigSoundOutputs(void){
    sinemin=SineTable[3071];
    sinemax=SineTable[1023];
    sineavg=SineTable[0];
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
    if(HAL_TIM_Base_Start_IT(&htim5)!=HAL_OK)error("Starting audio timer");
}
void CloseAudio(void){
    if(CurrentlyPlaying == P_TONE || CurrentlyPlaying == P_PAUSE_TONE) StopAudio();
        else if(CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_WAV || CurrentlyPlaying == P_MP3  || CurrentlyPlaying == P_MOD || CurrentlyPlaying == P_PAUSE_WAV ) {
        bcount[1] = bcount[2] = wav_filesize = 0;
        StopAudio();
        ForceFileClose(WAV_fnbr);
        FreeMemory(sbuff1); 
        FreeMemory(sbuff2); 
        FreeMemory(buffer);
        WAVcomplete = true;
        FreeMemory(mywav);
        FSerror = 0;
    }
    return;
}

void iconvert(uint16_t *ibuff, int16_t *sbuff, int count){
	int i;
	for(i=0;i<(count);i+=2){
		ibuff[i]=(uint16_t)(((float)sbuff[i]*(float)vol_left/100.0+32768.0))>>4;
		ibuff[i+1]=(uint16_t)(((float)sbuff[i+1]*(float)vol_right/100.0+32768.0))>>4;
	}
}


// The MMBasic command:  PLAY
void cmd_play(void) {
    char *tp;
    if(checkstring(cmdline, "STOP")) {
        CloseAudio();
        return;
    }
    if(checkstring(cmdline, "PAUSE")) {
        if(CurrentlyPlaying == P_TONE)
            CurrentlyPlaying = P_PAUSE_TONE;
        else if(CurrentlyPlaying == P_FLAC) CurrentlyPlaying = P_PAUSE_FLAC;
        else if(CurrentlyPlaying == P_MOD) CurrentlyPlaying = P_PAUSE_MOD;
        else if(CurrentlyPlaying == P_WAV)  CurrentlyPlaying = P_PAUSE_WAV;
        else
            error("Nothing playing");
        return;
    }

    if(checkstring(cmdline, "RESUME")) {
        if(CurrentlyPlaying == P_PAUSE_TONE) CurrentlyPlaying = P_TONE;
        else if(CurrentlyPlaying == P_PAUSE_FLAC) CurrentlyPlaying = P_FLAC;
        else if(CurrentlyPlaying == P_PAUSE_MOD) CurrentlyPlaying = P_MOD;
        else if(CurrentlyPlaying == P_PAUSE_WAV) CurrentlyPlaying = P_WAV;
        else
            error("Nothing to resume");  
        return;
    }

    if(checkstring(cmdline, "CLOSE")) {
        CloseAudio();
        return;
    }

    if((tp = checkstring(cmdline, "VOLUME"))) {
        getargs(&tp, 3,",");
        if(argc < 1) error("Argument count");
        if(*argv[0]) vol_left = getint(argv[0], 0, 100);
        if(argc == 3) vol_right = getint(argv[2], 0, 100);
        return;
    }


    if((tp = checkstring(cmdline, "TONE"))) {
        int f_left, f_right;
        unsigned int PlayDuration = 0xffffffff;                     // default is to play forever
        
        // get the command line arguments
        getargs(&tp, 5,",");                                       // this MUST be the first executable line in the function
        if(!(argc == 3 || argc == 5)) error("Argument count");

        if(CurrentlyPlaying == P_TTS || CurrentlyPlaying == P_WAV || CurrentlyPlaying == P_PAUSE_WAV || CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_PAUSE_FLAC) error("Sound output in use");
        if(CurrentlyPlaying == P_TONE || CurrentlyPlaying == P_PAUSE_TONE) StopAudio();                 // stop the current tone
        
        f_left = getint(argv[0], 1, 20000);                         // get the arguments
        f_right = getint(argv[2], 1, 20000);
        if(argc > 4) PlayDuration = getint(argv[4], 0, INT_MAX);
        if(PlayDuration == 0) return;
        
    	htim5.Init.Period = PSpeedDiv/PWM_FREQ-1;
    	htim5.Instance->ARR = PSpeedDiv/PWM_FREQ-1;
		__HAL_TIM_SET_COUNTER(&htim5, 0);

        PhaseM_left =  (unsigned int)(((unsigned long long)0xffffffff * (unsigned long long)f_left)  / (unsigned long long)PWM_FREQ);
        PhaseM_right = (unsigned int)(((unsigned long long)0xffffffff * (unsigned long long)f_right) / (unsigned long long)PWM_FREQ);
        PhaseAC_left = PhaseAC_right = 0;
        SoundPlay = PlayDuration;
        ConfigSoundOutputs();
        CurrentlyPlaying = P_TONE;
        return;
    }
    if((tp = checkstring(cmdline, "WAV"))) {
        char *p;
        int i __attribute((unused))=0;
        getargs(&tp, 3,",");                                  // this MUST be the first executable line in the function
        if(!(argc == 1 || argc == 3)) error("Argument count");

        if(CurrentlyPlaying != P_NOTHING) error("Sound output in use");

        if(!InitSDCard()) return;
        p = getCstring(argv[0]);                                    // get the file name
        WAVInterrupt = NULL;

        WAVcomplete = 0;
        if(argc == 3) {
            WAVInterrupt = GetIntAddress(argv[2]);					// get the interrupt location
            InterruptUsed = true;
        }
        if(strchr(p, '.') == NULL) strcat(p, ".WAV");
        WAV_fnbr = FindFreeFileNbr();
        if(!BasicFileOpen(p, WAV_fnbr, FA_READ)) return;
        mywav=drwav_open((drwav_read_proc)onRead, (drwav_seek_proc)onSeek, NULL);
//        PInt(mywav->totalSampleCount/mywav->sampleRate/2);MMPrintString(" Calculated duration in seconds\r\n");
//        PInt(mywav->channels);MMPrintString(" Channels\r\n");
//        PInt(mywav->bitsPerSample);MMPrintString(" Bits per sample\r\n");
//        PInt(mywav->sampleRate);MMPrintString(" Sample rate\r\n");
        sbuff1 = GetMemory(WAV_BUFFER_SIZE);
        sbuff2 = GetMemory(WAV_BUFFER_SIZE);
        ibuff1 = (uint16_t *)sbuff1;
        ibuff2 = (uint16_t *)sbuff2;
    	htim5.Init.Period = PSpeedDiv/mywav->sampleRate-1;
    	htim5.Instance->ARR = PSpeedDiv/mywav->sampleRate-1;
		__HAL_TIM_SET_COUNTER(&htim5, 0);
//        HAL_Delay(1000);
        bcount[1]=drwav_read_s16(mywav, WAV_BUFFER_SIZE/2, (drwav_int16*)sbuff1);
        bcount[2]=drwav_read_s16(mywav, WAV_BUFFER_SIZE/2, (drwav_int16*)sbuff2);
        iconvert(ibuff1, (int16_t *)sbuff1, bcount[1]);
        iconvert(ibuff2, (int16_t *)sbuff2, bcount[2]);
        wav_filesize=bcount[2];
//        PInt(bcount[2]);PIntComma(bcount[1]);PRet();
        HAL_Delay(500);
        CurrentlyPlaying = P_WAV;
        swingbuf=1;
        nextbuf=1;
        ppos=0;
        ConfigSoundOutputs();
        return;
   }
    if((tp = checkstring(cmdline, "FLAC"))) {
        drflac_bs  myuserdata;
        char *p;
        int i __attribute((unused))=0;
        getargs(&tp, 3,",");                                  // this MUST be the first executable line in the function
        if(!(argc == 1 || argc == 3)) error("Argument count");
        if(CurrentlyPlaying != P_NOTHING) error("Sound output in use");

        if(!InitSDCard()) return;
        p = getCstring(argv[0]);                                    // get the file name
        WAVInterrupt = NULL;

        WAVcomplete = 0;
        if(argc == 3) {
            WAVInterrupt = GetIntAddress(argv[2]);                  // get the interrupt location
            InterruptUsed = true;
        }

        // open the file
        if(strchr(p, '.') == NULL) strcat(p, ".FLAC");
        WAV_fnbr = FindFreeFileNbr();
        if(!BasicFileOpen(p, WAV_fnbr, FA_READ)) return;
        myflac=drflac_open((drflac_read_proc)onRead, (drflac_seek_proc)onSeek, &myuserdata);
        sbuff1 = GetMemory(WAV_BUFFER_SIZE);                     
        sbuff2 = GetMemory(WAV_BUFFER_SIZE);
        ibuff1 = (uint16_t *)sbuff1;
        ibuff2 = (uint16_t *)sbuff2;
    	htim5.Init.Period = PSpeedDiv/myflac->sampleRate-1;
    	htim5.Instance->ARR = PSpeedDiv/myflac->sampleRate-1;
		__HAL_TIM_SET_COUNTER(&htim5, 0);
//        PInt(myflac->bitsPerSample);MMPrintString(" Bits per sample\r\n");
//        PInt(myflac->sampleRate);MMPrintString(" Sample rate\r\n");
//        PInt(myflac->totalSampleCount);MMPrintString(" Total Samples\r\n");
//        PInt(myflac->totalSampleCount/myflac->sampleRate/2);MMPrintString(" Calculated duration in seconds\r\n");
        bcount[1]=drflac_read_s16(myflac, WAV_BUFFER_SIZE/2, (dr_int16*)sbuff1);
        bcount[2]=drflac_read_s16(myflac, WAV_BUFFER_SIZE/2, (dr_int16*)sbuff2);
        iconvert(ibuff1, (int16_t *)sbuff1, bcount[1]);
        iconvert(ibuff2, (int16_t *)sbuff2, bcount[2]);
        wav_filesize=bcount[2];
        CurrentlyPlaying = P_FLAC;
        swingbuf=1;
        nextbuf=1;
        ppos=0;
        ConfigSoundOutputs();
        return;
    }
    error("Unknown command");
}

/******************************************************************************************
Timer interrupt.
Used to send data to the DAC
*******************************************************************************************/

void __attribute__ ((optimize("-O2")))Audio_Interrupt(void)  {
    // play a tone
    if(CurrentlyPlaying == P_TONE){
        PhaseAC_left += PhaseM_left;
        PhaseAC_right += PhaseM_right;
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, (SineTable[PhaseAC_left>>20]  * vol_left) / 100);
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, (SineTable[PhaseAC_right>>20]  * vol_right) / 100);
    } else if(CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_WAV  || CurrentlyPlaying == P_FLAC) {
        if(bcount[1]==0 && bcount[2]==0 && playreadcomplete==1){
        	HAL_TIM_Base_Stop_IT(&htim5);
        }
        if(swingbuf){ //buffer is primed
        	if(swingbuf==1)flacbuff=(uint16_t *)sbuff1;
        	else flacbuff=(uint16_t *)sbuff2;
        	if(ppos<=bcount[swingbuf]){
        		HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, flacbuff[ppos++]);
        		HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, flacbuff[ppos++]);
        	}
        	if(ppos==bcount[swingbuf]){
        		bcount[swingbuf]=0;
        		ppos=0;
        		if(swingbuf==1)swingbuf=2;
        		else swingbuf=1;
        	}
        }
    } else {
        // play must be paused
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, sineavg);
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, sineavg);
    }
}

/******************************************************************************************
Stop playing the music or tone
*******************************************************************************************/
void StopAudio(void) {

	if(CurrentlyPlaying != P_NOTHING ) {
		HAL_TIM_Base_Stop_IT(&htim5);
		HAL_NVIC_DisableIRQ(TIM5_IRQn);
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, sineavg);
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, sineavg);
        ppos=0;
        CurrentlyPlaying = P_NOTHING;
    }
	SoundPlay = 0;

}


/******************************************************************************************
 * Maintain the WAV sample buffer
*******************************************************************************************/
void __attribute__ ((optimize("-O2")))checkWAVinput(void){
    if(swingbuf != nextbuf){ //IR has moved to next buffer
        if(CurrentlyPlaying == P_FLAC){
            if(swingbuf==2){
                bcount[1]=(volatile unsigned int)drflac_read_s16(myflac, WAV_BUFFER_SIZE/2, (dr_int16*)sbuff1);
                iconvert(ibuff1, (int16_t *)sbuff1, bcount[1]);
                wav_filesize = bcount[1];
            } else {
                bcount[2]=(volatile unsigned int)drflac_read_s16(myflac, WAV_BUFFER_SIZE/2, (dr_int16*)sbuff2);
                iconvert(ibuff2, (int16_t *)sbuff2, bcount[2]);
                wav_filesize = bcount[2];
            }
            nextbuf=swingbuf;
        } else
        if(CurrentlyPlaying == P_WAV){
            if(swingbuf==2){
                bcount[1]=(volatile unsigned int)drwav_read_s16(mywav, WAV_BUFFER_SIZE/2, (drwav_int16*)sbuff1);
                iconvert(ibuff1, (int16_t *)sbuff1, bcount[1]);
                wav_filesize = bcount[1];
            } else {
                bcount[2]=(volatile unsigned int)drwav_read_s16(mywav, WAV_BUFFER_SIZE/2, (drwav_int16*)sbuff2);
                iconvert(ibuff2, (int16_t *)sbuff2, bcount[2]);
                wav_filesize = bcount[2];
            }
            nextbuf=swingbuf;
        }
    }
    if(wav_filesize<=0 && ((CurrentlyPlaying == P_WAV) || (CurrentlyPlaying == P_FLAC) || (CurrentlyPlaying == P_MP3))){
        playreadcomplete=1;
    }

}
void __attribute__ ((optimize("-O2"))) audio_checks(void){
	if(SoundPlay && SoundPlay != 0xffffffff) {						// if we are still playing the sound and it is not forever
		SoundPlay--;
		if(SoundPlay < 1) {
    		StopAudio();
		}
	}

    if(playreadcomplete == 1) {                                    // close the WAV output if it has completed
        if(!(bcount[1] || bcount[2]) ){
             if(CurrentlyPlaying == P_FLAC)drflac_close(myflac);
            if(CurrentlyPlaying == P_WAV)drwav_close(mywav);
            StopAudio();
            FreeMemory(sbuff1);
            FreeMemory(sbuff2);
            FileClose(WAV_fnbr);
            WAVcomplete = true;
            playreadcomplete = 0;
            mywav = NULL;
        }
    }
}
