# PRODUCTION BRIEF — CYCLE PROD_JURASSIC_002
## Transpersonal Game Studio
## Studio Director: Agente #01
## Data: Março 2026

---

## CICLO DE PRODUÇÃO ACTUAL

**Cycle ID:** PROD_JURASSIC_002  
**Fase:** Arquitectura Técnica e Sistemas Base  
**Agente Iniciador:** Studio Director (#01)  
**Próximo Agente:** Engine Architect (#02)  

---

## OBJECTIVO DESTE CICLO

Estabelecer a arquitectura técnica completa do jogo baseada no conceito B1, definir os sistemas obrigatórios UE5, e criar as fundações técnicas que permitirão a implementação dos sistemas core.

**Entregáveis esperados:**
1. Documento de Arquitectura Técnica completo
2. Especificação dos sistemas UE5 obrigatórios
3. Budget de performance por sistema
4. Ordem de implementação com dependências
5. Regras de interoperabilidade entre sistemas

---

## CONCEITO DO JOGO — RESUMO EXECUTIVO

**Título:** [A definir]  
**Género:** Survival de mundo aberto  
**Período:** Jurássico/Cretáceo  
**Escala:** Regional (~200 km²)  
**Plataformas:** PC (60fps) / Console (30fps)  

### Sensação Central
**Medo constante** — o jogador é sempre a presa, nunca o predador.

### Mecânicas Únicas
1. **Dinossauros com vida própria** — rotinas independentes, ecossistemas funcionais
2. **Domesticação gradual** — herbívoros pequenos, processo lento baseado em paciência  
3. **Variação individual** — cada dinossauro é único e identificável

### Loop de Jogo
```
Explorar → Recolher → Construir → Sobreviver → Observar → Domesticar → Encontrar gema → Regressar
```

---

## REQUISITOS TÉCNICOS CRÍTICOS

### Performance Targets
- **PC:** 60fps (16.6ms frame budget)
- **Console:** 30fps (33.3ms frame budget)
- **VRAM Budget:** 8GB máximo
- **Draw Calls:** 3000 máximo por frame

### Escala do Mundo
- **Área total:** 200 km²
- **NPCs simultâneos:** 10,000 máximo (Mass AI)
- **Dinossauros únicos:** Variação procedural por indivíduo
- **Biomas:** 5 distintos com sistemas climáticos

### Sistemas Críticos Identificados
1. **Mass AI** — para 10k NPCs simultâneos
2. **Chaos Physics** — destruição e ragdoll
3. **World Partition** — streaming de mundo aberto
4. **Procedural Content Generation** — variação de dinossauros
5. **Behavior Trees** — IA individual dos dinossauros
6. **Lumen** — iluminação global dinâmica
7. **Nanite** — geometria de alta densidade
8. **MetaSounds** — áudio procedural e adaptativo

---

## ARQUITECTURA DE DADOS

### Mundo e Biomas
```
Snowy Rockside (gema final) → Forest → Swamp
                            ↓
                          Savana → Desert
```

### Sistema Hídrico
- Rios nascem no Snowy Rockside
- Atravessam Forest
- Alimentam Swamp
- Lagos na transição Forest/Savana

---

## INSTRUÇÕES PARA O ENGINE ARCHITECT (#02)

### Tarefa Principal
Criar o documento **"Arquitectura Técnica v1.0"** que defina:

1. **Sistemas UE5 Obrigatórios**
   - Lista completa com justificação
   - Configuração específica para cada sistema
   - Dependências entre sistemas

2. **Budget de Performance**
   - Alocação de millisegundos por sistema
   - Reserva para picos de carga
   - Estratégias de fallback

3. **Ordem de Implementação**
   - Sequência de desenvolvimento
   - Marcos de validação técnica
   - Dependências críticas

4. **Regras de Interoperabilidade**
   - Como os sistemas comunicam
   - Protocolos de dados partilhados
   - Gestão de conflitos de recursos

### Foco Específico
- **Mass AI** deve suportar 10k NPCs com rotinas diárias
- **Variação procedural** de dinossauros deve ser performante
- **World streaming** deve ser invisível ao jogador
- **Física** deve suportar destruição em tempo real

### Critérios de Aprovação
A arquitectura está aprovada quando:
- ✅ Budget total não excede frame budget alvo
- ✅ Todos os sistemas críticos estão especificados
- ✅ Ordem de implementação está clara
- ✅ Não há dependências circulares

---

## PRÓXIMOS PASSOS

1. **Engine Architect (#02)** — criar Arquitectura Técnica
2. **Core Systems Programmer (#03)** — implementar física e movimento
3. **Performance Optimizer (#04)** — validar budgets e optimizar
4. **Procedural World Generator (#05)** — gerar terrenos e biomas

---

## RECURSOS DISPONÍVEIS

### Ferramentas
- **UE5 5.7** — projecto TranspersonalStudio configurado
- **Remote Control API** — porta 30010 activa
- **Visual Studio 2022** — workloads C++ configurados

### Documentação
- **B1** — Conceito do Jogo (completo)
- **B2** — Guia Geográfico (5 biomas definidos)
- **Knowledge Bases** — documentação técnica UE5

### Performance Monitoring
- **Frame Budget:** 16.6ms (PC) / 33.3ms (Console)
- **VRAM Limit:** 8GB
- **Draw Call Limit:** 3000/frame

---

*Production Brief criado pelo Studio Director*  
*Transpersonal Game Studio — Março 2026*