#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.generated.h"

// ============================================================================
// CORE ENUMS - Used across all systems
// ============================================================================

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical    = 0,    // Physics, Core Systems
    High        = 1,    // AI, Combat
    Medium      = 2,    // Environment, Audio
    Low         = 3,    // VFX, UI
    Background  = 4     // Analytics, Logging
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Ultra       = 0,    // High-end PC
    High        = 1,    // Mid-range PC
    Medium      = 2,    // Console/Low-end PC
    Low         = 3,    // Mobile/Very low-end
    Potato      = 4     // Emergency fallback
};

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized   = 0,
    Initializing    = 1,
    Active          = 2,
    Paused          = 3,
    Error           = 4,
    Shutdown        = 5
};

UENUM(BlueprintType)
enum class EEng_WorldScale : uint8
{
    Small       = 0,    // < 1km²
    Medium      = 1,    // 1-4km²
    Large       = 2,    // 4-16km²
    Massive     = 3,    // > 16km² (requires World Partition)
    Unlimited   = 4     // Procedural infinite
};

UENUM(BlueprintType)
enum class EEng_PhysicsComplexity : uint8
{
    Simple      = 0,    // Basic collision only
    Standard    = 1,    // Standard physics
    Advanced    = 2,    // Full simulation with destruction
    Cinematic   = 3     // Maximum fidelity for cutscenes
};

// ============================================================================
// CORE STRUCTS - Shared data structures
// ============================================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 ErrorCount;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = EEng_SystemPriority::Medium;
        CurrentState = EEng_SystemState::Uninitialized;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        ErrorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Triangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceTier CurrentTier;

    FEng_PerformanceMetrics()
    {
        FrameTime = 16.67f; // 60 FPS target
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        CurrentTier = EEng_PerformanceTier::Medium;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    EEng_WorldScale Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FVector WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bUseWorldPartition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bUseProcGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxSimultaneousActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    EEng_PhysicsComplexity PhysicsComplexity;

    FEng_WorldConfiguration()
    {
        Scale = EEng_WorldScale::Medium;
        WorldSize = FVector(4000.0f, 4000.0f, 1000.0f); // 4km x 4km x 1km
        bUseWorldPartition = false;
        bUseProcGeneration = true;
        MaxSimultaneousActors = 10000;
        StreamingDistance = 2000.0f;
        PhysicsComplexity = EEng_PhysicsComplexity::Standard;
    }
};

// ============================================================================
// ARCHITECTURE CONSTANTS
// ============================================================================

namespace EngineArchitecture
{
    // Performance targets
    constexpr float TARGET_FPS_PC = 60.0f;
    constexpr float TARGET_FPS_CONSOLE = 30.0f;
    constexpr float MAX_FRAME_TIME_MS = 16.67f; // 60 FPS
    
    // Memory limits (MB)
    constexpr float MAX_MEMORY_PC = 8192.0f;       // 8GB
    constexpr float MAX_MEMORY_CONSOLE = 4096.0f;  // 4GB
    constexpr float MAX_MEMORY_MOBILE = 2048.0f;   // 2GB
    
    // World limits
    constexpr float WORLD_PARTITION_THRESHOLD = 4000.0f; // 4km²
    constexpr int32 MAX_ACTORS_WITHOUT_PARTITION = 10000;
    constexpr int32 MAX_ACTORS_WITH_PARTITION = 100000;
    
    // Physics limits
    constexpr int32 MAX_PHYSICS_BODIES = 5000;
    constexpr int32 MAX_DESTRUCTION_PIECES = 1000;
    constexpr float PHYSICS_TICK_RATE = 60.0f;
    
    // AI limits
    constexpr int32 MAX_AI_AGENTS_STANDARD = 500;
    constexpr int32 MAX_AI_AGENTS_MASS = 50000;
    constexpr float AI_UPDATE_FREQUENCY = 30.0f;
    
    // Audio limits
    constexpr int32 MAX_SIMULTANEOUS_SOUNDS = 128;
    constexpr int32 MAX_3D_SOUNDS = 64;
    constexpr float AUDIO_STREAMING_DISTANCE = 5000.0f;
}