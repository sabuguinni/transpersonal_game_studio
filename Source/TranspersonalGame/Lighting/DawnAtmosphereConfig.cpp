#include "DawnAtmosphereConfig.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UDawnAtmosphereConfig::UDawnAtmosphereConfig()
{
    CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
}

bool UDawnAtmosphereConfig::IsDawnActive() const
{
    return CurrentTimeOfDay == ELight_TimeOfDay::Dawn;
}

float UDawnAtmosphereConfig::GetDawnBlendFactor(float GameHour) const
{
    // Dawn window: 5:00 - 7:00
    // Peak at 6:00 (factor = 1.0), fade at edges
    const float DawnStart = 5.0f;
    const float DawnPeak  = 6.0f;
    const float DawnEnd   = 7.0f;

    if (GameHour < DawnStart || GameHour > DawnEnd)
    {
        return 0.0f;
    }

    if (GameHour <= DawnPeak)
    {
        // Ramp up from DawnStart to DawnPeak
        return (GameHour - DawnStart) / (DawnPeak - DawnStart);
    }
    else
    {
        // Ramp down from DawnPeak to DawnEnd
        return 1.0f - ((GameHour - DawnPeak) / (DawnEnd - DawnPeak));
    }
}

void UDawnAtmosphereConfig::ApplyDawnPaletteToWorld()
{
    // This function is intended to be called from Blueprint or editor
    // Actual lighting actor manipulation is done via UE5 Python in production
    // This stub logs the intent and palette values for debugging
    UE_LOG(LogTemp, Log, TEXT("[DawnAtmosphereConfig] ApplyDawnPaletteToWorld called"));
    UE_LOG(LogTemp, Log, TEXT("  Sun Pitch: %.1f, Yaw: %.1f, Intensity: %.1f"),
        DawnPalette.SunPitchDegrees,
        DawnPalette.SunYawDegrees,
        DawnPalette.SunIntensityLux);
    UE_LOG(LogTemp, Log, TEXT("  Fog Density: %.3f, Volumetric: %s"),
        DawnPalette.FogDensity,
        DawnPalette.bVolumetricFog ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("  SkyLight Intensity: %.2f"),
        DawnPalette.SkyLightIntensity);
}
