#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "EnvironmentAtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunElevation = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_TimeOfDaySettings GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_TimeOfDaySettings Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_TimeOfDaySettings Sunset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumeFogDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor VolumeFogColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentAtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 17.0f; // Golden hour (5 PM)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FEnvArt_BiomeAtmosphere> BiomeAtmospheres;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EBiomeType CurrentBiome = EBiomeType::Savana;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupVolumeFog(float Density, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericScattering();

private:
    void InitializeLightingActors();
    void UpdateSunPosition();
    void UpdateSkyColor();
    void UpdateFogSettings();
    FEnvArt_TimeOfDaySettings GetCurrentTimeSettings();
};