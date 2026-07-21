# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260711_006

## Constraint absoluta respeitada
`hugo_no_cpp_h_v2` (imp:MAX): **zero ficheiros .cpp/.h escritos**. Todo o trabalho de optimização foi aplicado ao vivo no mundo em execução via `ue5_execute` (python), consistente com o padrão já usado por #02/#03 neste ciclo (tags + Remote Control, sem edição de source).

## Contexto herdado do #03 (Core Systems Programmer)
Este ciclo, #03 tagueou todos os pawns de dinossauro (TRex/Raptor/Brachiosaurus/Triceratops) com `Core_PhysicsManaged` e normalizou o collision profile para `"Pawn"`. O meu trabalho consumiu directamente essa tag como ponto de integração, conforme pedido explícito do #03 no output anterior ("usar a tag `Core_PhysicsManaged` para decidir política de LOD/tick-rate de colisão dos dinossauros").

## Ações executadas (4x ue5_execute python + 2x ue5_execute console)

### PASS 1 — Tick rate distance-based (dinosaur pawns)
- Zona hub prioritária definida em `X=2100, Y=2400` (clearing do hero screenshot, `hugo_hub_quality_v2_fix`), raio 3000u.
- Todos os actores com tag `Core_PhysicsManaged` (dinossauros) dentro do raio: `tick_interval=0.0` (tick rate nativo, máxima fidelidade visual/gameplay — é onde o jogador está e onde a screenshot hero é composta).
- Fora do raio: `tick_interval=0.2` (5 ticks/seg em vez de 60+), reduzindo custo de CPU em pawns que não estão a ser observados nem interagidos.
- Nova tag `Perf_TickManaged` aplicada para rastreabilidade e para não reprocessar os mesmos actores em ciclos futuros.

### PASS 2 — Cull distance + remoção de simulação de física desnecessária
- `StaticMeshActor` (árvores, rochas, placeholders) fora do raio do hub: `cached_max_draw_distance=8000.0` para reduzir draw calls/fill rate.
- Dentro do raio do hub: `cached_max_draw_distance=0.0` (sem cull agressivo), protegendo a composição da hero screenshot (`hugo_hub_quality_v2_fix`, imp:MAX) — prioridade explícita sobre qualquer ganho de performance.
- Nos actores `Core_PhysicsManaged`, qualquer `PrimitiveComponent` com `is_simulating_physics()==True` foi desligado (`set_simulate_physics(False)`), já que os dinossauros placeholder não precisam de simulação de física contínua (ragdoll fica reservado para o sistema de combate/destruição do #03/#12 quando activado por evento, não always-on).
- Nova tag `Perf_CullManaged` aplicada aos `StaticMeshActor` processados.

### PASS 3 — Verificação e auditoria
- Contagem final confirmada via `GetAllActorsWithTag`-equivalent (filtro manual por tag): `Perf_TickManaged`, `Perf_CullManaged`, `Core_PhysicsManaged`.
- Amostra de dinossauros com distância ao hub e tick interval registada para auditoria (log UE5).
- `save_current_level()` chamado com sucesso (`ReturnValue: true`) na segunda tentativa.

### Console stats (baseline de referência)
- `stat fps`, `stat unit`, `stat memory` activados para dar visibilidade contínua no viewport headless — usados como baseline de referência para próximos ciclos comparativos (sem overlay visual disponível neste modo headless, mas o estado fica activo na sessão do editor para qualquer inspecção via Remote Control futura).

## Decisões técnicas e justificação
1. **Distância como critério único de LOD/tick** (em vez de câmara do jogador): neste ambiente headless não há PlayerController activo em runtime constante, por isso a zona hub fixa (`2100,2400`) é o proxy mais fiável e alinhado com a directiva `hugo_hub_quality_v2_fix` (imp:MAX) — o local que **tem** de parecer vivo e denso é exactamente onde não aplico nenhum corte de performance.
2. **Nenhum novo tipo/canal de colisão criado** — reutiliza collision profile `"Pawn"` já normalizado pelo #03, evitando qualquer necessidade de editar headers (respeita `hugo_no_cpp_h_v2`).
3. **Física desligada por default em placeholders** — dinossauros são pawns de gameplay, não ragdolls persistentes; simulação de física contínua nestes actores era puro desperdício de physics-thread budget sem benefício visual ou de gameplay no estado actual.
4. **Tags em vez de subsystem C++** — `Perf_TickManaged` / `Perf_CullManaged` seguem o mesmo padrão de scaffolding runtime que #02 (`BiomeSystem_Managed`) e #03 (`Core_PhysicsManaged`) já estabeleceram, garantindo descoberta cruzada entre agentes via `GetAllActorsWithTag` sem hardcoding de referências.
5. **Nenhum actor duplicado criado** (`hugo_naming_dedup_v2`) — todas as alterações foram feitas por modificação in-place de actores já existentes, sem spawnar novos actores.

## Dependências para o próximo agente
- **#05 (Procedural World Generator):** ao gerar novo terreno/biomas, aplicar a mesma zona hub (`2100,2400`, raio 3000u) como zona "sem cull agressivo" para manter a composição da hero screenshot consistente.
- **#12 (Combat AI):** física dos dinossauros está desligada por default (`is_simulating_physics=False`); qualquer sistema de ragdoll/combate deve activar `set_simulate_physics(True)` apenas no momento do evento (impacto, morte), não como estado persistente, para não reverter este ganho de performance.
- **#08 (Lighting & Atmosphere):** `cached_max_draw_distance=0.0` na zona hub está reservado para não conflituar com fog/atmospheric settings que possam depender de cull distance — coordenar antes de alterar.
- **Próximo ciclo #04:** medir `stat fps`/`stat unit`/`stat memory` numa sessão com viewport activo (fora do modo headless) para obter números reais de frame time e validar o ganho desta passagem distance-based tick/cull.
