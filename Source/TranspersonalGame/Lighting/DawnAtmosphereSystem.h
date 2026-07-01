#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "DawnAtmosphereSystem.generated.h"

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

USTRUCT(BlueprintType)
struct FLight_AtmospherePalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.72f, 0.52f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchDegrees = -12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYawDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.85f, 0.78f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.9f, 0.65f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    bool bVolumetricFog = true;
};

/**
 * ADawnAtmosphereSystem
 * Manages dynamic day/night cycle lighting for the Cretaceous prehistoric world.
 * Controls sun position, sky color, fog density and volumetric atmosphere
 * across 6 time-of-day states: Dawn, Morning, Midday, Afternoon, Dusk, Night.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dawn Atmosphere System"))
class TRANSPERSONALGAME_API ADawnAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ADawnAtmosphereSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Time of Day ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float CurrentTimeNormalized = 0.1f;  // 0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bAutoAdvanceTime = true;

    // --- Lighting References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> FogActor;

    // --- Palettes ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette AfternoonPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette NightPalette;

    // --- Ambient Sound References (Freesound IDs for Audio Agent) ---
    // Dawn: 749737 (dense forest birds), 800712 (forest birds wind)
    // Wind: 324573, 324888, 361216, 361431, 385833 (wind in trees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Audio")
    int32 DawnAmbientSoundID = 749737;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Audio")
    int32 WindAmbientSoundID = 800712;

    // --- Blueprint-callable functions ---
    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void SetTimeNormalized(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    float GetCurrentTimeNormalized() const { return CurrentTimeNormalized; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|Palettes")
    FLight_AtmospherePalette GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Palettes")
    void ApplyPalette(const FLight_AtmospherePalette& Palette);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Debug")
    void ApplyDawnPaletteNow();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Debug")
    void ApplyMiddayPaletteNow();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Debug")
    void ApplyDuskPaletteNow();

private:
    float ElapsedDayTime = 0.0f;

    void AdvanceDayCycle(float DeltaTime);
    FLight_AtmospherePalette InterpolatePalettes(const FLight_AtmospherePalette& A, const FLight_AtmospherePalette& B, float Alpha) const;
    void InitDefaultPalettes();
};
