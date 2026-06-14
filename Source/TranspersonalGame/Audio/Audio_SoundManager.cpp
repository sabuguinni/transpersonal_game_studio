#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    CurrentAmbientComponent = nullptr;
    CurrentMusicComponent = nullptr;
    CurrentAmbientSound = TEXT("");
    CurrentMusicTrack = TEXT("");
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCategoryVolumes();
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    
    RegisteredSounds.Empty();
    ActiveAudioComponents.Empty();
    CategoryVolumes.Empty();
    
    CurrentAmbientComponent = nullptr;
    CurrentMusicComponent = nullptr;
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.5f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::DinosaurSounds, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::WeatherSounds, 0.7f);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric ambient sounds
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundName = TEXT("ForestAmbient");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.7f;
    ForestAmbient.bLoop = true;
    ForestAmbient.FadeInTime = 2.0f;
    ForestAmbient.FadeOutTime = 2.0f;
    RegisterSound(ForestAmbient);

    // Register T-Rex footsteps
    FAudio_SoundEntry TRexFootsteps;
    TRexFootsteps.SoundName = TEXT("TRexFootsteps");
    TRexFootsteps.Category = EAudio_SoundCategory::DinosaurSounds;
    TRexFootsteps.Volume = 0.9f;
    TRexFootsteps.bLoop = false;
    RegisterSound(TRexFootsteps);

    // Register danger warning
    FAudio_SoundEntry DangerWarning;
    DangerWarning.SoundName = TEXT("DangerWarning");
    DangerWarning.Category = EAudio_SoundCategory::Voice;
    DangerWarning.Volume = 1.0f;
    DangerWarning.bLoop = false;
    RegisterSound(DangerWarning);

    // Register survival narration
    FAudio_SoundEntry SurvivalNarration;
    SurvivalNarration.SoundName = TEXT("SurvivalNarration");
    SurvivalNarration.Category = EAudio_SoundCategory::Voice;
    SurvivalNarration.Volume = 0.8f;
    SurvivalNarration.bLoop = false;
    RegisterSound(SurvivalNarration);
}

void UAudio_SoundManager::PlaySound(const FString& SoundName, AActor* WorldContext)
{
    if (!RegisteredSounds.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not registered: %s"), *SoundName);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundName];
    
    if (!SoundEntry.SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid sound asset for: %s"), *SoundName);
        return;
    }

    UAudioComponent* AudioComp = CreateAudioComponent(WorldContext);
    if (!AudioComp)
    {
        return;
    }

    AudioComp->SetSound(SoundEntry.SoundAsset.LoadSynchronous());
    AudioComp->SetVolumeMultiplier(CalculateFinalVolume(SoundEntry));
    AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
    AudioComp->bAutoDestroy = !SoundEntry.bLoop;

    if (SoundEntry.bLoop)
    {
        ActiveAudioComponents.Add(SoundName, AudioComp);
    }

    if (SoundEntry.FadeInTime > 0.0f)
    {
        AudioComp->FadeIn(SoundEntry.FadeInTime, CalculateFinalVolume(SoundEntry));
    }
    else
    {
        AudioComp->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("Playing sound: %s"), *SoundName);
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundName, FVector Location, AActor* WorldContext)
{
    if (!RegisteredSounds.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not registered: %s"), *SoundName);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundName];
    
    if (!SoundEntry.SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid sound asset for: %s"), *SoundName);
        return;
    }

    UWorld* World = WorldContext ? WorldContext->GetWorld() : GetWorld();
    if (!World)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        World,
        SoundEntry.SoundAsset.LoadSynchronous(),
        Location,
        CalculateFinalVolume(SoundEntry),
        SoundEntry.Pitch
    );

    UE_LOG(LogTemp, Log, TEXT("Playing sound at location: %s"), *SoundName);
}

void UAudio_SoundManager::StopSound(const FString& SoundName)
{
    if (ActiveAudioComponents.Contains(SoundName))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundName];
        if (IsValid(AudioComp))
        {
            const FAudio_SoundEntry* SoundEntry = RegisteredSounds.Find(SoundName);
            if (SoundEntry && SoundEntry->FadeOutTime > 0.0f)
            {
                AudioComp->FadeOut(SoundEntry->FadeOutTime, 0.0f);
            }
            else
            {
                AudioComp->Stop();
            }
        }
        ActiveAudioComponents.Remove(SoundName);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Stop();
        }
    }
    ActiveAudioComponents.Empty();

    if (IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent = nullptr;
    }

    if (IsValid(CurrentMusicComponent))
    {
        CurrentMusicComponent->Stop();
        CurrentMusicComponent = nullptr;
    }

    CurrentAmbientSound = TEXT("");
    CurrentMusicTrack = TEXT("");
}

void UAudio_SoundManager::StopSoundsByCategory(EAudio_SoundCategory Category)
{
    TArray<FString> SoundsToStop;
    
    for (const auto& Pair : ActiveAudioComponents)
    {
        if (RegisteredSounds.Contains(Pair.Key))
        {
            const FAudio_SoundEntry& SoundEntry = RegisteredSounds[Pair.Key];
            if (SoundEntry.Category == Category)
            {
                SoundsToStop.Add(Pair.Key);
            }
        }
    }

    for (const FString& SoundName : SoundsToStop)
    {
        StopSound(SoundName);
    }
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (const auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value) && RegisteredSounds.Contains(Pair.Key))
        {
            const FAudio_SoundEntry& SoundEntry = RegisteredSounds[Pair.Key];
            Pair.Value->SetVolumeMultiplier(CalculateFinalVolume(SoundEntry));
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update active sounds in this category
    for (const auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value) && RegisteredSounds.Contains(Pair.Key))
        {
            const FAudio_SoundEntry& SoundEntry = RegisteredSounds[Pair.Key];
            if (SoundEntry.Category == Category)
            {
                Pair.Value->SetVolumeMultiplier(CalculateFinalVolume(SoundEntry));
            }
        }
    }
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    const float* Volume = CategoryVolumes.Find(Category);
    return Volume ? *Volume : 1.0f;
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundEntry.SoundName, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundEntry.SoundName);
}

void UAudio_SoundManager::UnregisterSound(const FString& SoundName)
{
    StopSound(SoundName);
    RegisteredSounds.Remove(SoundName);
}

bool UAudio_SoundManager::IsSoundRegistered(const FString& SoundName) const
{
    return RegisteredSounds.Contains(SoundName);
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundName) const
{
    if (ActiveAudioComponents.Contains(SoundName))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundName];
        return IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::PlayAmbientSound(const FString& SoundName, float FadeInTime)
{
    if (CurrentAmbientSound == SoundName && IsValid(CurrentAmbientComponent) && CurrentAmbientComponent->IsPlaying())
    {
        return; // Already playing this ambient sound
    }

    // Stop current ambient sound
    if (IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->FadeOut(2.0f, 0.0f);
    }

    if (!RegisteredSounds.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Ambient sound not registered: %s"), *SoundName);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundName];
    
    CurrentAmbientComponent = CreateAudioComponent();
    if (!CurrentAmbientComponent)
    {
        return;
    }

    CurrentAmbientComponent->SetSound(SoundEntry.SoundAsset.LoadSynchronous());
    CurrentAmbientComponent->SetVolumeMultiplier(CalculateFinalVolume(SoundEntry));
    CurrentAmbientComponent->bAutoDestroy = false;
    
    CurrentAmbientComponent->FadeIn(FadeInTime, CalculateFinalVolume(SoundEntry));
    CurrentAmbientSound = SoundName;
}

void UAudio_SoundManager::StopAmbientSound(float FadeOutTime)
{
    if (IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->FadeOut(FadeOutTime, 0.0f);
        CurrentAmbientComponent = nullptr;
    }
    CurrentAmbientSound = TEXT("");
}

void UAudio_SoundManager::PlayMusic(const FString& MusicName, float FadeInTime)
{
    if (CurrentMusicTrack == MusicName && IsValid(CurrentMusicComponent) && CurrentMusicComponent->IsPlaying())
    {
        return; // Already playing this music
    }

    // Stop current music
    if (IsValid(CurrentMusicComponent))
    {
        CurrentMusicComponent->FadeOut(3.0f, 0.0f);
    }

    if (!RegisteredSounds.Contains(MusicName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Music not registered: %s"), *MusicName);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[MusicName];
    
    CurrentMusicComponent = CreateAudioComponent();
    if (!CurrentMusicComponent)
    {
        return;
    }

    CurrentMusicComponent->SetSound(SoundEntry.SoundAsset.LoadSynchronous());
    CurrentMusicComponent->SetVolumeMultiplier(CalculateFinalVolume(SoundEntry));
    CurrentMusicComponent->bAutoDestroy = false;
    
    CurrentMusicComponent->FadeIn(FadeInTime, CalculateFinalVolume(SoundEntry));
    CurrentMusicTrack = MusicName;
}

void UAudio_SoundManager::StopMusic(float FadeOutTime)
{
    if (IsValid(CurrentMusicComponent))
    {
        CurrentMusicComponent->FadeOut(FadeOutTime, 0.0f);
        CurrentMusicComponent = nullptr;
    }
    CurrentMusicTrack = TEXT("");
}

void UAudio_SoundManager::CrossfadeMusic(const FString& NewMusicName, float CrossfadeTime)
{
    // Start new music with fade in
    PlayMusic(NewMusicName, CrossfadeTime);
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(AActor* WorldContext)
{
    UWorld* World = nullptr;
    
    if (WorldContext)
    {
        World = WorldContext->GetWorld();
    }
    else
    {
        World = GetWorld();
    }
    
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world context for audio component creation"));
        return nullptr;
    }

    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create audio component"));
        return nullptr;
    }

    AudioComp->RegisterComponent();
    return AudioComp;
}

float UAudio_SoundManager::CalculateFinalVolume(const FAudio_SoundEntry& SoundEntry) const
{
    float CategoryVolume = GetCategoryVolume(SoundEntry.Category);
    return MasterVolume * CategoryVolume * SoundEntry.Volume;
}