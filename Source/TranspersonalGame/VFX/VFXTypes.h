#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "VFXTypes.generated.h"

/**
 * Unified VFX Types for Transpersonal Game Studio
 * Consolidates all VFX enums, structs and data types
 * VFX Agent #17 - Consolidation Phase
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Environmental storytelling
    EnvironmentalAmbient     UMETA(DisplayName = "Environmental Ambient"),
    WeatherEffects          UMETA(DisplayName = "Weather Effects"),
    VegetationInteraction   UMETA(DisplayName = "Vegetation Interaction"),
    
    // Creature behavior feedback
    DinosaurBreathing       UMETA(DisplayName = "Dinosaur Breathing"),
    DinosaurMovement        UMETA(DisplayName = "Dinosaur Movement"),
    DinosaurEmotional       UMETA(DisplayName = "Dinosaur Emotional States"),
    
    // Player interaction
    PlayerMovement          UMETA(DisplayName = "Player Movement"),
    CraftingEffects         UMETA(DisplayName = "Crafting Effects"),
    ToolUsage              UMETA(DisplayName = "Tool Usage"),
    
    // Tension amplifiers
    DangerIndicators        UMETA(DisplayName = "Danger Indicators"),
    StealthFeedback         UMETA(DisplayName = "Stealth Feedback"),
    ThreatProximity         UMETA(DisplayName = "Threat Proximity"),
    
    // Combat and survival
    ImpactEffects           UMETA(DisplayName = "Impact Effects"),
    BloodEffects            UMETA(DisplayName = "Blood Effects"),
    DestructionEffects      UMETA(DisplayName = "Destruction Effects"),
    
    // Special moments
    GemEffects              UMETA(DisplayName = "Gem Effects"),
    DomesticationEffects    UMETA(DisplayName = "Domestication Effects"),
    TimeTransition          UMETA(DisplayName = "Time Transition"),
    
    // UI and Interaction
    UIEffects               UMETA(DisplayName = "UI Effects"),
    InteractionFeedback     UMETA(DisplayName = "Interaction Feedback"),
    
    // Atmospheric
    AtmosphericEffects      UMETA(DisplayName = "Atmospheric Effects")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical = 0    UMETA(DisplayName = "Critical"),      // Combat impacts, predator attacks
    High = 1        UMETA(DisplayName = "High"),          // Environmental dangers, large creature movements  
    Medium = 2      UMETA(DisplayName = "Medium"),        // Ambient effects, small creature interactions
    Low = 3         UMETA(DisplayName = "Low"),           // Decorative effects, distant ambience
    Background = 4  UMETA(DisplayName = "Background")     // Subtle atmospheric effects
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),      // Barely noticeable, atmospheric
    Moderate    UMETA(DisplayName = "Moderate"),    // Clear but not overwhelming
    Prominent   UMETA(DisplayName = "Prominent"),   // Clearly visible, important moment
    Dramatic    UMETA(DisplayName = "Dramatic")     // High impact, critical moments
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),     // Close range (0-1000m), high detail
    Medium      UMETA(DisplayName = "Medium Quality"),   // Mid range (1000-3000m), balanced
    Low         UMETA(DisplayName = "Low Quality"),      // Far range (3000m+), performance optimized
    Disabled    UMETA(DisplayName = "Disabled")          // Too far, disabled for performance
};

USTRUCT(BlueprintType)
struct FVFXEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    EVFXIntensity Intensity;

    // LOD Chain - 3 levels for performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_Low;

    // Performance and culling settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float MaxDrawDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bUseDistanceLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bPooled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bAutoDestroy = true;

    // Gameplay integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Gameplay")
    bool bRequiresPlayerProximity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Gameplay")
    float PlayerProximityRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Gameplay")
    bool bRequiresLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Gameplay")
    bool bAffectsGameplay = false;

    // Narrative integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Narrative")
    FString EmotionalIntent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Narrative")
    FString NarrativePurpose;

    // Additional properties for pool management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    FString PoolName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Parameters")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Parameters")
    FLinearColor ColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Parameters")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Parameters")
    float Duration = 0.0f; // 0 = infinite/looping

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Parameters")
    bool bIsLooping = false;

    FVFXEffectData()
    {
        EffectName = TEXT("DefaultEffect");
        Category = EVFXCategory::EnvironmentalAmbient;
        Priority = EVFXPriority::Medium;
        Intensity = EVFXIntensity::Subtle;
        MaxDrawDistance = 5000.0f;
        MaxInstances = 10;
        bUseDistanceLOD = true;
        bPooled = true;
        bAutoDestroy = true;
        bRequiresPlayerProximity = false;
        PlayerProximityRadius = 1000.0f;
        bRequiresLineOfSight = false;
        bAffectsGameplay = false;
        EmotionalIntent = TEXT("");
        NarrativePurpose = TEXT("");
        PoolName = TEXT("DefaultPool");
        Scale = 1.0f;
        ColorTint = FLinearColor::White;
        Intensity = 1.0f;
        Duration = 0.0f;
        bIsLooping = false;
    }
};

USTRUCT(BlueprintType)
struct FVFXPoolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystemAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    FString PoolName = TEXT("DefaultPool");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    int32 InitialPoolSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    int32 MaxPoolSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    EVFXCategory Category = EVFXCategory::EnvironmentalAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    EVFXPriority Priority = EVFXPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    bool bCanGrow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    bool bAutoRecycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    float RecycleTimeout = 30.0f;

    FVFXPoolData()
    {
        PoolName = TEXT("DefaultPool");
        InitialPoolSize = 5;
        MaxPoolSize = 10;
        Category = EVFXCategory::EnvironmentalAmbient;
        Priority = EVFXPriority::Medium;
        MaxDistance = 5000.0f;
        bCanGrow = true;
        bAutoRecycle = true;
        RecycleTimeout = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct FVFXLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    float HighQualityDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    float MediumQualityDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX LOD")
    float LODUpdateInterval = 0.5f;

    FVFXLODSettings()
    {
        HighQualityDistance = 1000.0f;
        MediumQualityDistance = 3000.0f;
        CullDistance = 10000.0f;
        LODUpdateInterval = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FVFXPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveVFX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 CurrentVFXQuality = 2; // 0=Low, 1=Medium, 2=High

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float GlobalQualityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bEnableVFXCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bEnableVFXLOD = true;

    FVFXPerformanceSettings()
    {
        MaxActiveVFX = 100;
        CurrentVFXQuality = 2;
        GlobalQualityMultiplier = 1.0f;
        bEnableVFXCulling = true;
        bEnableVFXLOD = true;
    }
};