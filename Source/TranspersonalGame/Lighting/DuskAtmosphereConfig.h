#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DuskAtmosphereConfig.generated.h"

/**
 * FLight_DuskPalette — Dusk/Sunset lighting palette configuration
 * Used by the dynamic day/night cycle system to interpolate lighting at dusk.
 * Agent #08 — Lighting & Atmosphere — CAP v58
 */
USTRUCT(BlueprintType)
struct FLight_DuskPalette
{
	GENERATED_BODY()

	/** Sun pitch angle at dusk (near horizon, negative = below) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	float SunPitchDegrees = -5.0f;

	/** Sun light color at dusk — deep warm orange */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	FLinearColor SunColor = FLinearColor(1.0f, 0.55f, 0.235f, 1.0f);

	/** Sun intensity at dusk (lux) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	float SunIntensity = 3.2f;

	/** Fog density at dusk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	float FogDensity = 0.045f;

	/** Fog inscattering color — warm amber */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	FLinearColor FogColor = FLinearColor(0.85f, 0.42f, 0.18f, 1.0f);

	/** Volumetric fog albedo — amber */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	FLinearColor VolumetricFogAlbedo = FLinearColor(0.88f, 0.55f, 0.28f, 1.0f);

	/** Volumetric fog extinction scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	float VolumetricFogExtinction = 1.4f;

	/** SkyLight intensity at dusk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	float SkyLightIntensity = 1.6f;

	/** SkyLight color — warm amber sky */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	FLinearColor SkyLightColor = FLinearColor(1.0f, 0.706f, 0.392f, 1.0f);

	/** West fill light color — warm orange */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|FillLights")
	FLinearColor WestFillColor = FLinearColor(1.0f, 0.471f, 0.157f, 1.0f);

	/** West fill light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|FillLights")
	float WestFillIntensity = 8000.0f;

	/** East rim light color — violet/purple */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|FillLights")
	FLinearColor EastRimColor = FLinearColor(0.471f, 0.235f, 0.784f, 1.0f);

	/** East rim light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|FillLights")
	float EastRimIntensity = 4000.0f;

	/** Ground bounce light color — amber */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|FillLights")
	FLinearColor GroundBounceColor = FLinearColor(1.0f, 0.627f, 0.235f, 1.0f);

	/** Ground bounce light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|FillLights")
	float GroundBounceIntensity = 3500.0f;

	/** Rayleigh scattering scale for dusk atmosphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Atmosphere")
	float RayleighScatteringScale = 0.0331f;

	/** Mie scattering scale for dusk haze */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Atmosphere")
	float MieScatteringScale = 0.0042f;

	/** Mie anisotropy — forward scattering for sun glare */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Atmosphere")
	float MieAnisotropy = 0.88f;
};

/**
 * ELight_TimeOfDay — Time of day enumeration for the lighting system
 * Covers all 4 palettes implemented across cycles 006-009
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
	Night       UMETA(DisplayName = "Night"),
	Dawn        UMETA(DisplayName = "Dawn"),
	Midday      UMETA(DisplayName = "Midday"),
	Dusk        UMETA(DisplayName = "Dusk"),
	MAX         UMETA(Hidden)
};

/**
 * UDuskAtmosphereConfig — Data asset for dusk lighting configuration
 * Exposes dusk palette to Blueprint and editor for the day/night cycle system.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDuskAtmosphereConfig : public UObject
{
	GENERATED_BODY()

public:
	UDuskAtmosphereConfig();

	/** The dusk lighting palette — all parameters for sunset rendering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
	FLight_DuskPalette DuskPalette;

	/** Current time of day — used by the day/night cycle manager */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dusk;

	/** Transition duration in seconds between time-of-day palettes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float TransitionDurationSeconds = 120.0f;

	/** Whether the day/night cycle is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	bool bDayNightCycleActive = true;

	/** Returns the dusk palette for Blueprint use */
	UFUNCTION(BlueprintCallable, Category = "Lighting|Dusk")
	FLight_DuskPalette GetDuskPalette() const;

	/** Returns the sun pitch for the given time of day */
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	float GetSunPitchForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;

	/** Returns the fog density for the given time of day */
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	float GetFogDensityForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;
};
