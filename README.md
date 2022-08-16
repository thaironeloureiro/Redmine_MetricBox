# metricbox
Box com tela touch para exibição de métricas ágeis extraídas do Redmine.

# features
- Consulta individual de chamado
- WIP (Work in Progress) 
- Quantitativos de chamados em cada fase
- Quantidade de chamados por Categoria indicados pela cor (Verde: Corretiva, Laranja: Adaptativa, Amarelo: Evolutiva, Branco: Consulta)
- Leadtime
- Taxa de Rejeição
- Indicação de existência de chamados atualmnete marcados como: Rejeitado, Impedimento, Liberado em Espera
- Detalhamento de um chamado atualmente Rejeitado



# TTGO T-Call SIM800

<p align="center">
  <img width="500" src="/datasheet/board.jpg">
</p>

## Pinout
![pinout](https://github.com/Xinyuan-LilyGO/TTGO-T-Call/blob/master/datasheet/T-Call.jpg)

## SIM800 connection
| Name         | Pins |
| ------------ | ---- |
| SIM800_TX    | 26   |
| SIM800_RX    | 27   |
| SIM800_PWKEY | 4    |
| SIM800_RST   | 5    |
| SIM800_POWER | 23   |

## PINOUT no arquivo Setup21_ILI9488.h da biblioteca TFT_eSPI 
#define TFT_MISO 19 // (leave TFT SDO disconnected if other SPI devices share MISO)
#define TFT_MOSI 2
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC   14  // Data Command control pin
#define TFT_RST  33  // Reset pin (could connect to RST pin)
#define TOUCH_CS 13  // Chip select pin (T_CS) of touch screen


[<img align="right" height="48" src="/datasheet/buy.png">](https://www.aliexpress.com/item/33045221960.html)

Na IDE do Arduino, em configuração de placa, uso a placa DOIT ESP32 DEVKIT V1
