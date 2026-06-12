#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Light_AtmosphereController.generated.h"

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
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);

    FLight_AtmosphereSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        SkyLightIntensity = 1.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<ELight_TimeOfDay, FLight_AtmosphereSettings> AtmospherePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* FogActor;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_AtmosphereSettings GetCurrentAtmosphereSettings() const;

private:
    void InitializeAtmospherePresets();
    FLight_AtmosphereSettings InterpolateAtmosphereSettings(ELight_TimeOfDay TimeA, ELight_TimeOfDay TimeB, float Alpha) const;
    float GetSunAngle() const;
};