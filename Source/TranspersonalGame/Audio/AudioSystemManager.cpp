#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentBiome = EBiomeType::Savana;
    CurrentAmbienceComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing audio subsystem"));
    
    InitializeAudioSystem();
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->Stop();
        CurrentAmbienceComponent = nullptr;
    }
    
    ActiveAudioComponents.Empty();
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioSystem()
{
    LoadDefaultSounds();
    SetupBiomeConfigs();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio system initialized with %d sounds and %d biome configs"), 
           RegisteredSounds.Num(), BiomeAmbienceConfigs.Num());
}

void UAudioSystemManager::UpdateAudioSystem(float DeltaTime)
{
    // Clean up finished audio components
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveAudioComponents[i]) || !ActiveAudioComponents[i]->IsPlaying())
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* UAudioSystemManager::PlaySound2D(const FString& SoundName, float VolumeMultiplier)
{
    FAudio_SoundConfig* SoundConfig = FindSoundConfig(SoundName);
    if (!SoundConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound '%s' not found"), *SoundName);
        return nullptr;
    }

    if (USoundWave* SoundWave = SoundConfig->SoundAsset.LoadSynchronous())
    {
        UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(
            GetWorld(), 
            SoundWave, 
            SoundConfig->VolumeMultiplier * VolumeMultiplier,
            SoundConfig->PitchMultiplier
        );
        
        if (AudioComp)
        {
            ActiveAudioComponents.Add(AudioComp);
            AudioComp->Play();
            return AudioComp;
        }
    }
    
    return nullptr;
}

UAudioComponent* UAudioSystemManager::PlaySound3D(const FString& SoundName, const FVector& Location, float VolumeMultiplier)
{
    FAudio_SoundConfig* SoundConfig = FindSoundConfig(SoundName);
    if (!SoundConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound '%s' not found"), *SoundName);
        return nullptr;
    }

    if (USoundWave* SoundWave = SoundConfig->SoundAsset.LoadSynchronous())
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            SoundWave,
            Location,
            FRotator::ZeroRotator,
            SoundConfig->VolumeMultiplier * VolumeMultiplier,
            SoundConfig->PitchMultiplier,
            0.0f,
            nullptr,
            nullptr,
            true
        );
        
        if (AudioComp)
        {
            ActiveAudioComponents.Add(AudioComp);
            return AudioComp;
        }
    }
    
    return nullptr;
}

void UAudioSystemManager::StopSound(UAudioComponent* AudioComponent)
{
    if (IsValid(AudioComponent))
    {
        AudioComponent->Stop();
        ActiveAudioComponents.Remove(AudioComponent);
    }
}

void UAudioSystemManager::StopAllSounds()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    ActiveAudioComponents.Empty();
}

void UAudioSystemManager::SetCurrentBiome(EBiomeType NewBiome, const FVector& PlayerLocation)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Changing biome from %d to %d"), 
               (int32)CurrentBiome, (int32)NewBiome);
        
        CrossfadeToBiome(NewBiome);
        CurrentBiome = NewBiome;
    }
}

void UAudioSystemManager::UpdateBiomeAmbience(const FVector& PlayerLocation)
{
    // This would typically check distance to biome centers and crossfade
    // For now, we'll use a simple implementation
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        // Adjust volume based on movement or other factors
        float BaseVolume = 0.5f;
        CurrentAmbienceComponent->SetVolumeMultiplier(BaseVolume);
    }
}

void UAudioSystemManager::PlayFootstepSound(const FVector& Location, ESurfaceType SurfaceType, float MovementSpeed)
{
    FString SoundName;
    
    switch (SurfaceType)
    {
        case ESurfaceType::Grass:
            SoundName = TEXT("Footstep_Grass");
            break;
        case ESurfaceType::Stone:
            SoundName = TEXT("Footstep_Stone");
            break;
        case ESurfaceType::Mud:
            SoundName = TEXT("Footstep_Mud");
            break;
        case ESurfaceType::Sand:
            SoundName = TEXT("Footstep_Sand");
            break;
        default:
            SoundName = TEXT("Footstep_Default");
            break;
    }
    
    // Adjust volume and pitch based on movement speed
    float VolumeMultiplier = FMath::Clamp(MovementSpeed / 600.0f, 0.3f, 1.0f);
    float PitchMultiplier = FMath::Clamp(0.8f + (MovementSpeed / 1000.0f), 0.8f, 1.2f);
    
    PlaySound3D(SoundName, Location, VolumeMultiplier);
}

void UAudioSystemManager::PlayDamageSound(float DamageAmount, const FVector& ImpactLocation)
{
    FString SoundName;
    
    if (DamageAmount > 50.0f)
    {
        SoundName = TEXT("Damage_Heavy");
    }
    else if (DamageAmount > 20.0f)
    {
        SoundName = TEXT("Damage_Medium");
    }
    else
    {
        SoundName = TEXT("Damage_Light");
    }
    
    float VolumeMultiplier = FMath::Clamp(DamageAmount / 100.0f, 0.5f, 1.0f);
    PlaySound3D(SoundName, ImpactLocation, VolumeMultiplier);
}

void UAudioSystemManager::PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, const FVector& Location)
{
    FString SoundName = FString::Printf(TEXT("Dinosaur_%s_%s"), 
                                       *UEnum::GetValueAsString(Species), 
                                       *SoundType);
    
    PlaySound3D(SoundName, Location, 1.0f);
}

void UAudioSystemManager::TriggerScreenShakeAudio(float Intensity, const FVector& SourceLocation)
{
    FString SoundName;
    
    if (Intensity > 2.0f)
    {
        SoundName = TEXT("ScreenShake_Heavy");
    }
    else if (Intensity > 1.0f)
    {
        SoundName = TEXT("ScreenShake_Medium");
    }
    else
    {
        SoundName = TEXT("ScreenShake_Light");
    }
    
    PlaySound3D(SoundName, SourceLocation, Intensity);
}

void UAudioSystemManager::LoadDefaultSounds()
{
    // Register default sound configurations
    RegisteredSounds.Empty();
    
    // Footstep sounds
    FAudio_SoundConfig FootstepGrass;
    FootstepGrass.SoundName = TEXT("Footstep_Grass");
    FootstepGrass.VolumeMultiplier = 0.7f;
    FootstepGrass.bIs3D = true;
    FootstepGrass.AttenuationDistance = 500.0f;
    RegisteredSounds.Add(FootstepGrass);
    
    FAudio_SoundConfig FootstepStone;
    FootstepStone.SoundName = TEXT("Footstep_Stone");
    FootstepStone.VolumeMultiplier = 0.8f;
    FootstepStone.bIs3D = true;
    FootstepStone.AttenuationDistance = 800.0f;
    RegisteredSounds.Add(FootstepStone);
    
    // Damage sounds
    FAudio_SoundConfig DamageHeavy;
    DamageHeavy.SoundName = TEXT("Damage_Heavy");
    DamageHeavy.VolumeMultiplier = 1.0f;
    DamageHeavy.bIs3D = false;
    RegisteredSounds.Add(DamageHeavy);
    
    // Screen shake sounds
    FAudio_SoundConfig ScreenShakeHeavy;
    ScreenShakeHeavy.SoundName = TEXT("ScreenShake_Heavy");
    ScreenShakeHeavy.VolumeMultiplier = 0.9f;
    ScreenShakeHeavy.bIs3D = true;
    ScreenShakeHeavy.AttenuationDistance = 2000.0f;
    RegisteredSounds.Add(ScreenShakeHeavy);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Loaded %d default sounds"), RegisteredSounds.Num());
}

void UAudioSystemManager::SetupBiomeConfigs()
{
    BiomeAmbienceConfigs.Empty();
    
    // Savana biome
    FAudio_BiomeAmbience SavanaAmbience;
    SavanaAmbience.BiomeType = EBiomeType::Savana;
    SavanaAmbience.CrossfadeTime = 3.0f;
    SavanaAmbience.BiomeRadius = 10000.0f;
    BiomeAmbienceConfigs.Add(SavanaAmbience);
    
    // Forest biome
    FAudio_BiomeAmbience ForestAmbience;
    ForestAmbience.BiomeType = EBiomeType::Floresta;
    ForestAmbience.CrossfadeTime = 2.0f;
    ForestAmbience.BiomeRadius = 8000.0f;
    BiomeAmbienceConfigs.Add(ForestAmbience);
    
    // Desert biome
    FAudio_BiomeAmbience DesertAmbience;
    DesertAmbience.BiomeType = EBiomeType::Deserto;
    DesertAmbience.CrossfadeTime = 4.0f;
    DesertAmbience.BiomeRadius = 12000.0f;
    BiomeAmbienceConfigs.Add(DesertAmbience);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Setup %d biome ambience configs"), BiomeAmbienceConfigs.Num());
}

FAudio_SoundConfig* UAudioSystemManager::FindSoundConfig(const FString& SoundName)
{
    for (FAudio_SoundConfig& Config : RegisteredSounds)
    {
        if (Config.SoundName == SoundName)
        {
            return &Config;
        }
    }
    return nullptr;
}

void UAudioSystemManager::CrossfadeToBiome(EBiomeType NewBiome)
{
    // Stop current ambience
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->FadeOut(2.0f, 0.0f);
        CurrentAmbienceComponent = nullptr;
    }
    
    // Find new biome config
    for (const FAudio_BiomeAmbience& BiomeConfig : BiomeAmbienceConfigs)
    {
        if (BiomeConfig.BiomeType == NewBiome && BiomeConfig.AmbientSounds.Num() > 0)
        {
            // Start new ambience (would need actual sound assets)
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Starting ambience for biome %d"), (int32)NewBiome);
            break;
        }
    }
}

float UAudioSystemManager::CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation)
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    float MaxDistance = 2000.0f; // Default max audible distance
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    return 1.0f - (Distance / MaxDistance);
}