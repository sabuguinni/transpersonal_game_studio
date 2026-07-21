#include "EnvArt_AtmosphereController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AEnvArt_AtmosphereController::AEnvArt_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize atmosphere presets
    InitializeAtmospherePresets();
}

void AEnvArt_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing light actors in the scene
    FindLightActors();
    
    // If no lights found, create default lighting
    if (!SunLight || !SkyLightActor || !FogActor)
    {
        CreateDefaultLighting();
    }
    
    // Apply initial atmosphere settings
    SetTimeOfDay(CurrentTimeOfDay);
}

void AEnvArt_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        // Advance time
        CurrentTime += (DeltaTime * 24.0f) / DayDuration; // Convert real time to game hours
        
        if (CurrentTime >= 24.0f)
        {
            CurrentTime -= 24.0f;
        }
        
        UpdateTimeOfDay();
    }
}

void AEnvArt_AtmosphereController::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    FEnvArt_AtmosphereSettings TargetSettings;
    
    switch (CurrentTimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            TargetSettings = DawnSettings;
            CurrentTime = 6.0f;
            break;
        case EEnvArt_TimeOfDay::Morning:
            TargetSettings = MorningSettings;
            CurrentTime = 8.0f;
            break;
        case EEnvArt_TimeOfDay::Noon:
            TargetSettings = NoonSettings;
            CurrentTime = 12.0f;
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            TargetSettings = AfternoonSettings;
            CurrentTime = 16.0f;
            break;
        case EEnvArt_TimeOfDay::Dusk:
            TargetSettings = DuskSettings;
            CurrentTime = 18.0f;
            break;
        case EEnvArt_TimeOfDay::Night:
            TargetSettings = NightSettings;
            CurrentTime = 22.0f;
            break;
    }
    
    ApplyAtmosphereSettings(TargetSettings);
}

void AEnvArt_AtmosphereController::SetTime(float NewTime)
{
    CurrentTime = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateTimeOfDay();
}

void AEnvArt_AtmosphereController::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLight = SunLight->GetLightComponent();
        DirLight->SetLightColor(Settings.SunColor);
        DirLight->SetIntensity(Settings.SunIntensity);
        SunLight->SetActorRotation(Settings.SunRotation);
    }
    
    // Apply sky light settings
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        SkyComp->SetLightColor(Settings.SkyColor);
        SkyComp->SetIntensity(Settings.SkyIntensity);
        SkyComp->RecaptureSky();
    }
    
    // Apply fog settings
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogInscatteringColor(Settings.FogColor);
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetStartDistance(Settings.FogStartDistance);
    }
}

FEnvArt_AtmosphereSettings AEnvArt_AtmosphereController::GetCurrentAtmosphereSettings() const
{
    FEnvArt_AtmosphereSettings CurrentSettings;
    
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLight = SunLight->GetLightComponent();
        CurrentSettings.SunColor = DirLight->GetLightColor();
        CurrentSettings.SunIntensity = DirLight->Intensity;
        CurrentSettings.SunRotation = SunLight->GetActorRotation();
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        CurrentSettings.SkyColor = SkyComp->GetLightColor();
        CurrentSettings.SkyIntensity = SkyComp->Intensity;
    }
    
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        CurrentSettings.FogColor = FogComp->FogInscatteringColor;
        CurrentSettings.FogDensity = FogComp->FogDensity;
        CurrentSettings.FogHeightFalloff = FogComp->FogHeightFalloff;
        CurrentSettings.FogStartDistance = FogComp->StartDistance;
    }
    
    return CurrentSettings;
}

void AEnvArt_AtmosphereController::FindLightActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }
        
        // Find sky light
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
        }
        
        // Find fog actor
        TArray<AActor*> FogActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
        if (FogActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
        }
    }
}

void AEnvArt_AtmosphereController::CreateDefaultLighting()
{
    if (UWorld* World = GetWorld())
    {
        // Create directional light if not found
        if (!SunLight)
        {
            FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 1000);
            FRotator SpawnRotation = FRotator(-45.0f, 0.0f, 0.0f);
            SunLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SpawnLocation, SpawnRotation);
            if (SunLight)
            {
                SunLight->SetActorLabel(TEXT("Atmosphere_SunLight"));
            }
        }
        
        // Create sky light if not found
        if (!SkyLightActor)
        {
            FVector SpawnLocation = GetActorLocation();
            SkyLightActor = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            if (SkyLightActor)
            {
                SkyLightActor->SetActorLabel(TEXT("Atmosphere_SkyLight"));
            }
        }
        
        // Create fog actor if not found
        if (!FogActor)
        {
            FVector SpawnLocation = GetActorLocation();
            FogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            if (FogActor)
            {
                FogActor->SetActorLabel(TEXT("Atmosphere_HeightFog"));
            }
        }
    }
}

void AEnvArt_AtmosphereController::UpdateTimeOfDay()
{
    // Determine current time of day based on time
    EEnvArt_TimeOfDay NewTimeOfDay;
    
    if (CurrentTime >= 5.0f && CurrentTime < 7.0f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Dawn;
    }
    else if (CurrentTime >= 7.0f && CurrentTime < 11.0f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Morning;
    }
    else if (CurrentTime >= 11.0f && CurrentTime < 14.0f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Noon;
    }
    else if (CurrentTime >= 14.0f && CurrentTime < 17.0f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    }
    else if (CurrentTime >= 17.0f && CurrentTime < 20.0f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Dusk;
    }
    else
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Night;
    }
    
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
    }
    
    // Calculate sun rotation based on time
    FRotator SunRotation = CalculateSunRotation(CurrentTime);
    
    // Get current settings and interpolate
    FEnvArt_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
    CurrentSettings.SunRotation = SunRotation;
    
    ApplyAtmosphereSettings(CurrentSettings);
}

FRotator AEnvArt_AtmosphereController::CalculateSunRotation(float TimeHours) const
{
    // Sun moves 15 degrees per hour (360 degrees / 24 hours)
    float SunAngle = (TimeHours - 6.0f) * 15.0f; // 6 AM = 0 degrees (sunrise)
    
    // Convert to pitch (vertical angle)
    float Pitch = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    
    // Clamp pitch to reasonable values
    Pitch = FMath::Clamp(Pitch, -90.0f, 90.0f);
    
    return FRotator(Pitch, SunAngle, 0.0f);
}

void AEnvArt_AtmosphereController::InitializeAtmospherePresets()
{
    // Dawn settings (5-7 AM) - soft orange/pink light
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunRotation = FRotator(-10.0f, 45.0f, 0.0f);
    DawnSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DawnSettings.SkyIntensity = 0.5f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogHeightFalloff = 0.1f;
    
    // Morning settings (7-11 AM) - warm golden light
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunRotation = FRotator(-30.0f, 60.0f, 0.0f);
    MorningSettings.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    MorningSettings.SkyIntensity = 0.8f;
    MorningSettings.FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogHeightFalloff = 0.2f;
    
    // Noon settings (11 AM-2 PM) - bright white light
    NoonSettings.SunColor = FLinearColor::White;
    NoonSettings.SunIntensity = 6.0f;
    NoonSettings.SunRotation = FRotator(-80.0f, 90.0f, 0.0f);
    NoonSettings.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    NoonSettings.SkyIntensity = 1.0f;
    NoonSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogHeightFalloff = 0.3f;
    
    // Afternoon settings (2-5 PM) - warm white light
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AfternoonSettings.SunIntensity = 5.0f;
    AfternoonSettings.SunRotation = FRotator(-50.0f, 120.0f, 0.0f);
    AfternoonSettings.SkyColor = FLinearColor(0.4f, 0.6f, 0.9f, 1.0f);
    AfternoonSettings.SkyIntensity = 0.9f;
    AfternoonSettings.FogColor = FLinearColor(0.85f, 0.9f, 1.0f, 1.0f);
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogHeightFalloff = 0.25f;
    
    // Dusk settings (5-8 PM) - orange/red light
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunRotation = FRotator(-20.0f, 150.0f, 0.0f);
    DuskSettings.SkyColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SkyIntensity = 0.6f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogHeightFalloff = 0.15f;
    
    // Night settings (8 PM-5 AM) - cool blue moonlight
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunRotation = FRotator(10.0f, 180.0f, 0.0f);
    NightSettings.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.SkyIntensity = 0.3f;
    NightSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogHeightFalloff = 0.1f;
}