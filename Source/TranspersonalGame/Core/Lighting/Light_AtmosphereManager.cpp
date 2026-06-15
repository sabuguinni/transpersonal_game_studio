#include "Light_AtmosphereManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = UpdateInterval;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize default settings
    InitializeTimeOfDaySettings();
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find lighting actors in the world
    FindLightingActors();
    
    // Apply initial atmospheric settings
    UpdateAtmosphere();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Initialized with %d time of day settings"), TimeOfDaySettings.Num());
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time of day
        CurrentTimeOfDay += DeltaTime / DayDuration;
        if (CurrentTimeOfDay >= 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }

        // Update atmosphere at intervals
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= UpdateInterval)
        {
            UpdateSunPosition();
            UpdateAtmosphere();
            LastUpdateTime = 0.0f;
        }
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 1.0f);
    UpdateSunPosition();
    UpdateAtmosphere();
}

void ALight_AtmosphereManager::SetWeather(ELight_WeatherType NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        UpdateAtmosphere();
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Weather changed to %d"), (int32)NewWeather);
    }
}

void ALight_AtmosphereManager::ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetTemperature(Settings.Temperature);
        LightComp->SetVolumetricScatteringIntensity(Settings.VolumetricScattering);
        LightComp->SetCastVolumetricShadow(true);
    }

    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.6f);
    }
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay < 0.1f || CurrentTimeOfDay >= 0.9f)
        return ELight_TimeOfDay::Night;
    else if (CurrentTimeOfDay < 0.2f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay < 0.4f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay < 0.6f)
        return ELight_TimeOfDay::Noon;
    else if (CurrentTimeOfDay < 0.8f)
        return ELight_TimeOfDay::Afternoon;
    else
        return ELight_TimeOfDay::Dusk;
}

void ALight_AtmosphereManager::CreateCinematicLighting()
{
    // Enhanced cinematic lighting setup
    FLight_AtmosphereSettings CinematicSettings;
    CinematicSettings.SunIntensity = 10.0f;
    CinematicSettings.SunColor = FLinearColor(1.2f, 0.9f, 0.7f, 1.0f);
    CinematicSettings.Temperature = 4200.0f;
    CinematicSettings.FogDensity = 0.045f;
    CinematicSettings.FogColor = FLinearColor(1.1f, 0.8f, 0.5f, 1.0f);
    CinematicSettings.VolumetricScattering = 3.0f;

    ApplyAtmosphereSettings(CinematicSettings);
    
    // Set dramatic sun angle for golden hour effect
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(-25.0f, 45.0f, 0.0f));
    }

    UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Cinematic lighting applied"));
}

void ALight_AtmosphereManager::EnableVolumetricFog(bool bEnable)
{
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetVolumetricFog(bEnable);
        
        if (bEnable)
        {
            FogComp->SetVolumetricFogScatteringDistribution(0.6f);
            FogComp->SetVolumetricFogAlbedo(FColor(240, 220, 180));
            FogComp->SetVolumetricFogEmissive(FLinearColor(0.1f, 0.08f, 0.05f, 1.0f));
            FogComp->SetVolumetricFogExtinctionScale(0.8f);
        }
    }
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;

    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay - 0.25f) * 360.0f; // -90 at dawn, 90 at dusk
    float Elevation = FMath::Sin(CurrentTimeOfDay * PI) * 90.0f - 90.0f; // -90 to 90 degrees
    
    // Clamp elevation to prevent sun going below horizon too much
    Elevation = FMath::Clamp(Elevation, -15.0f, 90.0f);
    
    FRotator NewRotation(Elevation, SunAngle, 0.0f);
    SunLight->SetActorRotation(NewRotation);
}

void ALight_AtmosphereManager::UpdateAtmosphere()
{
    ELight_TimeOfDay CurrentPeriod = GetCurrentTimeOfDayEnum();
    
    if (TimeOfDaySettings.Contains(CurrentPeriod))
    {
        FLight_AtmosphereSettings* Settings = TimeOfDaySettings.Find(CurrentPeriod);
        if (Settings)
        {
            // Apply weather modifications
            FLight_AtmosphereSettings ModifiedSettings = *Settings;
            
            switch (CurrentWeather)
            {
                case ELight_WeatherType::Overcast:
                    ModifiedSettings.SunIntensity *= 0.6f;
                    ModifiedSettings.FogDensity *= 1.5f;
                    break;
                case ELight_WeatherType::Foggy:
                    ModifiedSettings.FogDensity *= 3.0f;
                    ModifiedSettings.SunIntensity *= 0.4f;
                    break;
                case ELight_WeatherType::Stormy:
                    ModifiedSettings.SunIntensity *= 0.3f;
                    ModifiedSettings.FogDensity *= 2.0f;
                    ModifiedSettings.SunColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
                    break;
                case ELight_WeatherType::Volcanic:
                    ModifiedSettings.FogDensity *= 4.0f;
                    ModifiedSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
                    ModifiedSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
                    break;
            }
            
            ApplyAtmosphereSettings(ModifiedSettings);
        }
    }
}

void ALight_AtmosphereManager::InitializeTimeOfDaySettings()
{
    // Dawn settings
    FLight_AtmosphereSettings DawnSettings;
    DawnSettings.SunIntensity = 4.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.Temperature = 3800.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    DawnSettings.VolumetricScattering = 1.5f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    // Morning settings
    FLight_AtmosphereSettings MorningSettings;
    MorningSettings.SunIntensity = 7.0f;
    MorningSettings.SunColor = FLinearColor(1.1f, 0.95f, 0.8f, 1.0f);
    MorningSettings.Temperature = 4200.0f;
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    MorningSettings.VolumetricScattering = 2.0f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Morning, MorningSettings);

    // Noon settings
    FLight_AtmosphereSettings NoonSettings;
    NoonSettings.SunIntensity = 10.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    NoonSettings.Temperature = 5500.0f;
    NoonSettings.FogDensity = 0.02f;
    NoonSettings.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    NoonSettings.VolumetricScattering = 1.8f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Noon, NoonSettings);

    // Afternoon settings
    FLight_AtmosphereSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 8.5f;
    AfternoonSettings.SunColor = FLinearColor(1.1f, 0.9f, 0.7f, 1.0f);
    AfternoonSettings.Temperature = 4500.0f;
    AfternoonSettings.FogDensity = 0.035f;
    AfternoonSettings.FogColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.VolumetricScattering = 2.2f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk settings
    FLight_AtmosphereSettings DuskSettings;
    DuskSettings.SunIntensity = 5.0f;
    DuskSettings.SunColor = FLinearColor(1.2f, 0.7f, 0.4f, 1.0f);
    DuskSettings.Temperature = 3200.0f;
    DuskSettings.FogDensity = 0.06f;
    DuskSettings.FogColor = FLinearColor(1.1f, 0.7f, 0.5f, 1.0f);
    DuskSettings.VolumetricScattering = 2.5f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    // Night settings
    FLight_AtmosphereSettings NightSettings;
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    NightSettings.Temperature = 8000.0f;
    NightSettings.FogDensity = 0.04f;
    NightSettings.FogColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightSettings.VolumetricScattering = 1.0f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Night, NightSettings);
}

void ALight_AtmosphereManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Found DirectionalLight"));
    }

    // Find sky light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Found SkyLight"));
    }

    // Find atmospheric fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Found ExponentialHeightFog"));
    }
}