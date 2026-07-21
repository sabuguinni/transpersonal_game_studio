#include "Eng_WeatherSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"

void UEng_WeatherSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentWeather = EEng_WeatherType::Clear;
    TargetWeather = EEng_WeatherType::Clear;
    RainIntensity = 0.0f;
    WindStrength = 0.0f;
    FogDensity = 0.1f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    TransitionDuration = 5.0f;
    
    WeatherUpdateInterval = 2.0f;
    MaxRainIntensity = 1.0f;
    MaxWindStrength = 1.0f;
    MaxFogDensity = 0.8f;
    
    LastWeatherUpdate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("WeatherSystemManager initialized"));
}

void UEng_WeatherSystemManager::Deinitialize()
{
    SunLight.Reset();
    SkyLight.Reset();
    
    Super::Deinitialize();
}

void UEng_WeatherSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastWeatherUpdate += DeltaTime;
    
    if (bIsTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
    
    if (LastWeatherUpdate >= WeatherUpdateInterval)
    {
        ApplyWeatherEffects();
        LastWeatherUpdate = 0.0f;
    }
}

void UEng_WeatherSystemManager::SetWeatherType(EEng_WeatherType NewWeather, float TransitionTime)
{
    if (NewWeather == CurrentWeather)
    {
        return;
    }
    
    TargetWeather = NewWeather;
    TransitionDuration = FMath::Max(0.1f, TransitionTime);
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Weather transitioning from %d to %d over %.1f seconds"), 
           (int32)CurrentWeather, (int32)NewWeather, TransitionTime);
}

void UEng_WeatherSystemManager::SetRainIntensity(float Intensity)
{
    RainIntensity = FMath::Clamp(Intensity, 0.0f, MaxRainIntensity);
    
    // Auto-set weather type based on rain intensity
    if (RainIntensity > 0.7f)
    {
        SetWeatherType(EEng_WeatherType::Storm, 3.0f);
    }
    else if (RainIntensity > 0.3f)
    {
        SetWeatherType(EEng_WeatherType::Rainy, 2.0f);
    }
    else if (RainIntensity <= 0.1f)
    {
        SetWeatherType(EEng_WeatherType::Clear, 2.0f);
    }
}

void UEng_WeatherSystemManager::SetWindStrength(float Strength)
{
    WindStrength = FMath::Clamp(Strength, 0.0f, MaxWindStrength);
}

void UEng_WeatherSystemManager::SetFogDensity(float Density)
{
    FogDensity = FMath::Clamp(Density, 0.0f, MaxFogDensity);
}

void UEng_WeatherSystemManager::SpawnLightning(FVector Location)
{
    if (CurrentWeather != EEng_WeatherType::Storm)
    {
        return;
    }
    
    // TODO: Spawn lightning VFX at location
    UE_LOG(LogTemp, Warning, TEXT("Lightning spawned at location: %s"), *Location.ToString());
    
    // Flash the directional light briefly
    if (SunLight.IsValid())
    {
        if (UDirectionalLightComponent* LightComp = SunLight->GetLightComponent())
        {
            float OriginalIntensity = LightComp->Intensity;
            LightComp->SetIntensity(OriginalIntensity * 3.0f);
            
            // TODO: Use timer to restore intensity after 0.1 seconds
        }
    }
}

void UEng_WeatherSystemManager::UpdateEnvironmentalLighting()
{
    UpdateLightingForWeather();
}

void UEng_WeatherSystemManager::UpdateWeatherTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        CurrentWeather = TargetWeather;
        bIsTransitioning = false;
        
        UE_LOG(LogTemp, Warning, TEXT("Weather transition completed to %d"), (int32)CurrentWeather);
    }
    
    // Interpolate weather parameters during transition
    float Alpha = TransitionProgress;
    
    switch (TargetWeather)
    {
        case EEng_WeatherType::Clear:
            RainIntensity = FMath::Lerp(RainIntensity, 0.0f, Alpha);
            FogDensity = FMath::Lerp(FogDensity, 0.1f, Alpha);
            break;
            
        case EEng_WeatherType::Cloudy:
            RainIntensity = FMath::Lerp(RainIntensity, 0.0f, Alpha);
            FogDensity = FMath::Lerp(FogDensity, 0.3f, Alpha);
            break;
            
        case EEng_WeatherType::Rainy:
            RainIntensity = FMath::Lerp(RainIntensity, 0.6f, Alpha);
            FogDensity = FMath::Lerp(FogDensity, 0.4f, Alpha);
            break;
            
        case EEng_WeatherType::Storm:
            RainIntensity = FMath::Lerp(RainIntensity, 1.0f, Alpha);
            FogDensity = FMath::Lerp(FogDensity, 0.6f, Alpha);
            WindStrength = FMath::Lerp(WindStrength, 0.8f, Alpha);
            break;
            
        case EEng_WeatherType::Foggy:
            RainIntensity = FMath::Lerp(RainIntensity, 0.1f, Alpha);
            FogDensity = FMath::Lerp(FogDensity, 0.8f, Alpha);
            break;
    }
}

void UEng_WeatherSystemManager::ApplyWeatherEffects()
{
    UpdateLightingForWeather();
    SpawnWeatherParticles();
}

void UEng_WeatherSystemManager::UpdateLightingForWeather()
{
    // Find sun light if not cached
    if (!SunLight.IsValid())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }
    
    if (SunLight.IsValid())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            float BaseIntensity = 3.0f;
            float WeatherMultiplier = 1.0f;
            
            switch (CurrentWeather)
            {
                case EEng_WeatherType::Clear:
                    WeatherMultiplier = 1.0f;
                    break;
                case EEng_WeatherType::Cloudy:
                    WeatherMultiplier = 0.7f;
                    break;
                case EEng_WeatherType::Rainy:
                    WeatherMultiplier = 0.5f;
                    break;
                case EEng_WeatherType::Storm:
                    WeatherMultiplier = 0.3f;
                    break;
                case EEng_WeatherType::Foggy:
                    WeatherMultiplier = 0.4f;
                    break;
            }
            
            LightComp->SetIntensity(BaseIntensity * WeatherMultiplier);
        }
    }
}

void UEng_WeatherSystemManager::SpawnWeatherParticles()
{
    // TODO: Implement particle system spawning for rain, snow, etc.
    if (RainIntensity > 0.1f)
    {
        UE_LOG(LogTemp, Log, TEXT("Rain particles should be active at intensity %.2f"), RainIntensity);
    }
}