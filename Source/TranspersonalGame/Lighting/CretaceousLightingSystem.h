#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "CretaceousLightingSystem.generated.h"

/**
 * Lighting preset for different times of day in the Cretaceous world.
 * Used by ACretaceousLightingSystem to interpolate between states.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDayPreset
{
	GENERATED_BODY()

	/** Display name for this preset (e.g. "Dawn", "Noon", "Dusk", "Night") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
	FString PresetName = TEXT("Default");

	/** Sun pitch angle in degrees (-90 = overhead, -10 = near horizon) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float SunPitchDegrees = -38.0f;

	/** Sun yaw angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float SunYawDegrees = 45.0f;

	/** Sun intensity in lux */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float SunIntensity = 12.0f;

	/** Sun color as linear color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
	FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.619f, 1.0f);

	/** SkyLight intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float SkyLightIntensity = 2.0f;

	/** Fog density */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.018f;

	/** Fog inscattering color (atmospheric tint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
	FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.95f, 1.0f);

	/** Volumetric fog extinction scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float VolumetricFogExtinction = 0.6f;

	/** Ambient temperature feel — affects survival system (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Survival", meta = (ClampMin = "-20.0", ClampMax = "60.0"))
	float AmbientTemperatureCelsius = 32.0f;

	/** Visibility distance for fog cutoff */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "1000.0", ClampMax = "200000.0"))
	float FogCutoffDistance = 80000.0f;
};

/**
 * Enum for current time of day phase.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
	Dawn       UMETA(DisplayName = "Dawn"),
	Morning    UMETA(DisplayName = "Morning"),
	Noon       UMETA(DisplayName = "Noon"),
	Afternoon  UMETA(DisplayName = "Afternoon"),
	Dusk       UMETA(DisplayName = "Dusk"),
	Night      UMETA(DisplayName = "Night")
};

/**
 * ACretaceousLightingSystem — Dynamic day/night cycle lighting controller.
 *
 * Manages the full Cretaceous atmosphere: directional sun, sky atmosphere,
 * skylight with real-time capture, volumetric height fog, and Lumen settings.
 * Integrates with the survival system to affect player temperature.
 *
 * Place one instance in MinPlayableMap. It self-configures on BeginPlay.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting System"))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
	GENERATED_BODY()

public:
	ACretaceousLightingSystem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// ── Day/Night Cycle ──────────────────────────────────────────────────────

	/** Enable real-time day/night cycle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
	bool bEnableDayNightCycle = true;

	/** Speed multiplier for day/night cycle (1.0 = real-time, 60.0 = 1 min per day) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight", meta = (ClampMin = "0.1", ClampMax = "3600.0"))
	float DayNightSpeedMultiplier = 120.0f;

	/** Current time of day in hours (0-24) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float CurrentTimeOfDayHours = 10.0f;

	/** Current phase of day */
	UPROPERTY(BlueprintReadOnly, Category = "Lighting|DayNight")
	ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Morning;

	// ── Lighting Presets ─────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
	FLight_TimeOfDayPreset DawnPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
	FLight_TimeOfDayPreset NoonPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
	FLight_TimeOfDayPreset DuskPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
	FLight_TimeOfDayPreset NightPreset;

	// ── Scene References ─────────────────────────────────────────────────────

	/** Reference to the directional sun light in the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
	ADirectionalLight* SunLight = nullptr;

	/** Reference to the sky light in the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
	ASkyLight* SkyLightActor = nullptr;

	/** Reference to the height fog actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
	AExponentialHeightFog* HeightFogActor = nullptr;

	// ── Survival Integration ─────────────────────────────────────────────────

	/** Current ambient temperature in Celsius (affects player survival stats) */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay|Survival")
	float CurrentAmbientTemperature = 32.0f;

	/** Whether it is currently night (affects dinosaur behavior — nocturnal species active) */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay|Survival")
	bool bIsNight = false;

	// ── Blueprint Events ─────────────────────────────────────────────────────

	/** Called when the time of day phase changes (e.g. Dawn → Morning) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
	void OnTimeOfDayPhaseChanged(ELight_TimeOfDay NewPhase, ELight_TimeOfDay OldPhase);

	/** Called every game hour to allow blueprint responses */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
	void OnGameHourTick(float CurrentHour);

	// ── Public API ───────────────────────────────────────────────────────────

	/** Set the time of day directly (0-24 hours) */
	UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
	void SetTimeOfDay(float NewHour);

	/** Get current ambient temperature for survival system integration */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Survival")
	float GetAmbientTemperature() const { return CurrentAmbientTemperature; }

	/** Get whether it is currently night */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Survival")
	bool GetIsNight() const { return bIsNight; }

	/** Apply a lighting preset immediately (for cutscenes or fast-travel) */
	UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
	void ApplyPreset(const FLight_TimeOfDayPreset& Preset);

	/** Force-find and cache scene lighting actors */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Setup")
	void FindAndCacheLightingActors();

private:
	/** Interpolate between two presets by alpha (0-1) */
	FLight_TimeOfDayPreset LerpPresets(const FLight_TimeOfDayPreset& A, const FLight_TimeOfDayPreset& B, float Alpha) const;

	/** Update sun rotation and color based on current time */
	void UpdateSunLight(const FLight_TimeOfDayPreset& Preset);

	/** Update sky light intensity */
	void UpdateSkyLight(const FLight_TimeOfDayPreset& Preset);

	/** Update height fog parameters */
	void UpdateHeightFog(const FLight_TimeOfDayPreset& Preset);

	/** Determine current phase from time of day */
	ELight_TimeOfDay CalculatePhase(float Hour) const;

	/** Accumulated time for hourly tick events */
	float HourAccumulator = 0.0f;

	/** Last hour that triggered OnGameHourTick */
	int32 LastHourTick = -1;
};
