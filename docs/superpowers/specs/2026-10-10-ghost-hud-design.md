# Especificação de Design: GhostHUD (Pure C Ultra-Lightweight Gamer HUD)

## 1. Visão Geral e Filosofia do Projeto
O **GhostHUD** é um monitor de desempenho de hardware ultra-otimizado desenvolvido em **C puro (Win32 API Nativa)**.
O nome **GhostHUD** reflete suas três propriedades fundamentais:
1. **Invisível no Consumo:** Consome apenas **~2 MB a 4 MB de memória RAM** e **0.0% de CPU**.
2. **Invisível no Jogo:** Modo "Click-Through" que faz o mouse passar direto através do HUD como um fantasma, nunca interceptando miras ou cliques.
3. **Invisível para Anti-Cheats:** Zero injeção de DLL, zero hooks de DirectX/Vulkan; apenas uma janela overlay do Windows.

---

## 2. Princípios de Design & UI/UX (Diretrizes Impeccable)
1. **Mini Cápsula HUD Flutuante:**
   - Dimensões compactas: ~260px de largura por 56px de altura.
   - Cantos arredondados desenhados com GDI double-buffering (`RoundRect`).
   - Fundo escuro translúcido com canal alfa per-pixel (`UpdateLayeredWindow` ou `SetLayeredWindowAttributes`).
   - Tipografia limpa de largura fixa (`Consolas` ou `Segoe UI`) para estabilidade numérica.
2. **Código de Cores Térmicas Inteligente:**
   - 🟢 Verde (#00E676): Carga e temperatura normais (< 72°C).
   - 🟡 Âmbar (#FFAB00): Carga alta / temperatura elevada (72°C a 82°C).
   - 🔴 Vermelho Coral (#FF5252): Temperatura crítica (> 83°C).
3. **Modos Operacionais:**
   - **Modo Jogo (Padrão, Travado):** Flag `WS_EX_TRANSPARENT` ativa. Cliques passam 100% para o jogo.
   - **Modo Ajuste (Destravado):** Flag `WS_EX_TRANSPARENT` removida; borda ciano suave ativa. Usuário pode clicar e arrastar o HUD com o mouse para qualquer posição da tela.
   - **Alternância:** Tecla de atalho global `F9` (`WM_HOTKEY`).

---

## 3. Arquitetura Técnica em C Puro (Zero Dependências Externas)

### 3.1 Coleta de Métricas de Hardware
- **CPU:** Calculado via função nativa do kernel do Windows `GetSystemTimes(&idleTime, &kernelTime, &userTime)`.
  - Compara a diferença de tempos em um intervalo de 1000ms:
  - `CPU% = (1.0 - (idle_delta / (kernel_delta + user_delta))) * 100.0`.
  - Zero sobrecarga, precisão exata do Gerenciador de Tarefas do Windows.
- **RAM:** Calculado via `GlobalMemoryStatusEx(&memInfo)`.
  - Retorna uso percentual da memória física do sistema instantaneamente.
- **GPU (NVIDIA RTX 3070 Ti):**
  - Carrega dinamicamente `C:\Windows\System32\nvml.dll` via `LoadLibraryA("nvml.dll")`.
  - Conecta com `GetProcAddress`:
    - `nvmlInit_v2`
    - `nvmlDeviceGetHandleByIndex_v2`
    - `nvmlDeviceGetUtilizationRates`
    - `nvmlDeviceGetTemperature`
    - `nvmlDeviceGetMemoryInfo`
    - `nvmlShutdown`
  - Se a DLL não estiver disponível, o HUD continua exibindo CPU e RAM graciosamente.

### 3.2 Gerenciamento de Janela & Renderização
- **Criação de Janela:**
  - `CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, ...)`
- **Renderização GDI Double-Buffered:**
  - `CreateCompatibleDC` + `CreateCompatibleBitmap` para evitar qualquer tipo de flickering na tela.
  - Desenha fundo arredondado escuro, divisores sutis e textos coloridos.
- **Click-Through Toggle:**
  - `SetWindowLongPtrW(hWnd, GWL_EXSTYLE, currentStyle | WS_EX_TRANSPARENT)`
- **Arrasto de Janela (Modo Ajuste):**
  - No `WM_LBUTTONDOWN`, dispara `ReleaseCapture(); SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);` permitindo arrasto suave pelo Windows.

### 3.3 Persistência & Bandeja do Sistema
- **Persistência de Configuração:**
  - Usa a API nativa do Windows `GetPrivateProfileIntA` e `WritePrivateProfileStringA` salvando as coordenadas em `ghosthud.ini`.
- **System Tray:**
  - `Shell_NotifyIconW` com menu de contexto rápido (Alternar F9, Resetar Posição, Sair).

---

## 4. Estrutura de Arquivos
```text
ghost-hud/
├── .gitignore
├── README.md
├── build.bat                  # Script de compilação em 1 clique
├── ghosthud.ini               # Arquivo de configuração persistente
├── docs/
│   └── superpowers/
│       ├── specs/
│       │   └── 2026-10-10-ghost-hud-design.md
│       └── plans/
│           └── 2026-10-10-ghost-hud-plan.md
└── src/
    ├── main.c                 # Entry point Win32 (WinMain, WndProc, Timer)
    ├── hardware.h             # Interface de leitura de hardware
    ├── hardware.c             # Implementação GetSystemTimes, GlobalMemory, NVML
    ├── ui.h                   # Interface de renderização GDI
    ├── ui.c                   # Desenho da cápsula, fontes, badges e métricas
    └── nvml_lite.h            # Definições mínimas dos tipos da NVML
```
