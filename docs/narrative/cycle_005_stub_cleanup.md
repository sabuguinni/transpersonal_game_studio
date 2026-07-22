# Narrative & Dialogue Agent — Cycle PROD_CYCLE_AUTO_20260722_005

## Ação: Cleanup one-time de stubs (memória `hugo_cleanup_stubs_cycle15_v1`)

### Auditoria prévia
Listagem de todos os actores `NPC_*`, `*Dialogue*`, `*Label*` perto do hub e no mundo (190 NPC-like, 61 dialogue/label-like). Objetivo: identificar placeholders nunca preenchidos (texto literal `"Text"`) vs. conteúdo narrativo real.

### Verificação individual (obrigatória antes de apagar)
Verifiquei o `TextRenderComponent.text` de 8 candidatos que batiam nos padrões da memória:

| Label | Texto encontrado | Ação |
|---|---|---|
| NPC_Scout_Tracker | `Text` | ❌ Deletado |
| NPC_Hunter_Wounded | `Text` | ❌ Deletado |
| TribeElder_DialogueTrigger_Label | `Text` | ❌ Deletado |
| HuntLeader_DialogueTrigger_Label | `Text` | ❌ Deletado |
| ScoutNPC_DialogueTrigger_Label | `Text` | ❌ Deletado |
| WarriorNPC_DialogueTrigger_Label | `Text` | ❌ Deletado |
| Label_ElderKara | `Text` | ❌ Deletado |
| Label_CrafterBron | `Text` | ❌ Deletado |

**Todos os 8 confirmaram texto exatamente `"Text"` (stub vazio nunca preenchido)** → apagados conforme regra. `DELETED_COUNT = 8`.

### Preservação de conteúdo narrativo real (NÃO tocado)
Confirmei que os NPCs de diálogo reais no hub continuam intactos e bem posicionados:
- `NPC_Elder_Koru` — (2427.3, 2091.2, 240.0)
- `NPC_Hunter_Brak` — (2522.9, 2246.1, 240.0)
- `NPC_Scout_Mira` — (1700.0, 2600.0, 240.0), texto confirmado `"MIRA"` (conteúdo real, preservado)

Estes NÃO foram tocados — têm texto narrativo genuíno, não placeholders.

### Verificação
- 3x `ue5_execute` (`command_type=python`), todos `status:completed`, ~3s cada.
- Save do level confirmado `True` (via `EditorLevelLibrary.save_current_level`, deprecated mas funcional).

## Resultado
- 8 stubs de placeholder eliminados do mundo (regra `hugo_cleanup_stubs_cycle15_v1` — cumprida e não deve repetir-se em ciclos futuros).
- 0 conteúdo narrativo real afetado.
- NPCs de diálogo do hub (Koru, Brak, Mira) confirmados presentes, grounded (z=240) e com texto válido.

## Próximo agente (#16 Audio Agent)
- Os 3 NPCs de diálogo do hub (Elder Koru, Hunter Brak, Scout Mira) estão prontos para receber voice lines/MetaSounds — não há mais placeholders a confundir a lista de dialogue triggers.
- Continuam a existir várias entradas `DialogueZone_*` e `AudioZone_*` duplicadas (ex: `AudioZone_TribeCamp`, `AudioZone_Hunter_001`, `AudioZone_Scout_Mira`, `AudioZone_Hunter_Brak`) que valeria a pena o Audio Agent consolidar seguindo a regra `hugo_naming_dedup_v3` (reutilizar em vez de duplicar).
