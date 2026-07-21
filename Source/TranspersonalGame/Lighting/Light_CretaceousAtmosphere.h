#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Light_CretaceousAtmosphere.generated.h"

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
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.86f, 0.78f, 0.71f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.71f, 0.63f, 0.47f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.004f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CretaceousAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    ALight_CretaceousAtmosphere();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core atmospheric components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Time and weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentDayTime = 0.5f; // 0.0 = midnight, 0.5 = noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;

    // Atmospheric settings for different times/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    TMap<ELight_TimeOfDay, FLight_AtmosphericSettings> TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    TMap<ELight_WeatherType, FLight_AtmosphericSettings> WeatherSettings;

    // Foundation lighting integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Lighting")
    float FoundationLightIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Lighting")
    float FoundationLightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Lighting")
    FLinearColor FoundationLightColor = FLinearColor(1.0f, 0.78f, 0.47f, 1.0f);

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherType(ELight_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetDayTime(float NewDayTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Foundation Lighting")
    void UpdateFoundationLighting();

    UFUNCTION(BlueprintCallable, Category = "Foundation Lighting")
    void CreateFoundationLight(FVector Location, const FString& LightName);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SetupCretaceousAtmosphere();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ResetToDefaultSettings();

private:
    // Internal update functions
    void UpdateSunPosition();
    void UpdateAtmosphericLighting();
    void UpdateFogSettings();
    void UpdateSkyAtmosphere();
    void TransitionBetweenSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha);

    // Time tracking
    float TimeAccumulator = 0.0f;
    ELight_TimeOfDay PreviousTimeOfDay;
    ELight_WeatherType PreviousWeather;

    // Component references (found at runtime)
    UPROPERTY()
    ADirectionalLight* MainDirectionalLight;

    UPROPERTY()
    ASkyLight* MainSkyLight;

    UPROPERTY()
    AExponentialHeightFog* MainHeightFog;

    UPROPERTY()
    ASkyAtmosphere* MainSkyAtmosphere;

    // Foundation light tracking
    UPROPERTY()
    TArray<class APointLight*> FoundationLights;

    // Settings initialization
    void InitializeTimeOfDaySettings();
    void InitializeWeatherSettings();
    void FindAtmosphericComponents();
};