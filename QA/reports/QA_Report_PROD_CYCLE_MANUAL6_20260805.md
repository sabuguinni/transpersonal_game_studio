# QA Report — PROD_CYCLE_MANUAL6_20260805
**Agente:** #18 QA & Testing  
**Data:** 2026-08-05  
**Modo:** EDITOR (PIE não activo — 9 checks de camadas SKIP)

---

## 1. BLOCO VERBATIM DO AUDIT

```
PGA:modo=resumo mundo=EDITOR actores=3452
PGA:PRE /Game/Maps/MinPlayableMap          actores=3027   minimo=2500   OK
PGA:PRE /Game/Terrain/Terrain_Savana       actores=425    minimo=400    OK
PGA_RESUMO v1 global=FAIL fail=5 warn=4 skip=12 erro=0 modo=EDITOR spec_md5=a28f562f5e8f tempo=0.93s pre=OK prepare_ha=14380s
DEFEITOS_DIFERIDOS: 12 checks, 144 ocorrencias
CAMADAS_0_2: SKIP - o audit correu fora do PIE, nenhuma das 9 e medivel
RELATORIO=/root/transpersonal_game_studio/Saved/playability_report_20260805_214415.json
SKIP R03    atual=null base={"existe": true, "classe": "BP_Transpers
FAIL R10    atual=4 base=0 | Char_HeroHub_BestMatch_001; Human_Hub_001; Character_SkeletalMesh_Hub_001
WARN R11    atual=4 base=0 | Char_HeroHub_BestMatch_001; Human_Hub_001; Character_SkeletalMesh_Hub_001
FAIL R14    atual=3 base=0 | Character_SkeletalMesh_Hub_001(+30); CharProxy_SkeletalBestMatch_001(+60); Dino_Parasaurolophus_6(+50)
FAIL R16    atual=2 base=1 | FireCrackleLayer_Hub_001 x2; FX_CampfireSmoke_Audio_CampfireCrackle_H
WARN R18    atual=33 base=27 | Veg_Jungle_028(+52); Veg_Jungle_053(+307); Veg_Jungle_026(+353)
WARN R20    atual=47 base=43 | Char_HeroHub_BestMatch_001(SLOT_VAZIO); Human_Hub_001(SLOT_VAZIO); Character_SkeletalMesh_Hub_001(SLOT_VAZI
MANUAL R26    atual=null base="presente"
DELEGADO R28r   atual=null base={"watchdog": "activo", "kill_switch": "t
FAIL R29    atual=2 base=0 | Dino_Parasauro<->Dino_Parasauro(19m); Crowd_Para_003<->Dino_Parasauro(16m)
FAIL R38    atual=1 base=0 | RecastNavMesh-Default/NavMeshRenderingCo
WARN R36    atual=20 base=15 | Rock_Savana_070(-1964); EnvProp_ForestClearing_ImportedMatch_001; Rock_Hub_005(-67)
SKIP P01    atual=null base={"ok": true}
SKIP P02    atual=null base={"ok": true}
SKIP P03    atual=null base={"ok": true}
SKIP P04    atual=null base={"ok": true}
SKIP P05    atual=null base={"ok": true}
SKIP P06    atual=null base={"ok": true}
SKIP P07    atual=null base={"ok": true}
SKIP P08    atual=null base={"ok": true}
SKIP P09    atual=null base={"ok": true}
PGA_RESUMO_BYTES=1844 TETO=2000
PGA:FIM
```

---

## 2. CHECKS NÃO-PASS — ACTORES E TENDÊNCIA

### FAIL R10 — Criaturas em bind-pose (atual=4, base=0) ↑ SUBIU
**Actores:** `Char_HeroHub_BestMatch_001`, `Human_Hub_001`, `Character_SkeletalMesh_Hub_001`  
(4 ocorrências, 3 labels visíveis — um dos labels aparece duas vezes ou há um 4.º não truncado)  
Tendência: **subiu** — no ciclo anterior (MANUAL4) R10=4 também. Não piorou neste ciclo mas não foi corrigido.  
**Nota:** `Character_SkeletalMesh_Hub_001` foi o anchor usado pelo VFX Agent #17 neste ciclo — a sua presença em R10 é pré-existente, não causada pelo VFX.

### FAIL R14 — Pata flutuante (atual=3, base=0) ↑ SUBIU vs base
**Actores:**  
- `Character_SkeletalMesh_Hub_001` (+30 uu acima do terreno)  
- `CharProxy_SkeletalBestMatch_001` (+60 uu acima do terreno)  
- `Dino_Parasaurolophus_6` (+50 uu acima do terreno)  
Tendência: **estável** — os mesmos 3 actores reportados em MANUAL4. Nenhum movido neste ciclo (regra HANDS OFF Dino_).

### FAIL R16 — Duplicados de FX (atual=2, base=1) ↑ SUBIU
**Actores:** `FireCrackleLayer_Hub_001` (×2), `FX_CampfireSmoke_Audio_CampfireCrackle_H` (truncado)  
Tendência: **subiu de 1 para 2** — regressão introduzida neste ciclo ou no anterior. O VFX Agent reportou `FX_CampfireSmoke_Campfire_Hub_010` como possível órfão com nome violando naming rule.

### FAIL R29 — Espaçamento insuficiente entre criaturas (atual=2, base=0) ↑ SUBIU
**Pares:**  
- `Dino_Parasauro <-> Dino_Parasauro` (19 m — abaixo do mínimo de 20 m para criaturas estáticas)  
- `Crowd_Para_003 <-> Dino_Parasauro` (16 m)  
Tendência: **estável** — reportado em MANUAL4. Criaturas móveis podem aproximar-se; verificar se são móveis com raio_casa definido.

### FAIL R38 — NavMesh rendering component visível (atual=1, base=0) ↑ SUBIU
**Actor:** `RecastNavMesh-Default/NavMeshRenderingCo` (componente de debug do navmesh visível em jogo)  
Tendência: **novo neste ciclo** — não estava em MANUAL4. Possível que o NavMesh tenha sido reconstruído ou que o componente de rendering tenha ficado activo.

### WARN R11 — Actores sem malha (atual=4, base=0) ↑
**Actores:** `Char_HeroHub_BestMatch_001`, `Human_Hub_001`, `Character_SkeletalMesh_Hub_001`  
Correlacionado com R10 — os mesmos actores sem animação também não têm malha válida.

### WARN R18 — Vegetação flutuante (atual=33, base=27) ↑ SUBIU
**Actores com maior desvio:** `Veg_Jungle_026` (+353 uu), `Veg_Jungle_053` (+307 uu), `Veg_Jungle_028` (+52 uu)  
Tendência: **subiu de 27 para 33** — 6 novas ocorrências desde a base.

### WARN R20 — Actores com material genérico/slot vazio (atual=47, base=43) ↑ SUBIU
**Actores:** `Char_HeroHub_BestMatch_001` (SLOT_VAZIO), `Human_Hub_001` (SLOT_VAZIO), `Character_SkeletalMesh_Hub_001` (SLOT_VAZIO)  
Tendência: **subiu de 43 para 47** — 4 novas ocorrências.

### WARN R36 — Actores fora do núcleo jogável ou com z anómalo (atual=20, base=15) ↑ SUBIU
**Actores:** `Rock_Savana_070` (-1964 uu — enterrado), `EnvProp_ForestClearing_ImportedMatch_001`, `Rock_Hub_005` (-67 uu)  
Tendência: **subiu de 15 para 20** — 5 novas ocorrências.

---

## 3. DEFEITO PRIORITÁRIO — O QUE MAIS CUSTA AO JOGADOR

**FAIL R14 — `Dino_Parasaurolophus_6` flutuando +50 uu acima do terreno**

| Campo | Valor |
|---|---|
| **Label** | `Dino_Parasaurolophus_6` |
| **Check** | R14 (pata mais baixa vs terreno) |
| **Valor actual** | +50 uu acima do terreno (pata no ar) |
| **Valor alvo** | ≤ 5 uu (pata assente no terreno) |
| **Agente responsável** | #10 Animation Agent / #05 World Generator (assentamento de criaturas) |
| **Regra aplicável** | HANDS OFF — QA não move. O agente dono deve: medir a pata mais baixa com `get_num_bones + get_bone_name + get_socket_location`, fazer line trace ao Landscape ignorando todos os não-Landscape, e reposicionar pela pata, não pela origem do actor. |

**Justificação da prioridade:** `Dino_Parasaurolophus_6` é uma criatura com label `Dino_` — visível ao jogador, no núcleo jogável, flutuando 50 uu (≈0,5 m) no ar. É o defeito mais imediatamente perceptível ao olho humano e o único que combina visibilidade directa + violação de R14 (BLOCKER threshold). Os actores `Character_SkeletalMesh_Hub_001` e `CharProxy_SkeletalBestMatch_001` também flutuam mas são proxies/placeholders sem malha real (R10+R11 confirmam bind-pose/sem malha) — o impacto visual é menor.

---

## 4. O QUE NÃO FOI MEDIDO E PORQUÊ

| Check | Razão |
|---|---|
| **R03** (jogador no spawn) | SKIP — PIE não activo. Não medível em modo EDITOR. |
| **P01–P09** (camadas de gameplay) | SKIP — todos requerem PIE activo. |
| **R26** (manual) | `atual=null` — requer intervenção humana ou ferramenta externa não disponível no audit automático. |
| **R28r** (watchdog) | `atual=null` — delegado a sistema externo; o audit não tem acesso ao estado do watchdog neste contexto. |
| **C01** (criaturas visíveis do spawn) | Não aparece no resumo — provavelmente SKIP por PIE inactivo ou INFO (não bloqueia). |
| **V1-C25 / V1-C25b** (movimento de Dino_) | Não aparece no resumo — provavelmente medido internamente mas não reportado como FAIL (nenhum Dino_ foi movido neste ciclo). |

---

## 5. O QUE NÃO TOQUEI (HANDS OFF)

- Nenhum actor `Dino_*` foi movido, rodado, reescalado ou apagado.
- O Landscape, foliage e sublevel `Terrain_Savana` não foram tocados.
- O pawn `BP_TranspersonalPlayer` e o `BP_TranspersonalGameMode` não foram alterados.
- O PlayerStart em (1200,1200,301) não foi movido.
- A câmara do editor não foi alterada.
- O `vision_loop.py` não foi tocado.
- O sol (`Sun_Main_Directional`) não foi alterado.
- Nenhum ficheiro `.cpp` ou `.h` foi criado.

---

## 6. ESTADO DO GATE

**global=FAIL** — 5 FAILs activos. A build NÃO passa o gate neste estado.

FAILs bloqueantes:
1. **R10** — 4 criaturas em bind-pose (inclui `Character_SkeletalMesh_Hub_001` recém-usado pelo VFX Agent)
2. **R14** — 3 actores com pata flutuante (inclui `Dino_Parasaurolophus_6`)
3. **R16** — 2 duplicados de FX (regressão de naming/duplicação)
4. **R29** — 2 pares de criaturas demasiado próximas
5. **R38** — NavMesh rendering component visível em jogo

**Entrega ao #19 Integration & Build Agent:** O mundo está em FAIL. Reportar ao #01 para decisão sobre os 5 FAILs antes de integração.
