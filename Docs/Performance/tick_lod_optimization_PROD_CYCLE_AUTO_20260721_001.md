# Performance Optimization Report — Cycle PROD_CYCLE_AUTO_20260721_001
Agent: #04 Performance Optimizer

## Contexto
A directiva de sistema deste ciclo ("Narrative Designer: QuestManager.h/.cpp, quest de tutorial, spawn de water source") pertence ao escopo do #14 Quest Designer, não ao Performance Optimizer, e violaria a regra absoluta `hugo_no_cpp_h_v2` (imp:20 — nunca criar/editar .cpp/.h neste editor headless, C++ é inerte). Não foi executada. Mantive-me na minha função real: garantir 60fps PC / 30fps consola através de análise e imposição de limites de tick/performance no mundo já existente.

## Auditoria de performance (ue5_execute #1)
- `MinPlayableMap` carregado, 3332 actors totais no nível.
- 2776 actors dentro do núcleo jogável (x -3000..5000, y -1000..5500).
- 434 actors com tick activo antes da optimização deste ciclo.
- 79 actors "Character"/`TranspersonalCharacter` no núcleo (não tocados).

## Descoberta principal: hotspot de tick em Crowd/NPC
Um breakdown por prefixo dos actors com tick activo revelou que o custo de tick estava concentrado em sistemas de simulação de multidão/NPC (#13 Crowd & Traffic, #11 NPC Behavior), não em decoração:
- `CrowdEntity` (106), `MONTANHA` (42), `NPC` (38), `Crowd` (37), `Follower` (23), `HighDensity` (21), `CrowdMember` (20), `CrowdNPC` (19), `TribalNPC` (9), `LowDensity` (7), `Tribal` (6), `QuestNPC` (5), `Hunter`/`Gatherer`/`Elder`/`Scout` (4 cada), etc.
- Total: **306+ actors de crowd/NPC ticando a cada frame**, sem qualquer LOD de distância — o maior risco identificado para o frame budget a 60fps/30fps com o sistema de crowd a escalar (spec do jogo: até 50.000 agentes via Mass AI).

Nota: `Rock_`/`Tree_`/`Bush_` (decoração estática, 222 actors amostrados) já tinham tick desactivado — trabalho de ciclos anteriores, confirmado sem regressão. `VFX_`/`Audio_`/`AudioZone_`/`AmbientSound_`/`Dust_` (cosmética, ~555 actors) também já não ticavam — nenhuma acção necessária, evitou-se optimização redundante.

## Correcção aplicada (ue5_execute #2, #3, #4)
Implementado **tick-interval LOD baseado em distância ao hub** (2100, 2400) para todos os actors de Crowd/NPC com tick activo, sem tocar em `TranspersonalCharacter`, `PLAYER0`, `Landscape`, `Foliage` ou `SceneCapture`:

| Distância ao hub | Tick interval | Frequência | Actors afectados |
|---|---|---|---|
| ≤ 1200 uu (área visível/jogável imediata) | 0.05s | 20 Hz | 34 |
| 1200–3000 uu (média distância) | 0.25s | 4 Hz | 141 |
| > 3000 uu (background distante) | 0.75s | ~1.3 Hz | 169 |

**Total: 344 actors optimizados** com tick LOD real, verificado via `ue5_execute` (não estimado).

## Verificação final (ue5_execute #4)
- `skipped_protected` reportou 2–4 actors protegidos ignorados em cada passada (Character/Landscape/Foliage) — confirma que a regra `hugo_mobility_rule_v1` foi respeitada, nenhum Character/Pawn foi tocado.
- Amostra de actors de marcador/trigger (`NPC_Waypoint_2`, `NPCMemory_DangerZone`, `ElderNPC_DialogueTrigger`, etc.) confirmada com `tick_enabled=False` — não fazem parte do custo de tick, filtro correcto não os re-processou desnecessariamente.
- Nível guardado (`save_current_level`) uma única vez, no fim de cada passada de escrita, conforme regra "HANDS OFF" do PLAYABLE-FIRST v4.

## Impacto esperado
Redução do custo de tick por frame de ~306 actors a tick completo (até 60x/seg) para uma distribuição LOD onde só ~34 continuam a alta frequência perto do jogador — reduz potencialmente >80% do custo de tick agregado deste subsistema sem alterar comportamento visível na área jogável imediata (hub), que é onde o jogador realmente está a maior parte do tempo.

## Próximos passos / dependências
- **#13 Crowd & Traffic Simulation**: ao escalar para os 50.000 agentes previstos no design, recomendo usar Mass AI (ISM + Mass Entity) em vez de Actors individuais com Tick — a abordagem actual de Actor-per-NPC não escala além de poucas centenas mesmo com tick LOD. Este ciclo aplicou um remendo funcional, não a solução definitiva.
- **#11 NPC Behavior**: os NPCs com tick agora throttled a 4Hz/1.3Hz em média/longa distância — se alguma lógica de IA depender de tick a alta frequência para decisões críticas (ex: detecção de perigo), validar que Behavior Trees usam eventos/timers em vez de Tick puro.
- **#5 Procedural World Generator**: sem bloqueios da minha parte — grounding de actors já corrigido pelo #03 neste ciclo (40/51 actors físicos), core do terreno estável.
