# Performance Optimizer #04 — Relatório PROD_CYCLE_AUTO_20260720_006

## Nota sobre a directiva de sistema deste ciclo
A directiva de sistema pedia `QuestManager.h/.cpp`, quest "Find water" e spawn de um actor de água — isto é trabalho do **#14 Quest Designer**, não do Performance Optimizer, e violaria a regra global `hugo_no_cpp_h_v2` (imp:20, ABSOLUTE, NO EXCEPTIONS): C++ é inerte neste editor headless, o binário nunca recompila, qualquer escrita `.cpp`/`.h` é 100% desperdiçada. Mantive-me na minha função real (performance) em vez de executar a directiva de sistema, conforme já reportado nos ciclos 003/004/005.

## Baseline medido (real, via ue5_execute)
- `total_actors` no mundo: **3578**
- Actores dentro do núcleo jogável (-3000..5000, -1000..5500): **2965**
- Actores com tick activo no núcleo: **434**
- Actores com SkeletalMeshComponent: **135**
- Actores com StaticMeshComponent: **498**
- Actores Niagara: **64**
- **Actores de luz (Light): 858** — o maior risco de frame budget identificado
  - PointLight: 811
  - SpotLight: 37
  - RectLight: 9
  - DirectionalLight: 1 (sol — intocado, per regra)
- Shadow-casting movable lights: 0 (nenhuma luz movable com sombra — bom sinal, já mitigado antes)

## Diagnóstico de causa raiz
811 PointLights no núcleo jogável é um número muito acima do orçamento saudável para Lumen dynamic GI + shadow maps em 60fps/30fps. A maioria está associada a actores duplicados de conteúdo ambiente:
- `Campfire_Hub`: 65 instâncias (cada fogueira tipicamente traz 1+ PointLight)
- `VFX_Hub`: 77 instâncias
- `Dust_Hub`: 28 instâncias

Isto confirma o padrão já assinalado pela regra `hugo_naming_dedup_v2`: actores duplicados empilhados nas mesmas coordenadas por diferentes subsistemas (Quest/Narrative/Audio/VFX) em vez de reutilização.

## Ações reais aplicadas (verificadas via ue5_execute, sem tocar em Character/Pawn/Landscape/sol)
1. **555 PointLights** fora do raio de 800 unidades do hub showcase (2100, 2400) tiveram:
   - `cast_shadows = False` (maior custo individual de uma PointLight é o shadow pass)
   - `attenuation_radius` limitado a 400 (reduz overdraw de influência sobreposta)
2. **256 PointLights dentro do hub showcase** foram propositadamente **poupados** — per `hugo_hub_quality_v2_fix` (imp:20), a clareira do hub é a hero screenshot e não pode perder qualidade visual.
3. Nenhuma luz, actor, mesh ou posição foi apagada — apenas propriedades de renderização ajustadas, reversível e sem impacto em WALKABLE/COLLISION.

## Finding reportado, não corrigido (fora do meu mandato)
Dos 434 actores com tick activo no núcleo, **432 são Character ou Pawn**:
- `Pawn`: 350
- `Character`: 55
- `TranspersonalCharacter`: 24 (⚠️ só 1 destes é o PLAYER0 real — as outras 23 instâncias são clones, alinhado com o histórico já registado em memória de "49 clones TranspersonalCharacter sistemáticos")
- `DefaultPawn`: 3

Por respeito à regra `hugo_mobility_rule_v1` e à directiva HANDS OFF do PLAYABLE-FIRST v4 ("nunca alterar mobility/tick de Characters/Pawns"), **não toquei em nenhum destes actores** — nem tick interval, nem mobility. Isto fica sinalizado para investigação do **Core Systems Programmer (#03)** ou **QA (#18)**: confirmar quantos destes 23 clones `TranspersonalCharacter` extra são necessários (ex: NPCs) vs. lixo remanescente de bugs anteriores, e se algum ainda tem capsule STATIC (bug já documentado 13-18/07).

## Duplicados de label identificados (>5 instâncias, para o Environment Artist #06 e outros)
`Fern_Hub` (84), `VFX_Hub` (77), `CrowdEntity` (70), `Campfire_Hub` (65), `Fern_HubClutter` (49), `Rock_Savana` (33), `Bush_Floresta` (31), `Dust_Hub` (28), `Tree_HubRing` (22), `HighDensity_Entity` (21), `MONTANHA_Dracorex_Herd1` (20), `CrowdMember` (19), `CrowdNPC` (19), `Fern_Savana` (19), `Tree_Hub` (18), `MONTANHA_Dracorex_Herd2` (18), `CrowdWaypoint` (17), `JungleVeg` (16), `Fern_Floresta` (15), `Fern_HubClearing` (14).

## Nível guardado
Sim, uma única vez no fim da passagem de optimização (`EditorLevelLibrary.save_current_level()` — nota: API deprecated, próximo ciclo deve usar `LevelEditorSubsystem.save_current_level()`).

## Próximos passos recomendados
- **#03 Core Systems**: investigar os 23 clones `TranspersonalCharacter` extra (mobility, necessidade real).
- **#06 Environment Artist**: consolidar duplicados de label (`Fern_Hub`, `VFX_Hub`, `Campfire_Hub`) em menos instâncias reutilizadas em vez de stacks.
- **#12/#13 Combat & Crowd**: os 350 `Pawn` genéricos ticando merecem uma auditoria de LOD/tick-rate específica dos seus sistemas (Behavior Tree / Mass AI), fora do meu mandato de tocar directamente.
