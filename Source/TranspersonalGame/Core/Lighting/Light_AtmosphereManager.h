#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/Actor.h"
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
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-35.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
};

/**
 * Manages atmospheric lighting for the Cretaceous period setting
 * Handles day/night cycle, weather transitions, and prehistoric atmosphere
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationInMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = true;

    // Lighting references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class APostProcessVolume* PostProcessVolume;

    // Atmosphere presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphereSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphereSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphereSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphereSettings NightSettings;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_AtmosphereSettings GetCurrentAtmosphereSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindLightingActors();

private:
    void UpdateSunPosition();
    void UpdateAtmosphereScattering();
    void UpdateFogSettings();
    void UpdatePostProcessing();
    FLight_AtmosphereSettings InterpolateSettings(const FLight_AtmosphereSettings& A, const FLight_AtmosphereSettings& B, float Alpha) const;
    float GetDayNightProgress() const;
};