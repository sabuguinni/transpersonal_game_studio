#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemCore.generated.h"

/**
 * Core VFX System for Transpersonal Game
 * Manages all visual effects with performance-aware LOD chain
 * Focus: Atmosphere, tension, and narrative punctuation
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Atmospheric     UMETA(DisplayName = "Atmospheric"),
    Combat          UMETA(DisplayName = "Combat"),
    Survival        UMETA(DisplayName = "Survival"),
    Mystical        UMETA(DisplayName = "Mystical"),
    Environmental   UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Dramatic    UMETA(DisplayName = "Dramatic"),
    Cinematic   UMETA(DisplayName = "Cinematic")
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality")
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> HighLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> MediumLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> LowLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDrawDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticleCount = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Lifetime = 10.0f;

    FVFXDefinition()
    {
        EffectName = NAME_None;
        Category = EVFXCategory::Environmental;
        Intensity = EVFXIntensity::Moderate;
        MaxDrawDistance = 5000.0f;
        MaxParticleCount = 1000;
        bAutoDestroy = true;
        Lifetime = 10.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    UNiagaraComponent* SpawnVFX(const FVFXDefinition& VFXDef, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXParameter(UNiagaraComponent* VFXComponent, const FName& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXVectorParameter(UNiagaraComponent* VFXComponent, const FName& ParameterName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXColorParameter(UNiagaraComponent* VFXComponent, const FName& ParameterName, const FLinearColor& Value);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "VFX LOD")
    EVFXLODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

    UFUNCTION(BlueprintCallable, Category = "VFX LOD")
    void SetLODLevel(EVFXLODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX LOD")
    void UpdateLODBasedOnDistance(const FVector& ViewerLocation);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    int32 GetActiveVFXCount() const { return ActiveVFXComponents.Num(); }

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    float GetTotalParticleCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void CleanupExpiredVFX();

    // Preset Effects for Common Game Moments
    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnDinosaurBreathVFX(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnCampfireSmokeVFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnFootstepDustVFX(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnTimeGemVFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnTensionAtmosphereVFX(const FVector& Location, float Radius = 1000.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<FName, FVFXDefinition> VFXLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFXLODLevel CurrentLODLevel = EVFXLODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float LODDistanceThresholds[3] = {1500.0f, 3000.0f, 5000.0f}; // High, Medium, Low

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxConcurrentVFX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float PerformanceThresholdFPS = 30.0f;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY()
    TMap<UNiagaraComponent*, float> VFXSpawnTimes;

    float LastPerformanceCheck = 0.0f;
    const float PerformanceCheckInterval = 1.0f;

    void InitializeVFXLibrary();
    UNiagaraSystem* SelectLODSystem(const FVFXDefinition& VFXDef, EVFXLODLevel LODLevel);
    void PerformanceOptimization();
    bool ShouldCullVFX(UNiagaraComponent* VFXComponent, const FVector& ViewerLocation);
};