#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemCore.generated.h"

/**
 * Core VFX System for Transpersonal Jurassic Game
 * Manages all visual effects with performance optimization and emotional impact focus
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Environmental Effects
    Weather         UMETA(DisplayName = "Weather"),
    Atmosphere      UMETA(DisplayName = "Atmosphere"),
    Vegetation      UMETA(DisplayName = "Vegetation"),
    
    // Survival Effects  
    Fire            UMETA(DisplayName = "Fire"),
    Smoke           UMETA(DisplayName = "Smoke"),
    Sparks          UMETA(DisplayName = "Sparks"),
    
    // Creature Effects
    DinosaurBreath  UMETA(DisplayName = "Dinosaur Breath"),
    DinosaurImpact  UMETA(DisplayName = "Dinosaur Impact"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    
    // Player Effects
    Exhaustion      UMETA(DisplayName = "Exhaustion"),
    Fear            UMETA(DisplayName = "Fear"),
    Discovery       UMETA(DisplayName = "Discovery"),
    
    // World Effects
    GemGlow         UMETA(DisplayName = "Gem Glow"),
    TimeDistortion  UMETA(DisplayName = "Time Distortion"),
    
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),
    Moderate    UMETA(DisplayName = "Moderate"), 
    Intense     UMETA(DisplayName = "Intense"),
    Extreme     UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality")
};

USTRUCT(BlueprintType)
struct FVFXEmotionalProfile
{
    GENERATED_BODY()

    // Emotional impact parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionLevel = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearFactor = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BeautyFactor = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float UrgencyLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FVFXPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticles = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullDistance = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXLODLevel LODLevel = EVFXLODLevel::High;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseGPUSimulation = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCastShadows = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemCore();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFX(EVFXCategory Category, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(class UNiagaraComponent* VFXComponent, bool bImmediate = false);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXIntensity(class UNiagaraComponent* VFXComponent, EVFXIntensity Intensity);
    
    // Emotional Impact System
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ApplyEmotionalProfile(class UNiagaraComponent* VFXComponent, const FVFXEmotionalProfile& Profile);
    
    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetPerformanceSettings(const FVFXPerformanceSettings& Settings);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateLOD(float DistanceToPlayer);
    
    // Survival-Specific Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void TriggerFireEffect(FVector Location, float Intensity = 1.0f, float Duration = 10.0f);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void TriggerSmokeEffect(FVector Location, FVector WindDirection, float Density = 1.0f);
    
    // Creature-Specific Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Creatures")
    void TriggerDinosaurBreath(FVector Location, FVector Direction, float Temperature = 37.0f);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Creatures")
    void TriggerImpactEffect(FVector Location, float Force, bool bIsLargeCreature = false);
    
    // Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Atmosphere")
    void SetAtmosphericTension(float TensionLevel);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Atmosphere")
    void TriggerWeatherTransition(float TransitionSpeed = 1.0f);

protected:
    // VFX Asset Registry
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    TMap<EVFXCategory, class UNiagaraSystem*> VFXAssets;
    
    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVFXPerformanceSettings PerformanceSettings;
    
    // Active VFX Tracking
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveVFXComponents;
    
    // Emotional State
    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    float CurrentTensionLevel = 0.0f;
    
private:
    // Internal management
    void CleanupFinishedVFX();
    void OptimizePerformance();
    UNiagaraSystem* GetVFXAsset(EVFXCategory Category);
    void ApplyLODSettings(class UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel);
};