#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UAudio_SoundManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultSounds();
}

void UAudio_SoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished audio components
    TArray<FString> ToRemove;
    for (auto& Pair : ActiveAudioComponents)
    {
        if (!IsValid(Pair.Value) || !Pair.Value->IsPlaying())
        {
            ToRemove.Add(Pair.Key);
        }
    }
    
    for (const FString& Key : ToRemove)
    {
        if (UAudioComponent* AudioComp = ActiveAudioComponents.FindRef(Key))
        {
            AudioComp->DestroyComponent();
        }
        ActiveAudioComponents.Remove(Key);
    }
}

void UAudio_SoundManager::PlaySound(const FString& SoundName, FVector Location, AActor* AttachToActor)
{
    FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound '%s' not found or invalid"), *SoundName);
        return;
    }

    USoundCue* SoundCue = SoundEntry->SoundCue.LoadSynchronous();
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Failed to load sound cue for '%s'"), *SoundName);
        return;
    }

    // Stop existing instance if playing
    if (ActiveAudioComponents.Contains(SoundName))
    {
        StopSound(SoundName);
    }

    UAudioComponent* AudioComponent = nullptr;
    
    if (SoundEntry->bIs3D)
    {
        if (AttachToActor)
        {
            AudioComponent = UGameplayStatics::SpawnSoundAttached(
                SoundCue, 
                AttachToActor->GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                EAttachLocation::KeepRelativeOffset,
                false
            );
        }
        else
        {
            AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
                GetWorld(),
                SoundCue,
                Location
            );
        }
    }
    else
    {
        AudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), SoundCue);
    }

    if (AudioComponent)
    {
        float FinalVolume = SoundEntry->Volume * GetCategoryVolume(SoundEntry->Category) * MasterVolume;
        AudioComponent->SetVolumeMultiplier(FinalVolume);
        AudioComponent->SetPitchMultiplier(SoundEntry->Pitch);
        
        ActiveAudioComponents.Add(SoundName, AudioComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing sound '%s' at volume %.2f"), *SoundName, FinalVolume);
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundName)
{
    if (UAudioComponent* AudioComp = ActiveAudioComponents.FindRef(SoundName))
    {
        AudioComp->Stop();
        AudioComp->DestroyComponent();
        ActiveAudioComponents.Remove(SoundName);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Stopped sound '%s'"), *SoundName);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Stop();
            Pair.Value->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Stopped all sounds"));
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    switch (Category)
    {
        case EAudio_SoundCategory::Ambient:
            AmbientVolume = Volume;
            break;
        case EAudio_SoundCategory::Dinosaur:
            DinosaurVolume = Volume;
            break;
        case EAudio_SoundCategory::Player:
            PlayerVolume = Volume;
            break;
        default:
            break;
    }
    
    // Update volume for currently playing sounds of this category
    for (auto& Pair : ActiveAudioComponents)
    {
        FAudio_SoundEntry* Entry = FindSoundEntry(Pair.Key);
        if (Entry && Entry->Category == Category && IsValid(Pair.Value))
        {
            float FinalVolume = Entry->Volume * GetCategoryVolume(Category) * MasterVolume;
            Pair.Value->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void UAudio_SoundManager::RegisterSound(const FString& SoundName, USoundCue* SoundCue, EAudio_SoundCategory Category)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Cannot register null sound cue for '%s'"), *SoundName);
        return;
    }

    FAudio_SoundEntry NewEntry;
    NewEntry.SoundName = SoundName;
    NewEntry.SoundCue = SoundCue;
    NewEntry.Category = Category;
    
    // Remove existing entry if it exists
    SoundLibrary.RemoveAll([&SoundName](const FAudio_SoundEntry& Entry) {
        return Entry.SoundName == SoundName;
    });
    
    SoundLibrary.Add(NewEntry);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Registered sound '%s'"), *SoundName);
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundName)
{
    if (UAudioComponent* AudioComp = ActiveAudioComponents.FindRef(SoundName))
    {
        return IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Initialize default prehistoric sound entries
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundName = TEXT("TRexRoar");
    TRexRoar.Category = EAudio_SoundCategory::Dinosaur;
    TRexRoar.Volume = 1.0f;
    TRexRoar.bIs3D = true;
    SoundLibrary.Add(TRexRoar);

    FAudio_SoundEntry RaptorCall;
    RaptorCall.SoundName = TEXT("RaptorCall");
    RaptorCall.Category = EAudio_SoundCategory::Dinosaur;
    RaptorCall.Volume = 0.8f;
    RaptorCall.bIs3D = true;
    SoundLibrary.Add(RaptorCall);

    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundName = TEXT("ForestAmbient");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.6f;
    ForestAmbient.bIs3D = false;
    SoundLibrary.Add(ForestAmbient);

    FAudio_SoundEntry PlayerFootsteps;
    PlayerFootsteps.SoundName = TEXT("PlayerFootsteps");
    PlayerFootsteps.Category = EAudio_SoundCategory::Player;
    PlayerFootsteps.Volume = 0.5f;
    PlayerFootsteps.bIs3D = true;
    SoundLibrary.Add(PlayerFootsteps);

    FAudio_SoundEntry StoneCrafting;
    StoneCrafting.SoundName = TEXT("StoneCrafting");
    StoneCrafting.Category = EAudio_SoundCategory::Player;
    StoneCrafting.Volume = 0.7f;
    StoneCrafting.bIs3D = true;
    SoundLibrary.Add(StoneCrafting);

    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Initialized %d default sounds"), SoundLibrary.Num());
}

FAudio_SoundEntry* UAudio_SoundManager::FindSoundEntry(const FString& SoundName)
{
    for (FAudio_SoundEntry& Entry : SoundLibrary)
    {
        if (Entry.SoundName == SoundName)
        {
            return &Entry;
        }
    }
    return nullptr;
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category)
{
    switch (Category)
    {
        case EAudio_SoundCategory::Ambient:
            return AmbientVolume;
        case EAudio_SoundCategory::Dinosaur:
            return DinosaurVolume;
        case EAudio_SoundCategory::Player:
            return PlayerVolume;
        default:
            return 1.0f;
    }
}