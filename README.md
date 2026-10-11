# 👻 GhostHUD

<div align="center">

![Language](https://img.shields.io/badge/Language-Pure%20C%20(C99%2FC11)-00599C?style=for-the-badge&logo=c)
![Size](https://img.shields.io/badge/Binary%20Size-20%20KB-success?style=for-the-badge)
![RAM](https://img.shields.io/badge/RAM%20Usage-~2%20MB-brightgreen?style=for-the-badge)
![CPU](https://img.shields.io/badge/CPU%20Usage-0.0%25-brightgreen?style=for-the-badge)
![Platform](https://img.shields.io/badge/OS-Windows%2010%2F11-0078D6?style=for-the-badge&logo=windows)
![Anti-Cheat Safe](https://img.shields.io/badge/Anti--Cheat-100%25%20Safe-brightgreen?style=for-the-badge)

**Monitor de desempenho de hardware ultra-otimizado em C puro para Windows. Desenhado para gamers que exigem zero sobrecarga no sistema e visual discreto sobreposto a qualquer jogo.**

</div>

---

## ⚡ Recursos Principais

- 🪶 **Tamanho de Apenas 20 KB:** O executável completo tem apenas **20 Kilobytes**, sem nenhuma dependência externa ou runtime.
- 💾 **Memória Irrisória (~2 MB de RAM):** Consome menos memória do que um único processo do Bloco de Notas.
- ⏱️ **0.0% de CPU:** Código C nativo rodando diretamente no loop de eventos do Windows com temporizador de precisão.
- 🎯 **Modo Click-Through Real (F9):**
  - 🔓 **Modo Ajuste:** Borda ciano. Clique e arraste o HUD com o mouse para onde preferir na tela.
  - 🔒 **Modo Jogo (Click-Through):** O mouse passa **100% através do HUD** direto para o jogo (miras, tiros e cliques nunca são perdidos).
- 📦 **Modo Minimizar / Maximizar (F8):**
  - **Normal / Maximizado (`320px x 56px`):** Cápsula completa com CPU %, GPU % e Temp, RAM % e micro barras dinâmicas de carga.
  - **Minimizado / Quadradinho (`64px x 56px`):** Um quadradinho minúsculo e ultra-discreto exibindo `C`, `G` e `R` condensados.
  - **Alternância Instantânea:** Pressione **`F8`**, dê um **duplo clique no widget** (no modo ajuste) ou selecione no menu do relógio!
- 🎨 **Escala Térmica e de Carga Progressiva em 4 Estágios:**
  - 🟢 **Verde (<55%):** Normal / Carga baixa
  - 🟡 **Amarelo (55% a 71%):** Carga moderada
  - 🟠 **Laranja (72% a 84%):** Carga elevada / Aquecendo
  - 🔴 **Vermelho Crítico (≥85% ou 90%+):** Alerta máximo de saturação ou superaquecimento!
- 🛡️ **Anti-Cheat Safe (100% Seguro):** Zero injeção de DLL. Utiliza os estilos estendidos oficiais do Windows (`WS_EX_TRANSPARENT`). Seguro no Vanguard, Easy Anti-Cheat, BattlEye e Ricochet.
- 🔔 **Bandeja do Sistema (Tray Icon):** Menu de clique direito perto do relógio para alternar modos, trocar tamanho, resetar posição e sair.

---

## 🖥️ Métricas Monitoradas

| Componente | Métrica | Implementação em C |
| :--- | :--- | :--- |
| **GPU** | Uso (%) e Temperatura (°C) | NVIDIA NVML (`nvml.dll` carregada dinamicamente) |
| **CPU** | Uso Percentual (%) | `GetSystemTimes` (Win32 Kernel API nativa) |
| **RAM** | Uso Percentual (%) | `GlobalMemoryStatusEx` (Win32 Kernel API nativa) |

---

## ⌨️ Atalhos e Controles

- **`F9`**: Alterna entre **Modo Jogo** (cliques passam direto) e **Modo Ajuste** (arrastar com o mouse).
- **`F8`**: Alterna entre **Modo Maximizado** (cápsula completa) e **Modo Minimizado** (quadradinho compacto).
- **Duplo Clique no Widget**: Alterna entre Minimizar e Maximizar (quando destravado).
- **Clique Direito no Ícone da Bandeja**: Menu completo de controle.

---

## 🔨 Como Compilar do Código Fonte

Você pode compilar com 1 clique usando o script incluído:
```cmd
build.bat
```

Ou usando qualquer compilador C:
```cmd
gcc -O3 src/main.c src/hardware.c src/ui.c -o GhostHUD.exe -lgdi32 -luser32 -lkernel32 -mwindows
```

---

## 📄 Licença

Distribuído sob a licença MIT.
