#include "Light_AtmosphericSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    HeightFog = nullptr;
    
    // Set default Cretaceous period settings
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FColor(255, 240, 200, 255);
    AtmosphericSettings.SunTemperature = 5500.0f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.FogInscatteringColor = FColor(180, 200, 255, 255);
    AtmosphericSettings.BloomIntensity = 0.675f;
    AtmosphericSettings.ExposureCompensation = 0.5f;
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    FindExistingComponents();
    UpdateAtmosphericSettings(AtmosphericSettings);
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        TimeAccumulator += DeltaTime;
        float CycleDurationSeconds = DayDurationMinutes * 60.0f;
        CurrentTimeOfDayFloat = FMath::Fmod(TimeAccumulator / CycleDurationSeconds, 1.0f);
        
        // Update time of day enum based on float value
        if (CurrentTimeOfDayFloat < 0.1f)
            CurrentTimeOfDay = ELight_TimeOfDay::Night;
        else if (CurrentTimeOfDayFloat < 0.2f)
            CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
        else if (CurrentTimeOfDayFloat < 0.4f)
            CurrentTimeOfDay = ELight_TimeOfDay::Morning;
        else if (CurrentTimeOfDayFloat < 0.6f)
            CurrentTimeOfDay = ELight_TimeOfDay::Midday;
        else if (CurrentTimeOfDayFloat < 0.8f)
            CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
        else if (CurrentTimeOfDayFloat < 0.9f)
            CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
        else
            CurrentTimeOfDay = ELight_TimeOfDay::Night;
        
        UpdateSunPosition();
    }
}

void ALight_AtmosphericSystem::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Convert enum to float for calculations
    switch (NewTimeOfDay)
    {
        case ELight_TimeOfDay::Night:
            CurrentTimeOfDayFloat = 0.05f;
            break;
        case ELight_TimeOfDay::Dawn:
            CurrentTimeOfDayFloat = 0.15f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentTimeOfDayFloat = 0.3f;
            break;
        case ELight_TimeOfDay::Midday:
            CurrentTimeOfDayFloat = 0.5f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentTimeOfDayFloat = 0.7f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentTimeOfDayFloat = 0.85f;
            break;
    }
    
    UpdateSunPosition();
}

void ALight_AtmosphericSystem::UpdateAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings)
{
    AtmosphericSettings = NewSettings;
    
    if (SunLight)
    {
        SunLight->SetIntensity(AtmosphericSettings.SunIntensity);
        SunLight->SetLightColor(FLinearColor(AtmosphericSettings.SunColor));
        SunLight->SetTemperature(AtmosphericSettings.SunTemperature);
        SunLight->SetCastShadows(true);
        SunLight->SetCastVolumetricShadow(true);
    }
    
    UpdateFogSettings();
    UpdatePostProcessSettings();
}

void ALight_AtmosphericSystem::SetSunAngle(float Elevation, float Azimuth)
{
    if (SunLight)
    {
        FRotator SunRotation;
        SunRotation.Pitch = -Elevation; // Negative because UE5 pitch is inverted
        SunRotation.Yaw = Azimuth;
        SunRotation.Roll = 0.0f;
        
        GetOwner()->SetActorRotation(SunRotation);
    }
}

void ALight_AtmosphericSystem::EnableVolumetricFog(bool bEnable)
{
    bEnableVolumetricFog = bEnable;
    
    if (HeightFog)
    {
        HeightFog->SetVolumetricFog(bEnableVolumetricFog);
    }
}

FLight_AtmosphericSettings ALight_AtmosphericSystem::GetCurrentSettings() const
{
    return AtmosphericSettings;
}

void ALight_AtmosphericSystem::UpdateSunPosition()
{
    if (!SunLight)
        return;
    
    // Calculate sun elevation based on time of day
    // Peak at midday (0.5), lowest at midnight (0.0 and 1.0)
    float SunElevation;
    if (CurrentTimeOfDayFloat <= 0.5f)
    {
        // Morning: rise from -90 to 90 degrees
        SunElevation = FMath::Lerp(-90.0f, 90.0f, CurrentTimeOfDayFloat * 2.0f);
    }
    else
    {
        // Evening: fall from 90 to -90 degrees
        SunElevation = FMath::Lerp(90.0f, -90.0f, (CurrentTimeOfDayFloat - 0.5f) * 2.0f);
    }
    
    // Sun moves from east (90°) to west (270°)
    float SunAzimuth = FMath::Lerp(90.0f, 270.0f, CurrentTimeOfDayFloat);
    
    SetSunAngle(SunElevation, SunAzimuth);
    
    // Adjust sun intensity based on elevation
    float IntensityMultiplier = FMath::Max(0.1f, FMath::Sin(FMath::DegreesToRadians(FMath::Max(0.0f, SunElevation))));
    if (SunLight)
    {
        SunLight->SetIntensity(AtmosphericSettings.SunIntensity * IntensityMultiplier);
    }
}

void ALight_AtmosphericSystem::UpdateFogSettings()
{
    if (HeightFog)
    {
        HeightFog->SetFogDensity(AtmosphericSettings.FogDensity);
        HeightFog->SetFogHeightFalloff(AtmosphericSettings.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(FLinearColor(AtmosphericSettings.FogInscatteringColor));
        HeightFog->SetVolumetricFog(bEnableVolumetricFog);
    }
}

void ALight_AtmosphericSystem::UpdatePostProcessSettings()
{
    // Find post process volume in the world
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    
    for (AActor* Actor : PostProcessVolumes)
    {
        APostProcessVolume* PPVolume = Cast<APostProcessVolume>(Actor);
        if (PPVolume && PPVolume->bUnbound)
        {
            FPostProcessSettings& Settings = PPVolume->Settings;
            
            // Enable Lumen if requested
            if (bEnableLumenGI)
            {
                Settings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
                Settings.ReflectionMethod = EReflectionMethod::Lumen;
            }
            
            // Apply atmospheric post process settings
            Settings.BloomIntensity = AtmosphericSettings.BloomIntensity;
            Settings.BloomThreshold = 1.0f;
            Settings.AutoExposureBias = AtmosphericSettings.ExposureCompensation;
            Settings.ColorGradingIntensity = 1.0f;
            
            // Mark settings as overridden
            Settings.bOverride_DynamicGlobalIlluminationMethod = true;
            Settings.bOverride_ReflectionMethod = true;
            Settings.bOverride_BloomIntensity = true;
            Settings.bOverride_BloomThreshold = true;
            Settings.bOverride_AutoExposureBias = true;
            Settings.bOverride_ColorGradingIntensity = true;
        }
    }
}

void ALight_AtmosphericSystem::InitializeComponents()
{
    // Components will be found from existing world actors
    // This system manages existing lighting components rather than creating new ones
}

void ALight_AtmosphericSystem::FindExistingComponents()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find DirectionalLight in the world
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        if (DirLight)
        {
            SunLight = DirLight->GetLightComponent();
        }
    }
    
    // Find SkyAtmosphere component
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyActors);
    if (SkyActors.Num() > 0)
    {
        ASkyAtmosphere* SkyActor = Cast<ASkyAtmosphere>(SkyActors[0]);
        if (SkyActor)
        {
            SkyAtmosphere = SkyActor->GetComponent<USkyAtmosphereComponent>();
        }
    }
    
    // Find ExponentialHeightFog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AExponentialHeightFog* FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
        if (FogActor)
        {
            HeightFog = FogActor->GetComponent();
        }
    }
}