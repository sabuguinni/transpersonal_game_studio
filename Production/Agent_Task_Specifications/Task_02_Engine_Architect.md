# TASK SPECIFICATION — ENGINE ARCHITECT (#02)
## Cycle: PROD_JURASSIC_002
## Transpersonal Game Studio

---

## TASK OVERVIEW

**Agent:** #02 Engine Architect  
**Previous Agent:** #01 Studio Director  
**Next Agent:** #03 Core Systems Programmer  
**Priority:** CRITICAL — blocks all technical agents  

---

## DELIVERABLE REQUIRED

### Primary Output
**"Technical Architecture v1.0"** — Complete technical specification document

### Must Include
1. **UE5 Systems Specification**
   - Complete list of required UE5 systems
   - Configuration parameters for each system
   - Justification for each system choice

2. **Performance Budget Allocation**
   - Millisecond allocation per system
   - Frame budget breakdown (16.6ms PC / 33.3ms Console)
   - Performance monitoring strategy

3. **Implementation Order**
   - Development sequence with dependencies
   - Critical path identification
   - Validation milestones

4. **System Interoperability Rules**
   - Data sharing protocols
   - Communication interfaces
   - Resource conflict resolution

---

## SPECIFIC REQUIREMENTS

### Mass AI System
- **Target:** 10,000 simultaneous NPCs
- **Requirement:** Individual behavior trees + daily routines
- **Performance:** Must fit within allocated budget
- **Integration:** With World Partition for streaming

### Procedural Dinosaur Variation
- **Target:** Unique physical variations per individual
- **Requirement:** Not just color — structure (horns, eyes, posture)
- **Performance:** Generation must be real-time
- **Memory:** Variations must be memory-efficient

### World Streaming
- **Target:** 200 km² seamless world
- **Requirement:** Invisible loading to player
- **Integration:** With Mass AI, procedural generation
- **Performance:** No frame drops during streaming

### Physics & Destruction
- **System:** Chaos Physics
- **Requirements:** Real-time destruction, ragdoll
- **Integration:** With Mass AI for NPC interactions
- **Performance:** Must scale with NPC count

---

## TECHNICAL CONSTRAINTS

### Performance Limits
- **PC:** 60fps (16.6ms frame budget)
- **Console:** 30fps (33.3ms frame budget)
- **VRAM:** 8GB maximum
- **Draw Calls:** 3000 per frame maximum

### Platform Requirements
- **Primary:** PC (Windows)
- **Secondary:** Console (PlayStation/Xbox)
- **Engine:** UE5 5.7
- **Language:** C++ for core systems

---

## SUCCESS CRITERIA

The architecture is approved when:
- ✅ Total budget allocation ≤ frame budget target
- ✅ All critical systems are specified
- ✅ Implementation order has no circular dependencies
- ✅ Each system has clear performance metrics
- ✅ Integration points are well-defined

---

## REFERENCE DOCUMENTS

### Available Knowledge Base
- **B1_conceito_do_jogo.md** — Game concept and mechanics
- **B2_guia_geografico_v1.md** — World structure and biomes
- **UE5 Technical Documentation** — In agent knowledge base

### Key Sections to Reference
- **Game Loop:** Explore → Gather → Build → Survive → Observe → Tame → Find Gem → Return
- **Unique Mechanics:** Independent dinosaur lives, gradual taming, individual variation
- **Core Feeling:** Constant fear — player is always prey

---

## OUTPUT FORMAT

### Document Structure
```
# Technical Architecture v1.0

## Executive Summary
## UE5 Systems Specification
## Performance Budget Allocation
## Implementation Order & Dependencies
## System Interoperability Rules
## Validation & Testing Strategy
## Risk Assessment & Mitigation
```

### Approval Message
When complete, send exact message to Studio Director:
> "Technical Architecture v[number] completed. Systems defined: [N]. Total budget allocated: [X]ms of [Y]ms available. Ready for distribution to technical agents."

---

## NEXT AGENT PREPARATION

### For Core Systems Programmer (#03)
Prepare specifications for:
- Character Movement Component
- Chaos Physics configuration
- Collision system setup
- Ragdoll implementation

### Dependencies to Define
- Which systems must be implemented first
- What interfaces need to be established
- Performance monitoring requirements

---

*Task Specification created by Studio Director*  
*Cycle PROD_JURASSIC_002 — Transpersonal Game Studio*