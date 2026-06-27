#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.25f; // Update 4× per second — sufficient for audio

    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    MusicCrossfadeSeconds = 2.0f;
    ThreatDetectionRadius = 500.0f;
    ScreenShakeBaseIntensity = 1.0f;
    ThreatUpdateTimer = 0.0f;
    WeatherRainIntensity = 0.0f;
    WeatherWindIntensity = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    UpdateAmbientLayers();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — periodic threat scan and ambient update
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ThreatUpdateTimer += DeltaTime;
    if (ThreatUpdateTimer >= 1.0f)
    {
        ThreatUpdateTimer = 0.0f;
        // Ambient volume modulation based on weather
        for (UAudioComponent* Comp : ActiveAmbientComponents)
        {
            if (Comp && Comp->IsActive())
            {
                float WeatherMod = FMath::Lerp(1.0f, 0.3f, WeatherRainIntensity);
                Comp->SetVolumeMultiplier(WeatherMod);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetBiome — transition ambient soundscape to new biome
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    CurrentBiome = NewBiome;
    UpdateAmbientLayers();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Biome changed to %d"), (int32)NewBiome);
}

// ─────────────────────────────────────────────────────────────────────────────
// SetThreatLevel — trigger adaptive music crossfade
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreat)
{
    if (NewThreat == CurrentThreatLevel)
    {
        return;
    }

    CrossfadeToThreatLevel(NewThreat);
    CurrentThreatLevel = NewThreat;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat level changed to %d"), (int32)NewThreat);
}

// ─────────────────────────────────────────────────────────────────────────────
// PlayFootstep — play a surface-appropriate footstep sound at location
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::PlayFootstep(FName SurfaceType, FVector Location, float Weight)
{
    for (const FAudio_FootstepConfig& Config : FootstepConfigs)
    {
        if (Config.SurfaceType == SurfaceType)
        {
            USoundBase* Sound = Config.FootstepSound.LoadSynchronous();
            if (Sound)
            {
                float PitchVariation = FMath::RandRange(1.0f - Config.PitchVariance, 1.0f + Config.PitchVariance);
                float Volume = Config.VolumeMultiplier * Weight;
                UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, Volume, PitchVariation);
            }
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// NotifyDinosaurNearby — escalate threat level based on proximity and mass
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::NotifyDinosaurNearby(float DistanceMeters, float DinosaurMass)
{
    // Large theropods (T-Rex ~8000kg) at close range = Combat threat
    // Smaller raptors (~80kg) at close range = Danger threat
    float ThreatScore = (DinosaurMass / 1000.0f) * (1.0f - FMath::Clamp(DistanceMeters / ThreatDetectionRadius, 0.0f, 1.0f));

    EAudio_ThreatLevel NewLevel = EAudio_ThreatLevel::Safe;
    if (ThreatScore > 8.0f)
    {
        NewLevel = EAudio_ThreatLevel::Combat;
    }
    else if (ThreatScore > 4.0f)
    {
        NewLevel = EAudio_ThreatLevel::Danger;
    }
    else if (ThreatScore > 1.0f)
    {
        NewLevel = EAudio_ThreatLevel::Cautious;
    }

    // Only escalate — never de-escalate via this path (de-escalation is time-based)
    if ((int32)NewLevel > (int32)CurrentThreatLevel)
    {
        SetThreatLevel(NewLevel);
    }

    // Screen shake for very heavy dinosaurs at close range
    if (DinosaurMass > 3000.0f && DistanceMeters < 50.0f)
    {
        float ShakeIntensity = FMath::Clamp((DinosaurMass / 8000.0f) * (1.0f - DistanceMeters / 50.0f), 0.0f, 1.0f);
        TriggerScreenShakeFromFootfall(ShakeIntensity);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerScreenShakeFromFootfall — apply camera shake to local player
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::TriggerScreenShakeFromFootfall(float Intensity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC)
    {
        return;
    }

    // Scale shake by base intensity setting
    float FinalIntensity = Intensity * ScreenShakeBaseIntensity;
    if (FinalIntensity < 0.01f)
    {
        return;
    }

    // Use built-in camera shake — Blueprint can override with a custom shake class
    PC->ClientStartCameraShake(UCameraShakeBase::StaticClass(), FinalIntensity);

    UE_LOG(LogTemp, Verbose, TEXT("AudioSystemManager: Screen shake triggered intensity=%.2f"), FinalIntensity);
}

// ─────────────────────────────────────────────────────────────────────────────
// SetWeatherIntensity — modulate ambient audio for weather conditions
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::SetWeatherIntensity(float RainIntensity, float WindIntensity, float ThunderProbability)
{
    WeatherRainIntensity = FMath::Clamp(RainIntensity, 0.0f, 1.0f);
    WeatherWindIntensity = FMath::Clamp(WindIntensity, 0.0f, 1.0f);

    // Probabilistic thunder — roll each call
    if (ThunderProbability > 0.0f)
    {
        float Roll = FMath::FRand();
        if (Roll < ThunderProbability)
        {
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Thunder event triggered"));
            // Blueprint event or MetaSound trigger would fire here
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateAmbientLayers — start/stop audio components for current biome
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::UpdateAmbientLayers()
{
    // Stop all current ambient components
    for (UAudioComponent* Comp : ActiveAmbientComponents)
    {
        if (Comp)
        {
            Comp->Stop();
            Comp->DestroyComponent();
        }
    }
    ActiveAmbientComponents.Empty();

    // Start layers matching the current biome
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Biome != CurrentBiome)
        {
            continue;
        }

        USoundBase* Sound = Layer.Sound.LoadSynchronous();
        if (!Sound)
        {
            continue;
        }

        UAudioComponent* Comp = NewObject<UAudioComponent>(this);
        if (!Comp)
        {
            continue;
        }

        Comp->SetSound(Sound);
        Comp->bAutoActivate = false;
        Comp->bIsUISound = false;
        Comp->RegisterComponent();
        Comp->SetVolumeMultiplier(GetVolumeForBiomeLayer(Layer));
        Comp->Play();

        ActiveAmbientComponents.Add(Comp);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updated ambient layers — %d active for biome %d"),
        ActiveAmbientComponents.Num(), (int32)CurrentBiome);
}

// ─────────────────────────────────────────────────────────────────────────────
// CrossfadeToThreatLevel — adjust music volume based on threat
// ─────────────────────────────────────────────────────────────────────────────
void AAudioSystemManager::CrossfadeToThreatLevel(EAudio_ThreatLevel NewLevel)
{
    // In a full implementation this would drive MetaSound parameters.
    // Here we log the transition for Blueprint/MetaSound to pick up.
    float ThreatNormalized = (float)((int32)NewLevel) / 4.0f; // 0..1 over 5 levels
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Crossfade to threat %.2f (level %d)"), ThreatNormalized, (int32)NewLevel);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetVolumeForBiomeLayer — compute volume accounting for weather
// ─────────────────────────────────────────────────────────────────────────────
float AAudioSystemManager::GetVolumeForBiomeLayer(const FAudio_AmbientLayer& Layer) const
{
    float Vol = Layer.BaseVolume;
    // Rain suppresses distant ambient sounds
    Vol *= FMath::Lerp(1.0f, 0.4f, WeatherRainIntensity);
    return FMath::Clamp(Vol, 0.0f, 2.0f);
}
