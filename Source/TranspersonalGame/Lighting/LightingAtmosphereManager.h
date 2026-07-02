#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
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
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct FLight_SunPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 7.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.82f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.022f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.78f, 0.65f, 0.45f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.95f, 0.88f, 0.72f, 1.0f);
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    // Current weather state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    // Day/night cycle speed (0 = static, 1 = real-time)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float DayCycleSpeed = 0.0f;

    // Current time in hours (0-24)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 15.0f;

    // Active sun palette
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLight_SunPalette ActivePalette;

    // Reference to directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    // Reference to height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Reference to sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // Apply a specific time-of-day palette
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeOfDayPalette(ELight_TimeOfDay TimeOfDay);

    // Apply a specific weather state
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyWeatherState(ELight_WeatherState Weather);

    // Get palette for a given time of day
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_SunPalette GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const;

    // Advance time by delta hours
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void AdvanceTime(float DeltaHours);

    // Auto-find and cache lighting actors in world
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void AutoFindLightingActors();

    // Apply the active palette to scene actors
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyActivePalette();

    // Ambient sound ID for current time of day (Freesound)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    int32 CurrentAmbientSoundID = 846981;

    // Forest ambience sound IDs by time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<int32> ForestAmbienceSoundIDs;

private:
    // Interpolate between two palettes
    FLight_SunPalette LerpPalettes(const FLight_SunPalette& A, const FLight_SunPalette& B, float Alpha) const;

    // Apply palette to actual scene components
    void ApplyPaletteToScene(const FLight_SunPalette& Palette);

    float TimeAccumulator = 0.0f;
};
