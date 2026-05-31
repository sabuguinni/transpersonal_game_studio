#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "Light_AtmosphericController.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float Temperature = 3200.0f;

    FLight_BiomeAtmosphere()
    {
        AmbientColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 8.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        Temperature = 3200.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    FRotator SunRotation = FRotator(-25.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    float SunIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    FLinearColor SunColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    float FogOpacityMultiplier = 1.0f;

    FLight_TimeSettings()
    {
        SunRotation = FRotator(-25.0f, 45.0f, 0.0f);
        SunIntensityMultiplier = 1.0f;
        SunColorTint = FLinearColor::White;
        FogOpacityMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericController : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Day/Night Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeState = ELight_TimeOfDay::Noon;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Biome Atmosphere Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    TMap<FString, FLight_BiomeAtmosphere> BiomeSettings;

    // Time of Day Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    // Light References
    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class ADirectionalLight* MainSun;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class AAtmosphericFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    TArray<class APointLight*> BiomeAtmosphericLights;

public:
    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyBiomeAtmosphere(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void InitializeLightReferences();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    ELight_TimeOfDay GetTimeStateFromHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetupCretaceousLighting();

private:
    void InitializeBiomeSettings();
    void InitializeTimeSettings();
    void UpdateLightingForTime();
    void InterpolateLightSettings(const FLight_TimeSettings& From, const FLight_TimeSettings& To, float Alpha);
};