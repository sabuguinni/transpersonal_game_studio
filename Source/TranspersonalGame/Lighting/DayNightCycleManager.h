#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

// === Light_DayPhase: Current phase of the day/night cycle ===
UENUM(BlueprintType)
enum class ELight_DayPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

// === Light_SkyPalette: Per-phase lighting parameters ===
USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.020f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogScattering = 0.5f;
};

// === ADayNightCycleManager: Drives the full day/night cycle ===
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === Day/Night Cycle Config ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds = 600.0f;  // 10 real minutes = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 0.35f;  // 0.0=midnight, 0.25=dawn, 0.5=midday, 0.75=dusk, 1.0=midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bPauseCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeMultiplier = 1.0f;

    // === Scene References ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SkyLightActor = nullptr;

    // === Palette Overrides ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette AfternoonPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette TwilightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette NightPalette;

    // === Blueprint-callable API ===
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_DayPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLight_SkyPalette GetPaletteForPhase(ELight_DayPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyPalette(const FLight_SkyPalette& Palette);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FString GetPhaseDisplayName() const;

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void AutoFindSceneActors();

private:
    void TickCycle(float DeltaTime);
    void BlendPalettes(const FLight_SkyPalette& From, const FLight_SkyPalette& To, float Alpha);
    void InitDefaultPalettes();

    float PreviousTimeOfDay = 0.0f;
    ELight_DayPhase CachedPhase = ELight_DayPhase::Morning;
};
