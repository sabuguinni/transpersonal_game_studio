#include "Light_AtmosphericSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default atmospheric settings for Cretaceous period
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    AtmosphericSettings.SkyLightIntensity = 1.2f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.FogColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    AtmosphericSettings.ColorSaturation = FVector4(1.1f, 1.0f, 0.9f, 1.0f);
    AtmosphericSettings.ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.0f);
    AtmosphericSettings.ColorGamma = FVector4(0.95f, 1.0f, 1.05f, 1.0f);
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and assign lighting actors in the world
    FindAndAssignLightingActors();
    
    // Apply initial atmospheric settings
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        // Update time progression
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeHours += TimeIncrement;
        
        if (CurrentTimeHours >= 24.0f)
        {
            CurrentTimeHours = 0.0f;
        }
        
        // Determine time of day based on current hours
        ELight_TimeOfDay NewTimeOfDay = CurrentTimeOfDay;
        if (CurrentTimeHours >= 5.0f && CurrentTimeHours < 8.0f)
        {
            NewTimeOfDay = ELight_TimeOfDay::Dawn;
        }
        else if (CurrentTimeHours >= 8.0f && CurrentTimeHours < 12.0f)
        {
            NewTimeOfDay = ELight_TimeOfDay::Morning;
        }
        else if (CurrentTimeHours >= 12.0f && CurrentTimeHours < 15.0f)
        {
            NewTimeOfDay = ELight_TimeOfDay::Midday;
        }
        else if (CurrentTimeHours >= 15.0f && CurrentTimeHours < 18.0f)
        {
            NewTimeOfDay = ELight_TimeOfDay::Afternoon;
        }
        else if (CurrentTimeHours >= 18.0f && CurrentTimeHours < 21.0f)
        {
            NewTimeOfDay = ELight_TimeOfDay::Dusk;
        }
        else
        {
            NewTimeOfDay = ELight_TimeOfDay::Night;
        }
        
        if (NewTimeOfDay != CurrentTimeOfDay)
        {
            CurrentTimeOfDay = NewTimeOfDay;
            UpdateAtmosphericLighting();
        }
        
        // Continuous updates for smooth transitions
        UpdateSunPosition();
        UpdateSunIntensity();
    }
}

void ALight_AtmosphericSystem::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Set corresponding time hours
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            CurrentTimeHours = 6.0f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentTimeHours = 9.0f;
            break;
        case ELight_TimeOfDay::Midday:
            CurrentTimeHours = 12.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentTimeHours = 15.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentTimeHours = 19.0f;
            break;
        case ELight_TimeOfDay::Night:
            CurrentTimeHours = 23.0f;
            break;
    }
    
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericSystem::SetWeatherType(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericSystem::UpdateAtmosphericLighting()
{
    // Get settings for current time and weather
    FLight_AtmosphericSettings CurrentSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, CurrentWeather);
    
    // Apply the settings
    ApplyAtmosphericSettings(CurrentSettings);
    
    // Update individual components
    UpdateSunPosition();
    UpdateSunIntensity();
    UpdateSkyLighting();
    UpdateFogSettings();
    UpdatePostProcessing();
    UpdateStructureLighting();
}

void ALight_AtmosphericSystem::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    AtmosphericSettings = Settings;
    
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->SetIntensity(Settings.SkyLightIntensity);
    }
    
    if (FogActor && FogActor->GetComponent())
    {
        FogActor->GetComponent()->SetFogDensity(Settings.FogDensity);
        FogActor->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogActor->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_AtmosphericSystem::UpdateStructureLighting()
{
    // Find all primitive structures in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Arch_PrimitiveStructure")))
        {
            // Find associated structure lights
            FVector StructureLocation = Actor->GetActorLocation();
            TArray<AActor*> NearbyLights;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), APointLight::StaticClass(), NearbyLights);
            
            for (AActor* LightActor : NearbyLights)
            {
                if (LightActor && FVector::Dist(LightActor->GetActorLocation(), StructureLocation) < 1000.0f)
                {
                    // Update interior lighting based on time of day
                    if (APointLight* PointLight = Cast<APointLight>(LightActor))
                    {
                        if (UPointLightComponent* LightComp = PointLight->GetPointLightComponent())
                        {
                            float InteriorIntensity = 300.0f;
                            FLinearColor InteriorColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);
                            
                            // Adjust based on time of day
                            switch (CurrentTimeOfDay)
                            {
                                case ELight_TimeOfDay::Dawn:
                                case ELight_TimeOfDay::Dusk:
                                    InteriorIntensity = 400.0f;
                                    InteriorColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
                                    break;
                                case ELight_TimeOfDay::Night:
                                    InteriorIntensity = 500.0f;
                                    InteriorColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
                                    break;
                                default:
                                    break;
                            }
                            
                            LightComp->SetIntensity(InteriorIntensity);
                            LightComp->SetLightColor(InteriorColor);
                        }
                    }
                }
            }
        }
    }
}

void ALight_AtmosphericSystem::FindAndAssignLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find DirectionalLight
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }
        
        // Find SkyLight
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
        }
        
        // Find ExponentialHeightFog
        TArray<AActor*> FogActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
        if (FogActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
        }
        
        // Find PostProcessVolume
        TArray<AActor*> PostProcessVolumes;
        UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
        if (PostProcessVolumes.Num() > 0)
        {
            PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
        }
    }
}

void ALight_AtmosphericSystem::CreateAtmosphericLightingSetup()
{
    if (UWorld* World = GetWorld())
    {
        // Create DirectionalLight if not exists
        if (!SunLight)
        {
            SunLight = World->SpawnActor<ADirectionalLight>();
            if (SunLight)
            {
                SunLight->SetActorLocation(FVector(0, 0, 1000));
                SunLight->SetActorRotation(FRotator(-45, 0, 0));
            }
        }
        
        // Create SkyLight if not exists
        if (!SkyLightActor)
        {
            SkyLightActor = World->SpawnActor<ASkyLight>();
            if (SkyLightActor)
            {
                SkyLightActor->SetActorLocation(FVector(0, 0, 500));
            }
        }
        
        // Create ExponentialHeightFog if not exists
        if (!FogActor)
        {
            FogActor = World->SpawnActor<AExponentialHeightFog>();
            if (FogActor)
            {
                FogActor->SetActorLocation(FVector(0, 0, 0));
            }
        }
        
        // Create PostProcessVolume if not exists
        if (!PostProcessVolume)
        {
            PostProcessVolume = World->SpawnActor<APostProcessVolume>();
            if (PostProcessVolume)
            {
                PostProcessVolume->SetActorLocation(FVector(0, 0, 0));
                PostProcessVolume->SetActorScale3D(FVector(100, 100, 100));
                PostProcessVolume->bUnbound = true;
            }
        }
        
        UpdateAtmosphericLighting();
    }
}

void ALight_AtmosphericSystem::UpdateSunPosition()
{
    if (SunLight)
    {
        // Calculate sun position based on time of day
        float SunAngle = (CurrentTimeHours / 24.0f) * 360.0f - 90.0f; // -90 to start at dawn
        float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 60.0f - 30.0f;
        float SunAzimuth = SunAngle;
        
        FRotator SunRotation = FRotator(SunElevation, SunAzimuth, 0);
        SunLight->SetActorRotation(SunRotation);
    }
}

void ALight_AtmosphericSystem::UpdateSunIntensity()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        float BaseIntensity = AtmosphericSettings.SunIntensity;
        float TimeMultiplier = 1.0f;
        
        // Adjust intensity based on time of day
        switch (CurrentTimeOfDay)
        {
            case ELight_TimeOfDay::Dawn:
            case ELight_TimeOfDay::Dusk:
                TimeMultiplier = 0.6f;
                break;
            case ELight_TimeOfDay::Night:
                TimeMultiplier = 0.1f;
                break;
            case ELight_TimeOfDay::Morning:
            case ELight_TimeOfDay::Afternoon:
                TimeMultiplier = 0.8f;
                break;
            default:
                TimeMultiplier = 1.0f;
                break;
        }
        
        SunLight->GetLightComponent()->SetIntensity(BaseIntensity * TimeMultiplier);
    }
}

void ALight_AtmosphericSystem::UpdateSkyLighting()
{
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        float BaseIntensity = AtmosphericSettings.SkyLightIntensity;
        float WeatherMultiplier = 1.0f;
        
        // Adjust based on weather
        switch (CurrentWeather)
        {
            case ELight_WeatherType::Cloudy:
                WeatherMultiplier = 0.8f;
                break;
            case ELight_WeatherType::Overcast:
                WeatherMultiplier = 0.6f;
                break;
            case ELight_WeatherType::Foggy:
                WeatherMultiplier = 0.4f;
                break;
            case ELight_WeatherType::Stormy:
                WeatherMultiplier = 0.3f;
                break;
            default:
                WeatherMultiplier = 1.0f;
                break;
        }
        
        SkyLightActor->GetLightComponent()->SetIntensity(BaseIntensity * WeatherMultiplier);
    }
}

void ALight_AtmosphericSystem::UpdateFogSettings()
{
    if (FogActor && FogActor->GetComponent())
    {
        float BaseDensity = AtmosphericSettings.FogDensity;
        float WeatherMultiplier = 1.0f;
        
        // Adjust fog based on weather
        switch (CurrentWeather)
        {
            case ELight_WeatherType::Foggy:
                WeatherMultiplier = 3.0f;
                break;
            case ELight_WeatherType::Overcast:
                WeatherMultiplier = 1.5f;
                break;
            case ELight_WeatherType::Stormy:
                WeatherMultiplier = 2.0f;
                break;
            default:
                WeatherMultiplier = 1.0f;
                break;
        }
        
        FogActor->GetComponent()->SetFogDensity(BaseDensity * WeatherMultiplier);
    }
}

void ALight_AtmosphericSystem::UpdatePostProcessing()
{
    if (PostProcessVolume)
    {
        if (UPostProcessComponent* PPComponent = PostProcessVolume->GetComponent())
        {
            // Apply color grading based on current settings
            // Note: Specific post-process property setting would require access to the settings struct
            // This is a simplified implementation
        }
    }
}

FLight_AtmosphericSettings ALight_AtmosphericSystem::GetSettingsForTimeAndWeather(ELight_TimeOfDay TimeOfDay, ELight_WeatherType Weather)
{
    FLight_AtmosphericSettings Settings = AtmosphericSettings;
    
    // Adjust settings based on time of day
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            Settings.SunIntensity = 3.0f;
            break;
        case ELight_TimeOfDay::Morning:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunIntensity = 4.0f;
            break;
        case ELight_TimeOfDay::Midday:
            Settings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
            Settings.SunIntensity = 5.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunIntensity = 4.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
            Settings.SunIntensity = 2.0f;
            break;
        case ELight_TimeOfDay::Night:
            Settings.SunColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
            Settings.SunIntensity = 0.5f;
            break;
    }
    
    // Adjust settings based on weather
    switch (Weather)
    {
        case ELight_WeatherType::Cloudy:
            Settings.SkyLightIntensity *= 0.8f;
            Settings.FogDensity *= 1.2f;
            break;
        case ELight_WeatherType::Overcast:
            Settings.SkyLightIntensity *= 0.6f;
            Settings.FogDensity *= 1.5f;
            Settings.ColorSaturation *= 0.9f;
            break;
        case ELight_WeatherType::Foggy:
            Settings.SkyLightIntensity *= 0.4f;
            Settings.FogDensity *= 3.0f;
            break;
        case ELight_WeatherType::Stormy:
            Settings.SkyLightIntensity *= 0.3f;
            Settings.FogDensity *= 2.0f;
            Settings.ColorSaturation *= 0.8f;
            Settings.ColorContrast *= 1.2f;
            break;
        default:
            break;
    }
    
    return Settings;
}