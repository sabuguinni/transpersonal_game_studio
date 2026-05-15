#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.63f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    int32 ParticleEmitterCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleSpawnRadius = 2000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // References to scene lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Scene References")
    ADirectionalLight* DirectionalLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Scene References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Scene References")
    TArray<AActor*> ParticleEmitters;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetupGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ConfigureAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RefreshAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float TimeHours); // 0-24 hour format

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherIntensity(float Intensity); // 0-1 range

protected:
    UFUNCTION()
    void FindSceneLightingActors();

    UFUNCTION()
    FRotator CalculateSunRotationForTime(float TimeHours);

    UFUNCTION()
    FLinearColor CalculateSunColorForTime(float TimeHours);

private:
    float CurrentTimeOfDay = 12.0f; // Start at noon
    float CurrentWeatherIntensity = 0.0f;
    bool bAtmosphericEffectsInitialized = false;
};