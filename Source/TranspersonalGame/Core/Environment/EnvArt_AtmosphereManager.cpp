#include "EnvArt_AtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
    TimeOfDaySpeed = 1.0f;
    bAutoAdvanceTime = true;
    CurrentTimeValue = 0.0f;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingPresets();
    FindOrCreateLightingActors();
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        CurrentTimeValue += DeltaTime * TimeOfDaySpeed;
        
        // Cycle through time of day every 600 seconds (10 minutes)
        if (CurrentTimeValue >= 600.0f)
        {
            CurrentTimeValue = 0.0f;
            int32 NextTimeIndex = (static_cast<int32>(CurrentTimeOfDay) + 1) % 6;
            CurrentTimeOfDay = static_cast<EEnvArt_TimeOfDay>(NextTimeIndex);
            UpdateAtmosphere();
        }
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    CurrentTimeValue = 0.0f;
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->SetActorRotation(Settings.SunRotation);
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->SetLightColor(Settings.SkyColor);
        SkyLightActor->GetLightComponent()->SetIntensity(Settings.SkyIntensity);
    }
    
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
        HeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        HeightFog->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }
}

void AEnvArt_AtmosphereManager::UpdateAtmosphere()
{
    if (LightingPresets.Contains(CurrentTimeOfDay))
    {
        ApplyLightingSettings(LightingPresets[CurrentTimeOfDay]);
    }
}

void AEnvArt_AtmosphereManager::CreateGoldenHourLighting()
{
    FEnvArt_LightingSettings GoldenHour;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f);
    GoldenHour.SunIntensity = 2.5f;
    GoldenHour.SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
    GoldenHour.SkyColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);
    GoldenHour.SkyIntensity = 0.8f;
    GoldenHour.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    GoldenHour.FogDensity = 0.015f;
    GoldenHour.FogHeightFalloff = 0.15f;
    
    ApplyLightingSettings(GoldenHour);
}

void AEnvArt_AtmosphereManager::AddVolumetricFog()
{
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetVolumetricFog(true);
        HeightFog->GetComponent()->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFog->GetComponent()->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
        HeightFog->GetComponent()->SetVolumetricFogEmissive(FLinearColor(0.1f, 0.1f, 0.05f));
        HeightFog->GetComponent()->SetVolumetricFogExtinctionScale(1.0f);
    }
}

void AEnvArt_AtmosphereManager::InitializeLightingPresets()
{
    // Dawn
    FEnvArt_LightingSettings Dawn;
    Dawn.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    Dawn.SunIntensity = 1.5f;
    Dawn.SunRotation = FRotator(-75.0f, 90.0f, 0.0f);
    Dawn.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f);
    Dawn.SkyIntensity = 0.3f;
    Dawn.FogColor = FLinearColor(0.9f, 0.8f, 0.9f, 1.0f);
    Dawn.FogDensity = 0.03f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dawn, Dawn);
    
    // Morning
    FEnvArt_LightingSettings Morning;
    Morning.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    Morning.SunIntensity = 3.0f;
    Morning.SunRotation = FRotator(-45.0f, 120.0f, 0.0f);
    Morning.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    Morning.SkyIntensity = 0.8f;
    Morning.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    Morning.FogDensity = 0.02f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Morning, Morning);
    
    // Noon
    FEnvArt_LightingSettings Noon;
    Noon.SunColor = FLinearColor::White;
    Noon.SunIntensity = 4.0f;
    Noon.SunRotation = FRotator(-10.0f, 180.0f, 0.0f);
    Noon.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    Noon.SkyIntensity = 1.0f;
    Noon.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    Noon.FogDensity = 0.01f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Noon, Noon);
    
    // Afternoon
    FEnvArt_LightingSettings Afternoon;
    Afternoon.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    Afternoon.SunIntensity = 3.5f;
    Afternoon.SunRotation = FRotator(-30.0f, 240.0f, 0.0f);
    Afternoon.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    Afternoon.SkyIntensity = 0.9f;
    Afternoon.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    Afternoon.FogDensity = 0.015f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Afternoon, Afternoon);
    
    // Dusk
    FEnvArt_LightingSettings Dusk;
    Dusk.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    Dusk.SunIntensity = 2.0f;
    Dusk.SunRotation = FRotator(-70.0f, 270.0f, 0.0f);
    Dusk.SkyColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    Dusk.SkyIntensity = 0.4f;
    Dusk.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    Dusk.FogDensity = 0.025f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dusk, Dusk);
    
    // Night
    FEnvArt_LightingSettings Night;
    Night.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    Night.SunIntensity = 0.1f;
    Night.SunRotation = FRotator(-120.0f, 0.0f, 0.0f);
    Night.SkyColor = FLinearColor(0.05f, 0.05f, 0.2f, 1.0f);
    Night.SkyIntensity = 0.1f;
    Night.FogColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    Night.FogDensity = 0.04f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Night, Night);
}

void AEnvArt_AtmosphereManager::FindOrCreateLightingActors()
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
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

FEnvArt_LightingSettings AEnvArt_AtmosphereManager::InterpolateLightingSettings(EEnvArt_TimeOfDay FromTime, EEnvArt_TimeOfDay ToTime, float Alpha)
{
    FEnvArt_LightingSettings Result;
    
    if (LightingPresets.Contains(FromTime) && LightingPresets.Contains(ToTime))
    {
        const FEnvArt_LightingSettings& From = LightingPresets[FromTime];
        const FEnvArt_LightingSettings& To = LightingPresets[ToTime];
        
        Result.SunColor = FLinearColor::LerpUsingHSV(From.SunColor, To.SunColor, Alpha);
        Result.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
        Result.SunRotation = FMath::Lerp(From.SunRotation, To.SunRotation, Alpha);
        Result.SkyColor = FLinearColor::LerpUsingHSV(From.SkyColor, To.SkyColor, Alpha);
        Result.SkyIntensity = FMath::Lerp(From.SkyIntensity, To.SkyIntensity, Alpha);
        Result.FogColor = FLinearColor::LerpUsingHSV(From.FogColor, To.FogColor, Alpha);
        Result.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
        Result.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    }
    
    return Result;
}