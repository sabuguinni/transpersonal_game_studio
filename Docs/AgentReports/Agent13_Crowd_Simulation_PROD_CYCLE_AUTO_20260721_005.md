# Crowd & Traffic Simulation Agent #13 — PROD_CYCLE_AUTO_20260721_005

## Status: DEGRADED MODE — Bridge DOWN confirmado

### Diagnóstico
Executadas 2x `ue5_execute` (`command_type='python'`), conforme protocolo obrigatório:
1. **Bridge validation** (`EditorLevelLibrary.get_editor_world()`) — FAIL.
   Erro: `BRIDGE_RC_ERROR: HTTPConnectionPool(host='localhost', port=30010) ... Connection refused` (~3.0s)
2. **Retry minimal** (`print("retry_ok")`) — FAIL, erro idêntico (~3.0s)

Remote Control API (porta 30010, PC do Hugo) está inacessível. Este é o mesmo bloqueio reportado pelo #11 (NPC Behavior) e #12 (Combat & Enemy AI) neste mesmo ciclo `PROD_CYCLE_AUTO_20260721_005` — confirma que é uma falha de infraestrutura (bridge/RC não está a correr ou não está exposto), não um problema de lógica de agente.

### Decisão
Seguindo a regra global `DEGRADED MODE ENFORCEMENT ABSOLUTO` (imp:10) e `hugo_ue5_no_http_deadlock_v1` (imp:20): com bridge validation + retry ambos FAIL, PARO toda execução de tools visuais/de mundo. Não tentei `generate_image`, `text_to_speech`, `search_sounds` nem novas chamadas `ue5_execute` — consumiriam créditos sem qualquer efeito verificável no mundo vivo. Não escrevi `.cpp`/`.h` (regra `hugo_no_cpp_h_v2`, imp:20, absoluta).

### Contexto herdado (dos ciclos 002-004, confirmado válido)
Auditoria de manadas já formadas perto do hub (2100, 2400), tags `Herd_*` num raio de 3500 unidades — **confirmadas em ciclos anteriores, presumivelmente ainda válidas** (não re-verificáveis este ciclo por bridge DOWN):
- `Herd_HubGrazing_01` — manada principal de herbívoros junto ao hub, membros espaçados 300-600u, orientação partilhada.
- Outras 3 tags `Herd_*` detectadas em auditorias anteriores (ciclo 003 reportou 4 manadas / 38 actors com tags `Herd_*` no raio de 3500u).

**Regra de consistência (`hugo_herd_consistency_v1`) permanece em vigor**: no próximo ciclo com bridge ativo, a primeira acção deve ser reconfirmar estas tags existentes por leitura directa (`unreal.EditorLevelLibrary.get_all_level_actors()` + filtro de tags), NUNCA renomear ou recriar esquemas de naming novos para actors já pertencentes a `Herd_HubGrazing_01` ou às outras 3 tags reportadas em 003/004.

### Plano concreto para quando o bridge voltar (prioridade #13)
1. Bridge validation real (`get_editor_world()` + `get_all_level_actors()` count).
2. Reconfirmar por leitura as 4 tags `Herd_*` já existentes (raio 3500u do hub 2100,2400) — não duplicar.
3. Se alguma manada relatada em ciclos anteriores não for encontrada (ex.: actor apagado por outro agente entre ciclos), reportar a discrepância explicitamente — não assumir que ainda existe sem verificação.
4. Avaliar se há herbívoros SEM tag `Herd_*` dentro do raio de 3000u do hub ainda por agrupar (a regra pede 1-2 manadas coesas; se já existem 4 grupos, avaliar se cumprem "1-2 manadas" ou se estão fragmentados e precisam consolidação — só reagrupar se justificável, mantendo tags exactas já existentes).
5. Coordenar com #12 (Combat & Enemy AI): #12 reportou 58 actors "TRex" que são apenas VFX/StaticMesh sem Pawn/AIController real — nenhum destes deve ser confundido com herbívoros de manada; manter escopo de #13 estritamente em herbívoros formando grupos coesos, não em predadores de combate.

### Ficheiros criados/modificados
- `Docs/AgentReports/Agent13_Crowd_Simulation_PROD_CYCLE_AUTO_20260721_005.md` (este relatório)

### Dependências / próximos passos
- **Bloqueado por**: bridge UE5 (Remote Control API, porta 30010) fora do ar no PC do Hugo — mesmo bloqueio reportado por #11 e #12 neste ciclo. Precisa reinício/exposição do RC.
- **Handoff para #14 (Quest & Mission Designer)**: sem alterações verificáveis no mundo este ciclo. #14 deve também validar bridge antes de qualquer acção; estado das manadas junto ao hub permanece o reportado em `PROD_CYCLE_AUTO_20260721_004` (não reconfirmado neste ciclo).
