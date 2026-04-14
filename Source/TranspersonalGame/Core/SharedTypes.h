#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - SHARED TYPES
 * Engine Architect #02 - Core Architecture Definition
 * 
 * This file contains ALL shared types used across the entire project.
 * ALL agents MUST use these types for cross-system communication.
 * 
 * CRITICAL: Before creating ANY new struct/enum, check if it exists here first.
 * Use the "Eng_" prefix for all architecture-level types.
 */

// ═══════════════════════════════════════════════════════════════
// CORE ARCHITECTURE ENUMS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical - Physics/Input"),
    High        UMETA(DisplayName = "High - Gameplay"),
    Medium      UMETA(DisplayName = "Medium - AI/Audio"),
    Low         UMETA(DisplayName = "Low - VFX/UI"),
    Background  UMETA(DisplayName = "Background - Streaming")
};

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra - High-end PC"),
    High        UMETA(DisplayName = "High - Mid-range PC"),
    Medium      UMETA(DisplayName = "Medium - Console/Low PC"),
    Low         UMETA(DisplayName = "Low - Mobile/Minimum"),
    Potato      UMETA(DisplayName = "Potato - Emergency fallback")
};

UENUM(BlueprintType)
enum class EEng_WorldScale : uint8
{
    Micro       UMETA(DisplayName = "Micro - Under 1km²"),
    Small       UMETA(DisplayName = "Small - 1-4km²"),
    Medium      UMETA(DisplayName = "Medium - 4-16km²"),
    Large       UMETA(DisplayName = "Large - 16-64km²"),
    Massive     UMETA(DisplayName = "Massive - Over 64km²")
};

UENUM(BlueprintType)
enum class EEng_SimulationComplexity : uint8
{
    Simple      UMETA(DisplayName = "Simple - Basic physics"),
    Standard    UMETA(DisplayName = "Standard - Full physics"),
    Complex     UMETA(DisplayName = "Complex - Advanced simulation"),
    Extreme     UMETA(DisplayName = "Extreme - Research-level")
};

// ═══════════════════════════════════════════════════════════════
// PERFORMANCE ARCHITECTURE STRUCTS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceConstraints
{
    GENERATED_BODY()

    // Target framerates (MANDATORY limits)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS_PC = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS_Console = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MinimumFPS = 20;

    // Memory constraints (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryUsage_PC = 8192;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryUsage_Console = 4096;

    // Entity limits for Mass simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleEntities = 2000;

    // LOD distances (Unreal Units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD0_Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD1_Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD2_Distance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 15000.0f;

    FEng_PerformanceConstraints()
    {
        // Default constructor with safe values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority = EEng_SystemPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bTickEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float TickInterval = 0.0f; // 0 = every frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 MaxInstancesPerFrame = 100;

    FEng_SystemConfiguration()
    {
        SystemName = TEXT("UnnamedSystem");
    }
};

// ═══════════════════════════════════════════════════════════════
// WORLD ARCHITECTURE STRUCTS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    EEng_WorldScale WorldScale = EEng_WorldScale::Large;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bUseLevelStreaming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 StreamingPoolSize = 512; // MB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float StreamingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxStreamingLevels = 16;

    // Procedural generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bUsePCG = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 PCG_Seed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float TerrainScale = 100.0f;

    FEng_WorldConfiguration()
    {
        // Default constructor
    }
};

// ═══════════════════════════════════════════════════════════════
// PHYSICS ARCHITECTURE STRUCTS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    EEng_SimulationComplexity SimulationLevel = EEng_SimulationComplexity::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bUseChaosPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsTimeStep = 0.016667f; // 60Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxPhysicsObjects = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float CollisionTolerance = 0.1f;

    FEng_PhysicsConfiguration()
    {
        // Default constructor
    }
};

// ═══════════════════════════════════════════════════════════════
// RENDERING ARCHITECTURE STRUCTS  
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_RenderingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bUseLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bUseNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bUseVirtualShadowMaps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 MaxDrawCalls = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 MaxTriangles = 2000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ViewDistance = 20000.0f;

    FEng_RenderingConfiguration()
    {
        // Default constructor
    }
};

// ═══════════════════════════════════════════════════════════════
// MASTER ARCHITECTURE CONFIGURATION
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MasterArchitecture
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_PerformanceConstraints Performance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_WorldConfiguration World;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_PhysicsConfiguration Physics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_RenderingConfiguration Rendering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_PerformanceLevel TargetPerformanceLevel = EEng_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bDebugMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bProfilingEnabled = true;

    FEng_MasterArchitecture()
    {
        // Initialize with safe defaults
    }
};