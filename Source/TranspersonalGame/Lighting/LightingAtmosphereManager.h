#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    HeatHaze    UMETA(DisplayName = "Heat Haze")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_AtmosphereComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_AtmosphereComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float CurrentTimeOfDayNormalized = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayNightCycleActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionSpeed = 0.5f;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimeOfDayPreset GetPresetForTime(float NormalizedTime) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunPitchForTime(float NormalizedTime) const;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UDirectionalLightComponent* CachedSunLight = nullptr;

    UPROPERTY()
    USkyLightComponent* CachedSkyLight = nullptr;

    UPROPERTY()
    UExponentialHeightFogComponent* CachedFog = nullptr;

    void FindLightingComponents();
    void ApplyTimeOfDayPreset(const FLight_TimeOfDayPreset& Preset);
};

UCLASS(ClassGroup = (TranspersonalGame), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    ULight_AtmosphereComponent* AtmosphereComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bAutoAdvanceTime = true;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyCretaceousGoldenHour();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyCretaceousMidday();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyCretaceousNight();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
