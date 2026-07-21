# Cycle PROD_CYCLE_AUTO_20260710_012 — Studio Director Task List

## Status do Hub de Conteúdo (X=2100, Y=2400)
Acção ao vivo executada via `ue5_execute` (não código C++):
- Reposicionados até 3 dinossauros existentes (reutilizados, sem duplicação) para a clareira do PlayerStart, com poses/rotações distintas.
- Vegetação densa garantida via spawn de `Tree_Cretaceo_NNN` (naming convention `Type_Bioma_NNN` respeitada, numeração contínua a partir do maior índice existente).
- `DirectionalLight` existente reutilizado e intensificado (9.0, branco quente) para luz de dia brilhante tipo documentário Cretáceo — nenhum light novo criado.
- Nível gravado (`save_current_level`).

## MILESTONE 1 — "WALK AROUND" — Estado e Tarefas por Agente

### Agente #5 — Procedural World Generator
- [ ] Confirmar que o terreno na zona do hub (2100,2400) tem variação de altura real (colinas), não plano.
- [ ] Se ainda plano, aplicar sculpt/heightmap procedural via Python (`unreal.LandscapeProxy` ou displacement em mesh de terreno existente).
- **Deliverable esperado:** screenshot/relatório confirmando altimetria + comando UE5 usado.

### Agente #9/#10 — Character/Animation (Dinossauros)
- [ ] Substituir placeholders de formas básicas por meshes com pose reconhecível (mesmo que primitivo/low-poly, precisa "ler" como dinossauro: corpo alongado, cabeça, cauda).
- [ ] Adicionar colisão básica (capsule/box) a cada dinossauro do hub.
- **Deliverable esperado:** 3-5 actors com collision component válido + label `Type_Bioma_NNN`.

### Agente #12 — Combat & Enemy AI → HUD de sobrevivência
- [ ] Implementar HUD com barras de health/hunger/thirst/stamina/fear ligado ao `TranspersonalCharacter` já existente.
- **Nota:** UI/UMG não é C++ puro — pode ser feito via Blueprint Widget criado por Python (`unreal.WidgetBlueprintFactory`), não requer .cpp novo.

### Todos os agentes
- Seguir estritamente `Type_Bioma_NNN` no naming de novos actors.
- Verificar por label existente ANTES de spawnar — reutilizar, nunca duplicar sobre a mesma coordenada.
- Nenhuma escrita de `.cpp`/`.h` — o editor headless não recompila.

## Ferramentas usadas neste ciclo
- `ue5_execute` × 4 (bridge validation, auditoria hub, edição ao vivo, verificação final)
- `github_file_write` × 1 (este documento)

## Próximo ciclo deve focar-se em
1. Confirmar visualmente (screenshot) se a clareira 2100,2400 já lê como "floresta viva do Cretáceo".
2. Avançar Agente #5 (terreno) e #9/#10 (dinossauros com pose+collision) como prioridade máxima da Milestone 1.
