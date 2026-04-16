# Automa-o-do-Sistema-Residencial-de-Abastecimento-de-gua-Projeto-Final---Embarcatech-
Sistema embarcado com RP2040 para automação da caixa d’água residencial. Monitora o nível com sensor ultrassônico, exibe informações em interface local e aciona a bomba automaticamente, reduzindo transbordamentos, desperdício de água e falhas no abastecimento.

## Arquitetura de firmware

O firmware foi organizado em módulos independentes:
- sensor_nivel: leitura do HC-SR04 e conversão de distância para percentual;
- controle_bomba: máquina de estados e lógica de acionamento da bomba;
- soft_start: lógica de zero-crossing e controle do TRIAC;
- display_oled: renderização das telas no OLED;
- interface_usuario: leitura dos botões e joystick;
- buzzer: geração de padrões sonoros;
- main: integração de todos os módulos.

## Tabela de pinos utilizados no projeto

| Módulo / Periférico            | Função                                      | GPIO | Tipo / Interface     | Observação |
|-------------------------------|---------------------------------------------|------|----------------------|------------|
| Botão A                       | Confirmar / avançar                         | 5    | Entrada digital      | Usado em `interface_usuario` |
| Botão B                       | Cancelar / voltar                           | 6    | Entrada digital      | Usado em `interface_usuario` |
| Buzzer A                      | Alerta sonoro                               | 10   | Saída PWM            | Usado em `buzzer` |
| Display OLED SSD1306          | SDA                                         | 14   | I2C1                 | Usado em `display_oled` |
| Display OLED SSD1306          | SCL                                         | 15   | I2C1                 | Usado em `display_oled` |
| Controle da bomba (simulado)  | Saída lógica da bomba                       | 16   | Saída digital        | Usado em `controle_bomba` |
| Zero-crossing                 | Referência de cruzamento por zero           | 17   | Entrada digital / IRQ| Usado em `soft_start` |
| HC-SR04                       | TRIG                                        | 18   | Saída digital        | Usado em `sensor_nivel` |
| HC-SR04                       | ECHO                                        | 19   | Entrada digital      | Usado em `sensor_nivel` |
| Controle do TRIAC             | Pulso de disparo do optotriac (MOC3021M)    | 20   | Saída digital        | Usado em `soft_start` |
| Buzzer B                      | Alerta sonoro                               | 21   | Saída PWM            | Usado em `buzzer` |
| Joystick                      | Eixo X                                      | 26   | ADC0                 | Usado em `interface_usuario` |
| Joystick                      | Eixo Y                                      | 27   | ADC1                 | Usado em `interface_usuario` |


## Circuito para controle de partida do motor
<img width="865" height="386" alt="image" src="https://github.com/user-attachments/assets/f35232fc-0bdb-412e-baf1-52d7ce5da8bb" />

## Circuito Zero-Crossing
<img width="1115" height="495" alt="image" src="https://github.com/user-attachments/assets/2453c28d-f60d-4131-bf55-8d25b06f4770" />

## Simulação no Wokwi
Acesse a simulação do sensor HC-SR04 no Wokwi:  
[Projeto no Wokwi](https://wokwi.com/projects/461329874578285569)

## Apresentação no Youtube
Acesse a apresentação do projeto no Youtube:
[Projeto no Youtube](https://youtu.be/WEvqHvgd9ds)
