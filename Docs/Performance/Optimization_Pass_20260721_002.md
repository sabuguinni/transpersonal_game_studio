# Performance Optimization Pass — Agent #04 (PROD_CYCLE_AUTO_20260721_002)

## Nota sobre a directiva de sistema
A directiva de sistema pedia criar `QuestManager.h/.cpp` (sistema de quests) + spawn de água — trabalho do **#14 Quest Designer**, e uma violação directa da regra absoluta `hugo_no_cpp_h_v2` (imp:20): este editor headless nunca recompila C++, qualquer escrita `.cpp`/`.h` teria efeito zero no jogo live. Não executado, pelo terceiro ciclo consecutivo. Mantive-me na função real de Performance Optimizer.

## Estado encontrado
Total de actores no mundo: **3344**. Dentro do núcleo jogável (x -3000..5000, y -1000..5500): **2788 actores**, incluindo:
- 676 PointLight
- 481 StaticMeshActor
- 350 Pawn
- 219 AmbientSound
- 106 TextRenderActor
- 64 NiagaraActor
- 56 SkeletalMeshActor (dinossauros)
- 24 TranspersonalCharacter (clones não-jogáveis)

**Risco de frame budget identificado:** ~826 PointLights no total do mundo (676 só no núcleo), a maioria com shadow casting activo — em deferred rendering isto é o maior risco de overdraw/shading cost por overlap de luzes num único frame, especialmente perto do hub (2100,2400) onde a densidade de vegetação e dinossauros é alta.

## Acções realizadas (4 ue5_execute)

1. **Auditoria de baseline**: contagem de actores por classe no núcleo jogável, tick-enabled count (434 actores com tick activo dentro do núcleo).

2. **Redução de custo de iluminação**: para PointLights com `intensity < 5000` (luzes de preenchimento/ambiente, não hero lights), desactivei `cast_shadows` e limitei `attenuation_radius` a um máximo de 800 unidades.
   - Resultado: **612 luzes com shadow desactivado** nesta passagem (670 luzes tocadas no total).

3. **Segunda passagem de limpeza**: tentativa de aplicar `cached_max_draw_distance` a StaticMeshActors e Niagara (0 alterações — já estavam configurados de ciclos anteriores, confirmando que o trabalho anterior do Optimizer já tinha tratado cull distances/tick nesses actores). Aplicado **`ONLY_TICK_POSE_WHEN_RENDERED`** em `visibility_based_anim_tick_option` nos 56 SkeletalMeshComponent (dinossauros) — anim ticking só corre quando o actor está a ser renderizado, poupando CPU quando o dinossauro está fora do frustum da câmara.

4. **Verificação final + save**: confirmei que os 35 actores `TranspersonalCharacter` no mundo **não foram tocados** (código skipa explicitamente a classe inteira, incluindo Character/Pawn, por `hugo_mobility_rule_v1`). Confirmei que **823 de 826 PointLights** no mundo já não fazem shadow casting. Guardei o nível uma única vez, no fim.

## Resultado mensurável
- **823 PointLights sem shadow casting** (redução massiva de custo de shadow map rendering por luz, que era o maior risco identificado nesta cena de alta densidade de luzes).
- **56 dinossauros (SkeletalMeshActor)** com anim tick optimizado para só correr quando visíveis — poupa CPU de animação para os que estão fora de câmara.
- **Zero actores de jogador tocados** — todos os 35 `TranspersonalCharacter` (incluindo PLAYER0) mantêm mobility/tick/input intactos.
- **Zero regressões**: nenhuma acção alterou posição, colisão ou terreno; apenas propriedades de rendering/tick.

## Decisões técnicas e justificação
- Threshold de `intensity < 5000` para desligar shadows: preserva shadow casting nas luzes "hero" (mais fortes, provavelmente pontos focais de composição do #08 Lighting), enquanto elimina o custo das luzes de preenchimento que dominam em número (maioria das 676).
- `attenuation_radius` capado a 800: evita overlap desnecessário de luzes de baixa intensidade que não deveriam iluminar uma área grande de qualquer forma.
- `ONLY_TICK_POSE_WHEN_RENDERED` em vez de desligar tick por completo nos dinossauros: mantém comportamento correcto quando entram no frustum, sem custo de animação quando não estão visíveis — trade-off seguro que não parte gameplay (comportamento/AI não depende disto, é puramente anim tick).
- Não toquei em Pawn/Character/TranspersonalCharacter em nenhuma fase, por `hugo_mobility_rule_v1` (regra crítica, causou bug de 3 dias no passado).

## Dependências / próximos passos
- **#08 Lighting & Atmosphere Agent**: rever se o threshold de intensity=5000 corresponde à intenção de design das luzes "hero" vs. preenchimento — posso ajustar o corte se houver luzes fracas que precisem de shadow por razões narrativas/composicionais.
- **#05 Procedural World Generator**: 350 Pawn + 24 TranspersonalCharacter clones no núcleo jogável é um número elevado para um mundo mínimo — validar se todos são necessários ou se há duplicação a limpar (não apaguei nada, per regra "não fazer mass-delete").
- **#18 QA**: validar em Pixel Streaming que a remoção de sombras nas luzes de preenchimento não degrada visualmente a cena do hub (2100,2400).
