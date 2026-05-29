#include "AtmosphericSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogAtmosphericSystem);

UAtmosphericSystem::UAtmosphericSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Default day/night cycle settings
    DayDuration = 1200.0f; // 20 minutes real time = 24 hours game time
    CurrentTimeOfDay = 6.0f; // Start at 6 AM
    bIsNightTime = false;
    
    // Default sun settings
    SunIntensity = 3.0f;
    SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f); // Warm daylight
    MoonIntensity = 0.1f;
    MoonColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f); // Cool moonlight
    
    // Default fog settings
    FogDensity = 0.02f;
    FogHeightFalloff = 0.2f;
    FogMaxOpacity = 1.0f;
    FogStartDistance = 0.0f;
    FogCutoffDistance = 0.0f;
    
    // Default weather settings
    CurrentWeatherType = EEnvArt_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
    RainIntensity = 0.0f;
    WindStrength = 0.5f;
    CloudCoverage = 0.3f;
    
    // Initialize atmospheric components references
    DirectionalLightActor = nullptr;
    SkyAtmosphereActor = nullptr;
    HeightFogActor = nullptr;
}

void UAtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: BeginPlay started"));
    
    // Find or create atmospheric actors
    InitializeAtmosphericActors();
    
    // Start day/night cycle
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DayNightTimerHandle,
            this,
            &UAtmosphericSystem::UpdateDayNightCycle,
            1.0f, // Update every second
            true
        );
    }
    
    UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: BeginPlay completed"));
}

void UAtmosphericSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update weather effects
    UpdateWeatherEffects(DeltaTime);
    
    // Update atmospheric parameters
    UpdateAtmosphericParameters();
}

void UAtmosphericSystem::InitializeAtmosphericActors()
{
    if (!GetWorld())
    {
        UE_LOG(LogAtmosphericSystem, Error, TEXT("AtmosphericSystem: No valid world"));
        return;
    }
    
    // Find existing directional light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        DirectionalLightActor = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: Found existing DirectionalLight"));
    }
    else
    {
        // Create directional light
        FVector SpawnLocation(0.0f, 0.0f, 1000.0f);
        FRotator SpawnRotation(-45.0f, 0.0f, 0.0f); // 45 degrees down
        
        DirectionalLightActor = GetWorld()->SpawnActor<ADirectionalLight>(
            ADirectionalLight::StaticClass(),
            SpawnLocation,
            SpawnRotation
        );
        
        if (DirectionalLightActor)
        {
            DirectionalLightActor->SetActorLabel(TEXT("AtmosphericSystem_Sun"));
            UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: Created DirectionalLight"));
        }
    }
    
    // Find existing sky atmosphere
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyAtmosphere::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(FoundActors[0]);
        UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: Found existing SkyAtmosphere"));
    }
    else
    {
        // Create sky atmosphere
        SkyAtmosphereActor = GetWorld()->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass());
        
        if (SkyAtmosphereActor)
        {
            SkyAtmosphereActor->SetActorLabel(TEXT("AtmosphericSystem_Sky"));
            UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: Created SkyAtmosphere"));
        }
    }
    
    // Find existing height fog
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: Found existing HeightFog"));
    }
    else
    {
        // Create height fog
        HeightFogActor = GetWorld()->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass());
        
        if (HeightFogActor)
        {
            HeightFogActor->SetActorLabel(TEXT("AtmosphericSystem_Fog"));
            UE_LOG(LogAtmosphericSystem, Log, TEXT("AtmosphericSystem: Created HeightFog"));
        }
    }
}

void UAtmosphericSystem::UpdateDayNightCycle()
{
    if (!GetWorld()) return;
    
    // Advance time
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentTimeOfDay += (24.0f / DayDuration) * DeltaTime;
    
    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
    
    // Determine if it's night time
    bIsNightTime = (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 18.0f);
    
    // Update sun position and intensity
    UpdateSunPosition();
    UpdateSunIntensity();
    
    UE_LOG(LogAtmosphericSystem, VeryVerbose, TEXT("Time: %.2f, IsNight: %s"), 
           CurrentTimeOfDay, bIsNightTime ? TEXT("true") : TEXT("false"));
}

void UAtmosphericSystem::UpdateSunPosition()
{
    if (!DirectionalLightActor) return;
    
    // Calculate sun angle based on time of day
    // 6 AM = 0°, 12 PM = 90°, 6 PM = 180°, 12 AM = 270°
    float SunAngle = ((CurrentTimeOfDay - 6.0f) / 24.0f) * 360.0f;
    
    // Convert to pitch (vertical angle)
    float SunPitch = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    
    // Set sun rotation
    FRotator SunRotation(SunPitch, 0.0f, 0.0f);
    DirectionalLightActor->SetActorRotation(SunRotation);
}

void UAtmosphericSystem::UpdateSunIntensity()
{
    if (!DirectionalLightActor || !DirectionalLightActor->GetLightComponent()) return;
    
    UDirectionalLightComponent* LightComp = DirectionalLightActor->GetLightComponent();
    
    float IntensityMultiplier = 1.0f;
    FLinearColor LightColor = SunColor;
    
    if (bIsNightTime)
    {
        // Night time - use moon settings
        IntensityMultiplier = MoonIntensity / SunIntensity;
        LightColor = MoonColor;
    }
    else
    {
        // Day time - calculate based on sun height
        float SunHeight = FMath::Sin(FMath::DegreesToRadians(((CurrentTimeOfDay - 6.0f) / 12.0f) * 180.0f));
        IntensityMultiplier = FMath::Max(0.1f, SunHeight);
    }
    
    LightComp->SetIntensity(SunIntensity * IntensityMultiplier);
    LightComp->SetLightColor(LightColor);
}

void UAtmosphericSystem::UpdateWeatherEffects(float DeltaTime)
{
    // Update weather transition
    switch (CurrentWeatherType)
    {
        case EEnvArt_WeatherType::Clear:
            RainIntensity = FMath::FInterpTo(RainIntensity, 0.0f, DeltaTime, WeatherTransitionSpeed);
            CloudCoverage = FMath::FInterpTo(CloudCoverage, 0.2f, DeltaTime, WeatherTransitionSpeed);
            break;
            
        case EEnvArt_WeatherType::Cloudy:
            RainIntensity = FMath::FInterpTo(RainIntensity, 0.0f, DeltaTime, WeatherTransitionSpeed);
            CloudCoverage = FMath::FInterpTo(CloudCoverage, 0.7f, DeltaTime, WeatherTransitionSpeed);
            break;
            
        case EEnvArt_WeatherType::Rainy:
            RainIntensity = FMath::FInterpTo(RainIntensity, 1.0f, DeltaTime, WeatherTransitionSpeed);
            CloudCoverage = FMath::FInterpTo(CloudCoverage, 0.9f, DeltaTime, WeatherTransitionSpeed);
            break;
            
        case EEnvArt_WeatherType::Stormy:
            RainIntensity = FMath::FInterpTo(RainIntensity, 1.5f, DeltaTime, WeatherTransitionSpeed);
            CloudCoverage = FMath::FInterpTo(CloudCoverage, 1.0f, DeltaTime, WeatherTransitionSpeed);
            break;
            
        case EEnvArt_WeatherType::Foggy:
            RainIntensity = FMath::FInterpTo(RainIntensity, 0.2f, DeltaTime, WeatherTransitionSpeed);
            CloudCoverage = FMath::FInterpTo(CloudCoverage, 0.8f, DeltaTime, WeatherTransitionSpeed);
            break;
    }
}

void UAtmosphericSystem::UpdateAtmosphericParameters()
{
    // Update fog based on weather
    if (HeightFogActor && HeightFogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        
        float WeatherFogMultiplier = 1.0f;
        if (CurrentWeatherType == EEnvArt_WeatherType::Foggy)
        {
            WeatherFogMultiplier = 3.0f;
        }
        else if (CurrentWeatherType == EEnvArt_WeatherType::Rainy || CurrentWeatherType == EEnvArt_WeatherType::Stormy)
        {
            WeatherFogMultiplier = 1.5f;
        }
        
        FogComp->SetFogDensity(FogDensity * WeatherFogMultiplier);
        FogComp->SetFogHeightFalloff(FogHeightFalloff);
        FogComp->SetFogMaxOpacity(FogMaxOpacity);
        FogComp->SetStartDistance(FogStartDistance);
        FogComp->SetFogCutoffDistance(FogCutoffDistance);
    }
}

void UAtmosphericSystem::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    bIsNightTime = (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 18.0f);
    
    UpdateSunPosition();
    UpdateSunIntensity();
    
    UE_LOG(LogAtmosphericSystem, Log, TEXT("Time of day set to: %.2f"), CurrentTimeOfDay);
}

void UAtmosphericSystem::SetWeatherType(EEnvArt_WeatherType NewWeatherType)
{
    if (CurrentWeatherType != NewWeatherType)
    {
        CurrentWeatherType = NewWeatherType;
        UE_LOG(LogAtmosphericSystem, Log, TEXT("Weather changed to: %d"), (int32)NewWeatherType);
        
        // Trigger weather change event
        OnWeatherChanged.Broadcast(CurrentWeatherType);
    }
}

void UAtmosphericSystem::SetFogDensity(float NewFogDensity)
{
    FogDensity = FMath::Clamp(NewFogDensity, 0.0f, 1.0f);
    UpdateAtmosphericParameters();
}

void UAtmosphericSystem::SetWindStrength(float NewWindStrength)
{
    WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 2.0f);
    
    // Broadcast wind change for vegetation and particle systems
    OnWindChanged.Broadcast(WindStrength);
}

float UAtmosphericSystem::GetTimeOfDay() const
{
    return CurrentTimeOfDay;
}

bool UAtmosphericSystem::IsNightTime() const
{
    return bIsNightTime;
}

EEnvArt_WeatherType UAtmosphericSystem::GetCurrentWeatherType() const
{
    return CurrentWeatherType;
}

float UAtmosphericSystem::GetRainIntensity() const
{
    return RainIntensity;
}

float UAtmosphericSystem::GetWindStrength() const
{
    return WindStrength;
}

float UAtmosphericSystem::GetCloudCoverage() const
{
    return CloudCoverage;
}

FLinearColor UAtmosphericSystem::GetCurrentSkyColor() const
{
    if (bIsNightTime)
    {
        return FLinearColor(0.1f, 0.1f, 0.3f, 1.0f); // Dark blue night sky
    }
    else
    {
        // Interpolate between dawn/dusk and midday colors
        float SunHeight = FMath::Sin(FMath::DegreesToRadians(((CurrentTimeOfDay - 6.0f) / 12.0f) * 180.0f));
        FLinearColor DawnDuskColor(1.0f, 0.6f, 0.3f, 1.0f); // Orange
        FLinearColor MiddayColor(0.5f, 0.7f, 1.0f, 1.0f); // Blue
        
        return FMath::Lerp(DawnDuskColor, MiddayColor, SunHeight);
    }
}

void UAtmosphericSystem::CreateVolumetricFog(FVector Location, float Radius, float Density)
{
    if (!GetWorld()) return;
    
    // Create a local fog volume at the specified location
    // This would typically involve creating a sphere volume with fog material
    UE_LOG(LogAtmosphericSystem, Log, TEXT("Creating volumetric fog at location: %s"), *Location.ToString());
    
    // Implementation would create a sphere actor with volumetric fog material
    // For now, log the request
}

void UAtmosphericSystem::SetGoldenHourLighting()
{
    // Set time to golden hour (around 7 AM or 6 PM)
    SetTimeOfDay(7.0f); // Morning golden hour
    
    // Adjust sun color for golden hour
    if (DirectionalLightActor && DirectionalLightActor->GetLightComponent())
    {
        FLinearColor GoldenColor(1.0f, 0.8f, 0.4f, 1.0f);
        DirectionalLightActor->GetLightComponent()->SetLightColor(GoldenColor);
        DirectionalLightActor->GetLightComponent()->SetIntensity(SunIntensity * 0.8f);
    }
    
    UE_LOG(LogAtmosphericSystem, Log, TEXT("Golden hour lighting applied"));
}