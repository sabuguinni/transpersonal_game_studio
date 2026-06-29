#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "LightingAtmosphereManager.generated.h"

// === LIGHTING PALETTE ENUM ===
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    BlueHour    UMETA(DisplayName = "Blue Hour"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

// === WEATHER STATE ENUM ===
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Rainy       UMETA(DisplayName = "Rainy")
};

// === LIGHTING PALETTE DATA ===
USTRUCT(BlueprintType)
struct FLight_Palette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -78.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.85f, 0.82f, 0.72f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.018f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AutoExposureBias = 0.3f;
};

/**
 * ALightingAtmosphereManager
 * Manages dynamic day/night cycle, weather transitions, and atmospheric lighting
 * for the prehistoric survival game world. Integrates with UE5 Lumen GI.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // === CURRENT STATE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|State")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|State")
    float TimeOfDaySeconds = 43200.0f; // 12:00 noon in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|State")
    float DayDurationSeconds = 1200.0f; // 20 minutes per full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|State")
    bool bDynamicDayNightEnabled = true;

    // === SCENE REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SkyLightActor;

    // === PALETTE LIBRARY ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    TArray<FLight_Palette> TimePalettes;

    // === BLUEPRINT CALLABLE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPalette(const FLight_Palette& Palette);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentHour() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetTimeOfDayFromHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void AdvanceTimeBySeconds(float Seconds);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_Palette GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeDefaultPalettes();

private:
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyLightingToScene(const FLight_Palette& Palette);
    FLight_Palette LerpPalettes(const FLight_Palette& A, const FLight_Palette& B, float Alpha) const;

    float TransitionAlpha = 0.0f;
    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::Midday;
};
