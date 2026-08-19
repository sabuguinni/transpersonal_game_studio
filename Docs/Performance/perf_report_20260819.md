# Performance Report — Ciclo PROD_CYCLE_AUTO_20260819_001
Agente #4 — Performance Optimizer

Papel deste ciclo: **MEDIR e PROPOR**, nunca executar. Todas as medições abaixo vêm de `ue5_execute` reais no editor UE5 ligado, com PIE fechado (`is_in_play_in_editor()=False`), mundo `MinPlayableMap`. Nenhum actor foi criado, movido, escalado ou apagado. Nenhum `Dino_*` foi tocado.

## 1. Censo de actores (verificado, `EditorActorSubsystem.get_all_level_actors()`)
| Métrica | Valor | Método |
|---|---|---|
| Total de actores | 3493 | contagem directa |
| StaticMeshActor | 845 | Counter por classe |
| PointLight | 795 | Counter por classe |
| Pawn | 512 | Counter por classe |
| Actor (genérico) | 375 | Counter por classe |
| AmbientSound | 252 | Counter por classe |
| **NiagaraActor** | **236** | Counter por classe |
| TriggerBox | 123 | Counter por classe |
| TargetPoint | 123 | Counter por classe |
| SkeletalMeshActor | 88 | Counter por classe |
| TriggerSphere | 35 | Counter por classe |
| SpotLight | 34 | Counter por classe |
| Emitter (Cascade) | 15 | Counter por classe |

Cap do servidor: 8000 actores. Ocupação actual: 3493/8000 = 43,7%. Headroom real mas finito, consistente com medição anterior (3746→3493, mundo estável/ligeiramente reduzido desde o último ciclo).

## 2. Tick por-actor — CONFIRMADO NÃO SER O GARGALO
Amostra de 500 actores lidos via `get_editor_property("actor_tick_enabled")`: **0 com tick activo**. Confirma a directiva: não proponho desligar ticks porque já estão todos desligados. Qualquer proposta de "optimizar tick" seria trabalho nulo.

## 3. FPS — duas fontes medidas, sem inventar números
- **Log do editor**, última linha `FPS_SAMPLE`: `frames=960 wall=8.0 pie=False fps=120.0 pior_frame_ms=9.1 n_espectadores=0`. Esta amostra é do estado sem PIE (editor parado), portanto **não representa o custo do jogo em execução** — é o chão (melhor caso).
- **Amostragem dupla de `SystemLibrary.get_frame_count()` + `GameplayStatics.get_real_time_seconds()`**, duas invocações `ue5_execute` separadas (nunca no mesmo tick, nunca com sleep): `Δframes=1501`, `Δtempo=12,754 s` → **117,69 fps médios de tick do editor** (inclui overhead do próprio editor, não é fps de jogo puro).
- **Conclusão honesta**: não tenho amostra fresca de fps **dentro de PIE** com jogador a mover-se — isso exige o PIE aberto, que não é meu mandato abrir (reservado ao #22). Reporto o que medi e não extrapolo para "60fps garantidos".

## 4. Luzes — custo real de iluminação dinâmica
Distribuição por mobility (838 luzes medidas em PointLight/SpotLight/RectLight):
| Tipo | Mobility | N |
|---|---|---|
| PointLight | STATIC | 759 |
| PointLight | MOVABLE | 18 |
| PointLight | STATIONARY | 18 |
| SpotLight | STATIC | 31 |
| SpotLight | STATIONARY | 3 |
| RectLight | STATIC | 9 |

**Leitura**: 795 PointLight no total, mas apenas 18 MOVABLE (custo de luz dinâmica real, recalculada todo o frame) e 18 STATIONARY (shadow dinâmica + lightmap). A esmagadora maioria (759+31+9=799) é STATIC — já pré-computada, custo de runtime próximo de zero. **Isto significa que "795 luzes" não é o número relevante de perigo**: o número relevante é 39 (MOVABLE+STATIONARY combinados), que é gerível. Se o Lighting Agent (#8) quiser mais luzes dinâmicas, o orçamento real está nesses 39, não nos 795.

## 5. VFX — o maior alvo de optimização identificado
- **236 NiagaraActor + 15 Emitter (Cascade) = 251 sistemas de partículas**, confirmando a memória de mundo.
- Apenas **7 sistemas distintos** são reutilizados 236 vezes:
  - `NS_Environment_CampfireSmoke` — **171 instâncias** (72% de todo o VFX do jogo)
  - `NS_Dino_Footstep` — 32
  - `NS_Environment_AmbientDust` — 15
  - `NS_Environmental_Wind` — 11
  - `NS_Atmospheric_DustMotes` — 3
  - `NS_Dust_Footstep_TRex` — 3
  - `NS_Fire_Campfire` — 1
- Todas as 171 instâncias de `NS_Environment_CampfireSmoke` **têm `ld_max_draw_distance` configurado** (6000–8000 uu) — não há sistemas "always-on" sem cull de distância. Isto é bom, já está parcialmente optimizado.
- **PROBLEMA MEDIDO: sobreposição espacial massiva.** Dos 14535 pares possíveis entre as 171 instâncias de campfire smoke, **4470 pares (30,7%) estão a menos de 500 uu um do outro**. Isto bate com o padrão de duplicação em cadeia já registado na memória do projecto (labels como `VFXAnchor_Campfire_Smoke_001`, `VFX_CampfireSmoke_ContentHub_001`, `VFX_Smoke_Campfire_Hub_006/007/010...` — nomes que embrulham conceitos uns nos outros, o anti-padrão documentado em `hugo_asset_first_v8`).

## 6. Ranked proposals (não executadas — para decisão do Hugo / próximo agente)

### P1 — ALTA prioridade, BAIXO risco: consolidar campfire smoke duplicado
- **O quê**: dos 171 `NS_Environment_CampfireSmoke`, ~30% estão empilhados a <500uu de outro da mesma malha/efeito. Proponho que o próximo agente que toque em VFX (não eu, respeito o tecto A05=986 decorativos) faça uma auditoria de labels e **elimine duplicados exactos na mesma posição** (não decoração nova — remoção de lixo de duplicação).
- **Custo esperado**: reduzir de 171 para uma estimativa de ~120-140 instâncias reais únicas pode cortar 20-30% do custo de simulação de partículas de fumo sem perda visual (o fumo sobreposto não é visível como "mais fumo", é desperdício).
- **Risco**: BAIXO se feito por label exacto e verificação de coordenada idêntica (raio <50uu = duplicado real; 50-500uu pode ser intencional/parte de composição de fogueira maior — não assumir).
- **Quem executa**: não eu (optimizer não move/apaga; é candidato ao #17 VFX Agent ou #19 Integration, respeitando A05).

### P2 — MÉDIA prioridade, BAIXO risco: distância de cull mais agressiva para campfire smoke fora do corredor visível
- **O quê**: os 8000uu de `ld_max_draw_distance` cobrem uma área maior que o corredor do PlayerStart (~150m / 15000uu segundo a memória de espaçamento). Reduzir para 4000-5000uu nas instâncias fora do corredor visível do spawn (1200,1200) corta simulação sem impacto visual porque o jogador nunca as vê de perto.
- **Custo esperado**: baixo mas real — cull de distância é uma operação de bounding-box, praticamente grátis de aplicar, e reduz partículas activas simultaneamente.
- **Risco**: BAIXO — não altera posição, malha, nem contagem de actores. É um ajuste de propriedade LOD.

### P3 — BAIXA prioridade (já bem gerido): luzes dinâmicas
- **O quê**: nada a propor agora — apenas 39 de 838 luzes são dinâmicas (MOVABLE+STATIONARY). Se o Lighting Agent quiser adicionar luzes dinâmicas novas (ex.: para o pedido "500 luzes dinâmicas" citado na minha directiva), a resposta é: **converter para STATIONARY sempre que a luz não precisa de se mover** (ex.: tochas fixas), reservando MOVABLE só para o que realmente muda (dia/noite, luzes de personagem). Orçamento sugerido: manter dinâmicas (MOVABLE) abaixo de 50 no total do nível visível de cada vez, usando streaming/sublevels para luzes fora do corredor activo.

### P4 — INFO, sem acção: fps não medido em condição de jogo real
- O único fps disponível é do editor sem PIE (120fps, chão optimista) e do tick do próprio editor (117,69fps, inclui overhead de UI do editor). **Não tenho uma amostra válida de fps em PIE com jogador activo** — isso precisa do #22 (único mandato para abrir PIE) para produzir uma amostra `FPS_SAMPLE` fresca com `pie=True` e `n_espectadores>0`. Recomendo ao próximo agente que corra o `prepare_game_for_play` e eu meça de novo no próximo ciclo.

## 7. O que NÃO fiz (por directiva)
- Não movi, apaguei ou reescalei nenhum actor `Dino_*`.
- Não toquei em Landscape, Terrain_Savana, câmara do viewport ou no pawn do jogador.
- Não escrevi `.cpp`/`.h` — C++ é inerte neste editor headless.
- Não spawnei nenhuma malha de `/Engine/BasicShapes`.
- Não abri PIE (mandato exclusivo do #22).

## 8. Handoff — próximo agente (#5 Procedural World Generator)
- Mundo estável em 3493 actores, headroom de 4507 até ao cap de 8000.
- R14 (pata do Ankylosaurus_10) foi corrigido/verificado como zero pelo #3 neste ciclo — grounding OK.
- Se o #5 gerar mais geografia/biomas, o orçamento de VFX já está no limite prático de eficiência (72% do VFX é um único efeito repetido 171x, 30% dele redundante) — recomendo não aumentar densidade de campfire/decoração nessas zonas novas sem primeiro resolver P1.
