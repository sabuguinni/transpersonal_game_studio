#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvArt_VolcanicAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_VolcanicIntensity : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    LowActivity UMETA(DisplayName = "Low Activity"),
    Moderate    UMETA(DisplayName = "Moderate"),
    HighActivity UMETA(DisplayName = "High Activity"),
    Erupting    UMETA(DisplayName = "Erupting")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float AshParticleIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_VolcanicAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_VolcanicAtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* AshParticleSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SteamVentSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VolcanicAmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WindAudio;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    EEnvArt_VolcanicIntensity VolcanicIntensity = EEnvArt_VolcanicIntensity::Moderate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float EffectRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDay = 16.0f; // Golden hour default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bDynamicTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f;

    // References to world lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    AExponentialHeightFog* WorldFog;

    // Blueprint Callable Functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SetVolcanicIntensity(EEnvArt_VolcanicIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    FLinearColor CalculateAtmosphericColor(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    float CalculateVisibilityAtDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere", CallInEditor)
    void RefreshAtmosphere();

private:
    // Internal state
    float CurrentTimeOfDay;
    float AtmosphereUpdateTimer;
    
    // Helper functions
    void FindWorldLightingActors();
    void UpdateTimeOfDayEffects(float DeltaTime);
    FLinearColor GetSunColorForTime(float Time) const;
    float GetSunIntensityForTime(float Time) const;
    FRotator GetSunRotationForTime(float Time) const;
};