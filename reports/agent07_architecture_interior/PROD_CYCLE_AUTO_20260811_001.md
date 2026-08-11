# Architecture & Interior Agent #07 — Ciclo PROD_CYCLE_AUTO_20260811_001

## PIE ABERTO: mundo do editor indisponível, não mutei nada.

### Verificação independente (2 chamadas ue5_execute, ambas com método próprio, distinto do #06)

**Chamada 1** — classes de subsistema chamadas incorrectamente sem instância (erro esperado e informativo):
- `UnrealEditorSubsystem.get_editor_world()` chamado na classe → `TypeError: descriptor needs an argument` (confirma que a classe existe mas exige instância)
- Fallback: `world_is_none = True`, `num_actors = 0`
- `EditorLevelLibrary.get_editor_world()` → `None` (API legada, concorda)
- `EditorAssetLibrary.list_assets("/Game/Maps", True, False)` → **0 assets** (Asset Registry ainda não responde — mesmo sinal que #06 reportou)

**Chamada 2** — via `unreal.get_editor_subsystem(...)` (instância correta, método que o #06 não usou):
- `UnrealEditorSubsystem` instanciado → `get_editor_world()` → `None`
- `EditorActorSubsystem` instanciado → `get_all_level_actors()` → `0` actores
- `LevelEditorSubsystem` instanciado → `get_current_level()` → `None`
- `SystemLibrary.get_frame_count()` → `12755824` (motor **vivo**, a avançar frames; ~8000 frames a mais que o valor reportado por #06 minutos antes — confirma que o processo continua a correr, apenas sem nível/mundo carregado)

### Conclusão
Quinta confirmação consecutiva (após #03/#04/#05/#06) de que **não há mundo de editor acessível**: `get_editor_world()=None`, `get_all_level_actors()=0`, `get_current_level()=None`, Asset Registry a devolver 0 resultados mesmo em `/Game/Maps`. O motor está vivo (frame count a incrementar) mas sem nível carregado — condição idêntica à documentada na regra do Brain "ANTES DE TUDO: O PIE PODE ESTAR ABERTO".

Segundo a regra explícita: parei em 2 chamadas em vez de repetir a bateria de testes uma terceira, quarta ou quinta vez com métodos redundantes. Qualquer tentativa de:
- medir/corrigir os 26 Ruin* + 1 StoneRuin* (R18),
- compor o acampamento com props do Tropical_Jungle_Pack (deliverable de criação deste ciclo),
- verificar Camp*/Hearth*/Fire* por label lookup,

...sobre `world=None` produziria **sucesso falso silencioso**: `spawn_actor_from_class` devolve `None` sem erro, `set_actor_location` não faz nada, e qualquer "actor criado" reportado seria alucinação, violando directamente a regra VERIFIED IN WORLD e a regra anti-alucinação.

### Trabalho NÃO executado e porquê
- Correção de assentamento dos 26 Ruin*/1 StoneRuin* (RuinOutpost_Biome_001 a -54uu, Ruin_Rocky_001 a -69uu, Ruin_Cretaceous_001 a -97uu, pendentes de nova medição) — impossível sem `get_actor_bounds`/`line_trace` reais.
- Deliverable de criação (≥3 props do acampamento por full path, seated pela footprint) — impossível sem `spawn_actor_from_class` funcional.
- Reuso de Camp*/Hearth*/Fire* existentes (71/16/12) por label lookup — impossível sem `get_all_level_actors()` a devolver dados reais.

### Dependências para o próximo agente (#8 Lighting & Atmosphere Agent)
- Bloqueio idêntico esperado. **Não repetir a bateria completa** de novo — referenciar este relatório e o do #06 em vez de reabrir a mesma investigação pela sexta vez.
- Sinal a escalar para #1/#2 (acumulando com o do #06): Asset Registry devolve 0 mesmo para paths estáticos como `/Game/Maps` — isto não é apenas "PIE sem nível", é falha de infraestrutura persistente ao longo de pelo menos 2 agentes consecutivos no mesmo ciclo. Motor vivo (frame count a incrementar ~8000 frames entre chamadas de agentes diferentes) mas editor world nulo em todos os subsistemas testados (UnrealEditorSubsystem, EditorActorSubsystem, LevelEditorSubsystem, EditorLevelLibrary). Recomenda-se reinício do editor/reconexão do bridge antes do próximo ciclo produzir qualquer resultado real.
- Quando o mundo voltar, prioridades documentadas mantêm-se: seatar os 27 Ruin*/StoneRuin* pela base dos bounds contra o Landscape (trace ignorando não-Landscape), e compor o campo do protagonista com ≥3 props reais do Tropical_Jungle_Pack (seated pela footprint de 9 pontos), medindo R18/R36 antes e depois.
