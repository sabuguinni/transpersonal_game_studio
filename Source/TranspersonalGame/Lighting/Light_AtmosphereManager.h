#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScattering = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AtmosphereColor = FLinearColor(1.0f, 0.86f, 0.71f, 1.0f);

    FLight_AtmosphereSettings()
    {
        SunIntensity = 8.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        SunTemperature = 5800.0f;
        FogDensity = 0.02f;
        VolumetricScattering = 2.0f;
        AtmosphereColor = FLinearColor(1.0f, 0.86f, 0.71f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_AtmosphereSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_AtmosphereSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_AtmosphereSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_AtmosphereSettings NightSettings;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLightingComponents();

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphereActor;

    void FindLightingActors();
    void UpdateSunPosition();
    void InterpolateLightingSettings();
    FLight_AtmosphereSettings GetInterpolatedSettings() const;
    float GetSunAngle() const;
};