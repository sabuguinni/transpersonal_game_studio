#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "EnvArt_AtmosphereSystem.generated.h"

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
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    FEnvArt_LightingSettings()
    {
        SunElevation = -30.0f;
        SunAzimuth = 45.0f;
        SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        SunIntensity = 3.0f;
        FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
        FogDensity = 0.02f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphereSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphereSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DayDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableDynamicLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> TimeOfDaySettings;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void AddVolumetricFog(const FVector& Location, float Radius = 2000.0f);

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    UExponentialHeightFogComponent* HeightFog;

    UPROPERTY()
    USkyAtmosphereComponent* SkyAtmosphere;

    float CurrentDayTime = 0.0f;

    void FindLightingComponents();
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);
    void InitializeTimeOfDaySettings();
};