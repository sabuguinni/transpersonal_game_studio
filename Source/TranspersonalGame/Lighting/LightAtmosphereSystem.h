#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightAtmosphereSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

USTRUCT(BlueprintType)
struct FLight_DayPhaseConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.65f, 0.85f, 1.0f);
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightAtmosphereComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightAtmosphereComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_DayPhaseConfig GoldenHourConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_DayPhaseConfig NoonConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_DayPhaseConfig NightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float CurrentTimeNormalized = 0.6f;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_DayPhaseConfig GetCurrentPhaseConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentSunAngle() const;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float ElapsedTime = 0.0f;
    void AdvanceDayNightCycle(float DeltaTime);
    FLight_DayPhaseConfig InterpolateConfigs(const FLight_DayPhaseConfig& A, const FLight_DayPhaseConfig& B, float Alpha) const;
};

UCLASS(ClassGroup = (TranspersonalGame), BlueprintType)
class TRANSPERSONALGAME_API ALightAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALightAtmosphereSystem();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    ULightAtmosphereComponent* AtmosphereComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* FogActor;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingConfig(const FLight_DayPhaseConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetGoldenHourPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetNightPreset();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyGoldenHourInEditor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
