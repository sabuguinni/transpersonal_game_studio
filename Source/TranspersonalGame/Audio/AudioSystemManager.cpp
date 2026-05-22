#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing audio subsystem"));
    
    InitializeCategoryVolumes();
    InitializeSoundLibrary();
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio subsystem initialized successfully"));
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundType::Music, 0.7f);
    CategoryVolumes.Add(EAudio_SoundType::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundType::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundType::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundType::UI, 0.9f);
}

void UAudioSystemManager::InitializeSoundLibrary()
{
    // Initialize with placeholder paths - will be populated with real assets
    SoundLibrary.Add(TEXT("TRexFootstep"), TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/SFX/TRex_Footstep"))));
    SoundLibrary.Add(TEXT("DamageHit"), TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/SFX/Damage_Hit"))));
    SoundLibrary.Add(TEXT("ScreenShake"), TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/SFX/Screen_Shake"))));
    SoundLibrary.Add(TEXT("DayAmbient"), TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/Ambient/Day_Forest"))));
    SoundLibrary.Add(TEXT("NightAmbient"), TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/Ambient/Night_Forest"))));
}

void UAudioSystemManager::PlaySound2D(const FString& SoundID, EAudio_SoundType SoundType, float Volume, float Pitch)
{
    if (TSoftObjectPtr<USoundBase>* SoundPtr = SoundLibrary.Find(SoundID))
    {
        if (USoundBase* Sound = SoundPtr->LoadSynchronous())
        {
            float FinalVolume = Volume * MasterVolume;
            if (float* CategoryVolume = CategoryVolumes.Find(SoundType))
            {
                FinalVolume *= *CategoryVolume;
            }
            
            UGameplayStatics::PlaySound2D(GetWorld(), Sound, FinalVolume, Pitch);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 2D sound %s"), *SoundID);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound %s"), *SoundID);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound %s not found in library"), *SoundID);
    }
}

void UAudioSystemManager::PlaySound3D(const FString& SoundID, const FVector& Location, EAudio_SoundType SoundType, float Volume, float Pitch)
{
    if (TSoftObjectPtr<USoundBase>* SoundPtr = SoundLibrary.Find(SoundID))
    {
        if (USoundBase* Sound = SoundPtr->LoadSynchronous())
        {
            float FinalVolume = Volume * MasterVolume;
            if (float* CategoryVolume = CategoryVolumes.Find(SoundType))
            {
                FinalVolume *= *CategoryVolume;
            }
            
            UAudioComponent* AudioComp = CreateAudioComponent(Location);
            if (AudioComp)
            {
                AudioComp->SetSound(Sound);
                AudioComp->SetVolumeMultiplier(FinalVolume);
                AudioComp->SetPitchMultiplier(Pitch);
                AudioComp->Play();
                
                ActiveSounds.Add(SoundID, AudioComp);
                UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 3D sound %s at location %s"), *SoundID, *Location.ToString());
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound %s"), *SoundID);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound %s not found in library"), *SoundID);
    }
}

void UAudioSystemManager::StopSound(const FString& SoundID)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundID))
    {
        if (UAudioComponent* AudioComp = *AudioCompPtr)
        {
            AudioComp->Stop();
            ActiveSounds.Remove(SoundID);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped sound %s"), *SoundID);
        }
    }
}

void UAudioSystemManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (UAudioComponent* AudioComp = SoundPair.Value)
        {
            AudioComp->Stop();
        }
    }
    ActiveSounds.Empty();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped all sounds"));
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Master volume set to %f"), MasterVolume);
}

void UAudioSystemManager::SetCategoryVolume(EAudio_SoundType SoundType, float Volume)
{
    CategoryVolumes.Add(SoundType, FMath::Clamp(Volume, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Category volume updated"));
}

void UAudioSystemManager::TriggerScreenShakeAudio(float Intensity, const FVector& EpicenterLocation)
{
    // Play low-frequency rumble for screen shake
    PlaySound3D(TEXT("ScreenShake"), EpicenterLocation, EAudio_SoundType::SFX, Intensity, 1.0f - (Intensity * 0.3f));
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Screen shake audio triggered with intensity %f"), Intensity);
}

void UAudioSystemManager::PlayDamageAudio(float DamageAmount, const FVector& HitLocation)
{
    // Scale volume and pitch based on damage amount
    float Volume = FMath::Clamp(DamageAmount / 100.0f, 0.3f, 1.0f);
    float Pitch = FMath::Clamp(1.0f + (DamageAmount / 200.0f), 0.8f, 1.5f);
    
    PlaySound3D(TEXT("DamageHit"), HitLocation, EAudio_SoundType::SFX, Volume, Pitch);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage audio played - Amount: %f, Volume: %f, Pitch: %f"), DamageAmount, Volume, Pitch);
}

void UAudioSystemManager::PlayFootstepAudio(const FVector& FootLocation, float CreatureSize, const FString& SurfaceType)
{
    // Scale volume and pitch based on creature size
    float Volume = FMath::Clamp(CreatureSize / 10.0f, 0.2f, 1.0f);
    float Pitch = FMath::Clamp(2.0f - (CreatureSize / 15.0f), 0.5f, 1.2f);
    
    FString FootstepSoundID = TEXT("TRexFootstep");
    if (CreatureSize < 2.0f)
    {
        FootstepSoundID = TEXT("SmallFootstep");
    }
    else if (CreatureSize > 8.0f)
    {
        FootstepSoundID = TEXT("TRexFootstep");
    }
    
    PlaySound3D(FootstepSoundID, FootLocation, EAudio_SoundType::SFX, Volume, Pitch);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Footstep audio - Size: %f, Surface: %s"), CreatureSize, *SurfaceType);
}

void UAudioSystemManager::TransitionToTimeOfDay(float TimeOfDay)
{
    // TimeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    bool bIsDay = (TimeOfDay > 0.25f && TimeOfDay < 0.75f);
    
    if (bIsDay)
    {
        StopSound(TEXT("NightAmbient"));
        PlaySound2D(TEXT("DayAmbient"), EAudio_SoundType::Ambient, 0.6f);
    }
    else
    {
        StopSound(TEXT("DayAmbient"));
        PlaySound2D(TEXT("NightAmbient"), EAudio_SoundType::Ambient, 0.8f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioned to %s ambient"), bIsDay ? TEXT("day") : TEXT("night"));
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(const FVector& Location)
{
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
        if (AudioComp)
        {
            AudioComp->SetWorldLocation(Location);
            AudioComp->bAutoDestroy = true;
            AudioComp->RegisterComponent();
            return AudioComp;
        }
    }
    return nullptr;
}