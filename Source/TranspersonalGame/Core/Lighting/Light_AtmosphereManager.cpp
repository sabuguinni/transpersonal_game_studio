#include "Light_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    bEnableDayNightCycle = true;
    DayDurationMinutes = 24.0f;
    CurrentTimeOfDay = 12.0f;
    bEnableWeatherSystem = true;
    WeatherIntensity = 0.0f;
    TimeAccumulator = 0.0f;
    CurrentPeriod = ELight_TimeOfDay::Noon;

    // Initialize Cretaceous atmospheric settings
    CretaceousSettings.SunIntensity = 5.0f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    CretaceousSettings.SunAngle = 45.0f;
    CretaceousSettings.RayleighScattering = 0.0331f;
    CretaceousSettings.MieScattering = 0.003996f;
    CretaceousSettings.AtmosphericFogDensity = 0.02f;
    CretaceousSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    VolumetricClouds = nullptr;
    PostProcessVolume = nullptr;
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAtmosphericComponents();
    ApplyCretaceousLighting();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_AtmosphereManager::InitializeAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing atmospheric components in the level
    TArray<AActor*> FoundActors;
    
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VolumetricClouds = Cast<AVolumetricCloud>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void ALight_AtmosphereManager::ApplyCretaceousLighting()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CretaceousSettings.SunIntensity);
        LightComp->SetLightColor(CretaceousSettings.SunColor);
        
        // Set sun angle for tropical Cretaceous period
        FRotator SunRotation = FRotator(-CretaceousSettings.SunAngle, 180.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }

    if (SkyAtmosphere)
    {
        // Configure sky atmosphere for Cretaceous period
        // Note: Actual property setting would require access to SkyAtmosphereComponent
        // This is a framework for the atmospheric settings
    }

    if (VolumetricClouds)
    {
        // Disable volumetric clouds for clear Cretaceous sky
        VolumetricClouds->SetActorHiddenInGame(true);
    }

    UpdatePostProcessing();
}

void ALight_AtmosphereManager::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Clamp(Hours, 0.0f, 24.0f);
    CurrentPeriod = CalculateTimeOfDayPeriod();
    UpdateSunPosition();
    UpdateAtmosphericScattering();
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDay() const
{
    return CurrentPeriod;
}

void ALight_AtmosphereManager::SetWeatherIntensity(float Intensity)
{
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdatePostProcessing();
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight)
    {
        return;
    }

    // Calculate sun angle based on time of day
    float SunAngleRadians = (CurrentTimeOfDay / 24.0f) * 2.0f * PI;
    float SunElevation = FMath::Sin(SunAngleRadians) * 90.0f;
    float SunAzimuth = (CurrentTimeOfDay / 24.0f) * 360.0f;

    // Clamp elevation to prevent sun going below horizon during day
    if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay <= 18.0f)
    {
        SunElevation = FMath::Max(SunElevation, 10.0f);
    }

    FRotator NewRotation = FRotator(-SunElevation, SunAzimuth, 0.0f);
    SunLight->SetActorRotation(NewRotation);

    // Update sun intensity and color based on time
    if (SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(GetSunIntensityForTime());
        LightComp->SetLightColor(GetSunColorForTime());
    }
}

void ALight_AtmosphereManager::UpdateAtmosphericScattering()
{
    if (!SkyAtmosphere)
    {
        return;
    }

    // Update atmospheric scattering based on time of day
    // Framework for dynamic atmospheric changes
}

void ALight_AtmosphereManager::DebugAtmosphereSettings()
{
    if (GEngine)
    {
        FString DebugMessage = FString::Printf(
            TEXT("Atmosphere Manager Debug:\nTime: %.2f\nPeriod: %s\nSun Intensity: %.2f\nWeather: %.2f"),
            CurrentTimeOfDay,
            *UEnum::GetValueAsString(CurrentPeriod),
            CretaceousSettings.SunIntensity,
            WeatherIntensity
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugMessage);
    }
}

void ALight_AtmosphereManager::UpdateDayNightCycle(float DeltaTime)
{
    TimeAccumulator += DeltaTime;
    
    // Convert real-time to game time
    float GameTimeIncrement = (DeltaTime / 60.0f) / DayDurationMinutes * 24.0f;
    CurrentTimeOfDay += GameTimeIncrement;
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    ELight_TimeOfDay NewPeriod = CalculateTimeOfDayPeriod();
    if (NewPeriod != CurrentPeriod)
    {
        CurrentPeriod = NewPeriod;
        UpdateSunPosition();
        UpdateAtmosphericScattering();
    }
}

void ALight_AtmosphereManager::CalculateSunAngle()
{
    // Calculate sun angle based on current time
    float TimeRatio = CurrentTimeOfDay / 24.0f;
    CretaceousSettings.SunAngle = FMath::Sin(TimeRatio * 2.0f * PI) * 90.0f;
}

void ALight_AtmosphereManager::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphere)
    {
        return;
    }

    // Update sky atmosphere properties for current conditions
    // Framework for dynamic sky changes
}

void ALight_AtmosphereManager::UpdatePostProcessing()
{
    if (!PostProcessVolume)
    {
        return;
    }

    // Update post-processing effects based on weather and time
    // Framework for atmospheric post-processing
}

ELight_TimeOfDay ALight_AtmosphereManager::CalculateTimeOfDayPeriod() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
    {
        return ELight_TimeOfDay::Dawn;
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
    {
        return ELight_TimeOfDay::Morning;
    }
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
    {
        return ELight_TimeOfDay::Noon;
    }
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
    {
        return ELight_TimeOfDay::Afternoon;
    }
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
    {
        return ELight_TimeOfDay::Dusk;
    }
    else
    {
        return ELight_TimeOfDay::Night;
    }
}

FLinearColor ALight_AtmosphereManager::GetSunColorForTime() const
{
    switch (CurrentPeriod)
    {
        case ELight_TimeOfDay::Dawn:
            return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Orange dawn
        case ELight_TimeOfDay::Morning:
            return FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm morning
        case ELight_TimeOfDay::Noon:
            return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // Pure white noon
        case ELight_TimeOfDay::Afternoon:
            return FLinearColor(1.0f, 0.95f, 0.85f, 1.0f); // Slightly warm afternoon
        case ELight_TimeOfDay::Dusk:
            return FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Orange dusk
        case ELight_TimeOfDay::Night:
            return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Cool moonlight
        default:
            return CretaceousSettings.SunColor;
    }
}

float ALight_AtmosphereManager::GetSunIntensityForTime() const
{
    switch (CurrentPeriod)
    {
        case ELight_TimeOfDay::Dawn:
            return CretaceousSettings.SunIntensity * 0.3f;
        case ELight_TimeOfDay::Morning:
            return CretaceousSettings.SunIntensity * 0.8f;
        case ELight_TimeOfDay::Noon:
            return CretaceousSettings.SunIntensity;
        case ELight_TimeOfDay::Afternoon:
            return CretaceousSettings.SunIntensity * 0.9f;
        case ELight_TimeOfDay::Dusk:
            return CretaceousSettings.SunIntensity * 0.4f;
        case ELight_TimeOfDay::Night:
            return CretaceousSettings.SunIntensity * 0.1f;
        default:
            return CretaceousSettings.SunIntensity;
    }
}