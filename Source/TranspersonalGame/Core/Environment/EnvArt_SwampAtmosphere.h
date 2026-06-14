#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "EnvArt_SwampAtmosphere.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphereTimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float MistParticleIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float PollenParticleIntensity;

    FEnvArt_AtmosphericSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunAngle = 30.0f;
        MistParticleIntensity = 0.5f;
        PollenParticleIntensity = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_SwampAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_SwampAtmosphere();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UExponentialHeightFogComponent* HeightFogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* MistParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* PollenParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    // Atmospheric Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_AtmosphereTimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bDynamicTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TimeOfDaySpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bAdaptToBiome;

    // Swamp-Specific Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp")
    float WaterMistIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp")
    float TreeCanopyDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp")
    bool bSpawnFireflies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp")
    float FireflyIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp")
    bool bPlaySwampAmbience;

    // Environmental Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bVolumetricLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float VolumetricScatteringIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bGodRays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float GodRayIntensity;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_AtmosphereTimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeSpecificAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SpawnMistEffects();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SpawnPollenEffects();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SpawnFireflyEffects();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateVolumetricLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateGodRays();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySwampAmbience();

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    float GetCurrentFogDensity() const;

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void PreviewAtmosphericChanges();

private:
    void InitializeAtmosphericComponents();
    void UpdateFogSettings();
    void UpdateLightingSettings();
    void UpdateParticleEffects();
    void UpdateAudioSettings();
    void HandleTimeOfDayTransition(float DeltaTime);
    FEnvArt_AtmosphericSettings GetSettingsForTimeOfDay(EEnvArt_AtmosphereTimeOfDay TimeOfDay);

    // Internal State
    float CurrentTimeOfDayProgress;
    float LastUpdateTime;
    bool bComponentsInitialized;
    TArray<AActor*> CachedDirectionalLights;
};