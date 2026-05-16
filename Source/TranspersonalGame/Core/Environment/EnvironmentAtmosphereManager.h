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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    FEnvArt_AtmosphereSettings()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunAngle = 45.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        FogHeightFalloff = 0.2f;
        SkyLightIntensity = 1.0f;
    }
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* FogActor;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphereActors();

private:
    void FindExistingAtmosphereActors();
    void UpdateLightingBasedOnTime();
    FRotator CalculateSunRotation(float TimeHours);
    FLinearColor CalculateSunColor(float TimeHours);
    float CalculateSunIntensity(float TimeHours);
};