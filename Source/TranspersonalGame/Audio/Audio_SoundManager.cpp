#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    CurrentAmbientAudio = nullptr;
    CurrentBiome = TEXT("");
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
    
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        CurrentAmbientAudio->Stop();
        CurrentAmbientAudio = nullptr;
    }
    
    RegisteredSounds.Empty();
    PlayingSounds.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.6f);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric sounds
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundName = TEXT("TRex_Roar");
    TRexRoar.Category = EAudio_SoundCategory::Dinosaur;
    TRexRoar.Priority = EAudio_Priority::High;
    TRexRoar.Volume = 1.0f;
    TRexRoar.b3D = true;
    TRexRoar.MaxDistance = 5000.0f;
    RegisteredSounds.Add(TRexRoar.SoundName, TRexRoar);
    
    FAudio_SoundEntry RaptorCall;
    RaptorCall.SoundName = TEXT("Raptor_Call");
    RaptorCall.Category = EAudio_SoundCategory::Dinosaur;
    RaptorCall.Priority = EAudio_Priority::High;
    RaptorCall.Volume = 0.8f;
    RaptorCall.b3D = true;
    RaptorCall.MaxDistance = 3000.0f;
    RegisteredSounds.Add(RaptorCall.SoundName, RaptorCall);
    
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundName = TEXT("Forest_Ambient");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Priority = EAudio_Priority::Low;
    ForestAmbient.Volume = 0.6f;
    ForestAmbient.bLoop = true;
    RegisteredSounds.Add(ForestAmbient.SoundName, ForestAmbient);
    
    FAudio_SoundEntry SavannaAmbient;
    SavannaAmbient.SoundName = TEXT("Savanna_Ambient");
    SavannaAmbient.Category = EAudio_SoundCategory::Ambient;
    SavannaAmbient.Priority = EAudio_Priority::Low;
    SavannaAmbient.Volume = 0.6f;
    SavannaAmbient.bLoop = true;
    RegisteredSounds.Add(SavannaAmbient.SoundName, SavannaAmbient);
}

void UAudio_SoundManager::RegisterSound(const FString& SoundName, USoundBase* SoundAsset, EAudio_SoundCategory Category, EAudio_Priority Priority)
{
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to register null sound asset: %s"), *SoundName);
        return;
    }
    
    FAudio_SoundEntry NewEntry;
    NewEntry.SoundName = SoundName;
    NewEntry.SoundAsset = SoundAsset;
    NewEntry.Category = Category;
    NewEntry.Priority = Priority;
    
    RegisteredSounds.Add(SoundName, NewEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundName);
}

UAudioComponent* UAudio_SoundManager::PlaySound2D(const FString& SoundName, float Volume, float Pitch, bool bLoop)
{
    FAudio_SoundEntry* SoundEntry = RegisteredSounds.Find(SoundName);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundName);
        return nullptr;
    }
    
    USoundBase* Sound = SoundEntry->SoundAsset.LoadSynchronous();
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load sound asset: %s"), *SoundName);
        return nullptr;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(Sound, false);
    if (!AudioComp)
    {
        return nullptr;
    }
    
    float FinalVolume = Volume * MasterVolume * GetCategoryVolume(SoundEntry->Category);
    AudioComp->SetVolumeMultiplier(FinalVolume);
    AudioComp->SetPitchMultiplier(Pitch);
    AudioComp->bAutoDestroy = !bLoop;
    
    if (bLoop)
    {
        AudioComp->SetUISound(true);
    }
    
    AudioComp->Play();
    
    FAudio_PlayingSound PlayingSound;
    PlayingSound.SoundName = SoundName;
    PlayingSound.AudioComponent = AudioComp;
    PlayingSound.Category = SoundEntry->Category;
    PlayingSound.StartTime = GetWorld()->GetTimeSeconds();
    PlayingSounds.Add(PlayingSound);
    
    return AudioComp;
}

UAudioComponent* UAudio_SoundManager::PlaySound3D(const FString& SoundName, FVector Location, float Volume, float Pitch, bool bLoop)
{
    FAudio_SoundEntry* SoundEntry = RegisteredSounds.Find(SoundName);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundName);
        return nullptr;
    }
    
    USoundBase* Sound = SoundEntry->SoundAsset.LoadSynchronous();
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load sound asset: %s"), *SoundName);
        return nullptr;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(Sound, true);
    if (!AudioComp)
    {
        return nullptr;
    }
    
    float FinalVolume = Volume * MasterVolume * GetCategoryVolume(SoundEntry->Category);
    AudioComp->SetVolumeMultiplier(FinalVolume);
    AudioComp->SetPitchMultiplier(Pitch);
    AudioComp->SetWorldLocation(Location);
    AudioComp->bAutoDestroy = !bLoop;
    
    if (SoundEntry->MaxDistance > 0.0f)
    {
        AudioComp->AttenuationSettings.AttenuationShape = EAttenuationShape::Sphere;
        AudioComp->AttenuationSettings.FalloffDistance = SoundEntry->MaxDistance;
    }
    
    AudioComp->Play();
    
    FAudio_PlayingSound PlayingSound;
    PlayingSound.SoundName = SoundName;
    PlayingSound.AudioComponent = AudioComp;
    PlayingSound.Category = SoundEntry->Category;
    PlayingSound.StartTime = GetWorld()->GetTimeSeconds();
    PlayingSounds.Add(PlayingSound);
    
    return AudioComp;
}

void UAudio_SoundManager::StopSound(const FString& SoundName)
{
    for (int32 i = PlayingSounds.Num() - 1; i >= 0; i--)
    {
        if (PlayingSounds[i].SoundName == SoundName)
        {
            if (PlayingSounds[i].AudioComponent && IsValid(PlayingSounds[i].AudioComponent))
            {
                PlayingSounds[i].AudioComponent->Stop();
            }
            PlayingSounds.RemoveAt(i);
        }
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (FAudio_PlayingSound& PlayingSound : PlayingSounds)
    {
        if (PlayingSound.AudioComponent && IsValid(PlayingSound.AudioComponent))
        {
            PlayingSound.AudioComponent->Stop();
        }
    }
    PlayingSounds.Empty();
}

void UAudio_SoundManager::StopSoundsByCategory(EAudio_SoundCategory Category)
{
    for (int32 i = PlayingSounds.Num() - 1; i >= 0; i--)
    {
        if (PlayingSounds[i].Category == Category)
        {
            if (PlayingSounds[i].AudioComponent && IsValid(PlayingSounds[i].AudioComponent))
            {
                PlayingSounds[i].AudioComponent->Stop();
            }
            PlayingSounds.RemoveAt(i);
        }
    }
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundName)
{
    CleanupFinishedSounds();
    
    for (const FAudio_PlayingSound& PlayingSound : PlayingSounds)
    {
        if (PlayingSound.SoundName == SoundName)
        {
            return true;
        }
    }
    return false;
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    const float* VolumePtr = CategoryVolumes.Find(Category);
    return VolumePtr ? *VolumePtr : 1.0f;
}

void UAudio_SoundManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location)
{
    FString SoundName = FString::Printf(TEXT("%s_%s"), *DinosaurType, *SoundType);
    PlaySound3D(SoundName, Location, 1.0f, 1.0f, false);
}

void UAudio_SoundManager::PlayNarrationLine(const FString& NarrationKey, float Delay)
{
    if (Delay > 0.0f)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NarrationKey]()
        {
            PlaySound2D(NarrationKey, 1.0f, 1.0f, false);
        }, Delay, false);
    }
    else
    {
        PlaySound2D(NarrationKey, 1.0f, 1.0f, false);
    }
}

void UAudio_SoundManager::StartAmbientAudio(const FString& BiomeName)
{
    if (CurrentBiome == BiomeName && CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        return; // Already playing correct ambient
    }
    
    StopAmbientAudio();
    
    FString AmbientSoundName = FString::Printf(TEXT("%s_Ambient"), *BiomeName);
    CurrentAmbientAudio = PlaySound2D(AmbientSoundName, 1.0f, 1.0f, true);
    CurrentBiome = BiomeName;
}

void UAudio_SoundManager::StopAmbientAudio()
{
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        CurrentAmbientAudio->Stop();
        CurrentAmbientAudio = nullptr;
    }
    CurrentBiome = TEXT("");
}

void UAudio_SoundManager::FadeInAmbient(const FString& BiomeName, float FadeTime)
{
    StartAmbientAudio(BiomeName);
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        CurrentAmbientAudio->FadeIn(FadeTime);
    }
}

void UAudio_SoundManager::FadeOutAmbient(float FadeTime)
{
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        CurrentAmbientAudio->FadeOut(FadeTime, 0.0f);
    }
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(USoundBase* Sound, bool b3D)
{
    if (!Sound || !GetWorld())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetWorld());
    if (!AudioComp)
    {
        return nullptr;
    }
    
    AudioComp->SetSound(Sound);
    AudioComp->bAllowSpatialization = b3D;
    AudioComp->bOverrideAttenuation = b3D;
    
    if (b3D)
    {
        AudioComp->AttenuationSettings.bAttenuate = true;
        AudioComp->AttenuationSettings.AttenuationShape = EAttenuationShape::Sphere;
        AudioComp->AttenuationSettings.dBAttenuationAtMax = -60.0f;
    }
    
    return AudioComp;
}

void UAudio_SoundManager::CleanupFinishedSounds()
{
    for (int32 i = PlayingSounds.Num() - 1; i >= 0; i--)
    {
        if (!PlayingSounds[i].AudioComponent || !IsValid(PlayingSounds[i].AudioComponent) || !PlayingSounds[i].AudioComponent->IsPlaying())
        {
            PlayingSounds.RemoveAt(i);
        }
    }
}

void UAudio_SoundManager::OnAudioFinished()
{
    CleanupFinishedSounds();
}