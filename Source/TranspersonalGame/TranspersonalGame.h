// Copyright Transpersonal Game Studio 2026

#pragma once

#include "CoreMinimal.h"

/**
 * Primary game module for Transpersonal Game Studio's Jurassic Survival Game
 * 
 * This module handles the core game systems for a survival game set in the age of dinosaurs.
 * Key features:
 * - Mass AI simulation for up to 50,000 dinosaur NPCs
 * - Procedural world generation with realistic ecosystems
 * - Advanced domestication system based on trust and patience
 * - Individual genetic variation system for unique dinosaurs
 * - Survival mechanics with constant predator/prey tension
 */

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalGame, Log, All);

// Core game constants
namespace TranspersonalGameConstants
{
    // Performance targets
    constexpr int32 TARGET_FPS_PC = 60;
    constexpr int32 TARGET_FPS_CONSOLE = 30;
    constexpr int32 MAX_MASS_ENTITIES = 50000;
    
    // World scale
    constexpr float WORLD_SIZE_KM = 16.0f;
    constexpr float WORLD_SIZE_UU = WORLD_SIZE_KM * 100000.0f; // 16km in Unreal Units
    
    // Gameplay constants
    constexpr float DOMESTICATION_BASE_TIME_HOURS = 48.0f; // Real-time hours for basic trust
    constexpr int32 MAX_GENETIC_VARIATIONS_PER_SPECIES = 1000;
    constexpr float PREDATOR_DETECTION_RANGE = 5000.0f; // 50 meters
}

// Forward declarations for core game classes
class ADinosaurCharacter;
class UEcosystemManager;
class UDomesticationSystem;
class UGeneticVariationSystem;
class USurvivalGameMode;