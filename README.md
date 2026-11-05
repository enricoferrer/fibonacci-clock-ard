# fibonacci-clock-ard

Relógio Fibonacci para a placa **ESP32-2432S028R** ("Cheap Yellow Display", tela de 2,8"). A hora vem da internet por Wi-Fi.

## Pastas

| Pasta | Conteúdo |
|---|---|
| `teste_tela/` | Teste das cores da tela. Rode primeiro. |
| `relogio_fibonacci/` | O relógio. |

## Como usar

1. Na Arduino IDE, escolha a placa **ESP32 Dev Module** e a porta da placa.
2. Abra `teste_tela/teste_tela.ino`, grave na placa e anote em qual modo (A ou B) as cores ficam certas.
3. Abra `relogio_fibonacci/relogio_fibonacci.ino`:
   - Na aba `config_wifi.h`, coloque o nome e a senha do Wi-Fi.
   - No bloco **CONFIGURACAO**, ajuste `INVERTER_CORES` conforme o teste: MODO A = `false`, MODO B = `true`.
4. Grave na placa.

O passo a passo completo, com a solução de problemas, está no `GUIA_PLACA_ESP32.md`, na pasta acima do repositório.

## Wi-Fi e Git

O arquivo `relogio_fibonacci/config_wifi.h` está no `.gitignore`, então a senha não vai para o Git. Em um clone novo, copie `config_wifi.exemplo.h` como `config_wifi.h` e preencha os dados.

## Bibliotecas

- Plataforma **esp32 by Espressif Systems**
- **Adafruit ILI9341**, com a dependência Adafruit GFX
