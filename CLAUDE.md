
## DISCIPLINA DE MEMÓRIA — ClaudeBrain vs memória local (CRÍTICO)

Há DOIS sítios de memória, e confundi-los perde conhecimento:
- **ClaudeBrain** (brain.transpersonalinternational.com) — PERSISTENTE e VISÍVEL aos 20 agentes de produção. É aqui que vive o conhecimento que os agentes precisam.
- **Memória local do Claude Code** (~/.claude/...) — só existe nas minhas sessões, INVISÍVEL aos agentes.

### Regra de gravação (aplicar SEMPRE):
Quando descubro ou corrijo algo, decido o sítio pelo CRITÉRIO:

**VAI PARA O BRAIN** (curl POST, scope=global, project=game_studio, confirmar por /get):
- Regras técnicas duráveis (ex.: "nunca fazer X", "a ferramenta Y agora faz Z")
- Lições de arquitectura (ex.: "o jogador vem da classe, não de actores colocados")
- Estados críticos que mudam como os agentes devem agir (ex.: "o bridge agora devolve stdout")
- Correcções que, se os agentes não souberem, os fazem repetir um erro
- importance=20 para regras críticas; 10 para contexto útil mas não crítico
- source=hugo_<nome_descritivo>_v1 ; tags relevantes

**FICA LOCAL** (memória do Claude Code, não Brain):
- Estado operacional da sessão (métodos de acesso SSH, credenciais, estado de sondas em curso)
- Detalhes de debugging que não são lição durável
- Notas de progresso de uma tarefa específica

**EM DÚVIDA se algo é durável → PERGUNTAR AO HUGO antes de assumir.**

### Verificação obrigatória:
- Antes de gravar no Brain: /get com termo único (o /search faz AND) para não duplicar.
- Depois de gravar: /get para CONFIRMAR (nunca confiar no success:true — regra 5).
- Token Brain: Bearer 776285e52a691f0148af147dcd0f4b3e3209682ee9bb2843adb8ee5a980bbf62

### Nunca dizer 'registei na memória' de forma ambígua:
Especificar sempre: 'gravei no BRAIN (id=X)' ou 'registei na memória LOCAL da sessão'. A ambiguidade entre os dois foi a causa de 3 dias de conhecimento (19-21/07) ficar invisível aos agentes.
