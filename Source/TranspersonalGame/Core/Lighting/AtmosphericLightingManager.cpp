#include "AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Math/UnrealMathUtility.h"

UAtmosphericLightingManager::UAtmosphericLightingManager()
{
    // Initialize default Cretaceous period settings
    TimeSettings.CurrentTimeOfDay = 12.0f; // Noon
    TimeSettings.DayDurationMinutes = 30.0f; // 30 real minutes = 1 game day
    TimeSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm tropical sun
    TimeSettings.SunIntensity = 5.0f;
    TimeSettings.AtmosphereScale = 0.0331f;
    TimeSettings.FogDensity = 0.02f;
    TimeSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    WeatherState.CurrentWeather = EWeatherType::Clear;
    WeatherState.CloudCoverage = 0.0f;
    WeatherState.RainIntensity = 0.0f;
    WeatherState.WindStrength = 0.5f;
    WeatherState.Temperature = 28.0f; // Tropical Cretaceous climate
}

void UAtmosphericLightingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: Initializing"));
    
    // Cache lighting components
    CacheLightingComponents();
    
    // Apply Cretaceous preset immediately
    ApplyCretaceousPreset();
    
    // Start time update timer
    if (UWorld* World = GetWorld())
    {
        float UpdateInterval = 1.0f; // Update every second
        World->GetTimerManager().SetTimer(TimeUpdateTimer, this, &UAtmosphericLightingManager::UpdateTimeOfDay, UpdateInterval, true);
    }
}

void UAtmosphericLightingManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimeUpdateTimer);
        World->GetTimerManager().ClearTimer(WeatherUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UAtmosphericLightingManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UAtmosphericLightingManager::SetTimeOfDay(float NewTime)
{
    TimeSettings.CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (TimeSettings.CurrentTimeOfDay < 0.0f)
    {
        TimeSettings.CurrentTimeOfDay += 24.0f;
    }
    
    UpdateSunPosition();
    UpdateAtmosphere();
    UpdateFog();
}

void UAtmosphericLightingManager::SetDayDuration(float Minutes)
{
    TimeSettings.DayDurationMinutes = FMath::Max(1.0f, Minutes);
}

void UAtmosphericLightingManager::SetWeather(EWeatherType NewWeather, float TransitionTime)
{
    if (NewWeather != WeatherState.CurrentWeather)
    {
        TargetWeather = NewWeather;
        WeatherTransitionProgress = 0.0f;
        
        if (UWorld* World = GetWorld())
        {
            float UpdateRate = 0.1f; // 10 updates per second
            World->GetTimerManager().SetTimer(WeatherUpdateTimer, this, &UAtmosphericLightingManager::UpdateWeatherTransition, UpdateRate, true);
        }
    }
}

void UAtmosphericLightingManager::SetCloudCoverage(float Coverage)
{
    WeatherState.CloudCoverage = FMath::Clamp(Coverage, 0.0f, 1.0f);
    
    if (VolumetricClouds && VolumetricClouds->GetVolumetricCloudComponent())
    {
        // Show/hide clouds based on coverage
        VolumetricClouds->SetActorHiddenInGame(Coverage < 0.1f);
    }
}

void UAtmosphericLightingManager::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    FRotator SunRotation = CalculateSunRotation(TimeSettings.CurrentTimeOfDay);
    SunLight->SetActorRotation(SunRotation);
    
    // Update sun color and intensity based on time
    FLinearColor SunColor = CalculateSunColor(TimeSettings.CurrentTimeOfDay);
    float SunIntensity = CalculateSunIntensity(TimeSettings.CurrentTimeOfDay);
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    LightComp->SetLightColor(SunColor);
    LightComp->SetIntensity(SunIntensity);
}

void UAtmosphericLightingManager::UpdateAtmosphere()
{
    if (!SkyAtmosphere)
        return;
    
    // Adjust atmosphere based on time of day and weather
    if (USkyAtmosphereComponent* AtmosphereComp = SkyAtmosphere->GetAtmosphereComponent())
    {
        AtmosphereComp->SetRayleighScatteringScale(TimeSettings.AtmosphereScale);
        
        // Adjust for weather
        if (WeatherState.CurrentWeather == EWeatherType::Storm)
        {
            AtmosphereComp->SetRayleighScatteringScale(TimeSettings.AtmosphereScale * 1.5f);
        }
    }
}

void UAtmosphericLightingManager::UpdateFog()
{
    if (!HeightFog || !HeightFog->GetComponent())
        return;
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    
    // Base fog settings
    FogComp->SetFogDensity(TimeSettings.FogDensity);
    FogComp->SetFogInscatteringColor(TimeSettings.FogColor);
    
    // Adjust for weather
    float WeatherFogMultiplier = 1.0f;
    switch (WeatherState.CurrentWeather)
    {
        case EWeatherType::Fog:
            WeatherFogMultiplier = 3.0f;
            break;
        case EWeatherType::Rain:
            WeatherFogMultiplier = 1.5f;
            break;
        case EWeatherType::Storm:
            WeatherFogMultiplier = 2.0f;
            break;
        default:
            break;
    }
    
    FogComp->SetFogDensity(TimeSettings.FogDensity * WeatherFogMultiplier);
}

void UAtmosphericLightingManager::ApplyCretaceousPreset()
{
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: Applying Cretaceous preset"));
    
    // Cretaceous period was warm and tropical
    TimeSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm golden sun
    TimeSettings.SunIntensity = 5.0f; // Bright tropical sun
    TimeSettings.AtmosphereScale = 0.0331f; // Clear tropical atmosphere
    TimeSettings.FogDensity = 0.02f; // Light atmospheric haze
    
    WeatherState.Temperature = 28.0f; // Warm tropical climate
    WeatherState.CloudCoverage = 0.2f; // Some scattered clouds
    
    ApplyTropicalLighting();
}

void UAtmosphericLightingManager::ApplyTropicalLighting()
{
    CacheLightingComponents();
    
    // Apply to directional light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(TimeSettings.SunColor);
        LightComp->SetIntensity(TimeSettings.SunIntensity);
    }
    
    // Apply to sky atmosphere
    if (SkyAtmosphere && SkyAtmosphere->GetAtmosphereComponent())
    {
        SkyAtmosphere->GetAtmosphereComponent()->SetRayleighScatteringScale(TimeSettings.AtmosphereScale);
    }
    
    // Hide volumetric clouds for clear tropical sky
    if (VolumetricClouds)
    {
        VolumetricClouds->SetActorHiddenInGame(true);
    }
    
    UpdateFog();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: Tropical lighting applied"));
}

void UAtmosphericLightingManager::FixAtmosphericLighting()
{
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: Executing atmospheric lighting fix"));
    
    CacheLightingComponents();
    
    // Force fix all lighting components
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(5.0f);
        LightComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
        UE_LOG(LogTemp, Warning, TEXT("Fixed DirectionalLight"));
    }
    
    if (SkyAtmosphere && SkyAtmosphere->GetAtmosphereComponent())
    {
        SkyAtmosphere->GetAtmosphereComponent()->SetRayleighScatteringScale(0.0331f);
        UE_LOG(LogTemp, Warning, TEXT("Fixed SkyAtmosphere"));
    }
    
    if (VolumetricClouds)
    {
        VolumetricClouds->SetActorHiddenInGame(true);
        UE_LOG(LogTemp, Warning, TEXT("Hid VolumetricClouds"));
    }
    
    // Save level
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SaveGameToSlot(TEXT("AtmosphericFix"), 0);
    }
}

void UAtmosphericLightingManager::ValidateLightingSetup()
{
    CacheLightingComponents();
    
    int32 ValidComponents = 0;
    
    if (SunLight)
    {
        ValidComponents++;
        UE_LOG(LogTemp, Warning, TEXT("DirectionalLight found and cached"));
    }
    
    if (SkyAtmosphere)
    {
        ValidComponents++;
        UE_LOG(LogTemp, Warning, TEXT("SkyAtmosphere found and cached"));
    }
    
    if (VolumetricClouds)
    {
        ValidComponents++;
        UE_LOG(LogTemp, Warning, TEXT("VolumetricClouds found and cached"));
    }
    
    if (HeightFog)
    {
        ValidComponents++;
        UE_LOG(LogTemp, Warning, TEXT("ExponentialHeightFog found and cached"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: %d/4 lighting components validated"), ValidComponents);
}

void UAtmosphericLightingManager::UpdateTimeOfDay()
{
    if (!bTimeSystemActive)
        return;
    
    // Calculate time increment based on day duration
    float TimeIncrement = (24.0f / (TimeSettings.DayDurationMinutes * 60.0f)); // Hours per second
    
    TimeSettings.CurrentTimeOfDay += TimeIncrement;
    if (TimeSettings.CurrentTimeOfDay >= 24.0f)
    {
        TimeSettings.CurrentTimeOfDay -= 24.0f;
    }
    
    UpdateSunPosition();
    UpdateAtmosphere();
}

void UAtmosphericLightingManager::UpdateWeatherTransition()
{
    WeatherTransitionProgress += 0.1f; // 10% per update
    
    if (WeatherTransitionProgress >= 1.0f)
    {
        WeatherState.CurrentWeather = TargetWeather;
        WeatherTransitionProgress = 1.0f;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(WeatherUpdateTimer);
        }
    }
    
    // Apply weather effects based on transition progress
    UpdateAtmosphere();
    UpdateFog();
}

void UAtmosphericLightingManager::CacheLightingComponents()
{
    if (bComponentsCached)
        return;
    
    FindDirectionalLight();
    FindSkyAtmosphere();
    FindVolumetricClouds();
    FindExponentialFog();
    
    bComponentsCached = true;
}

void UAtmosphericLightingManager::FindDirectionalLight()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }
}

void UAtmosphericLightingManager::FindSkyAtmosphere()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
        }
    }
}

void UAtmosphericLightingManager::FindVolumetricClouds()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            VolumetricClouds = Cast<AVolumetricCloud>(FoundActors[0]);
        }
    }
}

void UAtmosphericLightingManager::FindExponentialFog()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

FRotator UAtmosphericLightingManager::CalculateSunRotation(float TimeOfDay) const
{
    // Convert time of day to sun angle
    // 6 AM = sunrise (-90 degrees), 12 PM = noon (0 degrees), 6 PM = sunset (90 degrees)
    float SunAngle = (TimeOfDay - 12.0f) * 15.0f; // 15 degrees per hour
    
    return FRotator(SunAngle, 0.0f, 0.0f);
}

FLinearColor UAtmosphericLightingManager::CalculateSunColor(float TimeOfDay) const
{
    // Warm tropical sun color throughout the day
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) // Daytime
    {
        // Golden hour colors at sunrise/sunset
        if (TimeOfDay <= 7.0f || TimeOfDay >= 17.0f)
        {
            return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Golden
        }
        else
        {
            return FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm white
        }
    }
    else // Nighttime
    {
        return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Cool moonlight
    }
}

float UAtmosphericLightingManager::CalculateSunIntensity(float TimeOfDay) const
{
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) // Daytime
    {
        // Peak intensity at noon
        float NoonFactor = 1.0f - FMath::Abs(TimeOfDay - 12.0f) / 6.0f;
        return FMath::Lerp(1.0f, 5.0f, NoonFactor);
    }
    else // Nighttime
    {
        return 0.1f; // Minimal moonlight
    }
}