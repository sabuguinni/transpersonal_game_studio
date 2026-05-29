#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvironmentAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Night
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<EEnvArt_TimeOfDay, FEnvArt_AtmosphereSettings> AtmospherePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* HeightFog;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetVolumetricFog(bool bEnabled, float Density = 0.02f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindAndConfigureLightingActors();

private:
    float TimeOfDayProgress = 0.0f;
    
    void InitializeAtmospherePresets();
    void UpdateTimeProgression(float DeltaTime);
    void InterpolateAtmosphere(EEnvArt_TimeOfDay FromTime, EEnvArt_TimeOfDay ToTime, float Alpha);
};