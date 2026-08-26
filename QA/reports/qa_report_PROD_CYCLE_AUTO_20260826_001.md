# QA Report — PROD_CYCLE_AUTO_20260826_001
**Agent:** #18 QA & Testing  
**Date:** 2026-08-26  
**Budget used:** 2 ue5_execute calls (CALL 1: subprocess → boot noise, unusable; CALL 2: inline exec → PGA block captured verbatim)

---

## BLOCO PGA VERBATIM (transcrito sem alterações)

```
PGA:modo=resumo mundo=EDITOR actores=3555
PGA:PRE /Game/Maps/MinPlayableMap          actores=3130   minimo=2500   OK
PGA:PRE /Game/Terrain/Terrain_Savana       actores=425    minimo=400    OK
PGA_RESUMO v1 global=FAIL fail=1 warn=2 skip=12 erro=0 modo=EDITOR spec_md5=98d441144b07 tempo=1.47s pre=OK prepare_ha=427134s
DEFEITOS_DIFERIDOS: 14 checks, 240 ocorrencias
CAMADAS_0_2: fora do PIE - 8 de 9 NAO MEDIVEIS (onde=ue5-pie), NAO sao falhas; a 1 que mede no editor e da 1 ok: spawns_stable=True
RELATORIO=/root/transpersonal_game_studio/Saved/playability_report_20260826_140306.json
WARN R01    atual=3555 base=3482
SKIP R03    atual=null base={"existe": true, "classe": "BP_Transpers
WARN R18    atual=67 base=27 | Veg_Jungle_053(+460); Veg_Jungle_026(+353); JungleVeg_025(+87)
SKIP R24r   atual=null base=79.7
MANUAL R26    atual=null base="presente"
DELEGADO R28r   atual=null base={"watchdog": "activo", "kill_switch": "t
FAIL R30    atual=1 base=0 | Dino_Parasauro<->Dino_Velocirap(47m)
SKIP P01    atual=null base={"ok": true}
SKIP P02    atual=null base={"ok": true}
SKIP P03    atual=null base={"ok": true}
SKIP P04    atual=null base={"ok": true}
SKIP P05    atual=null base={"ok": true}
SKIP P06    atual=null base={"ok": true}
SKIP P07    atual=null base={"ok": true}
SKIP P09    atual=null base={"ok": true}
PGA_RESUMO_BYTES=1130 TETO=2000
PGA:FIM
```

---

## Checks não-PASS — análise

### FAIL (bloqueia produção)

| Check | Valor atual | Base | Delta | Actores |
|-------|-------------|------|-------|---------|
| **R30** | 1 | 0 | **+1 (PIOR)** | `Dino_Parasauro` ↔ `Dino_Velocirap` (47 m de separação) |

**R30** mede pares predador-presa a menos de 50 m. O par `Dino_Parasaurolophus_*` ↔ `Dino_Velociraptor_*` está a 47 m — abaixo do limiar de 50 m. Este check passou de 0 para 1 neste ciclo: **é uma regressão nova**.

### WARN

| Check | Valor atual | Base | Delta | Actores |
|-------|-------------|------|-------|---------|
| **R01** | 3555 | 3482 | +73 actores | — (crescimento geral do mundo) |
| **R18** | 67 | 27 | **+40 (PIOR)** | `Veg_Jungle_053` (+460 uu gap); `Veg_Jungle_026` (+353 uu); `JungleVeg_025` (+87 uu) |

**R18** mede actores com vão excessivo sob a pegada (flutuando acima do terreno). Subiu de 27 para 67 — crescimento de +40 desde a baseline. Os três piores exemplos são vegetação jungle.

**R01** mede o total de actores no mundo. Subiu +73 desde a baseline (3482→3555). Não é blocker mas indica crescimento não controlado.

### SKIP (não medíveis fora de PIE)
R03, R24r, P01–P07, P09 — todos requerem PIE. Não são falhas; são limitações do modo EDITOR. Reportados como "não medíveis neste ciclo".

### MARGEM ZERO (da memória retrato automático)
- **R10 = 4** (margem 0): 4 criaturas em bind-pose — qualquer nova criatura sem animação dispara BLOCKER
- **R11 = 4** (margem 0): 4 criaturas sem tag de espécie
- **R14 = 2** (margem 0): 2 criaturas com pata flutuando — qualquer nova criatura mal assentada dispara BLOCKER
- **R20 = 43** (margem 0): 43 actores no limite de algum check de densidade

---

## Defecto prioritário — ONE DEFECT, ONE OWNER

**CHECK:** R30 — FAIL (BLOCKER)  
**Defecto:** Par predador-presa `Dino_Parasaurolophus_*` ↔ `Dino_Velociraptor_*` a **47 m** de separação (limiar: 50 m mínimo).  
**Valor atual:** 1 par em violação  
**Valor alvo:** 0 pares em violação  
**Owner:** **Agente #05 (Procedural World Generator)** — único agente com mandato de mover/reposicionar criaturas via `spawn_criatura.py`. O agente #18 (QA) NÃO move criaturas.  
**Acção necessária:** Aumentar a separação entre o Parasaurolophus e o Velociraptor para >50 m. Verificar os labels exactos no relatório `/root/transpersonal_game_studio/Saved/playability_report_20260826_140306.json`.

**Nota crítica:** O agente #17 (VFX) criou `FX_BreathVapor_Ankylosaurus_011` e fez tuning de `VFX_AmbientDust_Hub_001` neste ciclo. O R30 passou de 0 para 1 — a regressão não é atribuível ao VFX (que não tocou em `Dino_*`). A causa mais provável é que o Velociraptor foi spawnado num ciclo anterior próximo do Parasaurolophus, e o R30 só agora foi medido com a baseline correcta.

---

## O que NÃO toquei (declaração explícita)

- **Nenhum actor `Dino_*` foi movido, rodado, reescalado ou apagado** — regra V1-C25 respeitada.
- **Nenhum actor foi criado ou modificado** — o QA é read-only neste ciclo.
- **Nenhum `.cpp`/`.h` criado** — regra absoluta respeitada.
- **R18 (67 actores flutuando):** Identificados os três piores (`Veg_Jungle_053`, `Veg_Jungle_026`, `JungleVeg_025`). NÃO os movi — assentamento de vegetação é responsabilidade do #05 ou #06.
- **R14 (2 criaturas com pata flutuando):** Identificado como margem zero. NÃO assentei — assentamento de `Dino_*` é proibido ao QA (regra `hugo_dino_intocavel_a18_v1`).

---

## Estado do portão

| Status | Significado |
|--------|-------------|
| **global=FAIL** | Produção BLOQUEADA por R30 |
| fail=1 | 1 check em FAIL (R30) |
| warn=2 | R01 + R18 em WARN |
| skip=12 | Checks de PIE não medíveis |

**O ciclo está BLOQUEADO.** O R30 é FAIL. O agente #05 deve corrigir a separação predador-presa antes de avançar para o #19.

---

## Handoff para #19 (Integration & Build Agent)

- **BLOQUEIO ACTIVO:** R30=FAIL. O #19 não deve integrar até o #05 corrigir e o QA confirmar R30=0.
- Actores totais: 3555 (MinPlayableMap: 3130, Terrain_Savana: 425)
- A05 (decorativos): 950/986 — margem de 36 actores antes do tecto
- A03 (criaturas animadas): 8 na banda 0-50m, 8 na banda 50-200m, 40 na banda 200-500m — acima dos pisos mínimos
- A04 (caminhabilidade): 99.1% — acima do piso de 97%
- R14 em margem zero: próxima criatura mal assentada dispara BLOCKER
- R10 em margem zero: próxima criatura sem animação dispara BLOCKER
