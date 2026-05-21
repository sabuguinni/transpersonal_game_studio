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
#include "Light_AtmosphericSystem.generated.h"

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
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Settings")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Settings")
    FRotator SunRotation = FRotator(-45.0f, 135.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    float MieScatteringScale = 4.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogDensity = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTemperature = 6200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float ColorSaturation = 1.15f;

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period settings
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    FLight_AtmosphericSettings CurrentSettings;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationInMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    APostProcessVolume* PostProcessActor;

    // Cretaceous period specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    bool bUseCretaceousAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousCO2Multiplier = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousHumidityLevel = 0.8f;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyCretaceousSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateAtmosphericScattering();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdatePostProcessing();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System", CallInEditor)
    void InitializeCretaceousLighting();

private:
    void UpdateDayNightCycle(float DeltaTime);
    FLinearColor CalculateSunColorForTime(float TimeOfDay) const;
    float CalculateSunIntensityForTime(float TimeOfDay) const;
    FRotator CalculateSunRotationForTime(float TimeOfDay) const;
};