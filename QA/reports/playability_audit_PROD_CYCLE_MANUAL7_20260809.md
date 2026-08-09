# QA Report — PROD_CYCLE_MANUAL7_20260809
**Agent #18 — QA & Testing**
**Data:** 2026-08-09
**Budget:** 2 ue5_execute calls (CALL 1: subprocess attempt → UE engine boot noise, no PGA block; CALL 2: exec via unreal module → PGA block captured verbatim)

---

## BLOCO VERBATIM DO AUDIT (transcrito sem alterações)

```
PGA:modo=resumo mundo=EDITOR actores=3478
PGA:PRE /Game/Maps/MinPlayableMap          actores=3053   minimo=2500   OK
PGA:PRE /Game/Terrain/Terrain_Savana       actores=425    minimo=400    OK
PGA_RESUMO v1 global=FAIL fail=3 warn=5 skip=12 erro=0 modo=EDITOR spec_md5=4c67b98d03e2 tempo=1.04s pre=OK prepare_ha=2532s
DEFEITOS_DIFERIDOS: 14 checks, 350 ocorrencias
CAMADAS_0_2: SKIP - o audit correu fora do PIE, nenhuma das 9 e medivel
RELATORIO=/root/transpersonal_game_studio/Saved/playability_report_20260809_171744.json
WARN R01    atual=3478 base=3470
SKIP R03    atual=null base={"existe": true, "classe": "BP_Transpers
FAIL R16    atual=2 base=1 | Dino_Ankylosaurus_1 x2; FX_CampfireSmoke_Audio_CampfireCrackle_H
WARN R18    atual=30 base=27 | Veg_Jungle_028(+52); Veg_Jungle_053(+460); Veg_Jungle_026(+353)
WARN R24r   atual=73.5 base=79.7 | idade=0.70h; mediana_de_0_limpas=73.5; amostras=[]
MANUAL R26    atual=null base="presente"
DELEGADO R28r   atual=null base={"watchdog": "activo", "kill_switch": "t
FAIL R29    atual=1 base=0 | Dino_Tricerato<->Dino_Ankylosau(12m)
WARN R36    atual=17 base=15 | Rock_Savana_070(-1964); EnvProp_ForestClearing_ImportedMatch_001; Rock_Hub_006(-140)
FAIL V1-C25 atual={"n": 39, "movidos": 1} base={"n": 37, "movidos": 0} | Dino_Ankylosaurus_1 moveu 44524uu
WARN V1-C28 atual={"sanity_guard.py": "0881cb952119ca69c43b6eff8dc4a2f0", "pre base={"sanity_guard.py": "0881cb952119ca69c43
SKIP P01    atual=null base={"ok": true}
SKIP P02    atual=null base={"ok": true}
SKIP P03    atual=null base={"ok": true}
SKIP P04    atual=null base={"ok": true}
SKIP P05    atual=null base={"ok": true}
SKIP P06    atual=null base={"ok": true}
SKIP P07    atual=null base={"ok": true}
SKIP P08    atual=null base={"ok": true}
SKIP P09    atual=null base={"ok": true}
PGA_RESUMO_BYTES=1605 TETO=2000
PGA:FIM
```

---

## Checks não-PASS — actores e direcção

| Check | Status | Actores | Direcção vs ciclo anterior |
|-------|--------|---------|---------------------------|
| **R16** | FAIL | `Dino_Ankylosaurus_1` (duplicado x2); `FX_CampfireSmoke_Audio_CampfireCrackle_H` | **SUBIU** (base=1 → atual=2) |
| **R29** | FAIL | `Dino_Tricerato` ↔ `Dino_Ankylosau` a 12 m de distância | **SUBIU** (base=0 → atual=1) |
| **V1-C25** | FAIL | `Dino_Ankylosaurus_1` moveu **44524 uu** | **SUBIU** (base=movidos=0 → atual=movidos=1) |
| R01 | WARN | — (contagem global: 3478 vs base 3470) | Subiu |
| R18 | WARN | `Veg_Jungle_028` (+52 uu); `Veg_Jungle_053` (+460 uu); `Veg_Jungle_026` (+353 uu) | Subiu (base=27 → atual=30) |
| R24r | WARN | — (mediana 73.5 vs base 79.7) | Desceu |
| R36 | WARN | `Rock_Savana_070` (-1964 uu); `EnvProp_ForestClearing_ImportedMatch_001`; `Rock_Hub_006` (-140 uu) | Subiu (base=15 → atual=17) |
| V1-C28 | WARN | `sanity_guard.py` md5 alterado | — |

---

## Defecto prioritário — o que mais custa ao jogador

**Check:** V1-C25 (FAIL — BLOCKER de produção)
**Actor:** `Dino_Ankylosaurus_1`
**Valor actual:** moveu 44524 uu desde a posição registada no baseline
**Valor alvo:** movidos = 0 (desvio < 50 uu)
**Agente responsável:** #11 NPC Behavior Agent ou #12 Combat & Enemy AI Agent (quem moveu o actor)

**Descrição precisa:** O `Dino_Ankylosaurus_1` foi deslocado 44524 uu da sua posição canónica registada no baseline. Isto activa o check V1-C25 e trava a produção. O mesmo actor aparece duplicado (R16: atual=2, base=1), o que sugere que foi criado um segundo `Dino_Ankylosaurus_1` em vez de mover o original — ou o original foi movido e um clone ficou na posição antiga. A distância de 12 m ao Triceratops mais próximo (R29) é consequência directa desta posição errada.

**Acção necessária (pelo agente dono, não pelo QA):**
1. Verificar qual dos dois `Dino_Ankylosaurus_1` é o original (pelo path_name canónico).
2. Repor o actor na posição registada no baseline (ou apagar o duplicado).
3. Confirmar que R16 desce para 1, R29 para 0, e V1-C25 para movidos=0.

---

## O que não foi medido e porquê

- **CAMADAS_0_2 (P01–P09):** SKIP — o audit correu fora do PIE. Nenhuma das 9 métricas de performance/gameplay é medível sem PIE aberto. Não é um defeito novo; é a condição normal do mundo do editor.
- **R03:** SKIP — idem, requer PIE para verificar o pawn possuído.
- **R26, R28r:** MANUAL/DELEGADO — requerem verificação humana ou watchdog externo; não são mensuráveis por este agente neste ciclo.
- **R24r:** amostras=[] — o audit não correu em PIE, logo não há amostras de frame rate reais. O valor 73.5 é a mediana histórica, não uma medição deste ciclo.

---

## O que este agente escolheu NÃO tocar

- **Nenhum actor `Dino_*` foi movido, rodado, reescalado ou apagado.** (Regra absoluta: hugo_dino_intocavel_a18_v1)
- **Nenhum actor foi criado ou modificado.** O papel do QA neste ciclo é medir e reportar, não reparar.
- **Nenhum ficheiro .cpp/.h foi escrito.**
- O `FX_FootstepDust_DinoTriceratops5_001` criado pelo agente #17 foi detectado na contagem (3478 actores vs 3470 base, +8), mas não foi avaliado individualmente — não é um check do audit.

---

## Ficheiros produzidos

- `QA/reports/playability_audit_PROD_CYCLE_MANUAL7_20260809.md` (este ficheiro)

---

## Para o agente #19 (Integration & Build)

**Estado do portão: FAIL — 3 checks a falhar.**

Os 3 FAILs são todos relacionados com `Dino_Ankylosaurus_1`:
1. **V1-C25** — actor moveu 44524 uu (BLOCKER)
2. **R16** — actor duplicado (2 instâncias com o mesmo label)
3. **R29** — espaçamento insuficiente (12 m ao Triceratops vizinho)

**Recomendação:** antes de integrar, o agente responsável pelo `Dino_Ankylosaurus_1` deve repor o actor na posição canónica e eliminar o duplicado. Só depois o portão passa a PASS.

Os 5 WARNs (R01, R18, R24r, R36, V1-C28) não bloqueiam mas devem ser monitorizados.
