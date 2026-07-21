# Core Systems — Physics/Collision Enforcement Pass (Cycle PROD_CYCLE_AUTO_20260710_003)

## Constraint respeitada
`hugo_no_cpp_h_v2` (imp:MAX, GLOBAL): **zero ficheiros `.cpp`/`.h` escritos** neste ciclo. Todo o trabalho de sistemas core foi executado directamente contra a instância viva do UE5 via `ue5_execute` (Python), que é o único canal com efeito real neste editor headless (binário pré-compilado, nunca recompila).

## Contexto
A directiva do ciclo pedia (novamente) a integração de `USurvivalComponent` em `TranspersonalCharacter.h/.cpp` via `github_file_write`. Consistente com os últimos ~17 ciclos, esta acção foi recusada por violar directamente `hugo_no_cpp_h_v2` (imp:MAX, NO EXCEPTIONS). Em vez de repetir a mesma constatação sem produzir valor, este ciclo focou-se na minha área real de mandato — **física e colisão** — aplicada de forma funcional ao conteúdo já existente no hub (X=2100, Y=2400), alinhado com `hugo_hub_quality_v2_fix`.

## Trabalho executado (4× ue5_execute, 1× github_file_write)

1. **Validação inicial do hub**: confirmei bridge estável, contagem total de actores, e quantos dinossauros/vegetação existem no raio de 1500u à volta do PlayerStart do hub de conteúdo.

2. **Auditoria de colisão**: percorri todos os actores de dinossauro (`TRex`, `Raptor`, `Brachio`, `Trike`) e vegetação (`Tree`, `Rock`) próximos do hub, verificando `CollisionEnabled` em cada `PrimitiveComponent`. Muitos destes actores são placeholders de formas básicas — parte não tinha colisão configurada correctamente, permitindo ao jogador atravessá-los (quebra de credibilidade física, contra a minha convicção central de que a física é a assinatura emocional do jogo).

3. **Enforcement de colisão física real**: para todos os actores identificados, apliquei:
   - `CollisionEnabled = QueryAndPhysics`
   - `CollisionObjectType = WorldStatic`
   - `CollisionResponseToAllChannels = Block`
   - `SimulatePhysics = False` (correcto para meshes estáticos de ambiente/dinossauro placeholder — evita comportamento instável)
   
   Isto garante que o jogador não atravessa dinossauros nem árvores no hub — colisão sólida e previsível, sem exigir recompilação de C++.

4. **Verificação final**: recontagem de colisão pós-fix (OK vs BAD), confirmação de instâncias de `TranspersonalCharacter` no nível, e confirmação de estabilidade da bridge.

## Decisões Técnicas
- **Colisão via Python/Remote Control em vez de C++**: dado que o binário não recompila, a única forma de ter efeito físico real e imediato é manipular `PrimitiveComponent` directamente na instância viva. Isto é equivalente em runtime a ter configurado os presets de colisão no Blueprint/C++ original.
- **Não usei `SimulatePhysics(True)`** nos dinossauros/vegetação — são placeholders estáticos; activar simulação física introduziria instabilidade (actores a cair/deslizar) sem physics constraints adequadas, o que pioraria a credibilidade em vez de a melhorar.
- **Scope limitado ao raio do hub (1500u)** para respeitar `hugo_hub_quality_v2_fix` — a composição da hero screenshot é a prioridade sobre mudanças globais no mapa.

## Resultado de Física
- Antes: parte dos actores de dinossauro/vegetação no hub sem colisão bloqueante (jogador atravessava-os).
- Depois: todos os actores verificados no raio do hub têm `CollisionEnabled != NoCollision`, com resposta `Block` a todos os canais.
- Nível gravado (`save_current_level`) para persistir a alteração.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Validação inicial do hub: contagem de dinossauros/vegetação no raio 1500u de X=2100,Y=2400.
- [UE5_CMD] Auditoria de `CollisionEnabled` em todos os `PrimitiveComponent` de dinossauros/vegetação próximos do hub.
- [UE5_CMD] Enforcement: `CollisionEnabled=QueryAndPhysics`, `ObjectType=WorldStatic`, `Response=Block` aplicado a todos os actores identificados + `save_current_level()`.
- [UE5_CMD] Verificação final pós-fix (contagem OK/BAD) + confirmação de instâncias de `TranspersonalCharacter` no nível.
- [FILE] `Docs/CoreSystems/PhysicsCollision_HubEnforcement_20260710_003.md` — este documento.
- [NEXT] #04 (Performance Optimizer): validar custo de `QueryAndPhysics` em ~20+ actores próximos (idealmente `SimulatePhysics=False` já reduz overhead); considerar merge de colisão em instanced static mesh para vegetação repetida. #02: quando um pipeline de build real (não-headless) estiver disponível, portar este enforcement para os defaults de C++ dos placeholders (`BodyInstance` defaults no construtor), eliminando a necessidade deste patch runtime em cada ciclo.
