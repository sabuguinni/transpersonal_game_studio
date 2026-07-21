#include "DuskAtmosphereConfig.h"

UDuskAtmosphereConfig::UDuskAtmosphereConfig()
{
	// Initialize with default dusk palette values
	DuskPalette.SunPitchDegrees = -5.0f;
	DuskPalette.SunColor = FLinearColor(1.0f, 0.55f, 0.235f, 1.0f);
	DuskPalette.SunIntensity = 3.2f;
	DuskPalette.FogDensity = 0.045f;
	DuskPalette.FogColor = FLinearColor(0.85f, 0.42f, 0.18f, 1.0f);
	DuskPalette.VolumetricFogAlbedo = FLinearColor(0.88f, 0.55f, 0.28f, 1.0f);
	DuskPalette.VolumetricFogExtinction = 1.4f;
	DuskPalette.SkyLightIntensity = 1.6f;
	DuskPalette.SkyLightColor = FLinearColor(1.0f, 0.706f, 0.392f, 1.0f);
	DuskPalette.WestFillColor = FLinearColor(1.0f, 0.471f, 0.157f, 1.0f);
	DuskPalette.WestFillIntensity = 8000.0f;
	DuskPalette.EastRimColor = FLinearColor(0.471f, 0.235f, 0.784f, 1.0f);
	DuskPalette.EastRimIntensity = 4000.0f;
	DuskPalette.GroundBounceColor = FLinearColor(1.0f, 0.627f, 0.235f, 1.0f);
	DuskPalette.GroundBounceIntensity = 3500.0f;
	DuskPalette.RayleighScatteringScale = 0.0331f;
	DuskPalette.MieScatteringScale = 0.0042f;
	DuskPalette.MieAnisotropy = 0.88f;

	CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
	TransitionDurationSeconds = 120.0f;
	bDayNightCycleActive = true;
}

FLight_DuskPalette UDuskAtmosphereConfig::GetDuskPalette() const
{
	return DuskPalette;
}

float UDuskAtmosphereConfig::GetSunPitchForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
	switch (TimeOfDay)
	{
		case ELight_TimeOfDay::Night:   return -15.0f;  // Moonlight, low angle
		case ELight_TimeOfDay::Dawn:    return  8.0f;   // Sunrise, just above horizon
		case ELight_TimeOfDay::Midday:  return -75.0f;  // Overhead (UE5 convention: negative = up)
		case ELight_TimeOfDay::Dusk:    return -5.0f;   // Near horizon, sunset
		default:                        return -45.0f;  // Default mid-afternoon
	}
}

float UDuskAtmosphereConfig::GetFogDensityForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
	switch (TimeOfDay)
	{
		case ELight_TimeOfDay::Night:   return 0.035f;  // Light night mist
		case ELight_TimeOfDay::Dawn:    return 0.055f;  // Morning mist, heavier
		case ELight_TimeOfDay::Midday:  return 0.018f;  // Clear midday, minimal fog
		case ELight_TimeOfDay::Dusk:    return 0.045f;  // Dusk haze, warm and thick
		default:                        return 0.025f;
	}
}
