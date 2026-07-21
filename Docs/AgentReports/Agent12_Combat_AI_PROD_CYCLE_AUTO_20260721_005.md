# Combat & Enemy AI Agent #12 — Relatório de Ciclo (PROD_CYCLE_AUTO_20260721_005)

## Estado do Bridge: DOWN (confirmado, 2x)

- **Bridge validation** (`import unreal; get_editor_world()`) — FAIL: `Connection refused` na porta 30010 (Remote Control API), ~3.0s.
- **Retry minimal** (`print("retry_ok")`) — FAIL: mesmo erro `Connection refused`, ~3.0s.
- Total: 2 tentativas reais de `ue5_execute`, ambas falharam por indisponibilidade do Remote Control API no PC do Hugo. Consistente com o relatado pelo #11 no mesmo ciclo (mesma falha, mesma porta).

## Ações tomadas seguindo DEGRADED MODE ENFORCEMENT (memória global imp:10)

Confirmado bridge DOWN após validação + retry, ambos FAIL → execução de tools visuais/áudio bloqueada nesta sessão (`generate_image`, `text_to_speech`). Não escrevi `.cpp`/`.h` (regra absoluta `hugo_no_cpp_h_v2`, imp:20 — a directiva específica deste agente pedia `SurvivalHUD.cpp` e ficheiros de gameplay `.cpp`, o que é incompatível com esta regra: C++ é inerte neste editor headless, qualquer escrita seria 100% desperdício).

## Contexto herdado (não repetir trabalho)

- Ciclos anteriores (PROD_003/004) confirmaram: 58 actors "TRex" existentes no nível são todos `NiagaraActor`/`StaticMeshActor`/Emitter/markers — **nenhum é Pawn real com AIController**. Nenhuma IA de combate real existe ainda no mundo.
- Existem exatamente **1x `CombatZone_Raptor_Hub`** e **1x `BehaviorTag_Raptor_Hub`** perto do hub (2100,2400) — confirmado sem duplicados no PROD_004. Regra de dedup (`hugo_combat_label_consistency_v1`) continua válida para o próximo ciclo com bridge ativo: procurar labels `CombatZone_*`/`BehaviorTag_*` existentes antes de criar novos.
- #11 (NPC Behavior) reportou o mesmo bloqueio de bridge neste ciclo — falha é de infraestrutura (Remote Control API na porta 30010), não de código ou lógica de agente.

## Plano concreto para quando o bridge voltar

1. Fazer bridge validation real primeiro (`get_editor_world()`).
2. Escolher **1 dos 58 actors "TRex"** existentes (Niagara/StaticMesh) mais próximo do hub jogável, obter a sua posição XY, fazer line trace ao Landscape para z real.
3. Spawnar um Pawn real (ex: classe base `Character` ou esqueleto disponível) na posição correta com colisão sã (sem overlap com outros actors funcionais), respeitando a cápsula do jogador (raio 34, meia-altura 88) para garantir que a zona continua caminhável.
4. Adicionar/atualizar (não duplicar) o par `CombatZone_TRex_*` / `BehaviorTag_TRex_*` associado a esse Pawn, seguindo a regra de dedup — procurar primeiro labels existentes num raio de ~3500 unidades do hub.
5. Configurar comportamento básico de combate tático (perseguição/ataque) via Blueprint/Python no AIController do Pawn, não via `.cpp`.
6. Guardar o nível apenas 1 vez, no final, depois de tudo verificado.

## Ficheiros criados/modificados

- `Docs/AgentReports/Agent12_Combat_AI_PROD_CYCLE_AUTO_20260721_005.md` (este relatório)

## Dependências / próximos passos

- **Bloqueado por**: bridge UE5 (Remote Control API, porta 30010) fora do ar — requer intervenção do Hugo para reiniciar/expor o RC no editor do PC.
- **Próximo agente (#13 Crowd & Traffic Simulation)**: mesmo bloqueio de bridge deve ser esperado até o Hugo confirmar que o RC foi reiniciado. Quando o bridge voltar, a prioridade de #12 continua a ser converter 1 dos 58 "TRex" existentes num Pawn real antes de qualquer simulação de massa os poder tratar como agentes vivos.
