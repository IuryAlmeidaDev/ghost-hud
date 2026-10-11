# GhostHUD Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Construir o GhostHUD — um monitor de hardware ultra-otimizado em C puro para Windows (< 4 MB de RAM, executável de ~60 KB, 0% CPU) com modo "click-through" e atalho global F9.

**Architecture:** Aplicação Win32 nativa em C (C99/C11). Módulo de hardware lê CPU via `GetSystemTimes`, RAM via `GlobalMemoryStatusEx` e GPU NVIDIA carregando `nvml.dll` dinamicamente em tempo de execução. Renderização em janela translúcida com GDI double-buffering. Alternância de click-through com `SetWindowLongPtr` e `WS_EX_TRANSPARENT`.

**Tech Stack:** C puro, Win32 API (kernel32, user32, gdi32, shell32), TCC / GCC.

**Spec:** `docs/superpowers/specs/2026-10-10-ghost-hud-design.md`

## Global Constraints
- Linguagem: C puro, sem dependências externas nem runtimes.
- Consumo de RAM: < 5 MB de RAM.
- Atalho global: tecla F9 para alternar modo jogo (click-through) e modo ajuste.
- Binário único executável: `GhostHUD.exe`.

---

### Task 1: Módulo de Leitura de Hardware em C (CPU, RAM e GPU NVML)

**Files:**
- Create: `src/nvml_lite.h`
- Create: `src/hardware.h`
- Create: `src/hardware.c`
- Create: `tests/test_hardware.c`

**Interfaces:**
- `HardwareMetrics` struct: `int cpu_usage`, `int gpu_usage`, `int gpu_temp`, `int ram_usage`
- `void hardware_init(void)`
- `void hardware_update(HardwareMetrics *out_metrics)`
- `void hardware_cleanup(void)`

- [ ] **Step 1: Criar `src/nvml_lite.h` com typedefs essenciais da NVML**
- [ ] **Step 2: Implementar `src/hardware.h` e `src/hardware.c`**
- [ ] **Step 3: Criar teste de validação de métricas `tests/test_hardware.c`**
- [ ] **Step 4: Compilar e executar teste com TCC para verificar leitura do hardware real**
- [ ] **Step 5: Commit**

---

### Task 2: Motor de Renderização GDI e UI Minimalista

**Files:**
- Create: `src/ui.h`
- Create: `src/ui.c`

**Interfaces:**
- `void ui_init(HWND hWnd)`
- `void ui_render(HDC hdc, int width, int height, const HardwareMetrics *metrics, BOOL locked)`
- `void ui_cleanup(void)`

- [ ] **Step 1: Implementar `src/ui.h` e `src/ui.c` com desenho GDI double-buffered**
- [ ] **Step 2: Implementar paleta de cores térmicas e fontes**
- [ ] **Step 3: Commit**

---

### Task 3: Ciclo de Vida Win32, Hotkey F9, Tray Icon e Persistência

**Files:**
- Create: `src/main.c`

**Interfaces:**
- `WinMain` com registro de janela, `CreateWindowExW`, `RegisterHotKey(hWnd, 1, 0, VK_F9)`, `SetTimer(hWnd, 1, 1000, NULL)`
- Suporte a arrastar no modo ajuste e click-through com `WS_EX_TRANSPARENT` no modo jogo
- Menu do System Tray com `Shell_NotifyIconW`
- Persistência de coordenadas X, Y em `ghosthud.ini`

- [ ] **Step 1: Implementar `src/main.c` completo**
- [ ] **Step 2: Validar compilação do aplicativo completo**
- [ ] **Step 3: Commit**

---

### Task 4: Script de Compilação Automatizada e Geração do Executável

**Files:**
- Create: `build.bat`
- Produce: `GhostHUD.exe`

- [ ] **Step 1: Criar `build.bat` para compilar o executável com 1 clique**
- [ ] **Step 2: Compilar e gerar `GhostHUD.exe` nativo**
- [ ] **Step 3: Validar tamanho do executável (< 100 KB) e execução**
- [ ] **Step 4: Commit**

---

### Task 5: Documentação e Publicação no GitHub

**Files:**
- Create: `README.md`
- Create: `.gitignore`

- [ ] **Step 1: Criar `.gitignore` e `README.md` em padrão showcase**
- [ ] **Step 2: Criar repositório `IuryAlmeidaDev/ghost-hud` no GitHub e fazer push**
