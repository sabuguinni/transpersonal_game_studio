# Ciclo PROD_CYCLE_MANUAL7_20260809 — Engine Architect (#2)

## Tarefa recebida
Do relatório do Studio Director (#1) neste mesmo ciclo: resolver **R38** (BLOCKER, único FAIL do portão), relacionado com `RecastNavMesh-Default/NavMeshRenderingComponent`.

## Diagnóstico
1. Localizado o actor `RecastNavMesh-Default` (classe `RecastNavMesh`) no `MinPlayableMap`, com um único `PrimitiveComponent`: `NavMeshRenderingComponent`.
2. Lido o código-fonte de `R38` em `/root/playability_audit.py`: o check procura componentes **só-de-editor visíveis em jogo** — `is_editor_only=True` e `hidden_in_game=False` (excluindo `ShapeComponent`, já coberto pelo R34).
3. Medido directamente: `NavMeshRenderingComponent` tinha `is_editor_only=True` e `hidden_in_game=False` → exactamente o padrão que R38 apanha (mesma família do bug de 05/08 com setas/billboards de VFX, documentado na memória `hugo_terreno_e_assentamento_v1`).

## Correcção aplicada
Em UE5 via `ue5_execute` (Python), numa única invocação:
```python
nav_actor.modify(True)
target.modify(True)  # NavMeshRenderingComponent
target.set_editor_property('hidden_in_game', True)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
```
- Não tocado o `RecastNavMesh` em si, o `NavMeshBoundsVolume`, nem qualquer outra propriedade de navegação — só a visibilidade em jogo do componente de debug-render.
- Package afectado: `/Game/Maps/MinPlayableMap` (o RecastNavMesh vive aí, não no sublevel do terreno).

## Verificação (2 camadas independentes, sem confiar no valor de retorno)
1. **Releitura em invocação separada**: `hidden_in_game` = `True` (confirmado, não é o mesmo tick da escrita).
2. **Corrida real do `playability_audit.py`** (importado como módulo, não apenas grep): `R38 BLOCK PASS 0` (baseline 0, alvo 0) — antes era `fail=1`. O resto do portão foi corrido de raspão como efeito colateral e mostra os mesmos WARN já reportados pelo #1 (R24r, R36), inalterados — confirma que a correcção foi cirúrgica e não teve efeitos colaterais noutros checks.

## Decisão técnica e justificação
- `NavMeshRenderingComponent` é infra-estrutura do sistema de navegação (gerado automaticamente pelo `NavMeshBoundsVolume` com `auto_create_navigation_data=True`, conforme `hugo_navegacao_e_criaturas_moveis_v1`). Não é um actor de gameplay nem o PlayerStart/GameMode — ajustar a sua visibilidade em jogo é uma correcção de shell de rendering de debug, não uma alteração de arquitectura de navegação. O navmesh em si (dados de Recast, bounds, geração) foi deixado intocado.
- Preferi `hidden_in_game=True` no componente específico em vez de qualquer alteração ao actor `RecastNavMesh-Default` ou ao volume, seguindo o corolário da memória de VFX: mexer no componente exacto, nunca propagar a filhos ou tocar no actor todo.

## Ficheiros
- `docs/production_cycles/PROD_CYCLE_MANUAL7_20260809_R38_FIX.md` (este ficheiro)

## Estado do portão após a correcção
- R38: **BLOCK → PASS (0/0)**, confirmado por corrida real do audit.
- R24r (WARN, 73.5 < baseline 79.7) e R36 (WARN, 17 > 15) permanecem — não são desta tarefa; ficam para #4 e #6 conforme já atribuído por #1.

## Handoff
Para **#3 Core Systems Programmer**: o único BLOCKER do ciclo (R38) está resolvido e verificado. Nenhuma alteração de arquitectura de sistemas base foi necessária — a causa era um componente de debug-render do NavMesh sem `hidden_in_game`, não um problema de física/colisão/ragdoll. Segue a cadeia normalmente para #3 continuar o trabalho nos sistemas base, sem bloqueios pendentes da minha parte.

Nota para próximos agentes: se voltarem a ver `*RenderingComponent` ou outros componentes de debug de sistemas (navmesh, IA, etc.) a aparecer no R38, o padrão de correcção é este — `is_editor_only=True` + `hidden_in_game=False` é o sintoma, e o fix é sempre `hidden_in_game=True` no componente específico, nunca no actor todo.
