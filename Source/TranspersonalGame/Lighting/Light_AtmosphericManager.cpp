#include "Light_AtmosphericManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default Cretaceous values
    TimeOfDay = 12.0f;
    DayDuration = 1200.0f;
    bEnableDayNightCycle = true;
    
    // Cretaceous atmospheric colors (warmer, more humid atmosphere)
    DawnColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    NoonColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    DuskColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    NightColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    
    FogDensity = 0.02f;
    FogHeightFalloff = 0.2f;
    SunIntensity = 5.0f;
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindOrCreateAtmosphericActors();
    ConfigureCretaceousDefaults();
    SetCretaceousAtmosphere();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle && DayDuration > 0.0f)
    {
        // Advance time of day
        TimeOfDay += (DeltaTime / DayDuration) * 24.0f;
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
        
        UpdateSunPosition();
        UpdateLightingColors();
    }
}

void ALight_AtmosphericManager::FindOrCreateAtmosphericActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing SkyAtmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphereActor = *ActorItr;
        break;
    }
    
    // Find existing DirectionalLight
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }
    
    // Find existing ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        AtmosphericFog = *ActorItr;
        break;
    }
    
    // Create missing actors if needed
    if (!SkyAtmosphereActor)
    {
        SkyAtmosphereActor = World->SpawnActor<ASkyAtmosphere>();
    }
    
    if (!SunLight)
    {
        SunLight = World->SpawnActor<ADirectionalLight>();
        if (SunLight)
        {
            SunLight->SetActorRotation(FRotator(-45.0f, 0.0f, 0.0f));
        }
    }
    
    if (!AtmosphericFog)
    {
        AtmosphericFog = World->SpawnActor<AExponentialHeightFog>();
    }
}

void ALight_AtmosphericManager::ConfigureCretaceousDefaults()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(SunIntensity);
        LightComp->SetLightColor(NoonColor);
        LightComp->SetCastShadows(true);
        LightComp->SetCastVolumetricShadow(true);
    }
    
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(FogDensity);
        FogComp->SetFogHeightFalloff(FogHeightFalloff);
        FogComp->SetFogInscatteringColor(FLinearColor(0.9f, 0.85f, 0.7f));
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.2f);
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateLightingColors();
}

void ALight_AtmosphericManager::SetCretaceousAtmosphere()
{
    ConfigureCretaceousDefaults();
    UpdateSunPosition();
    UpdateLightingColors();
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous atmosphere applied - TimeOfDay: %f"), TimeOfDay);
}

void ALight_AtmosphericManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    float TimeNormalized = TimeOfDay / 24.0f;
    FRotator SunRotation = CalculateSunRotation(TimeNormalized);
    SunLight->SetActorRotation(SunRotation);
}

void ALight_AtmosphericManager::UpdateLightingColors()
{
    if (!SunLight || !SunLight->GetLightComponent()) return;
    
    float TimeNormalized = TimeOfDay / 24.0f;
    FLinearColor CurrentColor = GetInterpolatedSkyColor(TimeNormalized);
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    LightComp->SetLightColor(CurrentColor);
    
    // Adjust intensity based on time of day
    float IntensityMultiplier = 1.0f;
    if (TimeOfDay < 6.0f || TimeOfDay > 18.0f) // Night
    {
        IntensityMultiplier = 0.1f;
    }
    else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f) // Dawn/Dusk
    {
        float BlendFactor = (TimeOfDay < 8.0f) ? (TimeOfDay - 6.0f) / 2.0f : (18.0f - TimeOfDay) / 2.0f;
        IntensityMultiplier = FMath::Lerp(0.1f, 1.0f, BlendFactor);
    }
    
    LightComp->SetIntensity(SunIntensity * IntensityMultiplier);
}

void ALight_AtmosphericManager::ApplyCretaceousPreset()
{
    SetCretaceousAtmosphere();
}

FLinearColor ALight_AtmosphericManager::GetInterpolatedSkyColor(float TimeNormalized)
{
    // Convert time to 0-1 range for easier interpolation
    float Hour = TimeNormalized * 24.0f;
    
    if (Hour >= 6.0f && Hour <= 8.0f) // Dawn
    {
        float BlendFactor = (Hour - 6.0f) / 2.0f;
        return FMath::Lerp(NightColor, DawnColor, BlendFactor);
    }
    else if (Hour >= 8.0f && Hour <= 12.0f) // Morning to Noon
    {
        float BlendFactor = (Hour - 8.0f) / 4.0f;
        return FMath::Lerp(DawnColor, NoonColor, BlendFactor);
    }
    else if (Hour >= 12.0f && Hour <= 16.0f) // Noon to Afternoon
    {
        return NoonColor;
    }
    else if (Hour >= 16.0f && Hour <= 18.0f) // Afternoon to Dusk
    {
        float BlendFactor = (Hour - 16.0f) / 2.0f;
        return FMath::Lerp(NoonColor, DuskColor, BlendFactor);
    }
    else if (Hour >= 18.0f && Hour <= 20.0f) // Dusk to Night
    {
        float BlendFactor = (Hour - 18.0f) / 2.0f;
        return FMath::Lerp(DuskColor, NightColor, BlendFactor);
    }
    else // Night
    {
        return NightColor;
    }
}

FRotator ALight_AtmosphericManager::CalculateSunRotation(float TimeNormalized)
{
    // Sun arc from east to west
    float SunAngle = (TimeNormalized - 0.25f) * 360.0f; // Start at 6 AM (east)
    float Elevation = FMath::Sin(TimeNormalized * PI) * 80.0f - 10.0f; // Arc height
    
    return FRotator(-Elevation, SunAngle, 0.0f);
}