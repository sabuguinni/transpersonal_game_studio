#include "World_BiomeWeatherSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UWorld_BiomeWeatherSystem::UWorld_BiomeWeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default biome weather preferences
    BiomeWeatherProbabilities.Add("Savana", 0.7f); // Clear weather preference
    BiomeWeatherProbabilities.Add("Floresta", 0.4f); // More rain likely
    BiomeWeatherProbabilities.Add("Deserto", 0.9f); // Very clear, occasional sandstorm
    BiomeWeatherProbabilities.Add("Pantano", 0.2f); // Foggy and rainy
    BiomeWeatherProbabilities.Add("Montanha", 0.3f); // Snow and storms

    // Initialize current settings
    CurrentSettings = FWorld_WeatherSettings();
    TargetSettings = CurrentSettings;
}

void UWorld_BiomeWeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find environment actors in the world
    FindEnvironmentActors();

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(CurrentSettings.AmbientVolume);
    }

    // Apply initial weather settings
    ApplyWeatherToEnvironment();
    UpdateAmbientAudio();

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeWeatherSystem: Initialized with weather %d at time %.1f"), 
           (int32)CurrentWeather, CurrentTime);
}

void UWorld_BiomeWeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update day/night cycle
    UpdateDayNightCycle(DeltaTime);

    // Handle weather transitions
    if (WeatherTransitionTimer > 0.0f)
    {
        WeatherTransitionTimer -= DeltaTime;
        float Alpha = 1.0f - (WeatherTransitionTimer / 5.0f); // 5 second transition
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
        
        InterpolateWeatherSettings(Alpha);
        ApplyWeatherToEnvironment();

        if (WeatherTransitionTimer <= 0.0f)
        {
            CurrentWeather = TargetWeather;
            CurrentSettings = TargetSettings;
        }
    }

    // Check for automatic weather changes
    if (bAutoWeatherChanges)
    {
        NextWeatherCheckTimer -= DeltaTime;
        if (NextWeatherCheckTimer <= 0.0f)
        {
            NextWeatherCheckTimer = 60.0f; // Check every minute
            
            float RandomValue = FMath::RandRange(0.0f, 1.0f);
            if (RandomValue < WeatherChangeChance)
            {
                TriggerWeatherChange();
            }
        }
    }
}

void UWorld_BiomeWeatherSystem::SetWeather(EWorld_WeatherType NewWeather, float TransitionTime)
{
    if (NewWeather == CurrentWeather)
        return;

    TargetWeather = NewWeather;
    TargetSettings = GetWeatherSettings(NewWeather);
    WeatherTransitionTimer = TransitionTime;

    // Spawn new weather effects
    SpawnWeatherEffect(NewWeather);

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeWeatherSystem: Transitioning to weather %d over %.1f seconds"), 
           (int32)NewWeather, TransitionTime);
}

void UWorld_BiomeWeatherSystem::SetTimeOfDay(EWorld_TimeOfDay NewTime, bool bInstant)
{
    CurrentTimeOfDay = NewTime;
    
    // Convert time of day to hour
    float TargetHour = 12.0f;
    switch (NewTime)
    {
        case EWorld_TimeOfDay::Dawn: TargetHour = 6.0f; break;
        case EWorld_TimeOfDay::Morning: TargetHour = 9.0f; break;
        case EWorld_TimeOfDay::Noon: TargetHour = 12.0f; break;
        case EWorld_TimeOfDay::Afternoon: TargetHour = 15.0f; break;
        case EWorld_TimeOfDay::Dusk: TargetHour = 18.0f; break;
        case EWorld_TimeOfDay::Night: TargetHour = 21.0f; break;
        case EWorld_TimeOfDay::Midnight: TargetHour = 0.0f; break;
    }

    if (bInstant)
    {
        CurrentTime = TargetHour;
    }
    else
    {
        // Smooth transition over 30 seconds
        // This would need a separate timer system for time transitions
        CurrentTime = TargetHour;
    }

    UpdateSunPosition();
}

void UWorld_BiomeWeatherSystem::SetCurrentTime(float NewTime)
{
    CurrentTime = FMath::Fmod(NewTime, 24.0f);
    if (CurrentTime < 0.0f) CurrentTime += 24.0f;

    CurrentTimeOfDay = GetTimeOfDayFromHour(CurrentTime);
    UpdateSunPosition();
}

FWorld_WeatherSettings UWorld_BiomeWeatherSystem::GetCurrentWeatherSettings() const
{
    return CurrentSettings;
}

void UWorld_BiomeWeatherSystem::ApplyWeatherToEnvironment()
{
    UpdateSunPosition();
    UpdateFogSettings();
    UpdateSkyAtmosphere();
    UpdatePostProcessing();
}

void UWorld_BiomeWeatherSystem::UpdateDayNightCycle(float DeltaTime)
{
    if (DayDurationMinutes <= 0.0f) return;

    // Convert real minutes to game hours
    float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
    CurrentTime += HoursPerSecond * DeltaTime;

    // Wrap around 24 hours
    if (CurrentTime >= 24.0f)
    {
        CurrentTime -= 24.0f;
    }

    // Update time of day enum
    EWorld_TimeOfDay NewTimeOfDay = GetTimeOfDayFromHour(CurrentTime);
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        UE_LOG(LogTemp, Log, TEXT("World_BiomeWeatherSystem: Time of day changed to %d at hour %.1f"), 
               (int32)CurrentTimeOfDay, CurrentTime);
    }

    UpdateSunPosition();
}

void UWorld_BiomeWeatherSystem::TriggerWeatherChange()
{
    // Select random weather based on current biome
    TArray<EWorld_WeatherType> PossibleWeathers = {
        EWorld_WeatherType::Clear,
        EWorld_WeatherType::Overcast,
        EWorld_WeatherType::LightRain,
        EWorld_WeatherType::Fog
    };

    // Add biome-specific weather
    // This is simplified - in a full implementation, we'd check the current biome
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        PossibleWeathers.Add(EWorld_WeatherType::HeavyRain);
        PossibleWeathers.Add(EWorld_WeatherType::Storm);
    }

    int32 RandomIndex = FMath::RandRange(0, PossibleWeathers.Num() - 1);
    EWorld_WeatherType NewWeather = PossibleWeathers[RandomIndex];

    if (NewWeather != CurrentWeather)
    {
        SetWeather(NewWeather, 5.0f);
    }
}

void UWorld_BiomeWeatherSystem::SetBiomeWeatherPreferences(const FString& BiomeName, const TMap<EWorld_WeatherType, float>& Preferences)
{
    // Store biome preferences for weather selection
    // This is a simplified implementation
    float ClearWeatherChance = 0.5f;
    if (Preferences.Contains(EWorld_WeatherType::Clear))
    {
        ClearWeatherChance = Preferences[EWorld_WeatherType::Clear];
    }
    
    BiomeWeatherProbabilities.Add(BiomeName, ClearWeatherChance);
}

EWorld_WeatherType UWorld_BiomeWeatherSystem::SelectBiomeAppropriateWeather(const FString& BiomeName) const
{
    float ClearChance = 0.5f;
    if (BiomeWeatherProbabilities.Contains(BiomeName))
    {
        ClearChance = BiomeWeatherProbabilities[BiomeName];
    }

    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    
    if (RandomValue < ClearChance)
    {
        return EWorld_WeatherType::Clear;
    }
    else if (BiomeName == "Deserto")
    {
        return FMath::RandBool() ? EWorld_WeatherType::Sandstorm : EWorld_WeatherType::Clear;
    }
    else if (BiomeName == "Montanha")
    {
        return FMath::RandBool() ? EWorld_WeatherType::Snow : EWorld_WeatherType::Overcast;
    }
    else if (BiomeName == "Pantano")
    {
        return FMath::RandBool() ? EWorld_WeatherType::Fog : EWorld_WeatherType::LightRain;
    }
    else
    {
        return FMath::RandBool() ? EWorld_WeatherType::LightRain : EWorld_WeatherType::Overcast;
    }
}

void UWorld_BiomeWeatherSystem::SpawnWeatherEffect(EWorld_WeatherType WeatherType)
{
    // Clear existing effects
    ClearWeatherEffects();

    UNiagaraSystem* EffectToSpawn = nullptr;
    
    switch (WeatherType)
    {
        case EWorld_WeatherType::LightRain:
        case EWorld_WeatherType::HeavyRain:
        case EWorld_WeatherType::Storm:
            EffectToSpawn = RainEffect;
            break;
        case EWorld_WeatherType::Snow:
            EffectToSpawn = SnowEffect;
            break;
        case EWorld_WeatherType::Sandstorm:
            EffectToSpawn = SandstormEffect;
            break;
        default:
            break;
    }

    if (EffectToSpawn && GetOwner())
    {
        ActiveWeatherEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            EffectToSpawn,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepWorldPosition,
            true
        );

        if (ActiveWeatherEffect)
        {
            // Adjust effect intensity based on weather type
            float Intensity = 1.0f;
            if (WeatherType == EWorld_WeatherType::LightRain) Intensity = 0.5f;
            else if (WeatherType == EWorld_WeatherType::HeavyRain) Intensity = 1.5f;
            else if (WeatherType == EWorld_WeatherType::Storm) Intensity = 2.0f;

            ActiveWeatherEffect->SetFloatParameter("Intensity", Intensity);
        }
    }
}

void UWorld_BiomeWeatherSystem::ClearWeatherEffects()
{
    if (ActiveWeatherEffect)
    {
        ActiveWeatherEffect->DestroyComponent();
        ActiveWeatherEffect = nullptr;
    }
}

void UWorld_BiomeWeatherSystem::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent) return;

    USoundCue* SoundToPlay = nullptr;
    
    switch (CurrentWeather)
    {
        case EWorld_WeatherType::LightRain:
        case EWorld_WeatherType::HeavyRain:
        case EWorld_WeatherType::Storm:
            SoundToPlay = RainAmbientSound;
            break;
        case EWorld_WeatherType::Sandstorm:
        case EWorld_WeatherType::Fog:
            SoundToPlay = WindAmbientSound;
            break;
        default:
            break;
    }

    if (SoundToPlay)
    {
        AmbientAudioComponent->SetSound(SoundToPlay);
        AmbientAudioComponent->SetVolumeMultiplier(CurrentSettings.AmbientVolume);
        if (!AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Play();
        }
    }
    else
    {
        AmbientAudioComponent->Stop();
    }
}

void UWorld_BiomeWeatherSystem::FindEnvironmentActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find fog actor
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Find sky atmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find post process volume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void UWorld_BiomeWeatherSystem::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent()) return;

    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTime - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle * 0.5f)) * 90.0f;
    
    // Clamp elevation to prevent sun going below horizon too much
    SunElevation = FMath::Clamp(SunElevation, -20.0f, 90.0f);

    FRotator NewRotation = FRotator(-SunElevation, SunAngle, 0.0f);
    SunLight->SetActorRotation(NewRotation);

    // Adjust sun intensity based on elevation
    float IntensityMultiplier = FMath::Clamp((SunElevation + 20.0f) / 110.0f, 0.1f, 1.0f);
    SunLight->GetLightComponent()->SetIntensity(CurrentSettings.SunIntensity * IntensityMultiplier);

    // Adjust sun color based on time of day
    FLinearColor SunColor = CurrentSettings.SunColor;
    if (SunElevation < 10.0f) // Sunrise/sunset
    {
        SunColor = FLinearColor(1.0f, 0.6f, 0.3f); // Orange/red tint
    }
    SunLight->GetLightComponent()->SetLightColor(SunColor);
}

void UWorld_BiomeWeatherSystem::UpdateFogSettings()
{
    if (!FogActor || !FogActor->GetComponent()) return;

    UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
    FogComponent->SetFogDensity(CurrentSettings.FogDensity);
    FogComponent->SetFogInscatteringColor(CurrentSettings.FogColor);
}

void UWorld_BiomeWeatherSystem::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphere || !SkyAtmosphere->GetAtmosphereComponent()) return;

    // Update sky atmosphere based on weather
    // This is a simplified implementation
    USkyAtmosphereComponent* AtmosphereComponent = SkyAtmosphere->GetAtmosphereComponent();
    // Additional sky atmosphere updates would go here
}

void UWorld_BiomeWeatherSystem::UpdatePostProcessing()
{
    if (!PostProcessVolume || !PostProcessVolume->GetComponent()) return;

    // Update post-processing effects based on weather
    // This is a simplified implementation
    UPostProcessComponent* PPComponent = PostProcessVolume->GetComponent();
    // Additional post-processing updates would go here
}

FWorld_WeatherSettings UWorld_BiomeWeatherSystem::GetWeatherSettings(EWorld_WeatherType WeatherType) const
{
    FWorld_WeatherSettings Settings;
    
    switch (WeatherType)
    {
        case EWorld_WeatherType::Clear:
            Settings.SunIntensity = 3.0f;
            Settings.FogDensity = 0.01f;
            Settings.SunColor = FLinearColor::White;
            Settings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
            Settings.AmbientVolume = 0.3f;
            break;
            
        case EWorld_WeatherType::Overcast:
            Settings.SunIntensity = 1.5f;
            Settings.FogDensity = 0.05f;
            Settings.SunColor = FLinearColor(0.8f, 0.8f, 0.9f);
            Settings.FogColor = FLinearColor(0.7f, 0.7f, 0.8f);
            Settings.AmbientVolume = 0.4f;
            break;
            
        case EWorld_WeatherType::LightRain:
            Settings.SunIntensity = 1.0f;
            Settings.FogDensity = 0.08f;
            Settings.SunColor = FLinearColor(0.6f, 0.7f, 0.8f);
            Settings.FogColor = FLinearColor(0.6f, 0.6f, 0.7f);
            Settings.RainIntensity = 0.5f;
            Settings.AmbientVolume = 0.6f;
            break;
            
        case EWorld_WeatherType::HeavyRain:
            Settings.SunIntensity = 0.5f;
            Settings.FogDensity = 0.12f;
            Settings.SunColor = FLinearColor(0.4f, 0.5f, 0.6f);
            Settings.FogColor = FLinearColor(0.4f, 0.4f, 0.5f);
            Settings.RainIntensity = 1.0f;
            Settings.AmbientVolume = 0.8f;
            break;
            
        case EWorld_WeatherType::Storm:
            Settings.SunIntensity = 0.3f;
            Settings.FogDensity = 0.15f;
            Settings.SunColor = FLinearColor(0.3f, 0.3f, 0.4f);
            Settings.FogColor = FLinearColor(0.2f, 0.2f, 0.3f);
            Settings.RainIntensity = 1.5f;
            Settings.WindStrength = 2.0f;
            Settings.AmbientVolume = 1.0f;
            break;
            
        case EWorld_WeatherType::Fog:
            Settings.SunIntensity = 0.8f;
            Settings.FogDensity = 0.25f;
            Settings.SunColor = FLinearColor(0.9f, 0.9f, 0.8f);
            Settings.FogColor = FLinearColor(0.8f, 0.8f, 0.7f);
            Settings.AmbientVolume = 0.2f;
            break;
            
        case EWorld_WeatherType::Sandstorm:
            Settings.SunIntensity = 0.4f;
            Settings.FogDensity = 0.3f;
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f);
            Settings.FogColor = FLinearColor(0.8f, 0.6f, 0.4f);
            Settings.WindStrength = 3.0f;
            Settings.AmbientVolume = 0.7f;
            break;
            
        case EWorld_WeatherType::Snow:
            Settings.SunIntensity = 1.2f;
            Settings.FogDensity = 0.06f;
            Settings.SunColor = FLinearColor(0.9f, 0.9f, 1.0f);
            Settings.FogColor = FLinearColor(0.9f, 0.9f, 1.0f);
            Settings.AmbientVolume = 0.3f;
            break;
    }
    
    return Settings;
}

EWorld_TimeOfDay UWorld_BiomeWeatherSystem::GetTimeOfDayFromHour(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f) return EWorld_TimeOfDay::Dawn;
    else if (Hour >= 7.0f && Hour < 11.0f) return EWorld_TimeOfDay::Morning;
    else if (Hour >= 11.0f && Hour < 14.0f) return EWorld_TimeOfDay::Noon;
    else if (Hour >= 14.0f && Hour < 17.0f) return EWorld_TimeOfDay::Afternoon;
    else if (Hour >= 17.0f && Hour < 20.0f) return EWorld_TimeOfDay::Dusk;
    else if (Hour >= 20.0f && Hour < 23.0f) return EWorld_TimeOfDay::Night;
    else return EWorld_TimeOfDay::Midnight;
}

void UWorld_BiomeWeatherSystem::InterpolateWeatherSettings(float Alpha)
{
    // Interpolate between current and target settings
    CurrentSettings.SunIntensity = FMath::Lerp(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, Alpha);
    CurrentSettings.FogDensity = FMath::Lerp(CurrentSettings.FogDensity, TargetSettings.FogDensity, Alpha);
    CurrentSettings.RainIntensity = FMath::Lerp(CurrentSettings.RainIntensity, TargetSettings.RainIntensity, Alpha);
    CurrentSettings.WindStrength = FMath::Lerp(CurrentSettings.WindStrength, TargetSettings.WindStrength, Alpha);
    CurrentSettings.AmbientVolume = FMath::Lerp(CurrentSettings.AmbientVolume, TargetSettings.AmbientVolume, Alpha);
    
    CurrentSettings.SunColor = FMath::Lerp(CurrentSettings.SunColor, TargetSettings.SunColor, Alpha);
    CurrentSettings.FogColor = FMath::Lerp(CurrentSettings.FogColor, TargetSettings.FogColor, Alpha);
    CurrentSettings.SkyColor = FMath::Lerp(CurrentSettings.SkyColor, TargetSettings.SkyColor, Alpha);
}