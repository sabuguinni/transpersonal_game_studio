#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight"),
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Foggy       UMETA(DisplayName = "Foggy"),
    DustStorm   UMETA(DisplayName = "Dust Storm"),
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Pitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Yaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Intensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    bool bAtmosphereSunLight = true;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogColor = FLinearColor(0.45f, 0.62f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogExtinctionScale = 1.2f;
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCretaceousAtmosphereComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCretaceousAtmosphereComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_SunConfig SunConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_FogConfig FogConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float DayNightCycleSpeed = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bEnableDynamicDayNight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float CurrentTimeHours = 14.5f;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentTimeHours() const { return CurrentTimeHours; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetTimeOfDayFromHours(float Hours) const;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

UCLASS(ClassGroup = (Lighting), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    UCretaceousAtmosphereComponent* AtmosphereComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    APostProcessVolume* PostProcessActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bAutoFindLightActors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float NightDurationSeconds = 300.0f;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(ELight_TimeOfDay TimeOfDay, ELight_WeatherState Weather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunPitch(float Pitch);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void AdvanceTimeOfDay(float DeltaHours);

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyMidnightPreset();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyDawnPreset();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    void AutoFindLightActors();
    void UpdateSunPosition(float TimeHours);
    FLinearColor GetSkyColorForTime(float TimeHours) const;
    float GetSunIntensityForTime(float TimeHours) const;
};
