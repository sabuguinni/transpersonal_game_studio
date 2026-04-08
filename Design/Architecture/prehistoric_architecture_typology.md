# TIPOLOGIA ARQUITECTÓNICA — MUNDO PRÉ-HISTÓRICO
## Transpersonal Game Studio — Architecture & Interior Agent
### Versão 1.0 — Março 2026

---

## PRINCÍPIOS FUNDAMENTAIS

### 1. ARQUITECTURA COMO NARRATIVA
Cada estrutura é um documento arqueológico. Os espaços contam histórias através de:
- **Marcas de uso** — desgaste em soleiras, manchas de fumo, riscos em madeira
- **Objectos abandonados** — ferramentas, utensílios, restos de refeições
- **Sinais de pressa** — portas deixadas abertas, objectos derrubados
- **Adaptações defensivas** — barricadas improvisadas, buracos de observação

### 2. VULNERABILIDADE COMO DESIGN
Toda a arquitectura reflecte a posição do humano como presa:
- **Entradas elevadas** — escadas recolhíveis, plataformas suspensas
- **Múltiplas saídas de emergência** — nunca apenas uma porta
- **Campos de visão** — janelas estratégicas para observar predadores
- **Materiais locais** — madeira, pedra, fibras vegetais disponíveis

### 3. INTEGRAÇÃO ECOLÓGICA
As estruturas coexistem com o ambiente natural:
- **Construção em árvores** — aproveitando troncos robustos
- **Camuflagem natural** — coberturas de vegetação, cores terrosas
- **Drenagem adaptada** — chuvas torrenciais do período jurássico
- **Ventilação cruzada** — clima húmido e quente

---

## CATEGORIAS ARQUITECTÓNICAS

### A. ABRIGOS DE SOBREVIVÊNCIA (Construíveis pelo Jogador)

#### A1. LEAN-TO BÁSICO
**Função:** Primeiro abrigo de emergência
**Materiais:** Galhos, folhas grandes, cipós
**Características:**
- Construção rápida (1-2 horas de jogo)
- Protecção contra chuva
- Visibilidade limitada do exterior
- Vulnerável a predadores grandes

#### A2. PLATAFORMA ELEVADA
**Função:** Abrigo defensivo intermédio
**Materiais:** Troncos, cordas de fibra, folhagem
**Características:**
- Elevada 3-4 metros do solo
- Escada recolhível
- Vista panorâmica da área
- Resistente a predadores terrestres pequenos/médios

#### A3. CASA NA ÁRVORE FORTIFICADA
**Função:** Base permanente avançada
**Materiais:** Madeira trabalhada, pedra, fibras tecidas
**Características:**
- Múltiplos níveis
- Sistema de polias para recursos
- Armazenamento seguro
- Defesas contra predadores voadores

### B. ESTRUTURAS ABANDONADAS (Exploráveis)

#### B1. ACAMPAMENTO DE EXPLORADORES
**História:** Expedição científica anterior que desapareceu
**Estado:** Parcialmente destruído, sinais de luta
**Conteúdo:**
- Equipamento científico danificado
- Diários com anotações sobre dinossauros
- Mapas parciais da região
- Restos de barricadas improvisadas

**Layout Interior:**
```
[Entrada Principal] → [Área de Trabalho] → [Dormitórios]
                   ↓
              [Armazém] → [Saída de Emergência]
```

#### B2. POSTO AVANÇADO MILITAR
**História:** Base militar de uma expedição de resgate
**Estado:** Abandonado ordenadamente, equipamento removido
**Conteúdo:**
- Estruturas de comunicação não-funcionais
- Depósito de munições vazio
- Planos de evacuação na parede
- Torre de observação parcialmente destruída

#### B3. LABORATÓRIO SUBTERRÂNEO
**História:** Instalação de pesquisa secreta
**Estado:** Selado, energia de emergência ainda funciona
**Conteúdo:**
- Espécimes preservados em tanques
- Computadores com dados corrompidos
- Sistemas de segurança ainda activos
- Câmaras de contenção vazias

### C. RUÍNAS ANTIGAS (Misteriosas)

#### C1. CÍRCULOS DE PEDRA
**Origem:** Desconhecida, pré-datam presença humana moderna
**Características:**
- Pedras dispostas em padrões geométricos
- Cristais incrustados que brilham ao anoitecer
- Possível ligação com as gemas de transporte temporal
- Dinossauros evitam a área

#### C2. ESTRUTURAS MEGALÍTICAS
**Origem:** Civilização perdida ou fenómeno natural
**Características:**
- Blocos de pedra impossíveis de mover manualmente
- Câmaras internas com ecos estranhos
- Gravuras que parecem mapas estelares
- Campos magnéticos anómalos

---

## ESPECIFICAÇÕES TÉCNICAS DE IMPLEMENTAÇÃO

### SISTEMA DE CONSTRUÇÃO PROCESSUAL
Usando PCG Framework do UE5 para gerar variações:

#### Parâmetros Base:
- **Estado de Conservação:** 0.0 (ruína) a 1.0 (novo)
- **Nível de Defesa:** 0.0 (vulnerável) a 1.0 (fortificado)
- **Integração Natural:** 0.0 (artificial) a 1.0 (camuflado)
- **Complexidade:** 0.0 (simples) a 1.0 (multi-nível)

#### Elementos Modulares:
- **Fundações:** Estacas, plataformas, escavações
- **Paredes:** Troncos, pedra empilhada, tecidos
- **Coberturas:** Folhagem, peles, estruturas inclinadas
- **Acessos:** Escadas, cordas, rampas, pontes

### NARRATIVA AMBIENTAL
Cada estrutura contém layers de storytelling:

#### Layer 1: Função Original
- Disposição dos espaços
- Ferramentas e equipamentos
- Sinais de actividade quotidiana

#### Layer 2: Eventos Disruptivos
- Danos estruturais
- Objectos fora do lugar
- Sinais de fuga ou luta

#### Layer 3: Ocupação Natural
- Vegetação invasiva
- Ninhos de animais
- Erosão e desgaste

#### Layer 4: Pistas Narrativas
- Documentos e mapas
- Objectos pessoais
- Marcas e gravuras

---

## INTEGRAÇÃO COM OUTROS SISTEMAS

### DINOSSAUROS E FAUNA
- **Estruturas como território:** Alguns dinossauros podem usar ruínas como ninhos
- **Rotas de migração:** Edifícios não bloqueiam caminhos naturais
- **Comportamento adaptativo:** Predadores aprendem a contornar defesas

### VEGETAÇÃO E AMBIENTE
- **Crescimento sobre estruturas:** Ivy, musgo, raízes
- **Materiais de construção:** Árvores específicas para diferentes usos
- **Camuflagem natural:** Estruturas que se misturam com a floresta

### SISTEMA DE CRAFTING
- **Materiais de demolição:** Estruturas abandonadas fornecem recursos
- **Blueprints encontrados:** Plantas arquitectónicas em ruínas
- **Evolução tecnológica:** Do primitivo ao sofisticado

---

## GUIDELINES DE LEVEL DESIGN

### DENSIDADE E DISTRIBUIÇÃO
- **1 estrutura major por 500m²** — evitar sobrepopulação
- **Clusters narrativos** — 3-5 estruturas relacionadas contam uma história
- **Marcos visuais** — torres e estruturas altas como pontos de referência

### ACESSIBILIDADE E FLOW
- **Múltiplos caminhos** — nunca apenas uma rota de acesso
- **Escalabilidade vertical** — aproveitar árvores e elevações naturais
- **Pontos de descanso** — áreas seguras para planeamento

### BALANCEAMENTO DE GAMEPLAY
- **Risco vs. Recompensa** — estruturas perigosas contêm melhores recursos
- **Progressão lógica** — complexidade aumenta com distância do spawn
- **Rejogabilidade** — layouts procedurais mantêm exploração fresca

---

*Documento criado pelo Architecture & Interior Agent*
*Transpersonal Game Studio — Março 2026*