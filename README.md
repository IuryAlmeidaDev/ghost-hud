# 👻 GhostHUD

<div align="center">

![Language](https://img.shields.io/badge/Language-Pure%20C%20(C99)-00599C?style=for-the-badge&logo=c)
![Binary Size](https://img.shields.io/badge/Binary%20Size-~40%20KB-success?style=for-the-badge)
![RAM](https://img.shields.io/badge/RAM%20Usage-~2.5%20MB-brightgreen?style=for-the-badge)
![CPU](https://img.shields.io/badge/CPU%20Usage-0.0%25-brightgreen?style=for-the-badge)
![GPU Vendors](https://img.shields.io/badge/GPUs-NVIDIA%20%7C%20AMD-76B900?style=for-the-badge)
![Platform](https://img.shields.io/badge/OS-Windows%2010%2F11-0078D6?style=for-the-badge&logo=windows)
![Anti-Cheat Safe](https://img.shields.io/badge/Anti--Cheat-100%25%20Safe-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)

**Overlay de monitoramento de hardware gamer ultra-leve, modular e elegante para Windows. Inspirado na estética obsidian de interfaces clássicas de MMORPGs.**

</div>

---

## 🌟 O que é o GhostHUD?

O **GhostHUD** foi desenvolvido em **C puro (Win32 API)** com uma única premissa: **zero impacto de desempenho (0.0% de CPU)** e **zero bloatware**. 

Diferente de softwares pesados como MSI Afterburner ou GeForce Experience, o GhostHUD é um executável único e portátil de apenas **~40 KB**, sem Electron, sem .NET e sem instalador obrigatório. Ele roda instantaneamente em qualquer máquina Windows 10 ou 11.

---

## ⚡ Recursos Principais

### 📊 Monitoramento de Hardware Completo
- **CPU:** Uso percentual (%) calculado via `GetSystemTimes` do kernel do Windows.
- **GPU (NVIDIA & AMD):** Uso (%) e Temperatura (°C) em tempo real.
- **VRAM Dedicada:** Consumo percentual e em Gigabytes da memória de vídeo.
- **RAM do Sistema:** Uso percentual e memória utilizada/total via `GlobalMemoryStatusEx`.
- **Ping de Rede (ms):** Latência real medida assincronamente via ICMP em thread de background (sem travar a interface).
- **Consumo de Energia (PWR):** Medição do TDP real puxado pela placa de vídeo em **Watts**.
- **Relógio In-Game:** Hora atual integrada discretamente na barra de título.

### 🎮 Suporte Multi-Vendor (NVIDIA & AMD Radeon)
- **NVIDIA:** Integração nativa com a biblioteca oficial **NVML** (`nvml.dll`).
- **AMD Radeon:** Integração nativa com a biblioteca **AMD Display Library (ADL)** (`atiadlxx.dll`).
- Carregamento 100% dinâmico: se o PC não tiver GPU dedicada, o app abre sem erros e continua monitorando CPU, RAM e Rede.

### ⚙️ Totalmente Modular (Menu da Engrenagem)
- Clique no ícone de **Engrenagem [⚙]** no canto superior direito para ativar ou desativar qualquer métrica individualmente.
- **Redimensionamento Inteligente:** A janela recalcula sua largura e redistribui os slots automaticamente, sem deixar espaços vazios.

### 🎨 4 Temas Visuais MMORPG
- 🌑 **Obsidian Slate (Padrão):** Ardósia e carvão escuro inspirado em interfaces clássicas de fantasia.
- 🩸 **Blood Moon (Carmesim):** Tons góticos de vinho e vermelho escuro.
- ⚡ **Cyber Neon (Ciano / Roxo):** Preto profundo com detalhes em ciano elétrico.
- 👑 **Royal Gold (Ouro Metálico):** Fundo ônix com acabamento em ouro nobre.

### 📐 Modos de Exibição
- **3 Escalas de Tamanho:** `Pequeno (Compacto)`, `Médio` e `Grande`.
- **2 Orientações:** 
  - **Horizontal:** Barra superior/inferior.
  - **Vertical:** Coluna lateral, ideal para fixar no canto do monitor como status bar de RPG.

### 🧲 Ergonomia Gamer Avançada
- **Snap Magnético nas Bordas:** Ao arrastar para perto do canto do monitor, o HUD "gruda" suavemente a 8px da borda para alinhamento milimétrico.
- **Modo Fantasma / Click-Through (`F10`):** Torna o HUD transparente aos cliques do mouse (`WS_EX_TRANSPARENT`). Suas miras, habilidades e cliques passam direto para o jogo.
- **Ajuste de Opacidade no Scroll:** Role a rodinha do mouse sobre o HUD para dosar a transparência (de 40% a 100%).
- **Auto-Fade:** O HUD esmaece para 30% quando o cursor está longe e acende ao se aproximar.
- **Alerta Térmico Crítico:** Se a GPU atingir 82°C+, a borda ganha contorno carmesim; a partir de 85°C, emite um bipe sutil de aviso sonoro.
- **Iniciar com o Windows:** Alternância direta no menu para inicialização com o sistema no registro do usuário.

---

## 🕹️ Tabela de Controles e Atalhos

| Ação | Como Utilizar |
| :--- | :--- |
| **Mover o HUD** | Clique com botão esquerdo e arraste (com alinhamento magnético) |
| **Menu de Configurações** | Clique na **Engrenagem [⚙]**, botão direito no HUD ou ícone da bandeja |
| **Alternar Modo Fantasma** | Tecla **`F10`** (ou no menu) |
| **Regular Transparência** | Role a rodinha do mouse (**Scroll**) sobre o HUD |
| **Minimizar para Bandeja** | O ícone permanece ativo perto do relógio do Windows |

---

## 🛠️ Como Compilar

O projeto pode ser compilado com qualquer compilador C compatível com Win32. 

Recomendamos o **Tiny C Compiler (TCC)** para compilação instantânea (< 0.5 segundo) gerando um binário ultra-compacto:

```powershell
# 1. Compilação do executável com TCC
tcc src/main.c src/hardware.c src/ui.c -o GhostHUD.exe -lgdi32 -luser32 -lkernel32

# 2. Embutir o ícone multi-resolução diretamente no executável
python tools_inject_icon.py
```

*Ou usando o GCC / MinGW:*
```powershell
gcc -O3 src/main.c src/hardware.c src/ui.c -o GhostHUD.exe -lgdi32 -luser32 -lkernel32 -mwindows
```

---

## 🛡️ Segurança Anti-Cheat

O GhostHUD **não realiza injeção de DLL**, **não intercepta chamadas do DirectX/Vulkan** e não modifica a memória de nenhum processo. Ele opera puramente através das APIs oficiais do Windows Desktop (`WS_EX_LAYERED`, `WS_EX_TRANSPARENT`), sendo **100% seguro** contra qualquer sistema anti-cheat (Vanguard, Easy Anti-Cheat, BattlEye, Ricochet, etc.).

---

## 📄 Licença

Distribuído sob a licença [MIT](LICENSE). Desenvolvido por [Iury Almeida](https://github.com/IuryAlmeidaDev).
