#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - SHARED ARCHITECTURE TYPES
 * Engine Architect Agent #02
 * 
 * This file contains ALL shared types used across the entire project.
 * ALL agents must check this file before creating new structs/enums.
 * 
 * NAMING CONVENTION: All types use "Eng_" prefix to avoid conflicts.
 */

// ═══════════════════════════════════════════════════════════════
// CORE SYSTEM ENUMS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0    UMETA(DisplayName = "Critical"),
    High = 1        UMETA(DisplayName = "High"),
    Normal = 2      UMETA(DisplayName = "Normal"),
    Low = 3         UMETA(DisplayName = "Low"),
    Background = 4  UMETA(DisplayName = "Background")
};

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized = 0   UMETA(DisplayName = "Uninitialized"),
    Initializing = 1    UMETA(DisplayName = "Initializing"),
    Running = 2         UMETA(DisplayName = "Running"),
    Paused = 3          UMETA(DisplayName = "Paused"),
    Error = 4           UMETA(DisplayName = "Error"),
    Shutdown = 5        UMETA(DisplayName = "Shutdown")
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Ultra = 0       UMETA(DisplayName = "Ultra"),
    High = 1        UMETA(DisplayName = "High"),
    Medium = 2      UMETA(DisplayName = "Medium"),
    Low = 3         UMETA(DisplayName = "Low"),
    Potato = 4      UMETA(DisplayName = "Potato")
};

// ═══════════════════════════════════════════════════════════════
// PHYSICS SYSTEM TYPES
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EEng_PhysicsLayer : uint8
{
    Default = 0         UMETA(DisplayName = "Default"),
    Terrain = 1         UMETA(DisplayName = "Terrain"),
    Vegetation = 2      UMETA(DisplayName = "Vegetation"),
    Characters = 3      UMETA(DisplayName = "Characters"),
    Dinosaurs = 4       UMETA(DisplayName = "Dinosaurs"),
    Structures = 5      UMETA(DisplayName = "Structures"),
    Projectiles = 6     UMETA(DisplayName = "Projectiles"),
    Triggers = 7        UMETA(DisplayName = "Triggers")
};

UENUM(BlueprintType)
enum class EEng_DestructionLevel : uint8
{
    Indestructible = 0  UMETA(DisplayName = "Indestructible"),
    Minor = 1           UMETA(DisplayName = "Minor"),
    Moderate = 2        UMETA(DisplayName = "Moderate"),
    Major = 3           UMETA(DisplayName = "Major"),
    Complete = 4        UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Gravity = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    EEng_DestructionLevel MaxDestructionLevel = EEng_DestructionLevel::Moderate;

    FEng_PhysicsSettings()
    {
        Gravity = -980.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.05f;
        bEnableRagdoll = true;
        MaxDestructionLevel = EEng_DestructionLevel::Moderate;
    }
};

// ═══════════════════════════════════════════════════════════════
// WORLD GENERATION TYPES
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Grassland = 0       UMETA(DisplayName = "Grassland"),
    Forest = 1          UMETA(DisplayName = "Forest"),
    Desert = 2          UMETA(DisplayName = "Desert"),
    Swamp = 3           UMETA(DisplayName = "Swamp"),
    Mountains = 4       UMETA(DisplayName = "Mountains"),
    River = 5           UMETA(DisplayName = "River"),
    Lake = 6            UMETA(DisplayName = "Lake"),
    Coast = 7           UMETA(DisplayName = "Coast")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 X = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 Y = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bIsLoaded = false;

    FEng_WorldCell()
    {
        X = 0;
        Y = 0;
        BiomeType = EEng_BiomeType::Grassland;
        Elevation = 0.0f;
        Temperature = 20.0f;
        Humidity = 0.5f;
        bIsLoaded = false;
    }
};

// ═══════════════════════════════════════════════════════════════
// CONSCIOUSNESS SYSTEM TYPES
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EEng_ConsciousnessState : uint8
{
    Awake = 0           UMETA(DisplayName = "Awake"),
    Dreaming = 1        UMETA(DisplayName = "Dreaming"),
    Meditation = 2      UMETA(DisplayName = "Meditation"),
    Unconscious = 3     UMETA(DisplayName = "Unconscious"),
    Transcendent = 4    UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ConsciousnessData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EEng_ConsciousnessState State = EEng_ConsciousnessState::Awake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Awareness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Coherence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Resonance = 0.0f;

    FEng_ConsciousnessData()
    {
        State = EEng_ConsciousnessState::Awake;
        Awareness = 1.0f;
        Coherence = 1.0f;
        Resonance = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// PERFORMANCE MONITORING TYPES
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceTier CurrentTier = EEng_PerformanceTier::High;

    FEng_PerformanceMetrics()
    {
        FrameRate = 60.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        CurrentTier = EEng_PerformanceTier::High;
    }
};

// ═══════════════════════════════════════════════════════════════
// SYSTEM COMMUNICATION TYPES
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString MessageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString Data;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority = EEng_SystemPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float Timestamp = 0.0f;

    FEng_SystemMessage()
    {
        SystemName = TEXT("");
        MessageType = TEXT("");
        Data = TEXT("");
        Priority = EEng_SystemPriority::Normal;
        Timestamp = 0.0f;
    }
};