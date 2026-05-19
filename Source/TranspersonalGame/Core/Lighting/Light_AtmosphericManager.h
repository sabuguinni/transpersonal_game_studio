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

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Humid       UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAbsorption = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorSaturation = FVector4(1.2f, 1.1f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorGamma = FVector4(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FLinearColor SceneColorTint = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period settings
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

    // Time and Weather Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    FLight_AtmosphericSettings CretaceousSettings;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdatePostProcessing();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void InitializeAtmosphericComponents();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ValidateAtmosphericSetup();

private:
    void FindOrCreateAtmosphericActors();
    void ConfigureDirectionalLight();
    void ConfigureSkyAtmosphere();
    void ConfigureHeightFog();
    void ConfigurePostProcess();
    
    float CalculateSunElevation(float TimeHours) const;
    float CalculateSunAzimuth(float TimeHours) const;
    FLinearColor CalculateSunColor(float TimeHours) const;
    float CalculateSunIntensity(float TimeHours) const;

    bool bIsInitialized = false;
    float LastUpdateTime = 0.0f;
};