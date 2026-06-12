#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorSaturation = FVector4(1.1f, 1.0f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorGamma = FVector4(0.95f, 1.0f, 1.05f, 1.0f);
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Structures")
    void UpdateStructureLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void FindAndAssignLightingActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void CreateAtmosphericLightingSetup();

private:
    float CurrentTimeHours = 12.0f;
    
    void UpdateSunPosition();
    void UpdateSunIntensity();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    void UpdatePostProcessing();
    FLight_AtmosphericSettings GetSettingsForTimeAndWeather(ELight_TimeOfDay TimeOfDay, ELight_WeatherType Weather);
};