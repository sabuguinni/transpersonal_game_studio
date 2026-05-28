#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "EnvArt_AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunElevation = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Intensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    FEnvArt_LightingSettings()
    {
        SunElevation = -30.0f;
        SunAzimuth = 45.0f;
        Intensity = 8.0f;
        LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeProgressionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<TObjectPtr<AExponentialHeightFog>> FogActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> TimeOfDaySettings;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnAtmosphericFog(FVector Location, FString Label);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor)
    void ApplyCurrentSettings();

private:
    UPROPERTY()
    float CurrentTimeValue = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    void InitializeTimeOfDaySettings();
    void UpdateSunPosition();
    void UpdateFogSettings();
    FEnvArt_LightingSettings GetInterpolatedSettings(float TimeValue) const;
};