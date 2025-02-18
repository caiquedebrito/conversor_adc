# Controle de LEDs RGB e Display com Joystick - BitDogLab

## 📌 Descrição
Este projeto explora o funcionamento do **conversor analógico-digital (ADC)** no RP2040 e a integração com diversos componentes na placa **BitDogLab**. O sistema utiliza um **joystick** para controlar LEDs RGB e movimentar um quadrado em um **display OLED SSD1306**. Além disso, há controle de estado dos LEDs e do display através de botões físicos.

## 🎯 Funcionalidades
✅ **Controle dos LEDs RGB via Joystick:**  
- O **LED Azul** ajusta o brilho conforme o **eixo Y**.  
- O **LED Vermelho** ajusta o brilho conforme o **eixo X**.  
- Os LEDs são controlados por **PWM** para variação suave da intensidade luminosa.  

✅ **Movimentação de um quadrado no display OLED SSD1306:**  
- O joystick controla a **posição do quadrado** (8x8 pixels) na tela.
- O display se comunica via **I2C**.

✅ **Funções dos botões:**  
- **Botão do joystick (GPIO 22):**
  - Alterna o estado do **LED Verde**.
  - Modifica a **borda do display** a cada acionamento.
- **Botão A (GPIO 5):**
  - Ativa ou desativa os **LEDs RGB**.

✅ **Tratamento de interrupções:**  
- Os botões utilizam **interrupções (IRQ)** para leitura eficiente.
- Implementação de **debouncing** via software para evitar leituras falsas.

## 🛠️ Requisitos
Para executar este projeto, você precisará de:
- **Placa BitDogLab (RP2040)**
- **Joystick** (conectado aos GPIOs 26 e 27)
- **Display OLED SSD1306** (I2C - GPIOs 14 e 15)
- **LED RGB** (conectado aos GPIOs 11, 12 e 13)
- **Botão do joystick** (GPIO 22)
- **Botão A** (GPIO 5)
- **Bibliotecas para Raspberry Pi Pico (RP2040)**:
  - `pico-sdk`
  - `pico-extras`
  - `ssd1306` (para comunicação com o display)
  - `hardware_adc` (para leitura do joystick)
  - `hardware_pwm` (para controle dos LEDs)

## 🚀 Como Executar
### 1️⃣ Configurar o Ambiente
1. Instale o **Pico SDK** e configure o ambiente de desenvolvimento.
2. Clone este repositório:
   ```bash
   git clone https://github.com/caiquedebrito/conversor_adc.git
   cd conversor_adc
   ```

### 2️⃣ Compilar e Carregar no RP2040
3. Compile o código-fonte usando CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
4. Conecte a placa **BitDogLab** ao computador e carregue o binário gerado:

### 3️⃣ Testar o Projeto
5. Após o upload, o programa será executado automaticamente.
6. Utilize o **joystick** para controlar os LEDs e movimentar o quadrado no display.
7. Pressione os **botões** para alternar os LEDs e modificar a borda do display.

## 📷 Demonstração
🎥 Um vídeo demonstrando o funcionamento do projeto pode ser acessado aqui: **[Link do Vídeo](https://youtube.com/SEU_VIDEO_AQUI)**.

## 📜 Estrutura do Código
```plaintext
/
├── src/
│   ├── main.c            # Código principal
├── CMakeLists.txt        # Configuração do CMake
└── README.md             # Documentação do projeto
```

## 🔗 Referências
- [Documentação RP2040](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Guia de uso do SSD1306 com RP2040](https://github.com/adafruit/Adafruit_SSD1306)

📌 **Dúvidas ou sugestões?** Abra uma issue no repositório! 🚀

