#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

// ============================================================
// Enums — Light_ prefix (Rule 2: unique names across project)
// ============================================================

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    HeatHaze    UMETA(DisplayName = "Heat Haze")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.55f, 0.75f, 0.95f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_RuinLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor Color = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Intensity = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AttenuationRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Temperature = 2800.0f;
};

// ============================================================
// ACretaceousLightingManager
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    // --------------------------------------------------------
    // Time of Day
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float CurrentTimeNormalized = 0.65f;  // 0=midnight, 0.5=noon, 0.65=golden hour

    // --------------------------------------------------------
    // Weather
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    // --------------------------------------------------------
    // Lumen Settings
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenGIEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenReflectionsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    float LumenSceneDetail = 1.0f;

    // --------------------------------------------------------
    // Ruin Lighting
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    TArray<FLight_RuinLightConfig> RuinLightConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    bool bRuinLightsEnabled = true;

    // --------------------------------------------------------
    // Cached References
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Refs", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Refs", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Refs", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Refs", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<APointLight>> RuinPointLights;

    // --------------------------------------------------------
    // Functions
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeOfDay(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyWeather(ELight_WeatherState Weather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunPosition(float Pitch, float Yaw);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableLumen(bool bGI, bool bReflections);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SpawnRuinLights();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyMidnightPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyStormPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimeOfDaySettings GetSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;

    // --------------------------------------------------------
    // AActor overrides
    // --------------------------------------------------------

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    void FindLightActorsInWorld();
    void ApplySettings(const FLight_TimeOfDaySettings& Settings);
};
