#include "LightingAtmosphereConfig.h"

ULightingAtmosphereConfig::ULightingAtmosphereConfig()
{
    // Defaults already set in header via member initializers.
    // CDO-safe: no world access, no pointer dereferences.
    CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
}

FLight_SunConfig ULightingAtmosphereConfig::GetSunConfigForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    FLight_SunConfig Config;

    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:
        Config.PitchDegrees   = -8.0f;
        Config.YawDegrees     = 80.0f;
        Config.Intensity      = 4.0f;
        Config.LightColor     = FColor(255, 180, 120, 255); // deep orange dawn
        Config.ShadowAmount   = 0.7f;
        break;

    case ELight_TimeOfDay::GoldenHour:
        Config.PitchDegrees   = -35.0f;
        Config.YawDegrees     = 45.0f;
        Config.Intensity      = 12.0f;
        Config.LightColor     = FColor(255, 220, 160, 255); // warm golden
        Config.ShadowAmount   = 0.85f;
        break;

    case ELight_TimeOfDay::Midday:
        Config.PitchDegrees   = -75.0f;
        Config.YawDegrees     = 0.0f;
        Config.Intensity      = 18.0f;
        Config.LightColor     = FColor(255, 248, 230, 255); // near-white hot
        Config.ShadowAmount   = 0.95f;
        break;

    case ELight_TimeOfDay::Dusk:
        Config.PitchDegrees   = -12.0f;
        Config.YawDegrees     = -60.0f;
        Config.Intensity      = 5.0f;
        Config.LightColor     = FColor(255, 140, 80, 255);  // deep amber dusk
        Config.ShadowAmount   = 0.75f;
        break;

    case ELight_TimeOfDay::Night:
        Config.PitchDegrees   = -5.0f;
        Config.YawDegrees     = 180.0f;
        Config.Intensity      = 0.3f;
        Config.LightColor     = FColor(120, 140, 200, 255); // cool moonlight blue
        Config.ShadowAmount   = 0.5f;
        break;

    default:
        // Fallback to golden hour
        Config = SunConfig;
        break;
    }

    Config.DynamicShadowDistance = SunConfig.DynamicShadowDistance;
    return Config;
}

FString ULightingAtmosphereConfig::GetTimeOfDayName(ELight_TimeOfDay TimeOfDay)
{
    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:        return TEXT("Dawn");
    case ELight_TimeOfDay::GoldenHour:  return TEXT("Golden Hour");
    case ELight_TimeOfDay::Midday:      return TEXT("Midday");
    case ELight_TimeOfDay::Dusk:        return TEXT("Dusk");
    case ELight_TimeOfDay::Night:       return TEXT("Night");
    default:                            return TEXT("Unknown");
    }
}
