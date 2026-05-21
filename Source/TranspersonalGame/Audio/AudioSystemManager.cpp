#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAudio_AudioSystemManager::AAudio_AudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create primary audio component
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    RootComponent = PrimaryAudioComponent;

    // Initialize default settings
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    EffectsVolume = 1.0f;
}

void AAudio_AudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioLibrary();
    
    // Start ambient audio
    PlayAmbientAudio(EAudio_AudioType::Ambient);
}

void AAudio_AudioSystemManager::InitializeAudioLibrary()
{
    // Initialize default audio entries
    // These would be populated with actual sound assets in a real implementation
    
    FAudio_SoundEntry AmbientEntry;
    AmbientEntry.AudioType = EAudio_AudioType::Ambient;
    AmbientEntry.Volume = AmbientVolume;
    AmbientEntry.bShouldLoop = true;
    AudioLibrary.Add(AmbientEntry);
    
    FAudio_SoundEntry FootstepEntry;
    FootstepEntry.AudioType = EAudio_AudioType::Footsteps;
    FootstepEntry.Volume = EffectsVolume;
    FootstepEntry.bShouldLoop = false;
    AudioLibrary.Add(FootstepEntry);
    
    FAudio_SoundEntry DamageEntry;
    DamageEntry.AudioType = EAudio_AudioType::Damage;
    DamageEntry.Volume = EffectsVolume;
    DamageEntry.bShouldLoop = false;
    AudioLibrary.Add(DamageEntry);
    
    FAudio_SoundEntry DinosaurEntry;
    DinosaurEntry.AudioType = EAudio_AudioType::Dinosaur;
    DinosaurEntry.Volume = EffectsVolume * 1.2f;
    DinosaurEntry.bShouldLoop = false;
    AudioLibrary.Add(DinosaurEntry);
}

void AAudio_AudioSystemManager::PlayAudioByType(EAudio_AudioType AudioType, FVector Location)
{
    // Find the audio entry for this type
    FAudio_SoundEntry* FoundEntry = AudioLibrary.FindByPredicate([AudioType](const FAudio_SoundEntry& Entry)
    {
        return Entry.AudioType == AudioType;
    });
    
    if (!FoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio entry not found for type: %d"), (int32)AudioType);
        return;
    }
    
    // Stop existing audio of this type
    StopAudioByType(AudioType);
    
    // Create new audio component
    UAudioComponent* NewAudioComponent = CreateAudioComponent(*FoundEntry);
    if (NewAudioComponent)
    {
        NewAudioComponent->SetWorldLocation(Location);
        NewAudioComponent->Play();
        ActiveAudioComponents.Add(AudioType, NewAudioComponent);
    }
}

void AAudio_AudioSystemManager::StopAudioByType(EAudio_AudioType AudioType)
{
    UAudioComponent** FoundComponent = ActiveAudioComponents.Find(AudioType);
    if (FoundComponent && *FoundComponent)
    {
        (*FoundComponent)->Stop();
        (*FoundComponent)->DestroyComponent();
        ActiveAudioComponents.Remove(AudioType);
    }
}

void AAudio_AudioSystemManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (auto& AudioPair : ActiveAudioComponents)
    {
        if (AudioPair.Value)
        {
            AudioPair.Value->SetVolumeMultiplier(MasterVolume);
        }
    }
}

void AAudio_AudioSystemManager::PlayFootstepAudio(FVector FootstepLocation, float Intensity)
{
    // Adjust volume based on intensity
    float AdjustedVolume = EffectsVolume * Intensity * MasterVolume;
    
    // Play footstep sound at location
    PlayAudioByType(EAudio_AudioType::Footsteps, FootstepLocation);
    
    // Apply intensity-based modifications
    UAudioComponent** FoundComponent = ActiveAudioComponents.Find(EAudio_AudioType::Footsteps);
    if (FoundComponent && *FoundComponent)
    {
        (*FoundComponent)->SetVolumeMultiplier(AdjustedVolume);
        (*FoundComponent)->SetPitchMultiplier(0.8f + (Intensity * 0.4f)); // Vary pitch based on intensity
    }
}

void AAudio_AudioSystemManager::PlayDamageAudio(FVector DamageLocation)
{
    PlayAudioByType(EAudio_AudioType::Damage, DamageLocation);
    
    // Add screen shake effect trigger here if needed
    if (GetWorld())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            // Trigger camera shake for damage feedback
            PC->ClientStartCameraShake(nullptr, 1.0f);
        }
    }
}

void AAudio_AudioSystemManager::PlayAmbientAudio(EAudio_AudioType EnvironmentType)
{
    FVector AmbientLocation = GetActorLocation();
    PlayAudioByType(EnvironmentType, AmbientLocation);
}

UAudioComponent* AAudio_AudioSystemManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry)
{
    UAudioComponent* NewComponent = NewObject<UAudioComponent>(this);
    if (NewComponent)
    {
        NewComponent->SetupAttachment(RootComponent);
        
        // Configure audio component
        if (SoundEntry.SoundCue.IsValid())
        {
            NewComponent->SetSound(SoundEntry.SoundCue.Get());
        }
        
        NewComponent->SetVolumeMultiplier(SoundEntry.Volume * MasterVolume);
        NewComponent->SetPitchMultiplier(SoundEntry.Pitch);
        NewComponent->bAutoActivate = false;
        
        if (SoundEntry.bShouldLoop)
        {
            NewComponent->bAutoActivate = true;
        }
        
        NewComponent->RegisterComponent();
    }
    
    return NewComponent;
}