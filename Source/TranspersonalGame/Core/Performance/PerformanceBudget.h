// PerformanceBudget.h
// Performance Optimizer — Agent #04
// Defines per-platform frame budget constants and per-system draw call / memory caps.
// All systems MUST query these caps before spawning actors or enabling expensive features.

#pragma once

#include "CoreMinimal.h"
#include "PerformanceBudget.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_Platform : uint8
{
    PC_High     UMETA(DisplayName = "PC High-End"),
    PC_Mid      UMETA(DisplayName = "PC Mid-Range"),
    Console     UMETA(DisplayName = "Console"),
    Mobile      UMETA(DisplayName = "Mobile")
};

UENUM(BlueprintType)
enum class EPerf_BudgetStatus : uint8
{
    OK          UMETA(DisplayName = "Within Budget"),
    Warning     UMETA(DisplayName = "Near Limit"),
    Critical    UMETA(DisplayName = "Over Budget"),
    Violated    UMETA(DisplayName = "Hard Limit Violated")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

/** Per-platform frame time targets (milliseconds) */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameTargets
{
    GENERATED_BODY()

    /** Target frame time in ms (16.67ms = 60fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float FrameTimeMS = 16.67f;

    /** GPU budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float GPUBudgetMS = 10.0f;

    /** CPU game thread budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float CPUGameThreadMS = 4.0f;

    /** CPU render thread budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float CPURenderThreadMS = 6.0f;
};

/** Draw call and actor count caps per platform */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DrawCallBudget
{
    GENERATED_BODY()

    /** Max static mesh actors in scene */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxStaticMeshActors = 500;

    /** Max skeletal mesh actors (characters, dinos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxSkeletalMeshActors = 50;

    /** Max dynamic point/spot lights */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDynamicLights = 20;

    /** Max Niagara particle systems active simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxParticleSystems = 30;

    /** Max total actors in level (all types) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxTotalActors = 300;

    /** Max simultaneous dinosaur AI agents (full BT evaluation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDinoAIAgents = 12;
};

/** Memory budget in MB per platform */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryBudget
{
    GENERATED_BODY()

    /** Texture streaming pool size (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 TextureStreamingPoolMB = 1024;

    /** Max mesh memory (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 MeshMemoryMB = 512;

    /** Max audio memory (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 AudioMemoryMB = 128;

    /** Max total GPU memory (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 TotalGPUMemoryMB = 4096;
};

/** Snapshot of current performance state */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RuntimeSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    float FrameTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    int32 ActiveStaticMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    int32 ActiveSkeletalMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    int32 ActiveDynamicLights = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    int32 ActiveParticleSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    EPerf_BudgetStatus OverallStatus = EPerf_BudgetStatus::OK;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    FString WorstOffender;
};

// ─────────────────────────────────────────────────────────────────────────────
// Static Budget Library
// ─────────────────────────────────────────────────────────────────────────────

/**
 * UPerf_BudgetLibrary
 * Blueprint-callable library that returns per-platform budgets and validates
 * current scene state against them. All other agents MUST call CheckBudget()
 * before spawning expensive actors.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_BudgetLibrary : public UObject
{
    GENERATED_BODY()

public:

    /** Returns frame time targets for the given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static FPerf_FrameTargets GetFrameTargets(EPerf_Platform Platform);

    /** Returns draw call budget for the given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static FPerf_DrawCallBudget GetDrawCallBudget(EPerf_Platform Platform);

    /** Returns memory budget for the given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static FPerf_MemoryBudget GetMemoryBudget(EPerf_Platform Platform);

    /**
     * Checks whether adding N actors of a given type would violate budget.
     * Returns EPerf_BudgetStatus::OK if safe, Warning if near limit, Critical/Violated if over.
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static EPerf_BudgetStatus CheckActorBudget(
        EPerf_Platform Platform,
        int32 CurrentStaticMeshCount,
        int32 CurrentSkeletalMeshCount,
        int32 CurrentDynamicLightCount,
        int32 AdditionalStatic,
        int32 AdditionalSkeletal,
        int32 AdditionalLights
    );

    /** Returns true if the scene is within all hard limits for the given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static bool IsWithinHardLimits(EPerf_Platform Platform, const FPerf_RuntimeSnapshot& Snapshot);

    /** Recommended LOD bias for the given platform (0 = full quality, 2 = aggressive LOD) */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static int32 GetRecommendedLODBias(EPerf_Platform Platform);

    /** Recommended shadow resolution for the given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    static int32 GetRecommendedShadowResolution(EPerf_Platform Platform);
};
