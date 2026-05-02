#include "World_WeatherSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UWorld_WeatherSystem::UWorld_WeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize weather settings
    CurrentWeather = FWorld_WeatherSettings();
    TargetWeather = FWorld_WeatherSettings();
    
    bWeatherActive = false;
    WeatherTimer = 0.0f;
    TransitionTimer = 0.0f;
    TransitionDuration = 5.0f;
    
    // Initialize component pointers
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    HeightFog = nullptr;
    RainEffect = nullptr;
    SnowEffect = nullptr;
    SandstormEffect = nullptr;
    WeatherAudio = nullptr;
}

void UWorld_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find environment actors in the world
    InitializeEnvironmentReferences();
    
    // Setup biome weather presets
    SetupBiomeWeatherPresets();
    
    // Start with clear weather
    SetWeather(EWorld_WeatherType::Clear, 1.0f, 0.0f);
}

void UWorld_WeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bWeatherActive)
    {
        // Update weather timer
        WeatherTimer += DeltaTime;
        
        // Update transition if active
        if (TransitionTimer < TransitionDuration)
        {
            UpdateWeatherTransition(DeltaTime);
        }
        
        // Check if weather duration has expired
        if (WeatherTimer >= CurrentWeather.Duration && CurrentWeather.WeatherType != EWorld_WeatherType::Clear)
        {
            // Transition to clear weather
            SetWeather(EWorld_WeatherType::Clear, 1.0f, 5.0f);
        }
    }
}

void UWorld_WeatherSystem::SetWeather(EWorld_WeatherType NewWeather, float NewIntensity, float TransitionTime)
{
    // Set target weather
    TargetWeather = GetWeatherPreset(NewWeather);
    TargetWeather.Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    // Setup transition
    TransitionDuration = FMath::Max(TransitionTime, 0.1f);
    TransitionTimer = 0.0f;
    
    // Reset weather timer
    WeatherTimer = 0.0f;
    bWeatherActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Weather System: Transitioning to %s (Intensity: %.2f, Duration: %.1fs)"), 
           *UEnum::GetValueAsString(NewWeather), NewIntensity, TransitionTime);
}

void UWorld_WeatherSystem::StartRandomWeather()
{
    // Get random weather type (excluding Clear for variety)
    TArray<EWorld_WeatherType> WeatherTypes = {
        EWorld_WeatherType::Cloudy,
        EWorld_WeatherType::Rain,
        EWorld_WeatherType::Storm,
        EWorld_WeatherType::Fog
    };
    
    int32 RandomIndex = FMath::RandRange(0, WeatherTypes.Num() - 1);
    EWorld_WeatherType RandomWeather = WeatherTypes[RandomIndex];
    
    float RandomIntensity = FMath::FRandRange(0.3f, 1.0f);
    float RandomDuration = FMath::FRandRange(120.0f, 600.0f); // 2-10 minutes
    
    SetWeather(RandomWeather, RandomIntensity, 8.0f);
    TargetWeather.Duration = RandomDuration;
}

void UWorld_WeatherSystem::ClearWeather(float TransitionTime)
{
    SetWeather(EWorld_WeatherType::Clear, 1.0f, TransitionTime);
}

void UWorld_WeatherSystem::SetBiomeWeatherPreferences(EBiomeType BiomeType)
{
    if (BiomeWeatherPresets.Contains(BiomeType))
    {
        const TArray<EWorld_WeatherType>& PreferredWeather = BiomeWeatherPresets[BiomeType];
        if (PreferredWeather.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, PreferredWeather.Num() - 1);
            EWorld_WeatherType SelectedWeather = PreferredWeather[RandomIndex];
            
            float BiomeIntensity = 1.0f;
            // Adjust intensity based on biome
            switch (BiomeType)
            {
                case EBiomeType::Desert:
                    BiomeIntensity = FMath::FRandRange(0.7f, 1.0f);
                    break;
                case EBiomeType::Swamp:
                    BiomeIntensity = FMath::FRandRange(0.5f, 0.9f);
                    break;
                case EBiomeType::SnowyMountain:
                    BiomeIntensity = FMath::FRandRange(0.6f, 1.0f);
                    break;
                default:
                    BiomeIntensity = FMath::FRandRange(0.4f, 0.8f);
                    break;
            }
            
            SetWeather(SelectedWeather, BiomeIntensity, 6.0f);
        }
    }
}

void UWorld_WeatherSystem::UpdateWeatherForBiome(EBiomeType BiomeType)
{
    // Only update if current weather is clear or low intensity
    if (CurrentWeather.WeatherType == EWorld_WeatherType::Clear || CurrentWeather.Intensity < 0.3f)
    {
        SetBiomeWeatherPreferences(BiomeType);
    }
}

void UWorld_WeatherSystem::InitializeEnvironmentReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("Weather System: Found DirectionalLight"));
    }
    
    // Find sky atmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("Weather System: Found SkyAtmosphere"));
    }
    
    // Find height fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("Weather System: Found ExponentialHeightFog"));
    }
    
    // Create weather effect components
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Rain effect component
        RainEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainEffect"));
        if (RainEffect)
        {
            RainEffect->SetupAttachment(Owner->GetRootComponent());
            RainEffect->SetAutoActivate(false);
        }
        
        // Snow effect component
        SnowEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SnowEffect"));
        if (SnowEffect)
        {
            SnowEffect->SetupAttachment(Owner->GetRootComponent());
            SnowEffect->SetAutoActivate(false);
        }
        
        // Sandstorm effect component
        SandstormEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SandstormEffect"));
        if (SandstormEffect)
        {
            SandstormEffect->SetupAttachment(Owner->GetRootComponent());
            SandstormEffect->SetAutoActivate(false);
        }
        
        // Weather audio component
        WeatherAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudio"));
        if (WeatherAudio)
        {
            WeatherAudio->SetupAttachment(Owner->GetRootComponent());
            WeatherAudio->SetAutoActivate(false);
        }
    }
}

void UWorld_WeatherSystem::UpdateWeatherTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    // Interpolate weather settings
    FWorld_WeatherSettings InterpolatedSettings;
    InterpolatedSettings.WeatherType = (Alpha > 0.5f) ? TargetWeather.WeatherType : CurrentWeather.WeatherType;
    InterpolatedSettings.Intensity = FMath::Lerp(CurrentWeather.Intensity, TargetWeather.Intensity, Alpha);
    InterpolatedSettings.SunIntensity = FMath::Lerp(CurrentWeather.SunIntensity, TargetWeather.SunIntensity, Alpha);
    InterpolatedSettings.SunColor = FMath::Lerp(CurrentWeather.SunColor, TargetWeather.SunColor, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(CurrentWeather.FogDensity, TargetWeather.FogDensity, Alpha);
    InterpolatedSettings.FogColor = FMath::Lerp(CurrentWeather.FogColor, TargetWeather.FogColor, Alpha);
    InterpolatedSettings.AmbientVolume = FMath::Lerp(CurrentWeather.AmbientVolume, TargetWeather.AmbientVolume, Alpha);
    
    // Apply interpolated settings
    ApplyWeatherSettings(InterpolatedSettings);
    
    // Complete transition
    if (Alpha >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        TransitionTimer = TransitionDuration;
        UpdateWeatherEffects();
        UpdateWeatherAudio();
    }
}

void UWorld_WeatherSystem::ApplyWeatherSettings(const FWorld_WeatherSettings& Settings)
{
    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity * Settings.Intensity);
        LightComp->SetLightColor(Settings.SunColor);
    }
    
    // Update fog
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity * Settings.Intensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void UWorld_WeatherSystem::UpdateWeatherEffects()
{
    // Deactivate all effects first
    if (RainEffect) RainEffect->Deactivate();
    if (SnowEffect) SnowEffect->Deactivate();
    if (SandstormEffect) SandstormEffect->Deactivate();
    
    // Activate appropriate effect based on weather
    switch (CurrentWeather.WeatherType)
    {
        case EWorld_WeatherType::Rain:
        case EWorld_WeatherType::Storm:
            if (RainEffect) RainEffect->Activate();
            break;
            
        case EWorld_WeatherType::Snow:
            if (SnowEffect) SnowEffect->Activate();
            break;
            
        case EWorld_WeatherType::Sandstorm:
            if (SandstormEffect) SandstormEffect->Activate();
            break;
            
        default:
            // No particle effects for other weather types
            break;
    }
}

void UWorld_WeatherSystem::UpdateWeatherAudio()
{
    if (!WeatherAudio) return;
    
    // Stop current audio
    WeatherAudio->Stop();
    
    // Set volume based on intensity
    float Volume = CurrentWeather.AmbientVolume * CurrentWeather.Intensity;
    WeatherAudio->SetVolumeMultiplier(Volume);
    
    // Play weather-appropriate audio
    if (Volume > 0.1f && CurrentWeather.WeatherType != EWorld_WeatherType::Clear)
    {
        WeatherAudio->Play();
    }
}

FWorld_WeatherSettings UWorld_WeatherSystem::GetWeatherPreset(EWorld_WeatherType WeatherType)
{
    FWorld_WeatherSettings Preset;
    Preset.WeatherType = WeatherType;
    
    switch (WeatherType)
    {
        case EWorld_WeatherType::Clear:
            Preset.SunIntensity = 3.0f;
            Preset.SunColor = FLinearColor::White;
            Preset.FogDensity = 0.02f;
            Preset.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
            Preset.AmbientVolume = 0.2f;
            Preset.Duration = 600.0f; // 10 minutes
            break;
            
        case EWorld_WeatherType::Cloudy:
            Preset.SunIntensity = 1.5f;
            Preset.SunColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            Preset.FogDensity = 0.04f;
            Preset.FogColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
            Preset.AmbientVolume = 0.3f;
            Preset.Duration = 480.0f; // 8 minutes
            break;
            
        case EWorld_WeatherType::Rain:
            Preset.SunIntensity = 0.8f;
            Preset.SunColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
            Preset.FogDensity = 0.08f;
            Preset.FogColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
            Preset.AmbientVolume = 0.7f;
            Preset.Duration = 300.0f; // 5 minutes
            break;
            
        case EWorld_WeatherType::Storm:
            Preset.SunIntensity = 0.3f;
            Preset.SunColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
            Preset.FogDensity = 0.12f;
            Preset.FogColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
            Preset.AmbientVolume = 1.0f;
            Preset.Duration = 180.0f; // 3 minutes
            break;
            
        case EWorld_WeatherType::Fog:
            Preset.SunIntensity = 0.5f;
            Preset.SunColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
            Preset.FogDensity = 0.25f;
            Preset.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            Preset.AmbientVolume = 0.4f;
            Preset.Duration = 420.0f; // 7 minutes
            break;
            
        case EWorld_WeatherType::Sandstorm:
            Preset.SunIntensity = 0.4f;
            Preset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            Preset.FogDensity = 0.15f;
            Preset.FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
            Preset.AmbientVolume = 0.8f;
            Preset.Duration = 240.0f; // 4 minutes
            break;
            
        case EWorld_WeatherType::Snow:
            Preset.SunIntensity = 1.0f;
            Preset.SunColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
            Preset.FogDensity = 0.06f;
            Preset.FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
            Preset.AmbientVolume = 0.5f;
            Preset.Duration = 360.0f; // 6 minutes
            break;
    }
    
    return Preset;
}

EWorld_WeatherType UWorld_WeatherSystem::GetRandomWeatherForBiome(EBiomeType BiomeType)
{
    if (BiomeWeatherPresets.Contains(BiomeType))
    {
        const TArray<EWorld_WeatherType>& WeatherTypes = BiomeWeatherPresets[BiomeType];
        if (WeatherTypes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, WeatherTypes.Num() - 1);
            return WeatherTypes[RandomIndex];
        }
    }
    
    return EWorld_WeatherType::Clear;
}

void UWorld_WeatherSystem::SetupBiomeWeatherPresets()
{
    // Forest biome - moderate weather
    BiomeWeatherPresets.Add(EBiomeType::Forest, {
        EWorld_WeatherType::Clear,
        EWorld_WeatherType::Cloudy,
        EWorld_WeatherType::Rain,
        EWorld_WeatherType::Fog
    });
    
    // Savanna biome - hot and dry
    BiomeWeatherPresets.Add(EBiomeType::Savanna, {
        EWorld_WeatherType::Clear,
        EWorld_WeatherType::Cloudy,
        EWorld_WeatherType::Storm
    });
    
    // Desert biome - extreme heat and sandstorms
    BiomeWeatherPresets.Add(EBiomeType::Desert, {
        EWorld_WeatherType::Clear,
        EWorld_WeatherType::Sandstorm
    });
    
    // Swamp biome - humid and foggy
    BiomeWeatherPresets.Add(EBiomeType::Swamp, {
        EWorld_WeatherType::Cloudy,
        EWorld_WeatherType::Rain,
        EWorld_WeatherType::Fog,
        EWorld_WeatherType::Storm
    });
    
    // Snowy Mountain biome - cold weather
    BiomeWeatherPresets.Add(EBiomeType::SnowyMountain, {
        EWorld_WeatherType::Clear,
        EWorld_WeatherType::Cloudy,
        EWorld_WeatherType::Snow,
        EWorld_WeatherType::Storm
    });
}