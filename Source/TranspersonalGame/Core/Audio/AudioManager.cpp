#include "AudioManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudioManager::UAudioManager()
{
    MasterVolume = 1.0f;
    MusicComponent = nullptr;
    AmbienceComponent = nullptr;
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Initializing prehistoric audio system"));
    
    InitializeCategoryVolumes();
    InitializeDefaultSounds();
    
    // Create persistent music and ambience components
    if (UWorld* World = GetWorld())
    {
        MusicComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        AmbienceComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        
        if (MusicComponent)
        {
            MusicComponent->bAutoDestroy = false;
        }
        
        if (AmbienceComponent)
        {
            AmbienceComponent->bAutoDestroy = false;
        }
    }
}

void UAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Shutting down audio system"));
    
    StopAllSounds();
    
    if (MusicComponent)
    {
        MusicComponent->Stop();
        MusicComponent = nullptr;
    }
    
    if (AmbienceComponent)
    {
        AmbienceComponent->Stop();
        AmbienceComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambience, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::DinosaurVocal, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::DinosaurMovement, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::PlayerAction, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.8f);
}

void UAudioManager::InitializeDefaultSounds()
{
    // Register default prehistoric sounds
    FAudio_SoundDefinition ForestAmbience;
    ForestAmbience.SoundName = TEXT("ForestAmbience");
    ForestAmbience.Category = EAudio_SoundCategory::Ambience;
    ForestAmbience.Volume = 0.6f;
    ForestAmbience.bLooping = true;
    ForestAmbience.bIs3D = false;
    RegisterSoundDefinition(ForestAmbience);
    
    FAudio_SoundDefinition TRexRoar;
    TRexRoar.SoundName = TEXT("TRexRoar");
    TRexRoar.Category = EAudio_SoundCategory::DinosaurVocal;
    TRexRoar.Volume = 1.0f;
    TRexRoar.MaxDistance = 10000.0f;
    TRexRoar.bIs3D = true;
    RegisterSoundDefinition(TRexRoar);
    
    FAudio_SoundDefinition HeavyFootsteps;
    HeavyFootsteps.SoundName = TEXT("HeavyFootsteps");
    HeavyFootsteps.Category = EAudio_SoundCategory::DinosaurMovement;
    HeavyFootsteps.Volume = 0.8f;
    HeavyFootsteps.MaxDistance = 5000.0f;
    HeavyFootsteps.bIs3D = true;
    RegisterSoundDefinition(HeavyFootsteps);
    
    FAudio_SoundDefinition PlayerFootsteps;
    PlayerFootsteps.SoundName = TEXT("PlayerFootsteps");
    PlayerFootsteps.Category = EAudio_SoundCategory::PlayerAction;
    PlayerFootsteps.Volume = 0.5f;
    PlayerFootsteps.MaxDistance = 1000.0f;
    PlayerFootsteps.bIs3D = true;
    RegisterSoundDefinition(PlayerFootsteps);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Registered %d default sound definitions"), SoundRegistry.Num());
}

void UAudioManager::PlaySound2D(const FString& SoundName, float VolumeMultiplier)
{
    if (!SoundRegistry.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Sound '%s' not found in registry"), *SoundName);
        return;
    }
    
    const FAudio_SoundDefinition& SoundDef = SoundRegistry[SoundName];
    
    if (UWorld* World = GetWorld())
    {
        if (SoundDef.SoundCue.IsValid())
        {
            float FinalVolume = CalculateFinalVolume(SoundDef, VolumeMultiplier);
            UGameplayStatics::PlaySound2D(World, SoundDef.SoundCue.Get(), FinalVolume, SoundDef.Pitch);
        }
    }
}

void UAudioManager::PlaySound3D(const FString& SoundName, const FVector& Location, float VolumeMultiplier)
{
    if (!SoundRegistry.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Sound '%s' not found in registry"), *SoundName);
        return;
    }
    
    const FAudio_SoundDefinition& SoundDef = SoundRegistry[SoundName];
    
    if (UWorld* World = GetWorld())
    {
        if (SoundDef.SoundCue.IsValid())
        {
            float FinalVolume = CalculateFinalVolume(SoundDef, VolumeMultiplier);
            
            if (SoundDef.bLooping)
            {
                // For looping sounds, create a persistent audio component
                UAudioComponent* AudioComp = CreateAudioComponent(SoundDef, Location);
                if (AudioComp)
                {
                    AudioComp->SetVolumeMultiplier(FinalVolume);
                    AudioComp->SetPitchMultiplier(SoundDef.Pitch);
                    AudioComp->Play();
                    
                    ActiveAudioComponents.Add(SoundName, AudioComp);
                }
            }
            else
            {
                // For one-shot sounds, use simple play at location
                UGameplayStatics::PlaySoundAtLocation(World, SoundDef.SoundCue.Get(), Location, 
                    FinalVolume, SoundDef.Pitch, 0.0f, nullptr, nullptr, true);
            }
        }
    }
}

void UAudioManager::StopSound(const FString& SoundName)
{
    if (ActiveAudioComponents.Contains(SoundName))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundName];
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->Stop();
        }
        ActiveAudioComponents.Remove(SoundName);
    }
}

void UAudioManager::StopAllSounds()
{
    for (auto& AudioPair : ActiveAudioComponents)
    {
        if (AudioPair.Value && AudioPair.Value->IsPlaying())
        {
            AudioPair.Value->Stop();
        }
    }
    ActiveAudioComponents.Empty();
}

void UAudioManager::UpdateMusicState(EAudio_ThreatLevel ThreatLevel, const FString& Biome, float TimeOfDay)
{
    CurrentMusicState.CurrentThreatLevel = ThreatLevel;
    CurrentMusicState.CurrentBiome = Biome;
    CurrentMusicState.TimeOfDay = TimeOfDay;
    
    UpdateDynamicMusic();
}

void UAudioManager::SetCombatMusic(bool bInCombat)
{
    CurrentMusicState.bInCombat = bInCombat;
    CurrentMusicState.IntensityLevel = bInCombat ? 1.0f : 0.0f;
    
    UpdateDynamicMusic();
}

void UAudioManager::FadeToMusic(const FString& MusicName, float FadeTime)
{
    if (!SoundRegistry.Contains(MusicName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Music '%s' not found"), *MusicName);
        return;
    }
    
    const FAudio_SoundDefinition& MusicDef = SoundRegistry[MusicName];
    
    if (MusicComponent && MusicDef.SoundCue.IsValid())
    {
        if (MusicComponent->IsPlaying())
        {
            MusicComponent->FadeOut(FadeTime, 0.0f);
        }
        
        MusicComponent->SetSound(MusicDef.SoundCue.Get());
        float FinalVolume = CalculateFinalVolume(MusicDef, 1.0f);
        MusicComponent->FadeIn(FadeTime, FinalVolume);
    }
}

void UAudioManager::SetAmbienceForBiome(EEng_BiomeType BiomeType)
{
    FString AmbienceName;
    
    switch (BiomeType)
    {
        case EEng_BiomeType::Forest:
            AmbienceName = TEXT("ForestAmbience");
            break;
        case EEng_BiomeType::Grassland:
            AmbienceName = TEXT("GrasslandAmbience");
            break;
        case EEng_BiomeType::Desert:
            AmbienceName = TEXT("DesertAmbience");
            break;
        case EEng_BiomeType::Swamp:
            AmbienceName = TEXT("SwampAmbience");
            break;
        case EEng_BiomeType::Mountains:
            AmbienceName = TEXT("MountainAmbience");
            break;
        default:
            AmbienceName = TEXT("ForestAmbience");
            break;
    }
    
    if (SoundRegistry.Contains(AmbienceName))
    {
        const FAudio_SoundDefinition& AmbienceDef = SoundRegistry[AmbienceName];
        
        if (AmbienceComponent && AmbienceDef.SoundCue.IsValid())
        {
            if (AmbienceComponent->IsPlaying())
            {
                AmbienceComponent->FadeOut(2.0f, 0.0f);
            }
            
            AmbienceComponent->SetSound(AmbienceDef.SoundCue.Get());
            float FinalVolume = CalculateFinalVolume(AmbienceDef, 1.0f);
            AmbienceComponent->FadeIn(2.0f, FinalVolume);
        }
    }
}

void UAudioManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, const FVector& Location)
{
    FString SoundName = DinosaurType + SoundType;
    PlaySound3D(SoundName, Location, 1.0f);
}

void UAudioManager::PlayFootstepSound(const FString& SurfaceType, const FVector& Location, bool bIsHeavy)
{
    FString SoundName = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
    SoundName += TEXT("Footstep") + SurfaceType;
    
    PlaySound3D(SoundName, Location, 1.0f);
}

void UAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudioManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (CategoryVolumes.Contains(Category))
    {
        return CategoryVolumes[Category];
    }
    return 1.0f;
}

bool UAudioManager::IsSoundPlaying(const FString& SoundName) const
{
    if (ActiveAudioComponents.Contains(SoundName))
    {
        const UAudioComponent* AudioComp = ActiveAudioComponents[SoundName];
        return AudioComp && AudioComp->IsPlaying();
    }
    return false;
}

void UAudioManager::RegisterSoundDefinition(const FAudio_SoundDefinition& SoundDef)
{
    SoundRegistry.Add(SoundDef.SoundName, SoundDef);
}

void UAudioManager::UpdateDynamicMusic()
{
    // Dynamic music logic based on threat level and context
    FString MusicName = TEXT("Ambient");
    
    if (CurrentMusicState.bInCombat)
    {
        MusicName = TEXT("Combat");
    }
    else
    {
        switch (CurrentMusicState.CurrentThreatLevel)
        {
            case EAudio_ThreatLevel::Safe:
                MusicName = TEXT("Peaceful");
                break;
            case EAudio_ThreatLevel::Cautious:
                MusicName = TEXT("Tension");
                break;
            case EAudio_ThreatLevel::Danger:
                MusicName = TEXT("Danger");
                break;
            case EAudio_ThreatLevel::Panic:
                MusicName = TEXT("Panic");
                break;
        }
    }
    
    // Add biome and time of day modifiers
    MusicName += CurrentMusicState.CurrentBiome;
    
    if (CurrentMusicState.TimeOfDay < 6.0f || CurrentMusicState.TimeOfDay > 20.0f)
    {
        MusicName += TEXT("Night");
    }
    
    FadeToMusic(MusicName, 3.0f);
}

UAudioComponent* UAudioManager::CreateAudioComponent(const FAudio_SoundDefinition& SoundDef, const FVector& Location)
{
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(World, SoundDef.SoundCue.Get());
        
        if (AudioComp)
        {
            AudioComp->bAutoDestroy = false;
            AudioComp->SetWorldLocation(Location);
            AudioComp->bAllowSpatialization = SoundDef.bIs3D;
            AudioComp->bOverrideAttenuation = true;
            AudioComp->AttenuationOverrides.bAttenuate = SoundDef.bIs3D;
            AudioComp->AttenuationOverrides.FalloffDistance = SoundDef.MaxDistance;
        }
        
        return AudioComp;
    }
    
    return nullptr;
}

float UAudioManager::CalculateFinalVolume(const FAudio_SoundDefinition& SoundDef, float VolumeMultiplier) const
{
    float CategoryVolume = GetCategoryVolume(SoundDef.Category);
    return SoundDef.Volume * VolumeMultiplier * CategoryVolume * MasterVolume;
}