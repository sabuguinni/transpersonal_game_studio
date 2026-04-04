# TRANSPERSONAL GAME STUDIO — ESTADO SESSÃO 3
## Abril 2026 · Hugo & Miguel Martins

---

## O QUE FOI FEITO NESTA SESSÃO

### Make.com — Orquestração completa
19 cenários criados e activos. O ciclo completo de comunicação entre os 19 agentes está operacional. Cada agente tem a variable `make_webhook_[próximo_agente]` configurada com o URL do Make.com correspondente. Ver documento `transpersonal_studio_make_estado_v1.md` para detalhe completo.

### GitHub
- Repositório: `sabuginni/transpersonal_game_studio`
- Branch principal: `main`
- Ficheiros UE5 carregados: `Config/`, `Source/`, `TranspersonalStudio.uproject`, `.gitignore`
- GitHub Desktop instalado e autenticado com conta Google OAuth
- Pasta local do repositório: `C:\Unreal Projects\transpersonal_game_studio_remote\transpersonal_game_studio`

### Documentos internos criados com o Miguel
Todos os documentos foram criados com input directo do Miguel e carregados nas KBs dos agentes correctos.

| Documento | Versão | Agentes com KB |
|-----------|--------|----------------|
| B2 — Premissa Narrativa | v1.1 | Studio Director, Narrative & Dialogue, Quest Designer, NPC Behavior |
| B3 — Style Guide Visual | v1.0 | Environment Artist, Architecture & Interior, Lighting & Atmosphere, Character Artist, VFX Agent |
| B4 — Guia Geográfico do Mundo | v1.0 | Studio Director, Procedural World Generator, Environment Artist |
| B5 — Referências de Áudio | v1.0 | Audio Agent |

---

## ESTADO COMPLETO DO PROJECTO

### Relevance AI
- ✅ 19 agentes configurados e publicados
- ✅ Long-Term Memory activa nos 12 agentes que precisam
- ✅ KBs carregadas por agente (KB específica + B1 + documentos B2-B5 conforme relevância)
- ✅ Variables make_webhook configuradas em todos os agentes
- ✅ Todos os agentes republicados após carregamento dos novos documentos

### Make.com
- ✅ 19 cenários activos (ver `transpersonal_studio_make_estado_v1.md`)
- ✅ Todos os cenários com toggle "Immediately as data arrives" activo

### UE5
- ✅ Visual Studio 2022 instalado
- ✅ Projecto `TranspersonalStudio` em `C:\Unreal Projects\TranspersonalStudio`
- ✅ Remote Control API activa em `http://localhost:30010`

### GitHub
- ✅ Repositório `sabuginni/transpersonal_game_studio` com ficheiros UE5
- ✅ GitHub Desktop instalado e autenticado

### Documentos internos
- ✅ B1 — Conceito do Jogo (sessões anteriores)
- ✅ B2 — Premissa Narrativa v1.1
- ✅ B3 — Style Guide Visual v1.0
- ✅ B4 — Guia Geográfico do Mundo v1.0
- ✅ B5 — Referências de Áudio v1.0

---

## PONTOS-CHAVE DOS DOCUMENTOS DO MIGUEL

### Premissa Narrativa (B2)
- Protagonista: paleontologista transportado ao tocar numa gema
- Começa na Forest — sozinho, sem outros humanos
- Exploração livre — sem guias ou obrigações de percurso
- Construção com materiais naturais (madeira, pedra)
- Ferramenta básica (rústica) e ferramenta avançada (mais elaborada)
- Missões claras e directas — não são puzzles
- Modo criativo: recursos infinitos, configurações ajustáveis
- Fim: escolha entre voltar ao presente ou ficar com os dinossauros
- Gema só aparece no Snowy Rockside após todas as missões concluídas

### Style Guide Visual (B3)
- Referências: ARK, Jurassic World Evolution 3
- Dia: azul, colorido, visível — como vida real
- Noite: roxo escuro, estrelas, aurora boreal ocasional
- Dinossauros: realismo científico — não como Jurassic Park
- Variação física individual por dinossauro
- HUD minimalista

### Guia Geográfico (B4)
- Mundo: ~200 km², fechado por montanhas intransponíveis
- 5 biomas: Forest, Swamp, Savana, Desert, Snowy Rockside
- Início do jogador: sempre na Forest
- Rios nascem nas montanhas e fluem para o interior
- Rios mais secos na Savana e Desert, congelados no Snowy Rockside
- Sem ruínas — mundo completamente natural
- Cavernas existem em vários biomas

### Referências de Áudio (B5)
- Música adaptativa: calma na exploração, intensa na fuga/ataque
- Música pode ser ligada/desligada nas definições
- Sons de ambiente sempre activos
- Dinossauros: sons realistas no mundo aberto
- Nas missões: todos os carnívoros têm sons dramáticos (tipo filme)

---

## PRÓXIMOS PASSOS

### Imediato
- [ ] Google Drive — criar estrutura `/Studio/` com 13 subpastas
- [ ] Configurar variables google_drive_* nos agentes

### A seguir
- [ ] Notion — bases de dados de NPCs, missões e bugs
- [ ] Configurar variables notion_* nos agentes
- [ ] Testar o fluxo completo: Miguel → Studio Director → Engine Architect → resposta

### Futuro
- [ ] ElevenLabs — ligar ao Audio Agent
- [ ] Freesound — ligar ao Audio Agent
- [ ] Activar Remote Control UE5 nos agentes que precisam

---

## NOTAS TÉCNICAS IMPORTANTES

- GitHub Desktop usa autenticação Google OAuth — não usa password nem token no terminal
- Pasta do repositório local: `C:\Unreal Projects\transpersonal_game_studio_remote\transpersonal_game_studio`
- Pasta do projecto UE5: `C:\Unreal Projects\TranspersonalStudio` (separada do repositório)
- Remote Control UE5: `http://localhost:30010/remote/info`
- Make.com — todos os cenários usam Authentication None, Method POST, Body application/json

---

*Transpersonal Game Studio · Hugo & Miguel Martins · Abril 2026*
*Estado da Sessão 3 — Make.com + GitHub + Documentos B2-B5*
