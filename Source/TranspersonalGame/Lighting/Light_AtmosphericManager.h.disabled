#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphericManager.generated.h"

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
struct TRANSPERSONALGAME_API FLight_CretaceousLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevationAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuthAngle = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    FLight_CretaceousLightingSettings()
    {
        // Default Cretaceous period lighting
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_CretaceousLightingSettings CretaceousSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ELight_TimeOfDay CurrentPeriod = ELight_TimeOfDay::Midday;

    // Actor references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyAtmosphere* AtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APostProcessVolume* PostProcessActor;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdatePostProcess();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDayPeriod() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void CreateMissingLightingActors();

private:
    void UpdateDayNightCycle(float DeltaTime);
    float CalculateSunElevation() const;
    float CalculateSunAzimuth() const;
    FLinearColor CalculateSunColor() const;
    float CalculateSunIntensity() const;
};