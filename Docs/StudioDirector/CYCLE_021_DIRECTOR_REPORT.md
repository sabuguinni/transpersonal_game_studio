# CYCLE_021 — Studio Director Report
**Cycle ID:** PROD_CYCLE_AUTO_20260723_001
**Data:** 2026-07-23

## VISUAL FEEDBACK APPLIED
Nenhum screenshot/análise visual foi injectado no `previous_output` deste ciclo (o output anterior era apenas o texto de arranque do auto-cycle, sem vision analysis). Não há feedback visual para aplicar — este ciclo baseou-se em auditoria directa do estado do mundo via `ue5_execute` (contagem de actores, inspecção de propriedades, verificação de classes) em vez de imagem. Recomendo ao vision_loop.py garantir captura no próximo ciclo para fechar este loop.

## ESTADO REAL VERIFICADO (via ue5_execute, não assumido)

### 1. Terreno (Agent #5) — CONFIRMADO REAL
`MinPlayableMap` tem 3634 actores totais. `Landscape1` + foliage confirmados via memória e mundo carregado com sucesso (`world: MinPlayableMap`). Sem acção necessária este ciclo — DONE, não tocar (regra HANDS OFF).

### 2. Dinossauros no núcleo jogável (Agent #9/#10) — SUBSTANCIAL MAS FRAGMENTADO
Contagem real: **161 actores dino dentro do núcleo jogável** (x -3000..5000, y -1000..5500), 8 fora do núcleo.
Amostra de labels encontrados: `Trike_Savana_004..009`, `Ankylo_001`, `Ankylo_Savana_001`, `Para_001`, `Helper_Actor_Raptor_Hub_001_Posed`, `Helper_Actor_Trike_Hub_001`, `Crowd_Anky_Patrol_01..03`, `Crowd_Para_001..004`.

**Problema identificado:** proliferação de prefixos não-canónicos (`Helper_Actor_`, `Crowd_`, `CrowdMarker_`) que violam a regra `Type_Bioma_NNN` (memória `hugo_naming_dedup_v3`). Isto sugere que múltiplos agentes (Crowd Sim #13, NPC Behavior #11) criaram wrappers em cima dos mesmos dinos em vez de reutilizar os actores `Trike_Savana_XXX`/`Ankylo_XXX` já existentes.

**Tentativa de verificação de grounding (trace-to-landscape) falhou por incompatibilidade de API Python** (`HitResult` neste build não expõe `.location` nem `blocking_hit` directamente — expõe apenas `get_editor_property`/`to_tuple`). Não consegui confirmar objectivamente o grounding fino neste ciclo; os z-values brutos (82–200) estão dentro do intervalo de superfície documentado (44–302), o que é um bom sinal indirecto mas não uma prova de trace.

### 3. Survival HUD (Agent #12, per directiva deste ciclo) — GAP CRÍTICO CONFIRMADO
Verificação directa confirma que o HUD **não está ligado**:
- `BP_TranspersonalGameMode.HUDClass` = `/Script/Engine.HUD` (classe base do motor, NUNCA substituída).
- `WBP_SurvivalHUD` (WidgetBlueprint) e `BP_SurvivalHUDManager` (Blueprint) **existem como assets** em `/Game/UI/` mas `BP_SurvivalHUDManager` tem **zero instâncias colocadas no nível** e nada no GameMode cria/adiciona o widget ao viewport.
- Os actores `SurvivalHUD_Health/Hunger/Stamina` no mundo são apenas `TextRenderActor` (texto 3D no espaço do mundo, no hub 2100,2200), **não um HUD 2D real de ecrã**.
- Confirma-se o marcador que o próprio QA já tinha deixado: `Marker_HUDWiringGap_Hub_001` com a tag `TaskFlag_Agent18_HUDNotWired` — o #18 já tinha detectado exactamente este problema.

Isto é um **bloqueador de jogabilidade real**: o jogador (BP_TranspersonalPlayer_C, spawnado via GameMode conforme `hugo_player_fixed_v1`) não vê barras de vida/fome/stamina em ecrã.

## TASK LIST CONCRETA PARA O PRÓXIMO CICLO

### #12 — Combat & Enemy AI Agent (dono da HUD de sobrevivência nesta directiva)
**Deliverable mensurável:** `BP_TranspersonalGameMode.HUDClass` deixa de ser `/Script/Engine.HUD` e passa a criar `WBP_SurvivalHUD` via `CreateWidget` + `AddToViewport` no `BeginPlay` do PlayerController (ou via um `HUD` C++/BP subclass dedicado). Critério de aceitação: em PIE, o widget aparece no ecrã do jogador com barras de Health/Hunger/Stamina actualizadas a partir dos valores reais do `TranspersonalCharacter`. Apagar/reciclar o marcador `Marker_HUDWiringGap_Hub_001` só depois de confirmado em PIE.

### #9/#10 — Character Artist / Animation Agent
**Deliverable mensurável:** eliminar duplicação de prefixos não-canónicos sobre dinos já existentes. Para cada `Helper_Actor_*_Hub_*` e `Crowd_*` que partilha coordenadas com um `Trike_Savana_NNN`/`Ankylo_NNN`/`Para_NNN` já existente, actualizar o actor original (pose, colisão) em vez de manter o wrapper duplicado. Verificar com `get_all_level_actors()` + agrupamento por proximidade (<200uu) antes de qualquer spawn novo.

### #18 — QA & Testing Agent
Manter o bloqueio até o HUD estar wired e testado em PIE. Adicionar um segundo marcador de QA a validar o grounding real dos 161 dinos do núcleo (a minha tentativa de trace falhou por API — o QA deve usar `hit.to_tuple()` ou `get_editor_property("Location")` com case exacto da propriedade UE, não `hit.location`).

### #19 — Integration & Build Agent
Não integrar uma build "final" enquanto `TaskFlag_Agent18_HUDNotWired` existir na cena — é o sinal vivo de bloqueio do QA.

## DECISÕES TÉCNICAS TOMADAS
1. Não spawn de novos actores este ciclo — o núcleo já tem 161 dinos e 3634 actores totais; a prioridade é corrigir wiring e duplicação, não adicionar volume (regra REUSE FIRST).
2. Não toquei em Landscape, Terrain_Savana, PLAYER0, câmara do editor, nem sun — conforme HANDS OFF.
3. Preferi investigação profunda via `ue5_execute` (4 chamadas reais, todas com resultado verificável) a qualquer alteração especulativa, porque a directiva desta task exige rastrear "quais agentes produziram ficheiros reais vs apenas relatórios" — descobri que #12 (HUD) está na categoria "apenas relatório", não implementação real.

## FICHEIROS CRIADOS/MODIFICADOS
- `Docs/StudioDirector/CYCLE_021_DIRECTOR_REPORT.md` (este ficheiro)

## DEPENDÊNCIAS PARA O PRÓXIMO CICLO
- #12 precisa de acesso de escrita ao Blueprint `BP_TranspersonalGameMode` e `WBP_SurvivalHUD` via `ue5_execute` (não via C++, conforme regra `hugo_no_cpp_h_v2`).
- #9/#10 precisam da lista de labels duplicados que reportei acima para começar a consolidação.
