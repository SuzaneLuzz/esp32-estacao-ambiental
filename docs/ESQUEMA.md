# Esquema de ligação e montagem

## Mapa de pinos

| GPIO | Uso | Tipo | Observação |
|---|---|---|---|
| 15 | DHT22 – dados | Digital | O módulo DHT22 já tem resistor de pull-up. Se usar o sensor avulso, coloque 10 kΩ entre DATA e 3V3. |
| 34 | LDR – AO | Analógico (ADC1) | Pino somente entrada. |
| 35 | Solo – AOUT | Analógico (ADC1) | Pino somente entrada. |
| 26 | Relé – IN | Digital (saída) | Veja `RELAY_ACTIVE_LOW` no `config.h`. |
| 25 | LED | Digital (saída) | Sempre com resistor de 220 Ω. |
| 27 | Buzzer | PWM (`tone`) | Buzzer passivo ou ativo. |
| 21 / 22 | OLED SDA / SCL | I2C | Endereço 0x3C. |
| 4 | Botão | Entrada com pull-up interno | Outro terminal no GND. |

## Por que esses pinos?

- **ADC1 (GPIO 32–39) para os sensores analógicos:** o ADC2 do ESP32 fica indisponível quando o Wi-Fi está ligado.
- **GPIO 34 e 35** são somente entrada, ideais para sensores e liberam os outros pinos para saídas.
- Foram evitados os pinos de *strapping* (0, 2, 12) e os da flash interna (6–11).

## Alimentação

- **DHT22, LDR, sensor de solo e OLED em 3V3.** O ADC do ESP32 aceita no máximo ~3,3 V; alimentar os sensores analógicos com 5 V pode danificar o pino.
- **Relé em 5 V (VIN)**, pois a bobina precisa dessa tensão. A entrada IN funciona com o sinal de 3,3 V do ESP32 na maioria dos módulos.
- **Nunca alimente o ventilador pelo ESP32.** O ventilador deve ter fonte própria, passando pelos contatos COM/NO do relé.
- Todos os GNDs (ESP32, sensores, fonte do ventilador) devem estar interligados, exceto o lado de carga do relé, que é isolado.

## Diagrama de blocos

```
                 3V3 ─┬───────────┬───────────┬───────────┐
                      │           │           │           │
                   [DHT22]     [LDR]      [SOLO]      [OLED]
                    DATA        AO         AOUT     SDA   SCL
                      │           │           │       │     │
 ┌────────────────────┴───────────┴───────────┴───────┴─────┴──┐
 │  GPIO15      GPIO34      GPIO35             GPIO21  GPIO22  │
 │                          ESP32 DevKit                        │
 │  GPIO26      GPIO25      GPIO27      GPIO4                   │
 └────┬───────────┬───────────┬───────────┬─────────────────────┘
      │           │           │           │
   [RELÉ]──5V  [220Ω]      [BUZZER]    [BOTÃO]
      │           │           │           │
  COM/NO →     [LED]         GND         GND
  ventilador      │
  (fonte própria) GND
```

## Checklist de testes da montagem

1. Serial mostra o cabeçalho CSV e as linhas a cada 5 s.
2. OLED exibe a tela de inicialização e depois as leituras.
3. Soprar ar quente no DHT22 (ou mudar no simulador) acima de 30 °C → relé liga; abaixo de 28 °C → desliga.
4. Tampar o LDR → LED acende; destampar → apaga.
5. Tirar o sensor de solo da terra → alarme de ATENÇÃO.
6. Desconectar o fio de dados do DHT22 → após ~6 s, alarme CRÍTICO.
7. Clique curto no botão troca a tela; pressão longa silencia o alarme.
8. Painel web abre no IP mostrado na tela 3.
