#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "EnvArt_AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphericZone : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ColorTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    int32 ParticleCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleLifetime = 10.0f;

    FEnvArt_AtmosphericSettings()
    {
        SunAngle = -15.0f;
        SunIntensity = 8.0f;
        ColorTemperature = 3200.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        ParticleCount = 50;
        ParticleLifetime = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    TMap<EEnvArt_AtmosphericZone, FEnvArt_AtmosphericSettings> ZoneSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TArray<UParticleSystemComponent*> ParticleEmitters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<UAudioComponent*> AmbientSounds;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetAtmosphericZone(EEnvArt_AtmosphericZone Zone);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateLighting(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateFog(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateParticleEffects(FVector Location, EEnvArt_AtmosphericZone Zone);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetupAmbientAudio(FVector Location, EEnvArt_AtmosphericZone Zone);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmospheric Control")
    void InitializeCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TransitionToZone(EEnvArt_AtmosphericZone FromZone, EEnvArt_AtmosphericZone ToZone, float TransitionTime = 2.0f);

private:
    void InitializeZoneSettings();
    void FindLightingActors();
    void CreateDefaultParticleEmitters();
    void SetupDefaultAmbientAudio();

    float CurrentTransitionTime;
    bool bIsTransitioning;
    EEnvArt_AtmosphericZone CurrentZone;
    EEnvArt_AtmosphericZone TargetZone;
    FEnvArt_AtmosphericSettings TransitionStartSettings;
    FEnvArt_AtmosphericSettings TransitionTargetSettings;
};