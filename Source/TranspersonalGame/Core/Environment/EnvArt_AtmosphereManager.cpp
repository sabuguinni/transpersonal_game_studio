#include "EnvArt_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEnvArt_AtmosphereManager::UEnvArt_AtmosphereManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    SetupDefaultSettings();
}

void UEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAtmosphereComponents();
    
    if (bAutoTransition)
    {
        TimeOfDayTimer = 0.0f;
    }
}

void UEnvArt_AtmosphereManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAtmosphere(DeltaTime);
    
    // Auto transition between time periods
    if (bAutoTransition)
    {
        TimeOfDayTimer += DeltaTime;
        if (TimeOfDayTimer >= TimeOfDayDuration)
        {
            TimeOfDayTimer = 0.0f;
            int32 CurrentIndex = static_cast<int32>(CurrentTimeOfDay);
            CurrentIndex = (CurrentIndex + 1) % 6; // 6 time periods
            SetTimeOfDay(static_cast<EEnvArt_TimeOfDay>(CurrentIndex));
        }
    }
}

void UEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        
        if (TimeOfDaySettings.Contains(NewTimeOfDay))
        {
            TransitionToSettings(TimeOfDaySettings[NewTimeOfDay]);
        }
    }
}

void UEnvArt_AtmosphereManager::SetWeather(EEnvArt_WeatherType NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        
        if (WeatherSettings.Contains(NewWeather))
        {
            TransitionToSettings(WeatherSettings[NewWeather]);
        }
    }
}

void UEnvArt_AtmosphereManager::TransitionToSettings(const FEnvArt_AtmosphereSettings& NewTargetSettings)
{
    TargetSettings = NewTargetSettings;
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
}

void UEnvArt_AtmosphereManager::ApplyGoldenHourLighting()
{
    FEnvArt_AtmosphereSettings GoldenHourSettings;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // Warm golden color
    GoldenHourSettings.SunIntensity = 2.5f;
    GoldenHourSettings.SunRotation = FRotator(-15.0f, 45.0f, 0.0f); // Low angle
    GoldenHourSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    GoldenHourSettings.FogDensity = 0.015f;
    GoldenHourSettings.SkyTint = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    TransitionToSettings(GoldenHourSettings);
}

void UEnvArt_AtmosphereManager::ApplyVolumetricFog()
{
    if (HeightFog)
    {
        HeightFog->SetVolumetricFog(true);
        HeightFog->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFog->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
        HeightFog->SetVolumetricFogEmissive(FLinearColor(0.1f, 0.1f, 0.05f));
        HeightFog->SetVolumetricFogExtinctionScale(1.0f);
    }
}

void UEnvArt_AtmosphereManager::CreateAtmosphericParticles()
{
    // This would typically spawn Niagara particle systems for dust, pollen, etc.
    // For now, we'll log the action
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Atmospheric particles created - dust and pollen effects"));
    }
}

void UEnvArt_AtmosphereManager::InitializeAtmosphereComponents()
{
    FindAtmosphereActors();
    
    if (SunLight && SunLight->GetLightComponent())
    {
        CurrentSettings = TimeOfDaySettings.Contains(CurrentTimeOfDay) ? 
            TimeOfDaySettings[CurrentTimeOfDay] : FEnvArt_AtmosphereSettings();
        
        // Apply initial settings
        InterpolateSettings(1.0f);
    }
}

void UEnvArt_AtmosphereManager::UpdateAtmosphere(float DeltaTime)
{
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime * TransitionSpeed;
        
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentSettings = TargetSettings;
        }
        
        InterpolateSettings(TransitionProgress);
    }
}

void UEnvArt_AtmosphereManager::InterpolateSettings(float Alpha)
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    
    // Interpolate sun properties
    FLinearColor InterpolatedColor = FMath::Lerp(CurrentSettings.SunColor, TargetSettings.SunColor, Alpha);
    float InterpolatedIntensity = FMath::Lerp(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, Alpha);
    FRotator InterpolatedRotation = FMath::Lerp(CurrentSettings.SunRotation, TargetSettings.SunRotation, Alpha);
    
    LightComp->SetLightColor(InterpolatedColor);
    LightComp->SetIntensity(InterpolatedIntensity);
    SunLight->SetActorRotation(InterpolatedRotation);
    
    // Interpolate fog properties
    if (HeightFog)
    {
        float InterpolatedDensity = FMath::Lerp(CurrentSettings.FogDensity, TargetSettings.FogDensity, Alpha);
        FLinearColor InterpolatedFogColor = FMath::Lerp(CurrentSettings.FogColor, TargetSettings.FogColor, Alpha);
        float InterpolatedFalloff = FMath::Lerp(CurrentSettings.FogHeightFalloff, TargetSettings.FogHeightFalloff, Alpha);
        
        HeightFog->SetFogDensity(InterpolatedDensity);
        HeightFog->SetFogInscatteringColor(InterpolatedFogColor);
        HeightFog->SetFogHeightFalloff(InterpolatedFalloff);
    }
    
    // Interpolate sky atmosphere
    if (SkyAtmosphere)
    {
        float InterpolatedThickness = FMath::Lerp(CurrentSettings.AtmosphereThickness, TargetSettings.AtmosphereThickness, Alpha);
        // Note: Sky tint would require material parameter updates in a full implementation
    }
}

void UEnvArt_AtmosphereManager::FindAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find height fog component
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (UExponentialHeightFogComponent* FogComp = Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            HeightFog = FogComp;
            break;
        }
        
        if (USkyAtmosphereComponent* SkyComp = Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphere = SkyComp;
        }
    }
}

void UEnvArt_AtmosphereManager::SetupDefaultSettings()
{
    // Dawn settings
    FEnvArt_AtmosphereSettings DawnSettings;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunRotation = FRotator(-10.0f, 75.0f, 0.0f);
    DawnSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    DawnSettings.FogDensity = 0.025f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);
    
    // Morning settings
    FEnvArt_AtmosphereSettings MorningSettings;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunRotation = FRotator(-25.0f, 60.0f, 0.0f);
    MorningSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningSettings.FogDensity = 0.015f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);
    
    // Midday settings
    FEnvArt_AtmosphereSettings MiddaySettings;
    MiddaySettings.SunColor = FLinearColor::White;
    MiddaySettings.SunIntensity = 4.0f;
    MiddaySettings.SunRotation = FRotator(-60.0f, 45.0f, 0.0f);
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.01f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Midday, MiddaySettings);
    
    // Setup weather settings
    FEnvArt_AtmosphereSettings ClearWeather;
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.SunIntensity = 3.0f;
    WeatherSettings.Add(EEnvArt_WeatherType::Clear, ClearWeather);
    
    FEnvArt_AtmosphereSettings FoggyWeather;
    FoggyWeather.FogDensity = 0.05f;
    FoggyWeather.SunIntensity = 1.5f;
    FoggyWeather.FogColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
    WeatherSettings.Add(EEnvArt_WeatherType::Foggy, FoggyWeather);
}