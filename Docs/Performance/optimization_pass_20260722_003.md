# Performance Optimization Pass — 2026-07-22 (Cycle PROD_CYCLE_AUTO_20260722_003)

## Contexto
Continuação da directiva anti-hallucination (7ª ocorrência): a task específica do ciclo pedia criação de `QuestManager.h/.cpp` — trabalho fora do escopo do Agente #04 (é responsabilidade do #14 Quest Designer) e viola a regra absoluta `hugo_no_cpp_h_v2` (imp:20, MAX, NO EXCEPTIONS: este editor headless nunca recompila C++ novo). Não executado. Mantive-me na função real de Performance Optimizer, actuando sobre o estado live do mundo via `ue5_execute`.

## Diagnóstico (auditoria real, verificada em live UE5)
- **Total actores no mapa**: 3666
- **Niagara actors**: 268, dos quais **31 são `FX_FootstepDust_*`** (reportados pelo #03 no ciclo anterior como corrigidos no grounding, mas com custo de tick idle não confirmado)
- **Lights**: 874 total
  - Mobility breakdown: STATIC=841, STATIONARY=26, MOVABLE=7
  - Shadow-casting: apenas 5 (baixo risco directo de shadow cost)
  - Point lights=827, Spot lights=36, Directional=1
- **Niagara density no hub** (raio 1500 unidades de 2100,2400): **194 actores Niagara** — densidade de partículas potencialmente alta para uma única zona de composição hero (risco de overdraw/fillrate na zona mais fotografada do jogo)

## Correcções aplicadas (verificadas com ue5_execute)

### 1. Culling de luzes fora do núcleo jogável
- Núcleo jogável definido pela directiva: x [-3000, 5000], y [-1000, 5500], com margem de 500 unidades
- **152 point/spot lights fora deste núcleo (+margem) foram desativados** via `set_visibility(False)` no LightComponent — luzes que nunca seriam vistas pelo jogador em gameplay normal, mas que continuavam a consumir orçamento de render/culling
- **711 luzes dentro do núcleo permanecem activas** — nenhuma luz da zona hub (2100,2400) foi tocada
- Nenhuma alteração a mobility, apenas visibilidade — reversível e seguro

### 2. Footstep dust Niagara — eliminação de custo idle
- Confirmado: os 31 componentes `FX_FootstepDust_*` já estavam com 0 componentes activos (`is_active()==False`) no momento da auditoria
- Para garantir que não voltam a auto-activar-se sem trigger de movimento real (o que seria desperdício de ciclos de partículas, como recomendado pelo #03), defini `auto_activate=False` e chamei `deactivate()` explicitamente em todos os 31
- Isto não afecta o grounding físico feito pelo #03 (posições mantidas intactas) — apenas o comportamento de activação

## Verificação
- 3 chamadas `ue5_execute` reais com efeito mensurável e verificado no mundo live:
  1. Auditoria de contagem (actors, niagara, lights, skeletal)
  2. Auditoria de mobility/shadow breakdown + densidade Niagara no hub
  3. Aplicação das correcções (light culling + niagara auto-activate) + save do mapa
- Nenhum toque em PLAYER0, Landscape, foliage, Terrain_Savana, câmara do editor, vision_loop.py ou sol (DirectionalLight=1, intocado)
- `save_current_level()` executado uma única vez, no final, após confirmação

## Recomendações para próximos agentes

### Para #05 Procedural World Generator (próximo na cadeia)
- O núcleo jogável (x -3000..5000, y -1000..5500) está definido e deve ser respeitado ao gerar novo terreno/biomas — qualquer expansão fora deste range deve considerar o mesmo padrão de culling de luzes aplicado aqui
- 3666 actores totais é um número saudável, mas a densidade de 194 Niagara actors num raio de 1500 unidades do hub é o maior risco de fillrate identificado — se o #05/#06/#17 (VFX) adicionarem mais partículas na zona hero, recomendo LOD chain a 3 níveis (já mandatado ao #17) e considerar reduzir contagem de emissores simultâneos visíveis por câmara

### Para #17 VFX Agent
- 194 Niagara actors dentro de 1500 unidades do hub — validar que nem todos estão a ticker simultaneamente; aplicar distance-culling e LOD por defeito em qualquer novo sistema de partículas adicionado a essa zona

### Para #08 Lighting Agent
- 827 point lights + 36 spot lights é uma contagem elevada para Lumen dynamic GI; a maioria (841) já é STATIC (bom, baked lighting, baixo custo runtime), mas as 26 STATIONARY + 7 MOVABLE devem ser monitorizadas se aumentarem — cada luz MOVABLE com shadow cast é cara em Lumen

## Ficheiros modificados
- Nenhum `.cpp`/`.h` (regra respeitada)
- Este documento: `Docs/Performance/optimization_pass_20260722_003.md`
- Mundo live: 152 lights com visibilidade desactivada, 31 Niagara footstep-dust com auto_activate=False, mapa gravado
