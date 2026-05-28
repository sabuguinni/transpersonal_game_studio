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
    Dawn,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 180.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAbsorption = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorSaturation = FVector4(1.1f, 1.05f, 0.95f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTint = 0.02f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings CretaceousSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class APostProcessVolume* PostProcessVolume;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeHours(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericScattering();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdatePostProcessSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void FindAtmosphericActors();

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    float GetNormalizedTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    float GetCurrentSunIntensity() const;

private:
    float TimeAccumulator = 0.0f;
    
    void UpdateDayNightCycle(float DeltaTime);
    FLight_AtmosphericSettings InterpolateAtmosphericSettings(ELight_TimeOfDay TimeOfDay, float NormalizedTime);
    FRotator CalculateSunRotation(float NormalizedTime);
};