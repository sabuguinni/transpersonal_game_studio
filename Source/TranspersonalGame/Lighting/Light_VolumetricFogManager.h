#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Light_VolumetricFogManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FLinearColor VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricFogManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricFogManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* FogComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog Settings")
    TMap<EBiomeType, FLight_BiomeFogSettings> BiomeFogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    float TimeOfDayFogIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    float WeatherFogIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    float FogTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    bool bEnableCretaceousAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float CretaceousHumidityFactor = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FogUpdateInterval = 0.1f;

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void SetBiomeFog(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void UpdateFogForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void UpdateFogForWeather(EWeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void ApplyCretaceousAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void TransitionToFogSettings(const FLight_BiomeFogSettings& TargetSettings, float TransitionDuration = 2.0f);

private:
    void InitializeBiomeFogSettings();
    void UpdateFogTransition(float DeltaTime);
    
    FLight_BiomeFogSettings CurrentFogSettings;
    FLight_BiomeFogSettings TargetFogSettings;
    float FogTransitionTime = 0.0f;
    float FogTransitionDuration = 0.0f;
    bool bIsTransitioning = false;
    float LastFogUpdateTime = 0.0f;
};