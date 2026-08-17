# QA Audit Report — PROD_CYCLE_AUTO_20260817_001
**Agent:** #18 QA & Testing  
**Date:** 2026-08-17  
**Budget used:** 2 ue5_execute calls (CALL 1: subprocess → boot noise only; CALL 2: inline exec → full PGA block captured)

---

## BLOCO PGA VERBATIM (transcrito sem alterações)

```
PGA:modo=dry-run mundo=EDITOR actores=3504
PGA:PRE /Game/Maps/MinPlayableMap          actores=3079   minimo=2500   OK
PGA:PRE /Game/Terrain/Terrain_Savana       actores=425    minimo=400    OK
PGA:CHECK    SEV    VER     ATUAL                    BASELINE    ALVO        CL
PGA:R01      WARN   WARN    3504                     3482        3818        DIV  <- 3504 > 3482
PGA:R02      BLOCK  PASS    {"n": 1, "label": "Playe {"n": 1, "l {"n": 1, "l    
PGA:R03      BLOCK  SKIP    null                     {"existe":  {"existe":     
PGA:R04      BLOCK  PASS    {"n": 1, "intensity": 60 {"n": 1, "i {"n": 1, "i    
PGA:R05r     BLOCK  PASS    {"n": 1, "intensity": 3. {"n": 1, "i {"n": 1, "i    
PGA:R06      BLOCK  PASS    0                        0           0              
PGA:R07      WARN   PASS    1                        1           1              
PGA:R08      INFO   INFO    94                       56          56             
PGA:R09      BLOCK  PASS    {"com_anim": 60, "total" {"com_anim" {"com_anim"    
PGA:R10      BLOCK  PASS    4                        4           0           DIF
PGA:R11      WARN   PASS    4                        4           0           DIF
PGA:R12      WARN   PASS    1                        1           0           DIF
PGA:R13      BLOCK  PASS    0                        0           0              
PGA:R14      BLOCK  PASS    2                        2           0           DIF
PGA:R15      BLOCK  PASS    0                        0           0              
PGA:R16      BLOCK  PASS    1                        1           0           DIF
PGA:R17      WARN   PASS    13                       13          0           DIF
PGA:R18      WARN   WARN    67                       27          0           DIF  <- 67 > 27
PGA:         exemplos: ['Veg_Jungle_053(+460)', 'Veg_Jungle_026(+353)', 'JungleVeg_025(+87)', 'JungleVeg_015(+138)', 'JungleVeg_010(+100)']
PGA:R19      BLOCK  FAIL    1                        0           0           DIF  <- 1 > 0
PGA:         exemplos: ['WorkSurface_Camp_001']
PGA:R20      WARN   PASS    43                       43          0           DIF
PGA:R21      BLOCK  PASS    48                       68          68          DIV
PGA:R22r     INFO   INFO    {"niagara": 236, "cascad {"niagara": {"niagara":    
PGA:R23      INFO   INFO    {"total": 839, "static": {"total": 8 {"total": 8    
PGA:R24r     INFO   PASS    81.2                     79.7        79.7        DIV
PGA:R25r     INFO   INFO    75                       80          80             
PGA:R26      WARN   MANUAL  null                     "presente"  "ausente"   DIF
PGA:R27r     WARN   PASS    []                       []          []             
PGA:R28r     WARN   DELEGADO null                    {"watchdog" {"watchdog"    
PGA:R29      BLOCK  PASS    0                        0           0              
PGA:R30      BLOCK  PASS    0                        0           0              
PGA:R31      BLOCK  PASS    1                        1           0           DIF
PGA:R32      BLOCK  PASS    1                        1           1           DIV
PGA:R38      BLOCK  PASS    0                        0           0              
PGA:R39      INFO   PASS    33                       33          36          DIF
PGA:R41      WARN   WARN    76.0                     80.2        0                <- 76.0 < 80.2
PGA:         exemplos: ['WorkSurface_Camp_001 x7', 'Rock_Floresta_401 x6', 'Rock_Savana_571 x4', 'Tree_Savana_001 x3', 'Tree_HubRing_014b x3']
PGA:R37      BLOCK  PASS    2                        2           0           DIF
PGA:R42      WARN   PASS    65                       65          0           DIF
PGA:R36      WARN   WARN    2                        1           0           DIF  <- 2 > 1
PGA:         exemplos: ['EnvProp_ForestClearing_ImportedMatch_001(-57)', 'WorkSurface_Camp_001(-132)']
PGA:R34      BLOCK  PASS    0                        0           0              
PGA:R35      BLOCK  PASS    0                        0           0              
```

---

## Checks NÃO-PASS — lista completa com actores e tendência

| Check | Sev | Status | Valor Actual | Baseline | Tendência | Actores exemplo |
|-------|-----|--------|-------------|----------|-----------|-----------------|
| **R01** | WARN | WARN | 3504 | 3482 | ↑ +22 vs baseline | — (contagem global de actores) |
| **R18** | WARN | WARN | 67 | 27 | ↑ +1 vs ciclo anterior (era 66) | `Veg_Jungle_053(+460)`, `Veg_Jungle_026(+353)`, `JungleVeg_025(+87)`, `JungleVeg_015(+138)`, `JungleVeg_010(+100)` |
| **R19** | BLOCK | **FAIL** | 1 | 0 | **NOVO FAIL** (era 0 no ciclo anterior) | **`WorkSurface_Camp_001`** |
| **R36** | WARN | WARN | 2 | 1 | ↑ +1 vs ciclo anterior (era 1) | `EnvProp_ForestClearing_ImportedMatch_001(-57)`, **`WorkSurface_Camp_001(-132)`** |
| **R41** | WARN | WARN | 76.0 | 80.2 | ↓ desceu (era ~76 no ciclo anterior) | `WorkSurface_Camp_001 x7`, `Rock_Floresta_401 x6`, `Rock_Savana_571 x4`, `Tree_Savana_001 x3`, `Tree_HubRing_014b x3` |

### Checks em PASS mas com margem zero (DIF — ainda não atingiram o alvo):
- **R10** BLOCK/PASS: 4 criaturas em bind-pose (alvo 0) — margem 0
- **R11** WARN/PASS: 4 (alvo 0) — margem 0
- **R14** BLOCK/PASS: 2 criaturas com pata flutuante (alvo 0) — margem 0
- **R16** BLOCK/PASS: 1 (alvo 0) — margem 0
- **R31** BLOCK/PASS: 1 (alvo 0) — margem 0

---

## Defecto prioritário — o que custa mais ao jogador

### **R19 BLOCKER: `WorkSurface_Camp_001` — actor enterrado/suspenso com colisão incorrecta**

**Check:** R19 (BLOCK/FAIL) — actor com colisão que bloqueia o jogador mas está mal posicionado  
**Actor:** `WorkSurface_Camp_001`  
**Valor actual:** 1 (era 0 no baseline — **NOVO FAIL introduzido neste ciclo ou no anterior**)  
**Alvo:** 0  
**Agente responsável:** #06 Environment Artist (actor de prop de ambiente) ou #07 Architecture & Interior Agent  

**Porquê é o mais crítico:**
- É o único BLOCKER em FAIL — todos os outros BLOCKERs estão em PASS.
- O mesmo actor aparece em **R36** com valor -132 (enterrado 132 uu abaixo do terreno) E em **R41** como o actor com mais ocorrências de sobreposição (x7).
- Um actor enterrado com colisão activa cria uma parede invisível que o jogador não consegue atravessar — é a pior categoria de bug de navegação.
- Aparece em R41 com 7 ocorrências de sobreposição, o que sugere que está a colidir com outros actores próximos.

**Dados precisos para o agente que vai corrigir:**
- Label: `WorkSurface_Camp_001`
- R19 valor: 1 (FAIL, threshold 0)
- R36 valor: -132 uu (enterrado 132 uu abaixo do terreno — acima do limite de -50 uu que define "enterrado")
- R41: 7 sobreposições com outros actores
- Acção necessária: localizar o actor, medir o terreno sob a sua pegada (9 pontos AABB), calcular o Z correcto pelo mínimo dos 9 pontos, reposicioná-lo. Verificar que a colisão não sobrepõe outros actores funcionais.
- **Fórmula obrigatória (R18/R36):** `base = origin.z - extent.z; zs = [trace(x+fx*ex, y+fy*ey) for fx,fy in 9 pontos]; vao = base - min(zs); novo_z = loc.z - vao`

---

## R18 — Tendência e actores

R18 subiu de 66 (ciclo anterior) para **67** (+1). Os actores com maior gap são vegetação de jungle:
- `Veg_Jungle_053` (+460 uu suspenso)
- `Veg_Jungle_026` (+353 uu suspenso)
- `JungleVeg_025` (+87 uu)
- `JungleVeg_015` (+138 uu)
- `JungleVeg_010` (+100 uu)

Estes têm espalhamento > 200 uu (encostas íngremes) — conforme a regra, **não devem ser movidos por Z**. São dívida legítima aguardando decisão de escala/localização.

---

## R36 — Actor novo enterrado

R36 subiu de 1 para **2**. O actor novo é `WorkSurface_Camp_001` (-132 uu). O actor anterior `EnvProp_ForestClearing_ImportedMatch_001` (-57 uu) mantém-se.

---

## O que não foi medido e porquê

- **R03 (SKIP):** Requer PIE aberto — o audit corre em modo EDITOR e o pawn não existe. Não mensurável neste ciclo sem abrir PIE (proibido para o #18).
- **R28r (DELEGADO):** Watchdog externo — fora do escopo do audit inline.
- **R26 (MANUAL):** Requer verificação humana — não automatizável.
- **Posições vivas das criaturas móveis:** Não medidas — as 6 criaturas móveis estão em PIE e as posições do editor são as canónicas (creatures_registry).

---

## Declaração explícita: o que não foi tocado

O #18 é **read-only no mundo**. Nenhum actor foi movido, rodado, reescalado ou apagado. Nenhum Dino_ foi tocado. O audit foi executado em modo observação pura.

---

## Ficheiros produzidos

- `QA/reports/playability_audit_PROD_CYCLE_AUTO_20260817_001.md` (este ficheiro)

---

## Handoff para #19 Integration & Build Agent

**Estado global:** WARN (1 FAIL activo — R19 `WorkSurface_Camp_001`)  
**Bloqueio QA:** **SIM — R19 está em FAIL (BLOCKER).** O `WorkSurface_Camp_001` está enterrado 132 uu e tem colisão activa. Isto é uma parede invisível para o jogador.  
**Acção necessária antes de integrar:** O agente responsável pelo `WorkSurface_Camp_001` (provavelmente #06 ou #07) deve corrigir o assentamento deste actor usando a fórmula de 9 pontos (mínimo, não máximo). Após correcção, o R19 e R36 devem baixar para 0 e 1 respectivamente.  
**Tudo o resto:** Os BLOCKERs restantes estão em PASS. A build pode avançar condicionalmente se o `WorkSurface_Camp_001` for corrigido ou temporariamente removido.
