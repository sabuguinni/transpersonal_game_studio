#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VFX_CretaceousAtmosphereController.generated.h"

UENUM(BlueprintType)
enum class EVFX_AtmosphereType : uint8
{
    Clear           UMETA(DisplayName = "Clear Tropical"),
    Dusty           UMETA(DisplayName = "Dusty Plains"),
    Humid           UMETA(DisplayName = "Humid Forest"),
    Volcanic        UMETA(DisplayName = "Volcanic Ash"),
    Windy           UMETA(DisplayName = "Windy Highlands")
};

USTRUCT(BlueprintType)
struct FVFX_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DustDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float PollenAmount = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HeatShimmerIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AtmosphereColor = FLinearColor(1.0f, 0.95f, 0.8f, 0.1f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WindStrength = 0.6f;

    FVFX_AtmosphereSettings()
    {
        DustDensity = 0.3f;
        PollenAmount = 0.5f;
        HeatShimmerIntensity = 0.4f;
        AtmosphereColor = FLinearColor(1.0f, 0.95f, 0.8f, 0.1f);
        WindStrength = 0.6f;
    }
};

/**
 * VFX Controller for realistic Cretaceous period atmospheric effects
 * Manages dust motes, pollen particles, heat shimmer, and volumetric lighting
 * Based on scientific research of Cretaceous climate conditions
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousAtmosphereController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousAtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core atmosphere control
    UFUNCTION(BlueprintCallable, Category = "VFX Atmosphere")
    void SetAtmosphereType(EVFX_AtmosphereType NewType);

    UFUNCTION(BlueprintCallable, Category = "VFX Atmosphere")
    void UpdateAtmosphereSettings(const FVFX_AtmosphereSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "VFX Atmosphere")
    void SetTimeOfDay(float TimeOfDayNormalized);

    // Particle effects management
    UFUNCTION(BlueprintCallable, Category = "VFX Particles")
    void SpawnDustMotes(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Particles")
    void SpawnPollenParticles(FVector Location, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Particles")
    void CreateHeatShimmer(FVector Location, float Intensity = 1.0f);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void UpdateVolumetricLighting(float SunAngle, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetWindEffect(FVector WindDirection, float Strength);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    int32 GetActiveParticleCount() const;

protected:
    // Current atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    EVFX_AtmosphereType CurrentAtmosphereType = EVFX_AtmosphereType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FVFX_AtmosphereSettings AtmosphereSettings;

    // Particle system components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> DustMotesComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> PollenComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> HeatShimmerComponent;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentLODLevel = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime = 0.0f;

    // Internal state
    float CurrentTimeOfDay = 0.5f;
    FVector CurrentWindDirection = FVector(1.0f, 0.0f, 0.0f);
    float CurrentWindStrength = 0.6f;

private:
    // Internal helper functions
    void InitializeParticleSystems();
    void UpdateParticleParameters();
    void UpdateLODSettings();
    void CleanupInactiveParticles();
    
    // Performance monitoring
    void CheckPerformanceThresholds();
    bool ShouldUpdateThisFrame() const;
    
    // Atmosphere type presets
    FVFX_AtmosphereSettings GetPresetForAtmosphereType(EVFX_AtmosphereType Type) const;
};