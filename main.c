#include "main.h"

void SystemClockConfigUpdate(void);
void GPIOConfig(void);
void TIM1Config(void);
int main(void)
{

	/*
		 * ================================================================
		 * TIM1 ile PWM ÜRETİMİ (STM32F4)
		 * ================================================================
		 *
		 * Amaç:
		 * ------
		 * TIM1 kanal 1 (PA8) üzerinden PWM sinyali üretmek.
		 *
		 * Duty cycle değeri yazılım tarafından değiştirilerek
		 * LED parlaklığının artıp azalması (fade efekti) gözlemlenir.
		 *
		 * PWM tipi:
		 * ---------
		 * - Edge-aligned (yukarı sayımlı)
		 * - PWM Mode 1 (OC1M = 110)
		 *
		 * Çalışma mantığı:
		 * -----------------
		 * CNT < CCR1  -> Çıkış HIGH
		 * CNT >= CCR1 -> Çıkış LOW
	*/

	HAL_Init();
    SystemClockConfigUpdate();
    GPIOConfig();
    TIM1Config();


  int i;
  while (1)
  {
	  // Duty cycle azaltma döngüsü
	  // CCR1 değeri azaldıkça LED parlaklığı azalır
	  for(i=999; i>0; i--){
		  TIM1->CCR1 = i;
		  HAL_Delay(55);
	  }

	  // Duty cycle artırma döngüsü
	  // CCR1 değeri arttıkça LED parlaklığı artar
	  for(i=0; i<999; i++){
	  	  TIM1->CCR1 = i;
	  	  HAL_Delay(55);
	  }
  }
  /* USER CODE END 3 */
}



void TIM1Config(void)
{
	RCC->APB2ENR |= (1<<0); // TIM1 için APB2 hattı enable


	/*******************      TIM1_CR1 REGISTER KONFİGÜRASYONU      *******************/

	/*
	   ARPE = 1
	   Auto-reload preload enable
	   ARR değeri buffer’a alınır ve update event ile yüklenir
    */

	TIM1->CR1 = 0; // Başlangıçta tüm bitleri temizle
	TIM1->CR1 |= (1U<<7); // ARPE=1


	/*******************      TIM1_CR2, SMCR, DIER REGISTER KONFİGÜRASYONU      *******************/

	// Bu register ların bitleri şu an lazım değil, o yüzden sadece bitleri temizledim
	TIM1->CR2 = 0;
	TIM1->SMCR = 0;
	TIM1->DIER = 0;


	/*******************      TIM1_CCMR1 REGISTER KONFİGÜRASYONU      *******************/

	/*
		  OC1M = 110 → PWM Mode 1

		  Çalışma mantığı (yukarı sayım):
		  CNT < CCR1  -> HIGH
		  CNT >= CCR1 -> LOW
	*/

	TIM1->CCMR1=0; // bitleri temizle
	TIM1->CCMR1 |= (6U<<4); // OC1M=110



	/*******************      TIM1_CCER REGISTER KONFİGÜRASYONU      *******************/

	// ÇIKIŞ AKTİF ETME (CH1)
	TIM1->CCER = 0; // Bitleri temizle
	TIM1->CCER |= (1U<<0); // CH1 enable


	/*******************      TIM1PSC/ARR/CCR1 AYARI      *******************/

	/* ------------------------------------------------------------
		  ZAMANLAYICI TEMEL AYARLAR
	   ------------------------------------------------------------

	   Timer clock = 168 MHz (APB2 clock frekansı x 2 = TIM1 clock giriş frekansı)

	   Prescaler = 167 → bölme oranı (167+1 = 168)
	   Timer clock = 168 MHz / 168 = 1 MHz


	   ARR = 999
	   PWM frekansı = 1 MHz / (999+1) = 1 kHz


	   Başlangıç duty cycle değeri
	   CCR1, HIGH süresini belirler
     */

	TIM1->PSC=167;
	TIM1->ARR = 999;
	TIM1->CCR1 = 999;

	printf("%d\n",TIM1->ARR);


	/*******************      TIM1_RCR REGISTER KNFİGÜRASYONU      *******************/

	TIM1->RCR = 0; // REPETITION COUNTER (KULLANILMIYOR)



	/*******************      TIM1_BDTR REGISTER KONFİGÜRASYONU      *******************/

	/*
	   MOE = 1 → Main Output Enable
	   TIM1 gibi advanced timer’larda çıkışın aktif olması için zorunlu
	*/

	TIM1->BDTR = 0;
	TIM1->BDTR |= (1<<15);


	/*******************      TIM1_EGR REGISTER KONFİGÜRASYONU      *******************/

	/* ------------------------------------------------------------
	   UPDATE EVENT (İLK SENKRONİZASYON)
	   ------------------------------------------------------------

	   UG (Update Generation):
	   PSC, ARR ve CCR registerlarını aktif hale getirir
	   Timer’ın doğru başlangıç değerleriyle başlamasını sağlar
	*/


	TIM1->EGR = 0; // Tüm bitleri temizle
	TIM1->EGR |= (1U<<0); // UEV yi aktif et

	TIM1->CR1 |= (1U<<0); // CEN = 1 → timer başlatılır

}


void GPIOConfig(){

	RCC->AHB1ENR |= (1<<0);

	// PortA_8. pini TIM1 in üreteceği PWM sinyalini çıkışa verecek olan pin olarak ayarla

	// Alternate Function modu
	GPIOA->MODER &= ~(3U<<16);
	GPIOA->MODER |= (1U<<17);

	GPIOA->OTYPER &= ~(1U<<8);

	GPIOA->OSPEEDR &= ~(3U<<16);
	GPIOA->OSPEEDR |= (1U<<17);

	GPIOA->PUPDR &= ~(3U<<16);

	// AF ayarı
	// TIM1 = AFH_AF1, 8. pin
	GPIOA->AFR[1] &= ~(0xF<<0); // Bitleri temizle
	GPIOA->AFR[1] |= (1U<<0);

}

int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}


void SystemClockConfigUpdate(){

	// Amaç: SYSCLK=168MHz çalıştırmak

	/* FLASH ayarları */
	FLASH->ACR |= (5<<0); // Bu satır LATENCY ayarını yapıyor, LATENCY = 5 → 5 wait states (STM32F4’de 168 MHz çalıştırmak için datasheet’e göre 5 wait state gerekiyor)
	FLASH->ACR |= (1<<8); // PRFTEN (Prefetch enable) bitini açar.
	FLASH->ACR |= (1<<9); // ICEN (Instruction cache enable) bitini açar.
	FLASH->ACR |= (1<<10); // DCEN (Data cache enable) bitini açar.


	RCC->CR |= (1<<16); // HSE enable edildi kullanılmak üzere
	while((RCC->CR&0x00020000)!=0x00020000); // HSE ready flag i 1 olup HSE nin çalışmaya hazır olduğunu söyleyene kadar bekle
	RCC->CR |= (1<<19); // HSE clock un çalışıp çalışmadığını izleyen controle detector ü enable et

	// PLL ayarları

	// PLL_M = 8
	RCC->PLLCFGR &= ~(0x3F<<0); // öncelikle ilgili bitleri temizledim.
	RCC->PLLCFGR |= (1<<3);
	// PLL_M çıkışı(PLL_N girişi) = 1MHz artık

	// PLL_N=336MHz olmalı
	RCC->PLLCFGR &= ~(0x1FF<<6); // ilgili bitleri temizle
	RCC->PLLCFGR |= (1<<14) | (1<<12) | (1<<10);

	// PLL_P=2
	RCC->PLLCFGR &= ~(3<<16); // Zaten bu hali ile PLL_P=2 olmuş olur

	RCC->PLLCFGR |= (1<<22);   // PLLSRC = HSE

	// PLL kullanacağımızı belirtmek için PLLON enable edilmeli
	RCC->CR |= (1<<24);

	while((RCC->CR & (1<<25)) == 0); // PLLRDY

	// PLLCLK yı SYSCLK kullanacağımı söylüyorum;
	RCC->CFGR &= ~(3<<0); // önce bitleri temizle
	RCC->CFGR |= (1<<1);

	// PLL selected as system clock
	RCC->CFGR &= ~(3<<0);
	RCC->CFGR |= (1<<1);

	while((RCC->CFGR & (3<<2)) != (2<<2)); //Switch’in tamamlandığını kontrol et

	// AHB Prescaler(HPRE biti) = 1 olmalı ki HCLK=168MHz olsun**********
	RCC->CFGR &= ~(0xF<<4);

	// SysTick frekans=168MHz olsun istediğim için HCLK/8 değil HCLK kaynağını direkt kullanacağım.
	// Bu nedenle STK_CTRL register ında clock source olarak HCLK yı seçmek istediğim için 2. biti "1" olarak ayarla
	SysTick->CTRL |= (1<<2);

	//  APB1=42, APB2=84 MHz de çalışsın diye PPRE1=4, PPRE2=2 olarak ayalarnmalı RCC_CFGR register ında
	RCC->CFGR &= ~(0x3F<<10); // bitleri temizle
	RCC->CFGR |= (1<<15) | (1<<12) | (1<<10);

	SystemCoreClockUpdate(); // Donanımda değiştirdiğimiz SYSCLK frekansını CMSIS tarafındaki SystemCoreClock değişkenine güncelleyerek,
	                         // yazılımın ve kütüphanelerin doğru CPU frekansını kullanmasını sağlamaktır.
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
