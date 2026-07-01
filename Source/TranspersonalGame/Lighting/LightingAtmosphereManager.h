#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightingAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AutoExposureBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // Day/night cycle speed (seconds per full cycle)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayCycleDurationSeconds = 1200.0f;

    // Whether the day/night cycle is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bDayCycleActive = false;

    // Current time in [0,1] where 0=midnight, 0.25=dawn, 0.5=midday, 0.75=dusk
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NormalizedTimeOfDay = 0.5f;

    // Palettes for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_TimeOfDayPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_TimeOfDayPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_TimeOfDayPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_TimeOfDayPalette NightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_TimeOfDayPalette StormyPalette;

    // References to scene lights (auto-found in BeginPlay)
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    ASkyLight* SkyLightActor = nullptr;

    // Apply a specific palette immediately
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPalette(const FLight_TimeOfDayPalette& Palette);

    // Apply time of day preset
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    // Get palette for a given time of day
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimeOfDayPalette GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const;

    // Find and cache scene light references
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindSceneLights();

    // Trigger weather event
    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void TriggerStorm(float DurationSeconds);

    // Is storm active
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Weather")
    bool bStormActive = false;

    // Storm remaining time
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Weather")
    float StormRemainingTime = 0.0f;

private:
    void InitializeDefaultPalettes();
    void UpdateDayCycle(float DeltaTime);
    ELight_TimeOfDay GetTimeOfDayFromNormalized(float NormalizedTime) const;
};
