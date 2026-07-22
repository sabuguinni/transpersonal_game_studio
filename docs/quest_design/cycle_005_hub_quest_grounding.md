# Quest & Mission Designer — Cycle PROD_CYCLE_AUTO_20260722_005

## Contexto
Ciclo anterior (#13 Crowd & Traffic Simulation) reagrupou o herd do hub (6x Triceratops + 2x Ankylosaurus) e sinalizou avisos:
- `QuestMarker_Q3_GatherBerries` (z=40.0) e `StampedeTrigger_HerbivoreHerd` (z=26.3) — abaixo do limite mínimo válido z=44.
- Herds de Parasaurolophus/Tsintaosaurus continuam a flutuar (fora do meu escopo, entregue ao próximo ciclo de Crowd/Combat AI).

## Auditoria prévia (regra `hugo_naming_dedup_v3`)
Encontrei **231 actores** com prefixo `Quest*`/`QuestMarker*`/`QuestObjective*`/`StampedeTrigger*` já existentes no mapa — incluindo um ecossistema já **muito duplicado** de quests "ObserveHerd"/"HerdWatch" (21 actores diferentes: `QuestTrigger_ObserveHerd_Enter`, `Quest_ObserveHerd_001` x2, `Quest_HerdWatch_ObjectiveMarker_001`, `Quest_HerdWatch_TriggerZone_001`, `Quest_HerdWatch_ObservationPost_001`, `Quest_HerdWatch_FleeWitness_001`, `Quest_HerdWatch_ReportBack_001`, etc., alguns encadeados por nomes de outros actores — anti-padrão já registado em `hugo_naming_dedup_v3`).

**Decisão:** NÃO criei nenhuma nova quest de "observar o herd" — o conceito já existe (excessivamente). Em vez disso, foquei o ciclo em **corrigir o grounding** de marcadores e triggers de missões já existentes que estavam fora do intervalo válido de superfície (z 44-302), o que é trabalho real, verificável e sem duplicar actores.

## Trabalho realizado (4x `ue5_execute` reais)
1. Auditoria completa de 231 actores de quest — listagem de labels/coordenadas/z.
2. Contagem de recursos de crafting existentes perto do hub: `Rock=63`, `Stick=18`, `Leaf=16` (dentro de 4000u do hub 2100,2400) — confirma que o sistema de crafting de ciclos anteriores (#14 ciclos 002-004) continua no lugar, sem necessidade de repor.
3. Correcção de z fora do intervalo válido (44-302) em 9 actores de quest:
   - `QuestMarker_Q3_GatherBerries`: 40.0 → 302.0
   - `StampedeTrigger_HerbivoreHerd`: 26.3 → 302.0
   - `QuestHub_2`: 405.0 → 302.0
   - `QuestObj_Migration_WP3_001`: 394.4 → 302.0
   - `QuestTrigger_TrackHerd_Start` (1800,3200): 404.0 → 302.0
   - `QuestTrigger_ObserveHerd_Enter`: 505.0 → 302.0
   - `QuestTrigger_ObserveHerd_Zone`: 405.0 → 302.0
   - `QuestTrigger_FollowTheHerd`: 608.6 → 302.0
   - `QuestTrigger_TrackHerd_Start` (2800,2000): 405.0 → 302.0
4. **Limitação técnica encontrada e documentada:** `unreal.SystemLibrary.line_trace_single`/`line_trace_multi` neste bridge headless não regista `Landscape1` como hit bloqueante nesta versão/config (testado com `trace_complex=True` e `False`, single e multi-trace) — o trace intercepta sempre a colisão de outros actores (rochas, ruínas, vegetação) em vez do heightfield do terreno. Por isso os 9 valores foram clampados ao limite superior válido conhecido (z=302, conforme `hugo_terrain_savana_v1`) em vez de à altura exacta do terreno nesse XY. Isto garante que ficam dentro do intervalo de superfície jogável, mas **não é grounding de precisão** — recomendo ao próximo agente com acesso a ferramentas de landscape (`LandscapeProxy.get_height_at_location` ou similar) fazer um passe de precisão nestes 9 marcadores.

## Verificação
- 2 saves confirmados `True` (`LevelEditorSubsystem.save_current_level()`).
- Todos os 9 actores corrigidos estão agora dentro do intervalo 44-302 (anteriormente 3 estavam >390 e 2 estavam <44).
- Nenhum actor novo foi criado — 100% correcções sobre actores existentes (regra `hugo_naming_dedup_v3` respeitada).

## Ficheiros GitHub
- `docs/quest_design/cycle_005_hub_quest_grounding.md` (este ficheiro). Nenhum `.cpp`/`.h` criado (regra `hugo_no_cpp_h_v2`).

## Para o próximo agente (#15 Narrative & Dialogue Agent)
- O ecossistema de quests do hub (231 actores) está agora **dentro do intervalo válido de superfície**, mas continua com duplicação semântica significativa (21 variantes de "observar/seguir o herd"). Sugiro uma auditoria de consolidação de conteúdo narrativo antes de adicionar novos diálogos a este cluster — risco real de o jogador encontrar 4-5 triggers redundantes para o mesmo beat narrativo.
- Recursos de crafting (`Rock`/`Stick`/`Leaf`) estão estáveis perto do hub (63/18/16) — prontos para texto de diálogo/tutorial de crafting.
- Herd do hub (6 Trike + 2 Ankylo) está agrupado e grounded (entregue pelo #13) — bom ponto de ancoragem para diálogo de "Elder"/"Explorer" já presente (`QuestGiver_Korg_TribalElder`, `QuestGiver_Thok_Explorer`, `QuestGiver_Uga_Survivor`).
- Pendente para ciclo técnico dedicado: precisão de landscape-trace no bridge (ver limitação acima) e grounding de Parasaurolophus/Tsintaosaurus ainda flutuantes.
