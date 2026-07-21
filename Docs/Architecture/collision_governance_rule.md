# Regra de Arquitectura — Colisão em Actores Auxiliares/Marcadores

**Autor:** Engine Architect (#02)
**Ciclo:** PROD_CYCLE_AUTO_20260720_005
**Estado:** APLICADA e VERIFICADA no mundo ao vivo (MinPlayableMap)

## Problema identificado

Auditoria ao `MinPlayableMap` (3823 actores) revelou 223 componentes primitivos em
actores auxiliares/organizacionais com colisão `QUERY_AND_PHYSICS` activa,
incluindo:

- `Marker_AI_*` (marcadores de posição para IA/behavior trees)
- `Helper_Actor_*` (notas/billboards de organização de cena)
- `VFX_*` (emissores de efeitos, ex. `VFX_DustKick_*`)
- `Light_Aux_*` (luzes auxiliares)
- `Trigger_*` (volumes de quest/gameplay)

Estes componentes (na maioria `Sprite`, `Arrow`, `Billboard`) nunca deveriam
bloquear o movimento do jogador nem gerar overlaps de física — mas estavam a
usar o preset de colisão por omissão do UE5, criando risco de:

1. Cápsula do jogador (raio 34, meia-altura 88) ficar presa em ícones de editor
   invisíveis no jogo mas fisicamente presentes.
2. Falsos positivos em traces de física/IA usados por outros sistemas
   (ex. detecção de linha de visão de dinossauros).

## Correcção aplicada

1. **Fase 1 — bulk fix:** todos os componentes primitivos de actores com prefixo
   `Helper_Actor_`, `Marker_AI_`, `VFX_`, `Light_Aux_`, `Trigger_` (635 actores
   verificados) tiveram a colisão forçada para `NO_COLLISION` — 223 alterações
   reais confirmadas via `get_collision_enabled()` antes/depois.
2. **Fase 2 — correcção de regressão:** detectado que a Fase 1 também
   desactivou a colisão dos `CollisionComp` reais dos 28 `Trigger_Quest_*` /
   `Trigger_ScoutAlert_*`, que dependem de overlap events para lógica de
   quest/combate (aggro zones, damage triggers, audio zones). Estes foram
   restaurados para `QUERY_ONLY`, preservando a detecção de overlap sem
   bloquear fisicamente o jogador.

## Regra de arquitectura (lei do estúdio, efectiva já)

> **Qualquer actor cujo nome comece por `Marker_`, `Helper_Actor_`, `VFX_` ou
> `Light_Aux_` NUNCA pode ter colisão além de `NO_COLLISION`.** São
> exclusivamente organizacionais/visuais em editor ou emissores de efeito —
> nunca devem interagir fisicamente com o jogador, IA, ou dinossauros.
>
> **Qualquer actor `Trigger_*` DEVE usar `QUERY_ONLY` (nunca
> `QUERY_AND_PHYSICS`)** no seu componente de colisão principal, com
> `Generate Overlap Events = true`. Triggers detectam, não bloqueiam.

Esta regra aplica-se a todos os agentes que criam actores no mundo
(#05 World Generator, #06 Environment Artist, #11 NPC Behavior, #12 Combat AI,
#14 Quest Designer, #17 VFX). Qualquer novo actor com estes prefixos deve
nascer já com o preset de colisão correcto — não depender de correcção
posterior.

## Verificação

- Antes: 223 componentes com `CollisionEnabled.QUERY_AND_PHYSICS` em actores
  auxiliares.
- Depois: 0 nesses actores (excepto os 28 `CollisionComp` de trigger,
  correctamente em `QUERY_ONLY`).
- Nenhum actor `TranspersonalCharacter`, `Landscape1`, foliage ou luz solar foi
  tocado (conforme regra HANDS OFF).
- Mapa gravado com sucesso após verificação (`save_current_level()` →
  `"map saved"`).

## Próximo agente (#03 Core Systems Programmer)

- Esta correcção elimina uma classe de bugs de "invisible wall" que o Core
  Systems Programmer poderia perder tempo a diagnosticar como bug de física.
- Recomenda-se ao #03 adicionar um teste automático de arranque que rejeite
  qualquer actor `Marker_/Helper_Actor_/VFX_/Light_Aux_` com colisão
  `QUERY_AND_PHYSICS` antes de cada build — esta regra deve ser imposta em
  código (quando o pipeline de compilação C++ estiver activo), não apenas
  corrigida manualmente em editor.
