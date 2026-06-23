#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "LightingAtmosphereConfig.generated.h"

/**
 * Lighting preset data for a specific time of day / weather state.
 * All values are tuned for Cretaceous-era prehistoric environment.
 * Used by ALightingAtmosphereManager to drive dynamic day/night cycle.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDayPreset
{
	GENERATED_BODY()

	/** Display name for this preset (e.g. "Golden Hour", "Midday", "Dusk") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
	FName PresetName;

	/** Sun pitch angle (MUST be negative to illuminate terrain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "-90.0", ClampMax = "0.0"))
	float SunPitch = -45.0f;

	/** Sun yaw angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
	float SunYaw = 45.0f;

	/** Sun intensity in lux */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "0.0", ClampMax = "150000.0"))
	float SunIntensity = 12.0f;

	/** Sun color tint (warm golden for golden hour, white for midday) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
	FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.63f, 1.0f);

	/** Sky light intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
	float SkyLightIntensity = 1.5f;

	/** Exponential height fog density */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.035f;

	/** Fog inscattering color (atmospheric haze tint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
	FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

	/** Post-process exposure bias (manual mode) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
	float ExposureBias = 1.2f;

	/** Bloom intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
	float BloomIntensity = 0.4f;

	/** Color saturation (RGBA — A = global) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|ColorGrading")
	FVector4 ColorSaturation = FVector4(1.1f, 1.05f, 0.95f, 1.0f);

	/** Transition blend time in seconds when switching presets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Transition")
	float BlendDuration = 5.0f;
};

/**
 * Weather state enum for atmosphere variation.
 */
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
	Clear        UMETA(DisplayName = "Clear Sky"),
	PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
	Overcast     UMETA(DisplayName = "Overcast"),
	Stormy       UMETA(DisplayName = "Stormy"),
	HeavyRain    UMETA(DisplayName = "Heavy Rain"),
	Foggy        UMETA(DisplayName = "Dense Fog"),
};

/**
 * Time of day enum for preset selection.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
	Dawn         UMETA(DisplayName = "Dawn"),
	Sunrise      UMETA(DisplayName = "Sunrise"),
	Morning      UMETA(DisplayName = "Morning"),
	Midday       UMETA(DisplayName = "Midday"),
	Afternoon    UMETA(DisplayName = "Afternoon"),
	GoldenHour   UMETA(DisplayName = "Golden Hour"),
	Dusk         UMETA(DisplayName = "Dusk"),
	Night        UMETA(DisplayName = "Night"),
	Midnight     UMETA(DisplayName = "Midnight"),
};
