#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemCore.generated.h"

/**
 * Core VFX System for Transpersonal Game
 * Manages all visual effects with performance-aware LOD system
 * Designed for survival horror atmosphere with minimal UI intrusion
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Environmental Effects
    Weather         UMETA(DisplayName = "Weather & Atmosphere"),
    Vegetation      UMETA(DisplayName = "Vegetation & Nature"),
    Water          UMETA(DisplayName = "Water & Fluids"),
    
    // Creature Effects
    DinosaurBreath  UMETA(DisplayName = "Dinosaur Breathing"),
    DinosaurMovement UMETA(DisplayName = "Dinosaur Movement"),
    BloodSplatter   UMETA(DisplayName = "Blood & Gore"),
    
    // Player Effects
    Crafting        UMETA(DisplayName = "Crafting & Tools"),
    Fire           UMETA(DisplayName = "Fire & Smoke"),
    PlayerMovement  UMETA(DisplayName = "Player Movement"),
    
    // Narrative Effects
    GemEffect      UMETA(DisplayName = "Time Crystal Effects"),
    Discovery      UMETA(DisplayName = "Discovery Moments"),
    Danger         UMETA(DisplayName = "Danger Indicators")
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High    = 0     UMETA(DisplayName = "High Quality"),
    Medium  = 1     UMETA(DisplayName = "Medium Quality"),
    Low     = 2     UMETA(DisplayName = "Low Quality"),
    Disabled = 3    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FVFXLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticles = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ViewDistanceMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateRate = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableCollision = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCastShadows = true;
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffect(EVFXCategory Category, FVector Location, FRotator Rotation = FRotator::ZeroRotator);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(EVFXCategory Category);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetLODLevel(EVFXLODLevel NewLOD);
    
    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void UpdatePerformanceLOD();
    
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    float GetCurrentGPULoad() const;
    
    // Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Atmosphere")
    void SetWeatherIntensity(float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "VFX Atmosphere")
    void TriggerDangerAtmosphere(bool bEnable);

protected:
    // VFX Assets Registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets")
    TMap<EVFXCategory, TSoftObjectPtr<UNiagaraSystem>> VFXAssets;
    
    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX LOD")
    TMap<EVFXLODLevel, FVFXLODSettings> LODSettings;
    
    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    EVFXLODLevel CurrentLOD;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    TMap<EVFXCategory, UNiagaraComponent*> ActiveEffects;
    
    // Performance Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Performance")
    float TargetFrameRate = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Performance")
    float LODDowngradeThreshold = 45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Performance")
    float LODUpgradeThreshold = 58.0f;

private:
    void InitializeLODSettings();
    void ApplyLODToEffect(UNiagaraComponent* Effect, EVFXLODLevel LOD);
    bool ShouldAdjustLOD() const;
};