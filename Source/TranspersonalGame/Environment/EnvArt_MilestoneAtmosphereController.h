#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvArt_MilestoneAtmosphereController.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphereMode : uint8
{
    GoldenHour      UMETA(DisplayName = "Golden Hour"),
    Overcast        UMETA(DisplayName = "Overcast"),
    Stormy          UMETA(DisplayName = "Stormy"),
    Dawn            UMETA(DisplayName = "Dawn"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    CretaceousForest    UMETA(DisplayName = "Cretaceous Forest"),
    CretaceousPlains    UMETA(DisplayName = "Cretaceous Plains"),
    CretaceousSwamp     UMETA(DisplayName = "Cretaceous Swamp"),
    CretaceousDesert    UMETA(DisplayName = "Cretaceous Desert"),
    VolcanicRegion      UMETA(DisplayName = "Volcanic Region")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableAtmosphericParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAmbientSounds = true;
};

USTRUCT(BlueprintType)
struct FEnvArt_BiomeAtmosphereData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEnvArt_BiomeType BiomeType = EEnvArt_BiomeType::CretaceousForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneExtents = FVector(2000.0f, 2000.0f, 1000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FString> AmbientSoundNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TArray<FString> ParticleEffectNames;
};

/**
 * Milestone 1 Atmosphere Controller - Manages lighting, fog, and atmospheric effects
 * for the minimum viable playable prototype. Provides golden hour lighting,
 * volumetric fog, and biome-specific atmospheric settings.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_MilestoneAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_MilestoneAtmosphereController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE ATMOSPHERE CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereMode(EEnvArt_AtmosphereMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnAtmosphericParticles();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupAmbientSoundTriggers();

    // === BIOME ATMOSPHERE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FEnvArt_BiomeAtmosphereData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeAtmosphere(EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEnvArt_BiomeType GetCurrentBiome(const FVector& WorldLocation) const;

    // === LIGHTING CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunAngle(float Elevation, float Azimuth);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunColor(const FLinearColor& NewColor);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateAtmosphericFillLights();

    // === FOG CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogColor(const FLinearColor& NewColor);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void EnableVolumetricFog(bool bEnable);

    // === PARTICLE EFFECTS ===

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void SpawnDustParticles(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void SpawnPollenParticles(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void CreateLightShaftEffects();

    // === AUDIO MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CreateSoundTrigger(const FVector& Location, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAmbientSounds(EEnvArt_BiomeType BiomeType);

protected:
    // === CORE COMPONENTS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === ATMOSPHERE SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_AtmosphereMode CurrentAtmosphereMode = EEnvArt_AtmosphereMode::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings DefaultAtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FEnvArt_BiomeAtmosphereData> BiomeZones;

    // === LIGHTING REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TArray<class ASpotLight*> FillLights;

    // === FOG REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "Fog")
    class AExponentialHeightFog* MainFog;

    // === PARTICLE SYSTEMS ===

    UPROPERTY(BlueprintReadOnly, Category = "Particles")
    TArray<class AActor*> ParticleActors;

    // === AUDIO TRIGGERS ===

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TArray<class ATriggerBox*> SoundTriggers;

    // === RUNTIME STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bAtmosphereInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon

private:
    // === INTERNAL HELPERS ===

    void FindExistingLightingActors();
    void InitializeDefaultSettings();
    void UpdateTimeOfDay(float DeltaTime);
    FEnvArt_AtmosphereSettings GetBiomeAtmosphereSettings(EEnvArt_BiomeType BiomeType) const;
};