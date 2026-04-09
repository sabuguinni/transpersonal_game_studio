#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    // Environmental VFX
    WeatherEffect,
    AtmosphericParticles,
    FogSystem,
    
    // Combat VFX
    DinosaurBreath,
    BloodSplatter,
    ImpactEffect,
    
    // Mystical VFX
    GemGlow,
    PortalEffect,
    ConsciousnessShift,
    
    // Survival VFX
    FireEffect,
    SmokeTrail,
    WaterSplash,
    
    // UI VFX
    MenuTransition,
    HUDEffect,
    NotificationGlow
};

UENUM(BlueprintType)
enum class EVFXQuality : uint8
{
    Low,
    Medium,
    High,
    Ultra
};

USTRUCT(BlueprintType)
struct FVFXEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DefaultScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXQuality MinimumQuality = EVFXQuality::Low;
};

/**
 * VFX Manager - Central system for managing all visual effects in the prehistoric world
 * Handles Niagara systems, performance optimization, and effect coordination
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFXManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFXType VFXType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFXType VFXType, USceneComponent* AttachComponent, const FName& AttachPointName = NAME_None, const FVector& RelativeLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXOfType(EVFXType VFXType);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void TriggerLightningStrike(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void UpdateFogDensity(float Density);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodEffect(const FVector& Location, const FVector& Normal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnImpactEffect(const FVector& Location, const FVector& Normal, const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnDinosaurBreathEffect(USceneComponent* MouthComponent, const FString& DinosaurType);

    // Mystical VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Mystical")
    void SpawnGemGlowEffect(const FVector& Location, const FLinearColor& GemColor);

    UFUNCTION(BlueprintCallable, Category = "VFX|Mystical")
    void TriggerConsciousnessShift(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Mystical")
    void SpawnPortalEffect(const FVector& Location, float PortalRadius = 200.0f);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXQuality(EVFXQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void EnableVFXLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    int32 GetActiveVFXCount() const;

    // Material Parameter Control
    UFUNCTION(BlueprintCallable, Category = "VFX|Materials")
    void SetGlobalVFXParameter(const FName& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "VFX|Materials")
    void SetGlobalVFXVectorParameter(const FName& ParameterName, const FLinearColor& Value);

protected:
    // VFX Data
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TMap<EVFXType, FVFXEffectData> VFXDatabase;

    // Active Effects Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY()
    TMap<EVFXType, TArray<UNiagaraComponent*>> ActiveVFXByType;

    // Performance Settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    int32 MaxConcurrentVFX = 100;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float VFXCullingDistance = 5000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    EVFXQuality CurrentVFXQuality = EVFXQuality::High;

    // Material Parameter Collection
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
    UMaterialParameterCollection* GlobalVFXParameters;

private:
    // Internal Functions
    void InitializeVFXDatabase();
    void CleanupFinishedEffects();
    bool ShouldSpawnVFX(EVFXType VFXType) const;
    void OptimizeVFXPerformance();

    // Timer Handles
    FTimerHandle CleanupTimerHandle;
    FTimerHandle PerformanceTimerHandle;
};