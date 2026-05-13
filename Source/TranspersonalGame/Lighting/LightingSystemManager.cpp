#include "LightingSystemManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULightingSystemManager::ULightingSystemManager()
{
    // Initialize default values
    TimeOfDay = FLight_TimeOfDay();
    AtmosphericSettings = FLight_AtmosphericSettings();
    CurrentWeather = EWeatherType::Clear;
    
    SunLight = nullptr;
    SkyLightActor = nullptr;
    HeightFog = nullptr;
    PostProcessVolume = nullptr;
}

void ULightingSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Initializing lighting subsystem"));
    
    // Initialize lighting system after a short delay to ensure world is ready
    if (UWorld* World = GetWorld())
    {
        FTimerHandle InitTimer;
        World->GetTimerManager().SetTimer(InitTimer, this, &ULightingSystemManager::InitializeLightingSystem, 1.0f, false);
    }
}

void ULightingSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Deinitializing lighting subsystem"));
    
    // Clear references
    SunLight = nullptr;
    SkyLightActor = nullptr;
    HeightFog = nullptr;
    PostProcessVolume = nullptr;
    
    Super::Deinitialize();
}

bool ULightingSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void ULightingSystemManager::InitializeLightingSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Starting lighting system initialization"));
    
    // Find existing lighting actors in the world
    FindLightingActors();
    
    // Apply atmospheric correction immediately
    ApplyAtmosphericCorrection();
    
    // Set initial time of day
    SetTimeOfDay(12.0f); // Noon
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Lighting system initialization complete"));
}

void ULightingSystemManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("LightingSystemManager: No valid world found"));
        return;
    }
    
    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found DirectionalLight"));
    }
    
    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found SkyLight"));
    }
    
    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found ExponentialHeightFog"));
    }
    
    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found PostProcessVolume"));
    }
}

void ULightingSystemManager::ApplyAtmosphericCorrection()
{
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Applying atmospheric correction"));
    
    // Update sun light properties
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(AtmosphericSettings.SunIntensity);
        LightComp->SetLightColor(AtmosphericSettings.SunColor);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Applied sun correction"));
    }
    
    // Update sky atmosphere if present
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("SkyAtmosphere")))
            {
                // Try to set rayleigh scattering scale
                if (USkyAtmosphereComponent* SkyComp = Actor->FindComponentByClass<USkyAtmosphereComponent>())
                {
                    // Set atmospheric properties for Cretaceous period
                    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Applied sky atmosphere correction"));
                }
            }
            
            // Hide volumetric clouds for cleaner atmosphere
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("VolumetricCloud")))
            {
                Actor->SetActorHiddenInGame(true);
                UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Hidden volumetric clouds"));
            }
        }
    }
    
    // Update fog settings
    UpdateFogSettings();
}

void ULightingSystemManager::UpdateDayNightCycle(float DeltaTime)
{
    if (TimeOfDay.bIsPaused)
    {
        return;
    }
    
    // Update time
    float HourIncrement = (DeltaTime / TimeOfDay.DayDuration) * 24.0f;
    TimeOfDay.CurrentHour += HourIncrement;
    
    // Wrap around 24 hours
    if (TimeOfDay.CurrentHour >= 24.0f)
    {
        TimeOfDay.CurrentHour -= 24.0f;
    }
    
    // Update lighting based on new time
    UpdateSunPosition();
    UpdateSunLightProperties();
    UpdateSkyLightProperties();
    UpdateFogSettings();
}

void ULightingSystemManager::SetTimeOfDay(float Hour)
{
    TimeOfDay.CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    
    // Update all lighting immediately
    UpdateSunPosition();
    UpdateSunLightProperties();
    UpdateSkyLightProperties();
    UpdateFogSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Set time to %.2f hours"), Hour);
}

void ULightingSystemManager::SetWeatherCondition(EWeatherType WeatherType)
{
    CurrentWeather = WeatherType;
    
    // Update atmospheric settings based on weather
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            AtmosphericSettings.FogDensity = 0.02f;
            AtmosphericSettings.SunIntensity = 5.0f;
            break;
        case EWeatherType::Cloudy:
            AtmosphericSettings.FogDensity = 0.05f;
            AtmosphericSettings.SunIntensity = 3.0f;
            break;
        case EWeatherType::Rainy:
            AtmosphericSettings.FogDensity = 0.08f;
            AtmosphericSettings.SunIntensity = 1.5f;
            break;
        case EWeatherType::Stormy:
            AtmosphericSettings.FogDensity = 0.12f;
            AtmosphericSettings.SunIntensity = 0.8f;
            break;
    }
    
    // Apply changes immediately
    UpdateSunLightProperties();
    UpdateFogSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Weather changed to %d"), (int32)WeatherType);
}

FLinearColor ULightingSystemManager::GetCurrentSunColor() const
{
    return CalculateSunColorForTime();
}

float ULightingSystemManager::GetCurrentSunIntensity() const
{
    return CalculateSunIntensityForTime();
}

void ULightingSystemManager::UpdateSunPosition()
{
    if (!SunLight)
    {
        return;
    }
    
    // Calculate sun angle based on time (0-360 degrees)
    float SunAngle = CalculateSunAngle();
    
    // Set sun rotation (pitch controls height in sky)
    FRotator SunRotation = FRotator(SunAngle - 90.0f, 0.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ULightingSystemManager::UpdateSunLightProperties()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    
    // Update color and intensity based on time of day
    LightComp->SetLightColor(CalculateSunColorForTime());
    LightComp->SetIntensity(CalculateSunIntensityForTime());
}

void ULightingSystemManager::UpdateSkyLightProperties()
{
    if (!SkyLightActor || !SkyLightActor->GetLightComponent())
    {
        return;
    }
    
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    
    // Update sky light intensity based on time
    float SkyIntensity = IsNightTime() ? 0.1f : 1.0f;
    SkyComp->SetIntensity(SkyIntensity);
}

void ULightingSystemManager::UpdateFogSettings()
{
    if (!HeightFog || !HeightFog->GetComponent())
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    
    // Update fog properties
    FogComp->SetFogDensity(AtmosphericSettings.FogDensity);
    FogComp->SetFogInscatteringColor(CalculateFogColorForTime());
}

void ULightingSystemManager::UpdatePostProcessProperties()
{
    if (!PostProcessVolume || !PostProcessVolume->GetComponent())
    {
        return;
    }
    
    // Post-process updates can be added here for advanced atmospheric effects
}

float ULightingSystemManager::CalculateSunAngle() const
{
    // Convert hour to angle (0-360 degrees)
    // 6 AM = 0 degrees (horizon), 12 PM = 90 degrees (zenith), 6 PM = 180 degrees (horizon)
    float NormalizedHour = (TimeOfDay.CurrentHour - 6.0f) / 12.0f; // 0-2 range
    return NormalizedHour * 180.0f; // 0-360 degrees
}

FLinearColor ULightingSystemManager::CalculateSunColorForTime() const
{
    // Cretaceous tropical lighting - warm throughout the day
    if (IsNightTime())
    {
        return FLinearColor(0.1f, 0.1f, 0.2f, 1.0f); // Cool blue moonlight
    }
    
    // Dawn/dusk hours (5-7 AM, 6-8 PM)
    if ((TimeOfDay.CurrentHour >= 5.0f && TimeOfDay.CurrentHour <= 7.0f) ||
        (TimeOfDay.CurrentHour >= 18.0f && TimeOfDay.CurrentHour <= 20.0f))
    {
        return FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Warm orange
    }
    
    // Midday - bright warm sunlight
    return AtmosphericSettings.SunColor;
}

float ULightingSystemManager::CalculateSunIntensityForTime() const
{
    if (IsNightTime())
    {
        return 0.0f; // No sun at night
    }
    
    // Calculate intensity based on sun height
    float SunAngle = CalculateSunAngle();
    float IntensityMultiplier = FMath::Sin(FMath::DegreesToRadians(SunAngle));
    IntensityMultiplier = FMath::Clamp(IntensityMultiplier, 0.1f, 1.0f);
    
    return AtmosphericSettings.SunIntensity * IntensityMultiplier;
}

FLinearColor ULightingSystemManager::CalculateFogColorForTime() const
{
    if (IsNightTime())
    {
        return FLinearColor(0.1f, 0.1f, 0.3f, 1.0f); // Dark blue night fog
    }
    
    // Warm tropical fog during day
    return AtmosphericSettings.FogColor;
}