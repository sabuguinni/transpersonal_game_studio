#include "EnvArt_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    InitializePresets();
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndConfigureLighting();
    SetGoldenHourLighting();
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle && DayDurationMinutes > 0.0f)
    {
        TimeAccumulator += DeltaTime;
        float DayDurationSeconds = DayDurationMinutes * 60.0f;
        float TimeProgress = FMath::Fmod(TimeAccumulator, DayDurationSeconds) / DayDurationSeconds;
        
        float TotalMinutes = TimeProgress * 24.0f * 60.0f;
        CurrentTimeOfDay.Hour = FMath::Floor(TotalMinutes / 60.0f);
        CurrentTimeOfDay.Minute = FMath::Fmod(TotalMinutes, 60.0f);
        
        UpdateLightingFromTimeOfDay();
    }
}

void AEnvArt_AtmosphereManager::InitializePresets()
{
    // Dawn (6:00 AM)
    DawnPreset.Hour = 6.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnPreset.SunIntensity = 2.0f;
    DawnPreset.SunAngle = 15.0f;
    DawnPreset.FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    DawnPreset.FogDensity = 0.05f;

    // Noon (12:00 PM)
    NoonPreset.Hour = 12.0f;
    NoonPreset.SunColor = FLinearColor::White;
    NoonPreset.SunIntensity = 4.0f;
    NoonPreset.SunAngle = 75.0f;
    NoonPreset.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    NoonPreset.FogDensity = 0.02f;

    // Dusk (18:00 PM) - Golden Hour
    DuskPreset.Hour = 18.0f;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DuskPreset.SunIntensity = 2.5f;
    DuskPreset.SunAngle = 20.0f;
    DuskPreset.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DuskPreset.FogDensity = 0.04f;

    // Night (0:00 AM)
    NightPreset.Hour = 0.0f;
    NightPreset.SunColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SunAngle = -30.0f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.2f, 0.3f, 1.0f);
    NightPreset.FogDensity = 0.08f;

    CurrentTimeOfDay = DuskPreset; // Start at golden hour
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float Hour, float Minute)
{
    CurrentTimeOfDay.Hour = FMath::Clamp(Hour, 0.0f, 23.0f);
    CurrentTimeOfDay.Minute = FMath::Clamp(Minute, 0.0f, 59.0f);
    UpdateLightingFromTimeOfDay();
}

void AEnvArt_AtmosphereManager::SetGoldenHourLighting()
{
    CurrentTimeOfDay = DuskPreset;
    UpdateLightingFromTimeOfDay();
}

void AEnvArt_AtmosphereManager::SetStormyWeather()
{
    CurrentTimeOfDay.SunColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    CurrentTimeOfDay.SunIntensity = 1.0f;
    CurrentTimeOfDay.FogColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
    CurrentTimeOfDay.FogDensity = 0.1f;
    UpdateLightingFromTimeOfDay();
}

void AEnvArt_AtmosphereManager::SetClearWeather()
{
    CurrentTimeOfDay = NoonPreset;
    UpdateLightingFromTimeOfDay();
}

void AEnvArt_AtmosphereManager::UpdateLightingFromTimeOfDay()
{
    if (!SunLight || !SkyLight || !HeightFog)
    {
        FindAndConfigureLighting();
        return;
    }

    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetLightColor(CurrentTimeOfDay.SunColor);
        SunLight->GetLightComponent()->SetIntensity(CurrentTimeOfDay.SunIntensity);
        
        FRotator SunRotation = FRotator(-CurrentTimeOfDay.SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }

    // Update sky light
    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetLightColor(CurrentTimeOfDay.SunColor * 0.7f);
        SkyLight->GetLightComponent()->SetIntensity(CurrentTimeOfDay.SunIntensity * 0.3f);
        SkyLight->GetLightComponent()->RecaptureSky();
    }

    // Update fog
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(CurrentTimeOfDay.FogColor);
        HeightFog->GetComponent()->SetFogDensity(CurrentTimeOfDay.FogDensity);
        HeightFog->GetComponent()->SetFogHeightFalloff(CurrentTimeOfDay.FogHeightFalloff);
    }
}

void AEnvArt_AtmosphereManager::FindAndConfigureLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find existing lighting actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

FEnvArt_TimeOfDay AEnvArt_AtmosphereManager::InterpolateTimePresets(float NormalizedTime)
{
    FEnvArt_TimeOfDay Result;
    
    // Simple interpolation between presets based on time
    if (NormalizedTime < 0.25f) // Dawn to Noon
    {
        float Alpha = NormalizedTime * 4.0f;
        Result.SunColor = FMath::Lerp(DawnPreset.SunColor, NoonPreset.SunColor, Alpha);
        Result.SunIntensity = FMath::Lerp(DawnPreset.SunIntensity, NoonPreset.SunIntensity, Alpha);
        Result.SunAngle = FMath::Lerp(DawnPreset.SunAngle, NoonPreset.SunAngle, Alpha);
    }
    else if (NormalizedTime < 0.75f) // Noon to Dusk
    {
        float Alpha = (NormalizedTime - 0.25f) * 2.0f;
        Result.SunColor = FMath::Lerp(NoonPreset.SunColor, DuskPreset.SunColor, Alpha);
        Result.SunIntensity = FMath::Lerp(NoonPreset.SunIntensity, DuskPreset.SunIntensity, Alpha);
        Result.SunAngle = FMath::Lerp(NoonPreset.SunAngle, DuskPreset.SunAngle, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (NormalizedTime - 0.75f) * 4.0f;
        Result.SunColor = FMath::Lerp(DuskPreset.SunColor, NightPreset.SunColor, Alpha);
        Result.SunIntensity = FMath::Lerp(DuskPreset.SunIntensity, NightPreset.SunIntensity, Alpha);
        Result.SunAngle = FMath::Lerp(DuskPreset.SunAngle, NightPreset.SunAngle, Alpha);
    }
    
    return Result;
}