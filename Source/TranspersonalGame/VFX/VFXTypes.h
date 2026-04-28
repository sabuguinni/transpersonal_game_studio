// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VFXTypes.generated.h"

/**
 * VFX Quality Levels for LOD Chain
 * Used to optimize performance across different hardware
 */
UENUM(BlueprintType)
enum class EVFXQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low Quality - Mobile/Weak Hardware"),
    Medium      UMETA(DisplayName = "Medium Quality - Standard Hardware"), 
    High        UMETA(DisplayName = "High Quality - High-end Hardware"),
    Cinematic   UMETA(DisplayName = "Cinematic Quality - Offline/Cutscenes")
};

/**
 * VFX Effect Types for categorization and management
 */
UENUM(BlueprintType)
enum class EVFXEffectType : uint8
{
    Environmental   UMETA(DisplayName = "Environmental Effects"),
    Combat          UMETA(DisplayName = "Combat Effects"),
    Creature        UMETA(DisplayName = "Creature Effects"),
    Weather         UMETA(DisplayName = "Weather Effects"),
    Destruction     UMETA(DisplayName = "Destruction Effects"),
    Magical         UMETA(DisplayName = "Consciousness/Magical Effects"),
    UI              UMETA(DisplayName = "UI/HUD Effects"),
    Ambient         UMETA(DisplayName = "Ambient/Atmospheric Effects")
};

/**
 * VFX Intensity Levels for dynamic scaling
 */
UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle - Background ambience"),
    Moderate    UMETA(DisplayName = "Moderate - Standard gameplay"),
    Intense     UMETA(DisplayName = "Intense - Combat/Action"),
    Extreme     UMETA(DisplayName = "Extreme - Boss fights/Climax")
};

/**
 * VFX System Configuration
 * Holds settings for the entire VFX system
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXSystemConfig
{
    GENERATED_BODY()

    // Global VFX quality level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    EVFXQualityLevel GlobalQuality = EVFXQualityLevel::Medium;

    // Maximum number of simultaneous VFX instances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEffects = 50;

    // Distance culling for VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    // Enable/disable VFX system entirely
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bEnableVFX = true;

    // Global VFX scale multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float GlobalScale = 1.0f;

    FVFXSystemConfig()
    {
        GlobalQuality = EVFXQualityLevel::Medium;
        MaxSimultaneousEffects = 50;
        CullingDistance = 5000.0f;
        bEnableVFX = true;
        GlobalScale = 1.0f;
    }
};

/**
 * VFX Effect Definition
 * Defines a specific visual effect with its properties
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXEffectDefinition
{
    GENERATED_BODY()

    // Unique identifier for this effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString EffectID;

    // Human-readable name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString DisplayName;

    // Effect type for categorization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EVFXEffectType EffectType = EVFXEffectType::Environmental;

    // Niagara systems for different quality levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UNiagaraSystem> LowQualitySystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UNiagaraSystem> MediumQualitySystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UNiagaraSystem> HighQualitySystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UNiagaraSystem> CinematicQualitySystem;

    // Performance cost (0-100, higher = more expensive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "100"))
    int32 PerformanceCost = 50;

    // Default duration (0 = infinite/looping)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float DefaultDuration = 0.0f;

    // Can this effect be pooled for performance?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCanBePooled = true;

    // Audio cue to play with this effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AudioCue;

    FVFXEffectDefinition()
    {
        EffectID = TEXT("");
        DisplayName = TEXT("Unnamed Effect");
        EffectType = EVFXEffectType::Environmental;
        PerformanceCost = 50;
        DefaultDuration = 0.0f;
        bCanBePooled = true;
    }
};

/**
 * VFX Spawn Parameters
 * Parameters for spawning a VFX effect
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXSpawnParams
{
    GENERATED_BODY()

    // World location to spawn the effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector Location = FVector::ZeroVector;

    // World rotation for the effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator Rotation = FRotator::ZeroRotator;

    // Scale of the effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector Scale = FVector::OneVector;

    // Intensity level for this instance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    EVFXIntensity Intensity = EVFXIntensity::Moderate;

    // Override duration (0 = use default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    float DurationOverride = 0.0f;

    // Actor to attach to (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
    TWeakObjectPtr<AActor> AttachToActor;

    // Socket name for attachment (if AttachToActor is set)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
    FName AttachSocketName = NAME_None;

    // Custom parameters to pass to the Niagara system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
    TMap<FString, float> CustomFloatParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
    TMap<FString, FVector> CustomVectorParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
    TMap<FString, FLinearColor> CustomColorParams;

    FVFXSpawnParams()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Intensity = EVFXIntensity::Moderate;
        DurationOverride = 0.0f;
        AttachSocketName = NAME_None;
    }
};

/**
 * Active VFX Instance
 * Represents a currently playing VFX effect
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FActiveVFXInstance
{
    GENERATED_BODY()

    // Unique instance ID
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    int32 InstanceID = 0;

    // Effect definition this instance is based on
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    FString EffectID;

    // The actual Niagara component
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    TWeakObjectPtr<UNiagaraComponent> NiagaraComponent;

    // Spawn parameters used for this instance
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    FVFXSpawnParams SpawnParams;

    // Time when this instance was created
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    float SpawnTime = 0.0f;

    // Is this instance currently active?
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    bool bIsActive = false;

    FActiveVFXInstance()
    {
        InstanceID = 0;
        EffectID = TEXT("");
        SpawnTime = 0.0f;
        bIsActive = false;
    }
};

/**
 * VFX Pool Entry
 * For object pooling of VFX components
 */
USTRUCT()
struct TRANSPERSONALGAME_API FVFX_VFXPoolEntry
{
    GENERATED_BODY()

    // The pooled Niagara component
    UPROPERTY()
    TWeakObjectPtr<UNiagaraComponent> Component;

    // Effect ID this component is configured for
    UPROPERTY()
    FString EffectID;

    // Is this component currently in use?
    UPROPERTY()
    bool bInUse = false;

    // Last time this component was used
    UPROPERTY()
    float LastUsedTime = 0.0f;

    FVFX_VFXPoolEntry()
    {
        EffectID = TEXT("");
        bInUse = false;
        LastUsedTime = 0.0f;
    }
};