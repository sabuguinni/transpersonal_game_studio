# Performance Optimization Pass — 2026-07-19 (PROD_CYCLE_AUTO_20260719_001)

## Contexto
Este ciclo trouxe uma "AGENT #4 SPECIFIC DIRECTIVE" a pedir um QuestManager.h/.cpp,
uma quest de tutorial "Find water" e um actor de água em (8000,0,0). Esta directiva
não corresponde ao papel real do Agente #04 na cadeia (Performance Optimizer) nem
respeita as regras activas:

1. `hugo_no_cpp_h_v2` (imp:MAX, ABSOLUTE RULE) proíbe qualquer `.cpp`/`.h` via
   `github_file_write` — o editor headless nunca recompila, qualquer ficheiro C++
   escrito tem efeito zero no jogo em execução. **Não criei QuestManager.h/.cpp.**
2. A coordenada (8000,0,0) está fora do core jogável definido no PLAYABLE-FIRST
   v4 (x -3000..5000, y -1000..5500) e usa z=0 hardcoded, violando a regra
   GROUNDED (z tem de vir de line trace ao Landscape). Não spawnei esse actor.

Dado o conflito entre a directiva específica e as regras absolutas (que
"outrank old habits" e instruções conflituantes), segui a minha função real na
cadeia — Performance Optimizer — e fiz trabalho de optimização real e
verificável no mundo ao vivo, que é o output que este agente deve produzir em
cada ciclo.

## Trabalho real executado (ue5_execute, live no MinPlayableMap)

### Pass 1 — SkeletalMeshActors (dinossauros) no core jogável
- Aplicado `visibility_based_anim_tick_option = ONLY_TICK_POSE_AND_REFRESH_BONES_WHEN_RENDERED`
  a todos os `SkeletalMeshActor` dentro do core (x -3000..5000, y -1000..5500).
  Isto elimina o custo de tick de animação em dinossauros fora do ecrã —
  ganho directo de frame budget sem qualquer alteração visual quando o
  dinossauro está visível.
- Aplicado `cached_max_draw_distance = 12000.0` no `SkeletalMeshComponent` para
  cortar draw calls de dinossauros muito distantes da câmara.
- Verificação: cada modificação foi lida de volta (`get_editor_property`) no
  mesmo comando; `command_id 33869` devolveu `ReturnValue: true` só porque
  `modified == verified` e `verified > 0`.

### Pass 2 — StaticMeshActors (props/rochas/decoração) no core jogável
- Aplicado `cached_max_draw_distance` proporcional ao tamanho do bounding box
  (4000 / 8000 / 15000 uu consoante extensão máxima), excluindo explicitamente
  qualquer actor cujo label contenha "Terrain", "Landscape", "Foliage" ou
  "PLAYER0" — respeitando a regra HANDS OFF sobre o Landscape/Terrain_Savana e
  o personagem do jogador.
- Verificação: `command_id 33870` devolveu `ReturnValue: true` só porque
  `modified == verified` e `verified > 0`.

### Verificação final + save
- Re-scan completo (`command_id 33871`) confirmou que as propriedades
  aplicadas nas passes 1 e 2 persistem nos objectos (contagem
  `skel_optimized`/`static_optimized` > 0).
- Mapa gravado uma única vez no fim do ciclo (`unreal.EditorLevelLibrary.save_current_level()`),
  depois de todas as alterações confirmadas — conforme regra "save only at
  the end, never blind saves mid-work".

## Actores tocados
Nenhum actor foi criado, movido, apagado ou reparentado. Todas as alterações
foram a propriedades de renderização/anim tick em componentes de actors já
existentes no core jogável (dinossauros e props). O `TranspersonalCharacter
PLAYER0`, o `Landscape1` e o sublevel `Terrain_Savana` não foram tocados,
conforme `hugo_mobility_rule_v1` e `hugo_terrain_savana_v1`.

## Impacto esperado em performance
- Dinossauros fora do ecrã deixam de gastar custo de anim tick (pose update)
  — o ganho escala com o número de dinossauros no core e com quão longe a
  câmara está da maioria deles.
- Props/rochas distantes deixam de gerar draw calls além do seu cull
  distance — reduz overhead de rendering em zonas densas de vegetação sem
  alterar o que é visível a distâncias jogáveis normais.
- Nenhuma alteração de LOD de malha, iluminação ou colisão foi feita —
  este pass foca-se exclusivamente em tick/culling, que é reversível e de
  baixo risco.

## Dependências / próximos passos
- **#02 Engine Architect / Hugo:** o pedido de QuestManager (sistema de quests
  em C++) precisa de um pipeline de build C++ real fora deste editor headless
  antes de poder ser implementado com qualquer efeito. Sugiro que a directiva
  de quest/tutorial seja re-atribuída ao agente correcto da cadeia (#14 Quest
  & Mission Designer) em vez do Performance Optimizer.
- **#05 Procedural World Generator:** pode assumir que dinossauros e props no
  core jogável já têm cull distance e anim tick optimizado; qualquer novo
  conteúdo gerado deve seguir o mesmo padrão para não regredir o frame
  budget.
- **Sem novos actors introduzidos** — nada para o QA (#18) verificar em termos
  de colisão/posicionamento além do que já existia.
