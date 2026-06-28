// PerformanceConfig.h — Performance Optimizer Agent #4
// Frame budget constants and scalability configuration for Transpersonal Game Studio
// Prehistoric survival game — 60fps PC / 30fps Console targets
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EPerf_ScalabilityTier : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_60fps        UMETA(DisplayName = "PC 60fps"),
    PC_Ultra        UMETA(DisplayName = "PC Ultra"),
};

UENUM(BlueprintType)
enum class EPerf_BudgetWarning : uint8
{
    None            UMETA(DisplayName = "None"),
    ActorCount      UMETA(DisplayName = "Actor Count Exceeded"),
    DynamicLights   UMETA(DisplayName = "Dynamic Light Budget Exceeded"),
    DrawCalls       UMETA(DisplayName = "Draw Call Budget Exceeded"),
    TextureMemory   UMETA(DisplayName = "Texture Memory Exceeded"),
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.6f; // 60fps default

    // Shadow rendering budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShadowBudgetMs = 3.0f;

    // Lumen GI budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LumenBudgetMs = 4.0f;

    // Draw call budget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    // Max dynamic actors in scene
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicActors = 500;

    // Max dynamic point/spot lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicLights = 20;

    // Max simultaneous Niagara systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxNiagaraSystems = 50;

    // Texture streaming pool (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TexturePoolMB = 2048;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    // Static mesh LOD distance scale (1.0 = default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float StaticMeshLODScale = 1.0f;

    // Skeletal mesh LOD bias (0 = no bias)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 SkeletalMeshLODBias = 0;

    // Foliage cull distance (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageCullDistance = 15000.0f;

    // Dinosaur AI tick rate at distance (Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoAITickRateNear = 30.0f; // < 2000cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoAITickRateMid = 10.0f;  // 2000-8000cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoAITickRateFar = 2.0f;   // > 8000cm (dormant)

    // Shadow distance for dynamic objects (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DynamicShadowDistance = 5000.0f;
};

// ============================================================
// MAIN CONFIG CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // Active scalability tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_ScalabilityTier ActiveTier = EPerf_ScalabilityTier::PC_60fps;

    // Frame budget for current tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_FrameBudget FrameBudget;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings LODSettings;

    // Get frame budget for a specific tier
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetBudgetForTier(EPerf_ScalabilityTier Tier) const;

    // Apply scalability settings to the engine
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyScalabilitySettings(EPerf_ScalabilityTier Tier);

    // Check if current scene is within budget
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_BudgetWarning CheckSceneBudget(int32 ActorCount, int32 DynamicLightCount, int32 DrawCallCount) const;

    // Get recommended Lumen trace distance for tier
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetLumenTraceDistance(EPerf_ScalabilityTier Tier) const;
};
