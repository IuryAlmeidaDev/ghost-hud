# 👻 GhostHUD

<div align="center">

![Language](https://img.shields.io/badge/Language-Pure%20C%20(C99%2FC11)-00599C?style=for-the-badge&logo=c)
![Size](https://img.shields.io/badge/Binary%20Size-15%20KB-success?style=for-the-badge)
![RAM](https://img.shields.io/badge/RAM%20Usage-~2%20MB-brightgreen?style=for-the-badge)
![CPU](https://img.shields.io/badge/CPU%20Usage-0.0%25-brightgreen?style=for-the-badge)
![Platform](https://img.shields.io/badge/OS-Windows%2010%2F11-0078D6?style=for-the-badge&logo=windows)
![Anti-Cheat Safe](https://img.shields.io/badge/Anti--Cheat-100%25%20Safe-brightgreen?style=for-the-badge)

**Monitor de desempenho de hardware ultra-otimizado em C puro para Windows. Desenhado para gamers que exigem zero sobrecarga no sistema e visual discreto sobreposto a qualquer jogo.**

</div>

---

## ⚡ Por que GhostHUD?

- 🪶 **Tamanho Surreal de 15 KB:** O executável completo tem apenas **15 Kilobytes**, compilado diretamente em código de máquina nativo.
- 💾 **Memória Irrisória (~2 MB de RAM):** Consome menos memória do que um único processo do Bloco de Notas do Windows.
- ⏱️ **0.0% de CPU:** Zero interpretadores, zero runtimes pesados e zero frameworks. Código C nativo rodando no loop de eventos do Windows.
- 🎯 **Modo Click-Through Real (F9):** Pressione `F9` para alternar entre:
  - 🔓 **Modo Ajuste:** Borda ciano suave. Clique e arraste o HUD com o mouse para onde preferir na tela.
  - 🔒 **Modo Jogo (Click-Through):** O mouse passa **100% através do HUD** direto para o jogo. Você mira, atira e clica sem perder nenhum clique.
- 🛡️ **Anti-Cheat Safe (100% Seguro):** Não injeta código em processos de jogos. Utiliza os estilos estendidos oficiais do Windows (`WS_EX_TRANSPARENT`). Seguro em jogos com Vanguard, Easy Anti-Cheat, BattlEye e Ricochet.
- 🟢 **Código de Cores Térmicas Inteligente:**
  - 🟢 **Verde (#00E676):** Carga e temperatura normais (< 72°C).
  - 🟡 **Âmbar (#FFAB00):** Carga alta (72°C a 82°C).
  - 🔴 **Vermelho (#FF5252):** Alerta térmico crítico (> 83°C).
- 🔔 **Bandeja do Sistema (Tray Icon):** Ícone discreto perto do relógio do Windows com menu de clique direito para alternar modos, resetar posição e sair.

---

## 🖥️ Métricas Monitoradas

| Componente | Métrica | Implementação em C |
| :--- | :--- | :--- |
| **GPU** | Uso (%) e Temperatura (°C) | NVIDIA NVML (`nvml.dll` carregada dinamicamente) |
| **CPU** | Uso Percentual (%) | `GetSystemTimes` (Win32 Kernel API nativa) |
| **RAM** | Uso Percentual (%) e MBs | `GlobalMemoryStatusEx` (Win32 Kernel API nativa) |

---

## 🚀 Como Usar

Basta dar dois cliques no executável:
👉 **`GhostHUD.exe`**

Ele abrirá instantaneamente em segundo plano sobreposto aos seus jogos!

### ⌨️ Atalhos
- **`F9`**: Alterna entre **Modo Jogo** (cliques passam direto) e **Modo Ajuste** (arrastar com o mouse).
- **Botão Direito no Ícone da Bandeja**: Menu rápido para resetar posição, alternar modos ou fechar o programa.

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
