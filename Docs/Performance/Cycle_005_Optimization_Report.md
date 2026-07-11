# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260711_005

## Bridge Status
OK durante todo o ciclo. 6 chamadas `ue5_execute` (4 python, 2 console commands), IDs 31744–31749, todas `completed` sem timeouts/retries.

## Constraints Absolutas Respeitadas
- `hugo_no_cpp_h_v2` (imp:MAX): **zero ficheiros .cpp/.h escritos**. Toda a optimização foi aplicada ao vivo no mundo em execução via Python/Remote Control.
- `hugo_no_camera_v2`: nenhuma alteração de câmara/viewport/FOV.
- `hugo_naming_dedup_v2`: nenhum actor novo foi criado — apenas actors existentes foram reconfigurados (mobility/cull), sem duplicação de labels.

## Diagnóstico de Baseline (Pré-Optimização)
Auditoria completa da cena via `EditorLevelLibrary.get_all_level_actors()`:
- Contagem total de actors, StaticMeshActors, Lights.
- Actors próximos do content hub (X=2100, Y=2400, raio 3000) contados individualmente.
- Verificação de mobility (Movable vs Static) em todos os StaticMeshComponents — muitos props estáticos (árvores/rochas) estavam configurados como `Movable` por defeito, o que impede batching/instancing do renderer e aumenta draw calls desnecessariamente.
- `stat unit` executado para obter breakdown Game/Draw/GPU thread como baseline de referência.

## Optimizações Aplicadas (Live, via Remote Control)
1. **Mobility enforcement**: todos os StaticMeshActors que NÃO são gameplay-relevantes (dinossauros, water source, triggers — identificados por label) foram forçados para `ComponentMobility.STATIC`. Isto permite ao renderer aplicar static lighting/batching e reduz custo de shadow rendering e draw calls por frame — crítico à medida que o content hub cresce em densidade (árvores adicionadas pelo #03 neste mesmo ciclo).
2. **Cull distance**: aplicado `LDCullDistance` (~8000 unidades) a todos os props estáticos não-gameplay, para que meshes de vegetação/decoração deixem de ser renderizados a distâncias onde não são visíveis relevantemente — reduz overdraw e triangle count no GPU thread sem impacto visual perceptível.
3. **Gameplay actors preservados**: dinossauros (TRex/Raptor/Brach), water source e trigger volumes mantidos com mobility original e sem cull agressivo, pois são elementos centrais da composição do hero screenshot (`hugo_hub_quality_v2_fix`) e não devem desaparecer a curta distância.
4. `r.ViewDistanceScale` confirmado em baseline (1.0) — sem runaway draw distance a consumir GPU budget.
5. `stat fps` executado antes e depois da passagem de optimização para comparação de responsividade do editor.

## Decisões Técnicas
- Prioridade dada à densidade recém-adicionada pelo #03 no content hub (Tree_Floresta_1xx): sem enforcement de mobility/cull, cada árvore extra como `Movable` StaticMeshActor aumenta linearmente o custo de draw calls. A correcção garante que crescimento de densidade visual (prioridade do #06/#08) não custa frame budget.
- Não foi possível implementar um `PerformanceMonitorSubsystem` em C++ (ex.: `UPerf_FrameBudgetSubsystem`) porque qualquer .h/.cpp neste editor headless é inerte (regra `hugo_no_cpp_h_v2`). Toda a lógica de optimização tem de ser reaplicada via python a cada ciclo até existir pipeline de recompilação C++ real — documentado como débito técnico recorrente.

## Dependências para a Cadeia
- **#05 Procedural World Generator**: ao gerar novo terreno/biomas, aplicar mobility=Static e cull distance nos meshes gerados desde a origem (não depender de passes de correcção reactivos deste agente).
- **#06 Environment Artist**: ao povoar vegetação adicional, usar StaticMeshComponent com mobility Static por defeito — evita reprocessamento manual a cada ciclo.
- **#02/#03 (débito técnico)**: quando existir pipeline de compilação C++ funcional, portar esta lógica para um subsystem nativo (`UPerf_CullDistanceManager`) em vez de reaplicar via Python todos os ciclos.
- **#08 Lighting**: confirmar que actors convertidos para Static mobility não quebram lightmaps/shadows dinâmicos esperados no ciclo dia/noite.
