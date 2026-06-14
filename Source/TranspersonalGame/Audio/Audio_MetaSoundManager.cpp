#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Set default volumes for prehistoric atmosphere
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;  // Subtle prehistoric ambience
    DialogueVolume = 1.0f; // Clear tribal dialogue
    MusicVolume = 0.5f;    // Background tribal music
    SFXVolume = 0.8f;      // Impactful dinosaur/survival sounds
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize prehistoric audio setup
    InitializePrehistoricAudio();
    
    // Set up cleanup timer for finished audio components
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &AAudio_MetaSoundManager::CleanupFinishedComponents,
            5.0f,  // Every 5 seconds
            true   // Loop
        );
    }
}

void AAudio_MetaSoundManager::RegisterSound(const FString& SoundID, EAudio_SoundCategory Category, USoundBase* SoundAsset, 
                                          float Volume, float Pitch, bool bLooping)
{
    if (!SoundAsset || SoundID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Invalid sound registration - SoundID: %s"), *SoundID);
        return;
    }

    // Check if sound already registered
    FAudio_SoundEntry* ExistingEntry = FindSoundEntry(SoundID);
    if (ExistingEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Sound %s already registered, updating..."), *SoundID);
        ExistingEntry->SoundAsset = SoundAsset;
        ExistingEntry->Category = Category;
        ExistingEntry->Volume = Volume;
        ExistingEntry->Pitch = Pitch;
        ExistingEntry->bLooping = bLooping;
        return;
    }

    // Create new sound entry
    FAudio_SoundEntry NewEntry;
    NewEntry.SoundID = SoundID;
    NewEntry.Category = Category;
    NewEntry.SoundAsset = SoundAsset;
    NewEntry.Volume = Volume;
    NewEntry.Pitch = Pitch;
    NewEntry.bLooping = bLooping;

    RegisteredSounds.Add(NewEntry);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Registered sound %s in category %d"), 
           *SoundID, (int32)Category);
}

bool AAudio_MetaSoundManager::PlaySound(const FString& SoundID, FVector Location, bool bAttachToActor, AActor* ActorToAttachTo)
{
    FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundID);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Sound %s not found"), *SoundID);
        return false;
    }

    USoundBase* SoundAsset = SoundEntry->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Failed to load sound asset for %s"), *SoundID);
        return false;
    }

    // Stop existing instance if not looping
    if (ActiveAudioComponents.Contains(SoundID))
    {
        StopSound(SoundID, false);
    }

    // Create audio component
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SoundAsset,
        Location.IsZero() ? GetActorLocation() : Location,
        FRotator::ZeroRotator,
        GetVolumeForCategory(SoundEntry->Category) * SoundEntry->Volume * MasterVolume,
        SoundEntry->Pitch
    );

    if (!AudioComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_MetaSoundManager: Failed to spawn audio component for %s"), *SoundID);
        return false;
    }

    // Configure looping
    if (SoundEntry->bLooping)
    {
        AudioComp->SetUISound(false);
        AudioComp->bAutoDestroy = false;
    }

    // Attach to actor if requested
    if (bAttachToActor && ActorToAttachTo)
    {
        AudioComp->AttachToComponent(
            ActorToAttachTo->GetRootComponent(),
            FAttachmentTransformRules::KeepWorldTransform
        );
    }

    // Store reference
    ActiveAudioComponents.Add(SoundID, AudioComp);

    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing sound %s"), *SoundID);
    return true;
}

void AAudio_MetaSoundManager::StopSound(const FString& SoundID, bool bFadeOut)
{
    if (UAudioComponent** AudioCompPtr = ActiveAudioComponents.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        if (AudioComp && IsValid(AudioComp))
        {
            if (bFadeOut)
            {
                AudioComp->FadeOut(1.0f, 0.0f);
            }
            else
            {
                AudioComp->Stop();
            }
        }
        ActiveAudioComponents.Remove(SoundID);
    }
}

void AAudio_MetaSoundManager::StopAllSounds(EAudio_SoundCategory Category, bool bFadeOut)
{
    TArray<FString> SoundsToStop;
    
    for (const auto& Pair : ActiveAudioComponents)
    {
        FAudio_SoundEntry* Entry = FindSoundEntry(Pair.Key);
        if (Entry && Entry->Category == Category)
        {
            SoundsToStop.Add(Pair.Key);
        }
    }

    for (const FString& SoundID : SoundsToStop)
    {
        StopSound(SoundID, bFadeOut);
    }
}

void AAudio_MetaSoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    switch (Category)
    {
        case EAudio_SoundCategory::Ambient:
            AmbientVolume = Volume;
            break;
        case EAudio_SoundCategory::Dialogue:
            DialogueVolume = Volume;
            break;
        case EAudio_SoundCategory::Music:
            MusicVolume = Volume;
            break;
        case EAudio_SoundCategory::SFX:
            SFXVolume = Volume;
            break;
        case EAudio_SoundCategory::Narration:
            DialogueVolume = Volume; // Use dialogue volume for narration
            break;
    }

    // Update active sounds of this category
    for (const auto& Pair : ActiveAudioComponents)
    {
        FAudio_SoundEntry* Entry = FindSoundEntry(Pair.Key);
        if (Entry && Entry->Category == Category && Pair.Value && IsValid(Pair.Value))
        {
            float NewVolume = GetVolumeForCategory(Category) * Entry->Volume * MasterVolume;
            Pair.Value->SetVolumeMultiplier(NewVolume);
        }
    }
}

float AAudio_MetaSoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    switch (Category)
    {
        case EAudio_SoundCategory::Ambient:
            return AmbientVolume;
        case EAudio_SoundCategory::Dialogue:
            return DialogueVolume;
        case EAudio_SoundCategory::Music:
            return MusicVolume;
        case EAudio_SoundCategory::SFX:
            return SFXVolume;
        case EAudio_SoundCategory::Narration:
            return DialogueVolume;
        default:
            return 1.0f;
    }
}

void AAudio_MetaSoundManager::PlayDialogue(const FString& DialogueID, AActor* Speaker)
{
    FVector SpeakerLocation = Speaker ? Speaker->GetActorLocation() : GetActorLocation();
    PlaySound(DialogueID, SpeakerLocation, true, Speaker);
}

void AAudio_MetaSoundManager::PlayAmbientLoop(const FString& AmbientID, float FadeInTime)
{
    FAudio_SoundEntry* Entry = FindSoundEntry(AmbientID);
    if (Entry)
    {
        Entry->bLooping = true;
        Entry->FadeInTime = FadeInTime;
        PlaySound(AmbientID);
    }
}

void AAudio_MetaSoundManager::PlayNarration(const FString& NarrationID)
{
    // Stop any existing narration
    StopAllSounds(EAudio_SoundCategory::Narration, true);
    
    // Play new narration
    PlaySound(NarrationID);
}

bool AAudio_MetaSoundManager::IsSoundPlaying(const FString& SoundID) const
{
    if (UAudioComponent* const* AudioCompPtr = ActiveAudioComponents.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        return AudioComp && IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void AAudio_MetaSoundManager::InitializePrehistoricAudio()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Initializing prehistoric audio system"));
    
    // This would be where we register default prehistoric sounds
    // For now, we'll set up the system to be ready for sound registration
    
    // Clear any existing registrations
    RegisteredSounds.Empty();
    ActiveAudioComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Prehistoric audio system initialized"));
}

FAudio_SoundEntry* AAudio_MetaSoundManager::FindSoundEntry(const FString& SoundID)
{
    for (FAudio_SoundEntry& Entry : RegisteredSounds)
    {
        if (Entry.SoundID == SoundID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

float AAudio_MetaSoundManager::GetVolumeForCategory(EAudio_SoundCategory Category) const
{
    return GetCategoryVolume(Category);
}

void AAudio_MetaSoundManager::CleanupFinishedComponents()
{
    TArray<FString> ComponentsToRemove;
    
    for (const auto& Pair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComp = Pair.Value;
        if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            ComponentsToRemove.Add(Pair.Key);
        }
    }

    for (const FString& SoundID : ComponentsToRemove)
    {
        ActiveAudioComponents.Remove(SoundID);
    }
}