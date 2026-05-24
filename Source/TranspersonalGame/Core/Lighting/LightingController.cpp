#include "LightingController.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ALightingController::ALightingController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    SkyLight = nullptr;
    HeightFog = nullptr;
    
    InitializeBiomeSettings();
}

void ALightingController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingActors();
    UpdateLighting();
}

void ALightingController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        // Advance time - 24 hours in DayDurationMinutes real minutes
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        UpdateLighting();
    }
}

void ALightingController::InitializeBiomeSettings()
{
    // Swamp - Dark, misty atmosphere
    FLight_BiomeLightingSettings SwampSettings;
    SwampSettings.SunColor = FLinearColor(0.8f, 0.9f, 0.7f);
    SwampSettings.SunIntensity = 2.5f;
    SwampSettings.SkyColor = FLinearColor(0.6f, 0.7f, 0.6f);
    SwampSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.7f);
    SwampSettings.FogDensity = 0.08f;
    SwampSettings.FogHeightFalloff = 0.1f;
    BiomeLightingSettings.Add(ELight_BiomeType::Swamp, SwampSettings);
    
    // Forest - Filtered green light
    FLight_BiomeLightingSettings ForestSettings;
    ForestSettings.SunColor = FLinearColor(0.9f, 1.0f, 0.8f);
    ForestSettings.SunIntensity = 2.0f;
    ForestSettings.SkyColor = FLinearColor(0.4f, 0.7f, 0.5f);
    ForestSettings.FogColor = FLinearColor(0.6f, 0.8f, 0.6f);
    ForestSettings.FogDensity = 0.04f;
    ForestSettings.FogHeightFalloff = 0.3f;
    BiomeLightingSettings.Add(ELight_BiomeType::Forest, ForestSettings);
    
    // Savanna - Bright, clear light
    FLight_BiomeLightingSettings SavannaSettings;
    SavannaSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f);
    SavannaSettings.SunIntensity = 4.0f;
    SavannaSettings.SkyColor = FLinearColor(0.5f, 0.8f, 1.0f);
    SavannaSettings.FogColor = FLinearColor(0.9f, 0.95f, 1.0f);
    SavannaSettings.FogDensity = 0.01f;
    SavannaSettings.FogHeightFalloff = 0.5f;
    BiomeLightingSettings.Add(ELight_BiomeType::Savanna, SavannaSettings);
    
    // Desert - Harsh, warm light
    FLight_BiomeLightingSettings DesertSettings;
    DesertSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    DesertSettings.SunIntensity = 5.0f;
    DesertSettings.SkyColor = FLinearColor(0.8f, 0.9f, 1.0f);
    DesertSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.8f);
    DesertSettings.FogDensity = 0.005f;
    DesertSettings.FogHeightFalloff = 0.8f;
    BiomeLightingSettings.Add(ELight_BiomeType::Desert, DesertSettings);
    
    // Mountain - Cool, crisp light
    FLight_BiomeLightingSettings MountainSettings;
    MountainSettings.SunColor = FLinearColor(0.9f, 0.95f, 1.0f);
    MountainSettings.SunIntensity = 3.5f;
    MountainSettings.SkyColor = FLinearColor(0.4f, 0.6f, 0.9f);
    MountainSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MountainSettings.FogDensity = 0.03f;
    MountainSettings.FogHeightFalloff = 0.15f;
    BiomeLightingSettings.Add(ELight_BiomeType::Mountain, MountainSettings);
}

void ALightingController::FindOrCreateLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing lighting actors
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (!SunLight)
        {
            SunLight = *ActorItr;
            break;
        }
    }
    
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (!SkyAtmosphere)
        {
            SkyAtmosphere = *ActorItr;
            break;
        }
    }
    
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (!SkyLight)
        {
            SkyLight = *ActorItr;
            break;
        }
    }
    
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (!HeightFog)
        {
            HeightFog = *ActorItr;
            break;
        }
    }
}

void ALightingController::InitializeLightingActors()
{
    FindOrCreateLightingActors();
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create missing actors if needed
    if (!SunLight)
    {
        SunLight = World->SpawnActor<ADirectionalLight>();
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("Sun_DirectionalLight"));
        }
    }
    
    if (!SkyAtmosphere)
    {
        SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>();
        if (SkyAtmosphere)
        {
            SkyAtmosphere->SetActorLabel(TEXT("SkyAtmosphere"));
        }
    }
    
    if (!SkyLight)
    {
        SkyLight = World->SpawnActor<ASkyLight>();
        if (SkyLight)
        {
            SkyLight->SetActorLabel(TEXT("SkyLight"));
        }
    }
    
    if (!HeightFog)
    {
        HeightFog = World->SpawnActor<AExponentialHeightFog>();
        if (HeightFog)
        {
            HeightFog->SetActorLabel(TEXT("ExponentialHeightFog"));
        }
    }
}

void ALightingController::SetBiome(ELight_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    UpdateLighting();
}

void ALightingController::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLighting();
}

void ALightingController::UpdateLighting()
{
    UpdateSunPosition();
    UpdateAtmosphere();
    UpdateFog();
}

void ALightingController::UpdateSunPosition()
{
    if (!SunLight) return;
    
    float SunAngle = CalculateSunAngle();
    
    // Calculate sun rotation
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // East-West movement could be added here
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
    
    // Update sun intensity based on time
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    if (LightComp && BiomeLightingSettings.Contains(CurrentBiome))
    {
        const FLight_BiomeLightingSettings& Settings = BiomeLightingSettings[CurrentBiome];
        
        // Calculate intensity multiplier based on sun angle
        float IntensityMultiplier = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunAngle + 90.0f)), 0.0f, 1.0f);
        
        // Night time minimum intensity
        IntensityMultiplier = FMath::Max(IntensityMultiplier, 0.1f);
        
        LightComp->SetIntensity(Settings.SunIntensity * IntensityMultiplier);
        
        // Interpolate sun color based on time
        FLinearColor NightColor = Settings.SunColor * 0.3f; // Darker at night
        FLinearColor CurrentColor = InterpolateDayNightColor(Settings.SunColor, NightColor, IntensityMultiplier);
        LightComp->SetLightColor(CurrentColor);
    }
}

void ALightingController::UpdateAtmosphere()
{
    if (!SkyLight) return;
    
    USkyLightComponent* SkyLightComp = Cast<ASkyLight>(SkyLight)->GetLightComponent();
    if (SkyLightComp && BiomeLightingSettings.Contains(CurrentBiome))
    {
        const FLight_BiomeLightingSettings& Settings = BiomeLightingSettings[CurrentBiome];
        
        // Update sky light color
        float TimeRatio = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(CalculateSunAngle() + 90.0f)), 0.1f, 1.0f);
        FLinearColor NightSkyColor = Settings.SkyColor * 0.2f;
        FLinearColor CurrentSkyColor = InterpolateDayNightColor(Settings.SkyColor, NightSkyColor, TimeRatio);
        
        SkyLightComp->SetLightColor(CurrentSkyColor);
        SkyLightComp->SetIntensity(TimeRatio * 2.0f);
    }
}

void ALightingController::UpdateFog()
{
    if (!HeightFog) return;
    
    UExponentialHeightFogComponent* FogComp = Cast<AExponentialHeightFog>(HeightFog)->GetComponent();
    if (FogComp && BiomeLightingSettings.Contains(CurrentBiome))
    {
        const FLight_BiomeLightingSettings& Settings = BiomeLightingSettings[CurrentBiome];
        
        // Update fog properties
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        
        // Interpolate fog color
        float TimeRatio = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(CalculateSunAngle() + 90.0f)), 0.2f, 1.0f);
        FLinearColor NightFogColor = Settings.FogColor * 0.4f;
        FLinearColor CurrentFogColor = InterpolateDayNightColor(Settings.FogColor, NightFogColor, TimeRatio);
        
        FogComp->SetFogInscatteringColor(CurrentFogColor);
    }
}

float ALightingController::CalculateSunAngle() const
{
    // Convert time of day (0-24) to sun angle (-90 to 90 degrees)
    // 6 AM = -90°, 12 PM = 0°, 6 PM = 90°, 12 AM = -90°
    float NormalizedTime = (CurrentTimeOfDay - 6.0f) / 12.0f; // 0 at 6AM, 1 at 6PM
    return FMath::Lerp(-90.0f, 90.0f, NormalizedTime);
}

FLinearColor ALightingController::InterpolateDayNightColor(const FLinearColor& DayColor, const FLinearColor& NightColor, float TimeRatio)
{
    return FLinearColor::LerpUsingHSV(NightColor, DayColor, TimeRatio);
}