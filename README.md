# STM32F4 TIM1 PWM Fade Project

## 📌 Proje Açıklaması

Bu proje, STM32F4 serisi mikrodenetleyicide TIM1 advanced timer modülü kullanılarak PWM sinyali üretimini göstermektedir.  

PA8 (TIM1_CH1) pininden üretilen PWM sinyalinin duty cycle değeri yazılım tarafından dinamik olarak değiştirilerek LED parlaklığında artış/azalış (fade effect) elde edilmektedir.

Proje **register seviyesinde (bare-metal)** geliştirilmiştir.

---

# Kullanılan Timer: TIM1 (Advanced Control Timer)

TIM1 ve TIM8, STM32F4 ailesinde **advanced-control timer** olarak geçer.

Bu timer’ların temel özellikleri:
- PWM üretimi
- Complementary çıkışlar (CHxN)
- Dead-time insertion
- Break input (BDTR register)
- MOE (Main Output Enable) kontrolü

---

# PWM Temel Çalışma Prensibi

PWM sinyali şu mantıkla üretilir:
CNT < CCR1 → Output HIGH ; CNT ≥ CCR1 → Output LOW


Bu yapı **PWM Mode 1 (OC1M = 110)** için geçerlidir.

---

#  Timer Frekans Hesaplamaları

## 1. Timer Clock

STM32F4 üzerinde TIM1 genellikle:
TIM_CLK = 168 MHz

## 2. Prescaler (PSC)

Prescaler, timer clock’u böler: TIM_counter_clock = TIM_CLK / (PSC + 1)

ÖRNEK: PSC = 167 => 168 MHz / 168 = 1 MHz = TIM_counter_clock

## 3. Auto Reload Register (ARR)

PWM periyodunu belirler: PWM_frequency = TIM_counter_clock / (ARR + 1)

ÖRNEK: ARR = 999 => 1 MHz / 1000 = 1 kHz PWM(PWM frekansı)

## 4. Duty Cycle Hesabı (CCR1)

Duty cycle formülü: Duty (%) = (CCR1 / (ARR + 1)) × 100

ÖRNEK: CCR1 = 500 => ARR = 999

Duty = (500 / 1000) × 100 = 50%


# PWM Output Behavior

## PWM Mode 1 (OC1M = 110)

| CNT durumu | Çıkış |
|------------|------|
| CNT < CCR1 | HIGH |
| CNT ≥ CCR1 | LOW |


# Advanced Timer (TIM1/TIM8) Özellikleri

## BDTR (Break and Dead-Time Register)
- MOE (Main Output Enable) → çıkışı aktif eder
- Break input → güvenlik mekanizması
- Dead-time → half-bridge sürücüler için gecikme ekler

## EGR (Event Generation Register)
- UG (Update Generation)
- PSC / ARR / CCR shadow registerlarını günceller
- Timer başlangıcını senkronize eder

## ARPE (Auto Reload Preload Enable)
- ARR buffer’lama sağlar
- Güncellemeler UEV (Update Event) ile uygulanır


# Projede Yapılan İşlem

1. TIM1 PWM Mode 1 olarak yapılandırıldı
2. PA8 pinine alternate function atandı
3. PSC ve ARR ile 1 kHz PWM üretildi
4. CCR1 değeri yazılım ile değiştirildi
5. Duty cycle artırılıp azaltılarak LED fade efekti oluşturuldu


# Gözlemlenen Davranış

- CCR1 arttıkça LED parlaklığı artar
- CCR1 azaldıkça LED parlaklığı azalır
- PWM frekansı sabit kalır (1 kHz)

# Demo Video


# Documentation
1. STM32F407 Reference manual
2. STM32 Datasheet


