#include "World_WeatherSystem.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AWorld_WeatherSystem::AWorld_WeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create fog component
    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("FogComponent"));
    if (FogComponent)
    {
        FogComponent->SetupAttachment(RootComponent);
        FogComponent->SetFogDensity(0.02f);
        FogComponent->SetFogInscatteringColor(FLinearColor::White);
        FogComponent->SetFogHeightFalloff(0.2f);
        FogComponent->SetFogMaxOpacity(1.0f);
    }

    // Initialize default weather settings
    WeatherSettings.WeatherType = EWorld_WeatherType::Clear_Hot;
    WeatherSettings.FogDensity = 0.02f;
    WeatherSettings.FogColor = FLinearColor::White;
    WeatherSettings.WindIntensity = 1.0f;
    WeatherSettings.WindDirection = FVector(1, 0, 0);
    WeatherSettings.Temperature = 25.0f;
    WeatherSettings.Humidity = 0.5f;

    BiomeType = EBiomeType::Savanna;
    EffectRadius = 15000.0f;
    bDynamicWeather = true;
    WeatherTransitionSpeed = 1.0f;
}

void AWorld_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherForBiome();
    ApplyWeatherSettings();
    CreateWindEffects();
}

void AWorld_WeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDynamicWeather)
    {
        UpdateDynamicWeather(DeltaTime);
    }

    if (bIsTransitioning)
    {
        CurrentTransitionTime += DeltaTime;
        float Alpha = FMath::Clamp(CurrentTransitionTime / TargetTransitionTime, 0.0f, 1.0f);

        // Interpolate weather settings
        WeatherSettings.FogDensity = FMath::Lerp(StartWeatherSettings.FogDensity, TargetWeatherSettings.FogDensity, Alpha);
        WeatherSettings.FogColor = FLinearColor::LerpUsingHSV(StartWeatherSettings.FogColor, TargetWeatherSettings.FogColor, Alpha);
        WeatherSettings.WindIntensity = FMath::Lerp(StartWeatherSettings.WindIntensity, TargetWeatherSettings.WindIntensity, Alpha);
        WeatherSettings.Temperature = FMath::Lerp(StartWeatherSettings.Temperature, TargetWeatherSettings.Temperature, Alpha);
        WeatherSettings.Humidity = FMath::Lerp(StartWeatherSettings.Humidity, TargetWeatherSettings.Humidity, Alpha);

        ApplyWeatherSettings();

        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            WeatherSettings = TargetWeatherSettings;
        }
    }
}

void AWorld_WeatherSystem::SetWeatherType(EWorld_WeatherType NewWeatherType)
{
    WeatherSettings.WeatherType = NewWeatherType;
    
    switch (NewWeatherType)
    {
        case EWorld_WeatherType::Clear_Hot:
            WeatherSettings.FogDensity = 0.01f;
            WeatherSettings.FogColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            WeatherSettings.Temperature = 35.0f;
            WeatherSettings.Humidity = 0.2f;
            break;
            
        case EWorld_WeatherType::Foggy_Humid:
            WeatherSettings.FogDensity = 0.08f;
            WeatherSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.6f, 1.0f);
            WeatherSettings.Temperature = 22.0f;
            WeatherSettings.Humidity = 0.9f;
            break;
            
        case EWorld_WeatherType::Rainy_Dense:
            WeatherSettings.FogDensity = 0.05f;
            WeatherSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
            WeatherSettings.Temperature = 18.0f;
            WeatherSettings.Humidity = 0.95f;
            break;
            
        case EWorld_WeatherType::Sandstorm_Dry:
            WeatherSettings.FogDensity = 0.12f;
            WeatherSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.4f, 1.0f);
            WeatherSettings.Temperature = 42.0f;
            WeatherSettings.Humidity = 0.1f;
            WeatherSettings.WindIntensity = 3.0f;
            break;
            
        case EWorld_WeatherType::Snowy_Windy:
            WeatherSettings.FogDensity = 0.06f;
            WeatherSettings.FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
            WeatherSettings.Temperature = -5.0f;
            WeatherSettings.Humidity = 0.8f;
            WeatherSettings.WindIntensity = 2.5f;
            break;
    }
    
    ApplyWeatherSettings();
}

void AWorld_WeatherSystem::ApplyWeatherSettings()
{
    if (FogComponent)
    {
        FogComponent->SetFogDensity(WeatherSettings.FogDensity);
        FogComponent->SetFogInscatteringColor(WeatherSettings.FogColor);
    }
    
    UpdateFogSettings();
}

void AWorld_WeatherSystem::UpdateFogSettings()
{
    if (!FogComponent)
        return;

    // Adjust fog settings based on weather type
    switch (WeatherSettings.WeatherType)
    {
        case EWorld_WeatherType::Foggy_Humid:
            FogComponent->SetFogHeightFalloff(0.1f);
            FogComponent->SetStartDistance(100.0f);
            break;
            
        case EWorld_WeatherType::Sandstorm_Dry:
            FogComponent->SetFogHeightFalloff(0.3f);
            FogComponent->SetStartDistance(500.0f);
            break;
            
        case EWorld_WeatherType::Snowy_Windy:
            FogComponent->SetFogHeightFalloff(0.15f);
            FogComponent->SetStartDistance(200.0f);
            break;
            
        default:
            FogComponent->SetFogHeightFalloff(0.2f);
            FogComponent->SetStartDistance(1000.0f);
            break;
    }
}

void AWorld_WeatherSystem::CreateWindEffects()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    // Clear existing wind markers
    for (APointLight* Marker : WindMarkers)
    {
        if (IsValid(Marker))
        {
            Marker->Destroy();
        }
    }
    WindMarkers.Empty();

    // Create new wind markers based on wind intensity
    int32 NumMarkers = FMath::RoundToInt(WeatherSettings.WindIntensity * 3.0f);
    for (int32 i = 0; i < NumMarkers; ++i)
    {
        FVector SpawnLocation = GetActorLocation() + FMath::VRand() * EffectRadius * 0.5f;
        SpawnLocation.Z = GetActorLocation().Z + FMath::RandRange(500.0f, 1500.0f);

        APointLight* WindMarker = World->SpawnActor<APointLight>(SpawnLocation, FRotator::ZeroRotator);
        if (WindMarker)
        {
            UPointLightComponent* LightComp = WindMarker->GetPointLightComponent();
            if (LightComp)
            {
                LightComp->SetIntensity(0.5f * WeatherSettings.WindIntensity);
                LightComp->SetLightColor(FLinearColor(0.3f, 0.8f, 1.0f, 1.0f));
                LightComp->SetAttenuationRadius(2000.0f);
                LightComp->SetCastShadows(false);
            }
            
            WindMarkers.Add(WindMarker);
        }
    }
}

FWorld_WeatherSettings AWorld_WeatherSystem::GetCurrentWeatherSettings() const
{
    return WeatherSettings;
}

bool AWorld_WeatherSystem::IsPlayerInWeatherZone(APawn* Player) const
{
    if (!Player)
        return false;

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= EffectRadius;
}

void AWorld_WeatherSystem::TransitionToWeather(EWorld_WeatherType TargetWeather, float TransitionTime)
{
    if (bIsTransitioning)
        return;

    StartWeatherSettings = WeatherSettings;
    TargetWeatherSettings = WeatherSettings;
    TargetWeatherSettings.WeatherType = TargetWeather;
    
    // Set target weather properties
    SetWeatherType(TargetWeather);
    TargetWeatherSettings = WeatherSettings;
    
    // Restore current settings for interpolation
    WeatherSettings = StartWeatherSettings;
    
    bIsTransitioning = true;
    CurrentTransitionTime = 0.0f;
    TargetTransitionTime = TransitionTime;
}

void AWorld_WeatherSystem::InitializeWeatherForBiome()
{
    WeatherSettings = GetDefaultSettingsForBiome(BiomeType);
    SetWeatherType(WeatherSettings.WeatherType);
}

void AWorld_WeatherSystem::UpdateDynamicWeather(float DeltaTime)
{
    // Simple dynamic weather variation
    static float WeatherTimer = 0.0f;
    WeatherTimer += DeltaTime * WeatherTransitionSpeed;
    
    float Variation = FMath::Sin(WeatherTimer * 0.1f) * 0.2f;
    
    // Apply subtle variations to current weather
    if (FogComponent)
    {
        float BaseDensity = WeatherSettings.FogDensity;
        float VariedDensity = BaseDensity + (BaseDensity * Variation);
        FogComponent->SetFogDensity(FMath::Max(0.001f, VariedDensity));
    }
}

FWorld_WeatherSettings AWorld_WeatherSystem::GetDefaultSettingsForBiome(EBiomeType Biome) const
{
    FWorld_WeatherSettings DefaultSettings;
    
    switch (Biome)
    {
        case EBiomeType::Savanna:
            DefaultSettings.WeatherType = EWorld_WeatherType::Clear_Hot;
            break;
        case EBiomeType::Swamp:
            DefaultSettings.WeatherType = EWorld_WeatherType::Foggy_Humid;
            break;
        case EBiomeType::Forest:
            DefaultSettings.WeatherType = EWorld_WeatherType::Rainy_Dense;
            break;
        case EBiomeType::Desert:
            DefaultSettings.WeatherType = EWorld_WeatherType::Sandstorm_Dry;
            break;
        case EBiomeType::Mountain:
            DefaultSettings.WeatherType = EWorld_WeatherType::Snowy_Windy;
            break;
        default:
            DefaultSettings.WeatherType = EWorld_WeatherType::Clear_Hot;
            break;
    }
    
    return DefaultSettings;
}