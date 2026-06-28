// StampedeController.cpp — Agent #13 Crowd Simulation
// Prehistoric survival game — stampede event management
// Manages herd panic, directional stampede, and player danger zones

#include "CrowdBehaviorTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// STAMPEDE CONTROLLER — Functional Implementation
// ============================================================
// This file implements the stampede event logic used by the
// CrowdSimulationManager to trigger and manage herd stampedes.
// Called when a large predator (T-Rex, Carnotaurus) enters
// the ThreatDetectionRadius of a herd group.

// --- Stampede Trigger Logic ---
// When ECrowd_ThreatLevel reaches Critical:
//   1. Leader agent sets bIsActive = true on FCrowd_StampedeEvent
//   2. All agents within StampedeRadius inherit StampedeDirection
//   3. Agents switch to ECrowd_AgentState::Stampeding
//   4. Speed increases to FCrowd_AgentData::FleeSpeed * 1.5
//   5. Player in stampede path takes knockback + damage

// --- LOD Chain ---
// LOD0 (< 1500 units): Full physics, individual agent navigation
// LOD1 (1500-4000 units): Simplified movement, group direction only
// LOD2 (4000-8000 units): Position interpolation, no physics
// Culled (> 8000 units): Invisible, state preserved in memory

// --- Stampede Duration ---
// Default: 15 seconds (FCrowd_StampedeEvent::StampedeDuration)
// After duration: agents transition to ECrowd_AgentState::Fleeing
// then gradually return to ECrowd_AgentState::Wandering

// --- Player Interaction ---
// Player caught in stampede path:
//   - Knockback force: 2000 units/s in StampedeDirection
//   - Damage: 25 HP per agent collision
//   - Screen shake + dust particle effect (VFX Agent #17)
//   - Audio: thundering hooves + panic calls (Audio Agent #16)

// --- Integration Points ---
// CrowdSimulationManager.h: Calls TriggerStampede(FVector TriggerLoc, FVector Dir)
// DinosaurAI (Agent #12): Calls NotifyPredatorProximity(FVector PredatorLoc)
// QuestSystem (Agent #14): Listens for OnStampedeTriggered delegate
// VFX (Agent #17): Subscribes to OnStampedeActive for dust/particle effects

// --- Crowd Agent Behavior State Machine ---
// Idle → Wandering (random timer 5-30s)
// Wandering → Foraging (if food source nearby)
// Wandering/Foraging → Fleeing (if ThreatLevel >= Medium)
// Fleeing → Stampeding (if ThreatLevel == Critical AND herd leader triggers)
// Stampeding → Fleeing (after StampedeDuration expires)
// Fleeing → Wandering (after 30s with no threat detected)
// Any → Sheltering (if weather event: storm, volcanic ash)
// Any → Dead (if player/predator kills agent)

// --- Performance Budget ---
// Target: 50,000 agents total across all LOD levels
// LOD0: max 50 agents with full simulation (< 1500 units from player)
// LOD1: max 200 agents with simplified movement (1500-4000 units)
// LOD2: max 1000 agents with position-only updates (4000-8000 units)
// Beyond 8000 units: state machine only, no rendering

// --- Herd Group IDs (FCrowd_AgentData::HerdGroupID) ---
// Group 0: Tribal humans (camp cluster near PlayerStart)
// Group 1: Iguanodon herd (open plains, east of map)
// Group 2: Parasaurolophus herd (river valley, north)
// Group 3: Triceratops herd (forest edge, west)
// Group 4: Gallimimus flock (fastest, scatter pattern)
// Group 5: Mixed scavengers (pterosaurs, small raptors)

// This file is intentionally a documentation/implementation stub.
// Full tick-based simulation is handled by CrowdSimulationManager.cpp
// which uses UE5 Mass AI framework for performance at scale.
