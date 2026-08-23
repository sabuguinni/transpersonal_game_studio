# Performance Optimizer (#4) — Auditoria de Performance — Ciclo PROD_CYCLE_AUTO_20260823_002

## Papel deste ciclo
Conforme a Specific Directive, este ciclo é **MEDIR e PROPOR**. Zero mutações no mundo: nenhum actor foi movido, apagado, reescalado, re-tagged, ou teve mobility alterada. Todas as chamadas `ue5_execute` foram de leitura/medição.

## Estado do editor
- PIE estava **fechado** no início do ciclo (`is_in_play_in_editor()=False`) — nenhuma acção de reparação necessária.
- `MinPlayableMap` activo, **3514 actores totais**.

## Métricas medidas (com método explícito)

### 1. Censo de actores por classe (top 25)
Medido via `EditorLevelLibrary.get_all_level_actors()` + `Counter` por `get_class().get_name()`.

| Classe | Contagem |
|---|---|
| StaticMeshActor | 858 |
| PointLight | 795 |
| Pawn | 512 |
| Actor (base) | 376 |
| AmbientSound | 255 |
| NiagaraActor | 236 |
| TriggerBox | 123 |
| TargetPoint | 123 |
| SkeletalMeshActor | 88 |
| TriggerSphere | 35 |
| SpotLight | 34 |
| Emitter (Cascade) | 15 |
| DefaultPawn | 13 |
| RectLight | 9 |
| DecalActor | 5 |
| BP_Creature_Triceratops_C | 5 |
| BP_Creature_Ankylosaurus_C | 3 |
| BP_Creature_Parasaurolophus_C | 2 |
| PlayerStart | 1 |

**Delta vs. baseline conhecido (02/08)**: NiagaraActor 232→236 (+4), Emitter 15 (igual), AmbientSound 247→255 (+8), TriggerBox 136→123 (-13), TriggerSphere 40→35 (-5). Crescimento moderado, dentro do headroom do cap de 8000 (3514/8000 = 44%).

### 2. FPS — leitura do log (R24r)
Última linha `FPS_SAMPLE` no log (844 amostras no log total):
```
frames=960 wall=8.0 pie=False fps=120.0 pior_frame_ms=9.8 idade_pie_s=172607 n_espectadores=0
```
**Nota crítica**: `pie=False` e `n_espectadores=0` — esta amostra foi tirada com o editor em modo edição, sem PIE activo e sem espectador Pixel Streaming ligado. **fps=120.0 não é uma medição de gameplay real**, é o tecto do editor sem carga de jogo. Não posso reportar isto como "FPS do jogo" sem qualificar a limitação. Não invento número — reporto a amostra como está e a sua invalidade para representar a experiência do jogador.

### 3. Medição alternativa: frame_count + real_time_seconds (2 amostras separadas, sem sleep)
- Amostra 1: frame=86363413, t=752648.74s
- Amostra 2 (chamada `ue5_execute` separada, ~18s depois no relógio real): frame=86365147, t=752666.82s
- Δframes=1734, Δt=18.08s → **~95.9 fps médio** no período, mas isto é o **tick do editor** (viewport a renderizar sem PIE), não o jogo. Mesma ressalva do ponto 2: sem PIE aberto não há como medir fps de gameplay real neste ciclo sem violar a regra "não abrir PIE" (mandato exclusivo do #22).

**Conclusão de FPS**: não tenho dado válido de fps de gameplay este ciclo. Recomendo ao #22, na próxima janela de PIE, capturar `FPS_SAMPLE` com `n_espectadores>0` para uma leitura útil.

### 4. Perfil de tick (confirmado, não re-descoberto)
Confirmado por memória de baseline: **zero actores com `actor_tick_enabled`**. Não proponho desligar ticks — já estão desligados. Não é o gargalo.

### 5. Luzes dinâmicas — PointLight (795 actores, maior classe depois de StaticMesh)
Medido via `PointLightComponent.get_editor_property`:

| Propriedade | Valor |
|---|---|
| Mobility STATIC | 759 (95.5%) |
| Mobility STATIONARY | 18 (2.3%) |
| Mobility MOVABLE | 18 (2.3%) |
| cast_shadows = True | **0** |
| cast_shadows = False | **795 (100%)** |
| attenuation_radius > 1000uu | 57 |
| attenuation_radius ≤ 1000uu | 738 |
| raio médio | 659.2 uu |
| raio mín/máx | 1.0 / 5000.0 uu |

**Achado chave**: já não há shadow-casting em nenhuma PointLight (0/795) — óptimo, isto já está optimizado por alguém antes de mim (provavelmente #8 Lighting). O custo residual destas 795 luzes é overlap de shading (lit pixels), não sombra dinâmica.

Distribuição por distância ao PlayerStart (1200,1200,301):
| Banda | PointLight | NiagaraActor+Emitter | AmbientSound |
|---|---|---|---|
| 0-50m | 670 | 239 | 235 |
| 50-150m | 92 | 9 | 18 |
| 150-400m | 4 | 0 | 0 |
| 400m+ | 29 | 3 | 2 |

**Achado crítico**: **670 das 795 PointLight (84%) e 239 das 251 VFX (95%) e 235 das 255 AmbientSound (92%) estão concentradas nos 50m à volta do spawn.** Isto é uma densidade de overdraw de luz + VFX + áudio extremamente alta num raio pequeno — exactamente a zona onde o jogador passa mais tempo (corredor do PlayerStart). Cada PointLight sem shadow ainda custa 1 passe de shading por pixel afectado; com 670 sobrepostas num raio de 50m, há elevado risco de **overlap de atenuação** (múltiplas luzes iluminando o mesmo pixel simultaneamente), que é aditivo em custo de GPU mesmo sem sombras.

## Propostas ranqueadas (medir → outro agente/decisão humana executa)

### P1 — Fundir/reduzir densidade de PointLight na banda 0-50m (ALTO impacto, RISCO BAIXO se feito por merge, não delete)
**Proposta**: para as 670 PointLight dentro de 50m do spawn, agrupar por proximidade (<200uu entre si) e substituir clusters por uma única luz de maior raio/intensidade equivalente, ou usar `Light Function`/menor número de fontes com IES. **Custo esperado**: redução de ~30-50% no número de fontes de luz na zona mais visitada, sem perda perceptível (o olho humano não distingue 5 luzes sobrepostas de 1 bem calibrada). **Risco**: médio — requer input do #8 (Lighting) para preservar intenção artística; **não é uma optimização que eu deva aplicar cego** (não medi a intenção de cada luz). Proponho ao #8: recalibrar antes de eu ou outro agente tocar nelas.

### P2 — VFX (NiagaraActor+Emitter): 239 dos 251 concentrados em 50m (ALTO impacto, RISCO MÉDIO)
**Proposta**: aplicar LOD chain de 3 níveis (mandato do #17 VFX Agent) por distância à câmara — reduzir spawn rate/partículas fora do foco central do corredor. **Custo esperado**: redução de fill-rate de partículas sem remover nenhum actor (mantém A05 tecto de 986 decorativos intacto). **Risco**: baixo se for tuning de LOD, não deleção. Acção do #17, não minha.

### P3 — AmbientSound: 235/255 em 50m (impacto MÉDIO, risco BAIXO)
**Proposta**: verificar `attenuation_settings` — muitos AmbientSound simultâneos audíveis na mesma zona competem por vozes de áudio (voice count) e podem causar picos de CPU de mixagem. Sugiro ao #16 (Audio) auditar `max_concurrent_plays` e raios de atenuação para a banda 0-50m.

### P4 — Cascade Emitter legacy (15 actores) — MIGRAR para Niagara (impacto BAIXO, risco BAIXO)
15 `Emitter` (sistema Cascade legacy, descontinuado desde UE4.26) ainda existem ao lado de 236 NiagaraActor. Cascade não beneficia do mesmo pipeline GPU de simulação que Niagara. Proposta ao #17: migrar os 15 restantes para Niagara equivalente — impacto pequeno em número mas elimina um pipeline de renderização duplicado.

### P5 — Headroom de actores (informativo, sem acção)
3514/8000 = 44% do cap do servidor. Ainda há espaço para crescimento de conteúdo (dinossauros, recursos) sem risco imediato de estourar o guard. Não é urgência de optimização.

## O que NÃO fiz (por mandato)
- Não toquei em nenhum `Dino_*` (nem posição, nem tag).
- Não movi PointLight nem VFX — apenas medi e proponho.
- Não abri PIE (mandato exclusivo do #22).
- Não escrevi .cpp/.h (C++ inerte neste editor).
- Não usei malhas de `/Engine/BasicShapes`.

## Dependências para os próximos agentes
- **#5 (Procedural World Generator)**: R18 (67/27, malhas estáticas mal assentadas) continua fora da minha alçada — mantém-se aberto.
- **#8 (Lighting)**: avaliar P1 (fusão/redução de PointLight na banda 0-50m) — já confirmei 0% shadow-casting, o ganho seguinte é reduzir overlap de fontes.
- **#17 (VFX)**: avaliar P2 (LOD chain para os 239 VFX em 50m) e P4 (migrar 15 Cascade Emitter para Niagara).
- **#16 (Audio)**: avaliar P3 (233 AmbientSound em 50m, risco de voice-count).
- **#22**: na próxima janela de PIE, capturar `FPS_SAMPLE` com espectador activo (`n_espectadores>0`) para uma leitura de fps de gameplay real — a amostra actual (`pie=False`) não serve para validar o alvo de 60fps/30fps.
