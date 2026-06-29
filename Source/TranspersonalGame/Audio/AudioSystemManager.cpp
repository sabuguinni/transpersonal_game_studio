#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager Implementation
// ============================================================

UAudio_AudioSystemManager::UAudio_AudioSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for audio state checks
}

void UAudio_AudioSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize with safe defaults
    CurrentBiomeZone = EAudio_BiomeZone::OpenPlain;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentWeatherState = EAudio_WeatherState::Clear;
    TensionReleaseTimer = 0.0f;
    bPredatorTracked = false;
    LastHealthValue = 100.0f;
    HeartbeatTimer = 0.0f;
    HeartbeatInterval = 1.5f;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Initialized — Biome: OpenPlain, Danger: Safe"));
}

void UAudio_AudioSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickHeartbeat(DeltaTime);
    TickTensionRelease(DeltaTime);
}

// ---- Biome & Ambient ----

void UAudio_AudioSystemManager::SetBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentBiomeZone == NewZone) return;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Biome transition: %d -> %d"),
        (int32)CurrentBiomeZone, (int32)NewZone);

    CurrentBiomeZone = NewZone;
    UpdateAmbientLayerVolumes();
}

void UAudio_AudioSystemManager::SetWeatherState(EAudio_WeatherState NewWeather)
{
    if (CurrentWeatherState == NewWeather) return;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Weather transition: %d -> %d"),
        (int32)CurrentWeatherState, (int32)NewWeather);

    CurrentWeatherState = NewWeather;

    // Storm weather overrides biome ambient with heavy rain
    if (NewWeather == EAudio_WeatherState::Storm || NewWeather == EAudio_WeatherState::HeavyRain)
    {
        // Reduce ambient wildlife sounds — animals go quiet in storms
        AmbientVolume = 0.2f;
    }
    else
    {
        AmbientVolume = 0.8f;
    }

    UpdateAmbientLayerVolumes();
}

void UAudio_AudioSystemManager::UpdateAmbientLayerVolumes()
{
    for (FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        bool bShouldBeActive = (Layer.BiomeZone == CurrentBiomeZone);
        if (bShouldBeActive != Layer.bIsActive)
        {
            Layer.bIsActive = bShouldBeActive;
            UE_LOG(LogTemp, Verbose, TEXT("[AudioSystemManager] Layer %s: %s"),
                Layer.SoundAsset ? *Layer.SoundAsset->GetName() : TEXT("NULL"),
                bShouldBeActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
        }
    }
}

// ---- Danger / Music Tension ----

void UAudio_AudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger level: %d -> %d"),
        (int32)CurrentDangerLevel, (int32)NewLevel);

    CurrentDangerLevel = NewLevel;

    // Adjust music volume based on danger
    switch (NewLevel)
    {
        case EAudio_DangerLevel::Safe:
            MusicVolume = 0.3f;
            break;
        case EAudio_DangerLevel::Uneasy:
            MusicVolume = 0.5f;
            break;
        case EAudio_DangerLevel::Threat:
            MusicVolume = 0.7f;
            break;
        case EAudio_DangerLevel::Imminent:
            MusicVolume = 0.9f;
            break;
        case EAudio_DangerLevel::Combat:
            MusicVolume = 1.0f;
            break;
    }
}

void UAudio_AudioSystemManager::NotifyPredatorNearby(FName SpeciesName, float Distance)
{
    bPredatorTracked = true;
    TensionReleaseTimer = 0.0f;

    // Scale danger level by distance
    if (Distance < 500.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Imminent);
    }
    else if (Distance < 1000.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Threat);
    }
    else if (Distance < PredatorTensionRadius)
    {
        SetDangerLevel(EAudio_DangerLevel::Uneasy);
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Predator nearby: %s at %.0f units"),
        *SpeciesName.ToString(), Distance);
}

void UAudio_AudioSystemManager::NotifyPredatorLost()
{
    if (!bPredatorTracked) return;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Predator lost — starting tension release timer (%.1fs)"),
        TensionReleaseDelay);

    // Don't immediately relax — start countdown
    bPredatorTracked = false;
    TensionReleaseTimer = TensionReleaseDelay;
}

void UAudio_AudioSystemManager::TickTensionRelease(float DeltaTime)
{
    if (TensionReleaseTimer <= 0.0f) return;

    TensionReleaseTimer -= DeltaTime;

    if (TensionReleaseTimer <= 0.0f)
    {
        TensionReleaseTimer = 0.0f;
        SetDangerLevel(EAudio_DangerLevel::Safe);
        UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Tension released — returning to Safe"));
    }
}

// ---- Dinosaur Sounds ----

void UAudio_AudioSystemManager::PlayDinoSound(FName SpeciesName, FName SoundType, FVector Location)
{
    FAudio_DinoSoundProfile* Profile = FindDinoProfile(SpeciesName);
    if (!Profile) return;

    USoundBase* SoundToPlay = nullptr;

    if (SoundType == FName("Idle"))        SoundToPlay = Profile->IdleSound;
    else if (SoundType == FName("Alert"))  SoundToPlay = Profile->AlertSound;
    else if (SoundType == FName("Attack")) SoundToPlay = Profile->AttackSound;
    else if (SoundType == FName("Death"))  SoundToPlay = Profile->DeathSound;

    if (SoundToPlay && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, Location, SFXVolume);
    }
}

void UAudio_AudioSystemManager::TriggerDinoFootstep(FName SpeciesName, FVector Location, float Mass)
{
    FAudio_DinoSoundProfile* Profile = FindDinoProfile(SpeciesName);
    if (!Profile) return;

    if (Profile->FootstepSound && GetWorld())
    {
        float VolumeScale = FMath::Clamp(Mass / 5000.0f, 0.1f, 2.0f);
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Profile->FootstepSound, Location, VolumeScale * SFXVolume);
    }

    // Screen shake for heavy dinos (T-Rex, Brachiosaurus)
    if (Profile->FootstepShakeMagnitude > 0.5f)
    {
        float DistanceToPlayer = 9999.0f;
        if (GetWorld())
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC && PC->GetPawn())
            {
                DistanceToPlayer = FVector::Dist(PC->GetPawn()->GetActorLocation(), Location);
            }
        }

        // Only shake if within 1500 units
        if (DistanceToPlayer < 1500.0f)
        {
            float ShakeFalloff = 1.0f - (DistanceToPlayer / 1500.0f);
            TriggerScreenShake(Profile->FootstepShakeMagnitude * ShakeFalloff, 0.3f);
        }
    }
}

// ---- Survival Feedback ----

void UAudio_AudioSystemManager::UpdateSurvivalStats(float Health, float Hunger, float Thirst, float Stamina)
{
    LastHealthValue = Health;

    // Adjust heartbeat speed based on health
    if (Health < 25.0f)
    {
        HeartbeatInterval = 0.6f; // Fast, panicked
    }
    else if (Health < 50.0f)
    {
        HeartbeatInterval = 1.0f; // Moderate
    }
    else
    {
        HeartbeatInterval = 99999.0f; // No heartbeat when healthy
    }

    // Hunger audio cue
    if (Hunger < 20.0f && SurvivalSounds.HungerGrowl && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurvivalSounds.HungerGrowl,
            GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector, SFXVolume);
    }

    // Thirst audio cue
    if (Thirst < 15.0f && SurvivalSounds.ThirstGasp && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurvivalSounds.ThirstGasp,
            GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector, SFXVolume);
    }

    // Stamina exhausted
    if (Stamina < 5.0f && SurvivalSounds.StaminaExhausted && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurvivalSounds.StaminaExhausted,
            GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector, SFXVolume);
    }
}

void UAudio_AudioSystemManager::PlayDamageImpact(float DamageAmount)
{
    if (!SurvivalSounds.DamageImpact || !GetWorld()) return;

    float VolumeScale = FMath::Clamp(DamageAmount / 50.0f, 0.3f, 1.5f);
    FVector Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurvivalSounds.DamageImpact, Location, VolumeScale * SFXVolume);

    // Screen flash feedback — trigger small shake on damage
    TriggerScreenShake(VolumeScale * 0.5f, 0.2f);
}

void UAudio_AudioSystemManager::PlayCraftingSound(FName CraftingType)
{
    if (!SurvivalSounds.CraftingStone || !GetWorld()) return;

    FVector Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurvivalSounds.CraftingStone, Location, SFXVolume);

    UE_LOG(LogTemp, Verbose, TEXT("[AudioSystemManager] Crafting sound: %s"), *CraftingType.ToString());
}

void UAudio_AudioSystemManager::PlayFireSound(bool bIgniting)
{
    USoundBase* FireSound = bIgniting ? SurvivalSounds.FireIgnite : SurvivalSounds.FireExtinguish;
    if (!FireSound || !GetWorld()) return;

    FVector Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, Location, SFXVolume);
}

// ---- Screen Shake ----

void UAudio_AudioSystemManager::TriggerScreenShake(float Magnitude, float Duration)
{
    if (!GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // Use built-in camera shake via console command as a lightweight approach
    // Full implementation would use UCameraShakeBase subclass
    FString ShakeCmd = FString::Printf(TEXT("shake %.2f %.2f"), Magnitude, Duration);
    UE_LOG(LogTemp, Verbose, TEXT("[AudioSystemManager] Screen shake: magnitude=%.2f duration=%.2f"),
        Magnitude, Duration);
}

// ---- Heartbeat Tick ----

void UAudio_AudioSystemManager::TickHeartbeat(float DeltaTime)
{
    if (HeartbeatInterval > 9999.0f) return; // No heartbeat needed

    HeartbeatTimer += DeltaTime;
    if (HeartbeatTimer >= HeartbeatInterval)
    {
        HeartbeatTimer = 0.0f;

        if (SurvivalSounds.LowHealthHeartbeat && GetWorld())
        {
            FVector Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurvivalSounds.LowHealthHeartbeat,
                Location, SFXVolume * (1.0f - (LastHealthValue / 100.0f)));
        }
    }
}

// ---- Helpers ----

FAudio_DinoSoundProfile* UAudio_AudioSystemManager::FindDinoProfile(FName SpeciesName)
{
    for (FAudio_DinoSoundProfile& Profile : DinoSoundProfiles)
    {
        if (Profile.DinoSpecies == SpeciesName)
        {
            return &Profile;
        }
    }
    return nullptr;
}
