#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SkyLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SkyLightIntensity;

    FEnvArt_BiomeAtmosphere()
    {
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        SkyLightIntensity = 1.0f;
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
    class UExponentialHeightFogComponent* FogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkyLightComponent* SkyLightComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<EBiomeType, FEnvArt_BiomeAtmosphere> BiomeAtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableDynamicWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WeatherTransitionSpeed;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor)
    void RefreshAtmosphericSettings();

private:
    void InitializeBiomeSettings();
    void UpdateFogSettings(const FEnvArt_BiomeAtmosphere& Settings);
    void UpdateSkyLightSettings(const FEnvArt_BiomeAtmosphere& Settings);
    void InterpolateAtmosphere(const FEnvArt_BiomeAtmosphere& From, const FEnvArt_BiomeAtmosphere& To, float Alpha);

    EBiomeType CurrentBiome;
    float AtmosphereBlendTime;
    bool bIsTransitioning;
};