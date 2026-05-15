#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Player, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.5f);

    // Initialize mute states
    for (auto& Pair : CategoryVolumes)
    {
        CategoryMuted.Add(Pair.Key, false);
    }

    CurrentAmbientComponent = nullptr;
    MasterVolume = 1.0f;
    MaxAudioDistance = 5000.0f;
    MaxConcurrentSounds = 32;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeSoundLibraries();
    
    // Set up cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UAudio_SoundManager::CleanupFinishedComponents,
            2.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    // Stop all active audio
    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    if (IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
    }
    
    ActiveAudioComponents.Empty();
    CurrentAmbientComponent = nullptr;
    
    Super::Deinitialize();
}

UAudioComponent* UAudio_SoundManager::PlaySound2D(USoundCue* SoundCue, float Volume, float Pitch)
{
    if (!IsValid(SoundCue))
    {
        return nullptr;
    }

    if (ActiveAudioComponents.Num() >= MaxConcurrentSounds)
    {
        CleanupFinishedComponents();
        if (ActiveAudioComponents.Num() >= MaxConcurrentSounds)
        {
            return nullptr; // Too many sounds playing
        }
    }

    UAudioComponent* AudioComponent = CreateAudioComponent(SoundCue, Volume, Pitch);
    if (AudioComponent)
    {
        AudioComponent->Play();
        ActiveAudioComponents.Add(AudioComponent);
    }

    return AudioComponent;
}

UAudioComponent* UAudio_SoundManager::PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float Volume, float Pitch)
{
    if (!IsValid(SoundCue))
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundCue,
        Location,
        FRotator::ZeroRotator,
        Volume * MasterVolume,
        Pitch
    );

    if (AudioComponent)
    {
        ActiveAudioComponents.Add(AudioComponent);
    }

    return AudioComponent;
}

UAudioComponent* UAudio_SoundManager::PlaySoundAttached(USoundCue* SoundCue, USceneComponent* AttachComponent, float Volume, float Pitch)
{
    if (!IsValid(SoundCue) || !IsValid(AttachComponent))
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(
        SoundCue,
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepWorldPosition,
        false,
        Volume * MasterVolume,
        Pitch
    );

    if (AudioComponent)
    {
        ActiveAudioComponents.Add(AudioComponent);
    }

    return AudioComponent;
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (const float* VolumePtr = CategoryVolumes.Find(Category))
    {
        return *VolumePtr;
    }
    return 1.0f;
}

void UAudio_SoundManager::MuteCategoryToggle(EAudio_SoundCategory Category)
{
    bool* MutedPtr = CategoryMuted.Find(Category);
    if (MutedPtr)
    {
        *MutedPtr = !(*MutedPtr);
    }
}

void UAudio_SoundManager::StopAllSoundsInCategory(EAudio_SoundCategory Category)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; --i)
    {
        UAudioComponent* Component = ActiveAudioComponents[i];
        if (IsValid(Component))
        {
            // Note: In a real implementation, you'd need to track which category each component belongs to
            Component->Stop();
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

void UAudio_SoundManager::StartAmbientLoop(USoundCue* AmbientSound, float Volume)
{
    if (!IsValid(AmbientSound))
    {
        return;
    }

    // Stop current ambient if playing
    if (IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
    }

    // Start new ambient
    CurrentAmbientComponent = PlaySound2D(AmbientSound, Volume * GetCategoryVolume(EAudio_SoundCategory::Ambient));
    if (CurrentAmbientComponent)
    {
        CurrentAmbientComponent->SetUISound(false);
        CurrentAmbientComponent->bAutoDestroy = false;
    }
}

void UAudio_SoundManager::StopAmbientLoop(float FadeOutTime)
{
    if (IsValid(CurrentAmbientComponent))
    {
        if (FadeOutTime > 0.0f)
        {
            CurrentAmbientComponent->FadeOut(FadeOutTime, 0.0f);
        }
        else
        {
            CurrentAmbientComponent->Stop();
        }
        CurrentAmbientComponent = nullptr;
    }
}

void UAudio_SoundManager::CrossfadeAmbient(USoundCue* NewAmbientSound, float CrossfadeTime)
{
    if (!IsValid(NewAmbientSound))
    {
        return;
    }

    // Fade out current ambient
    if (IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->FadeOut(CrossfadeTime, 0.0f);
    }

    // Start new ambient with fade in
    UAudioComponent* NewComponent = PlaySound2D(NewAmbientSound, 0.0f);
    if (NewComponent)
    {
        NewComponent->FadeIn(CrossfadeTime, GetCategoryVolume(EAudio_SoundCategory::Ambient) * MasterVolume);
        CurrentAmbientComponent = NewComponent;
    }
}

void UAudio_SoundManager::UpdateAudioForDangerLevel(float DangerLevel)
{
    // Adjust ambient volume based on danger
    float AmbientVolume = FMath::Lerp(0.7f, 0.3f, DangerLevel);
    SetCategoryVolume(EAudio_SoundCategory::Ambient, AmbientVolume);

    // Increase UI volume for warnings
    float UIVolume = FMath::Lerp(0.9f, 1.0f, DangerLevel);
    SetCategoryVolume(EAudio_SoundCategory::UI, UIVolume);
}

void UAudio_SoundManager::UpdateAudioForTimeOfDay(float TimeOfDay)
{
    // Adjust ambient sounds for day/night cycle
    // TimeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    
    float DayIntensity = 1.0f - FMath::Abs((TimeOfDay - 0.5f) * 2.0f);
    float NightIntensity = 1.0f - DayIntensity;
    
    // Day sounds louder during day, night sounds louder at night
    SetCategoryVolume(EAudio_SoundCategory::Environment, 0.4f + (DayIntensity * 0.4f));
}

void UAudio_SoundManager::UpdateAudioForWeather(const FString& WeatherType)
{
    if (WeatherType == TEXT("Rain"))
    {
        SetCategoryVolume(EAudio_SoundCategory::Ambient, 0.9f);
        SetCategoryVolume(EAudio_SoundCategory::Environment, 0.4f);
    }
    else if (WeatherType == TEXT("Storm"))
    {
        SetCategoryVolume(EAudio_SoundCategory::Ambient, 1.0f);
        SetCategoryVolume(EAudio_SoundCategory::Environment, 0.2f);
    }
    else if (WeatherType == TEXT("Clear"))
    {
        SetCategoryVolume(EAudio_SoundCategory::Ambient, 0.6f);
        SetCategoryVolume(EAudio_SoundCategory::Environment, 0.7f);
    }
}

void UAudio_SoundManager::PlayDinosaurSound(const FString& DinosaurType, FVector Location, float Distance)
{
    FAudio_SoundData* SoundData = DinosaurSounds.Find(DinosaurType);
    if (!SoundData || !SoundData->SoundCue.IsValid())
    {
        return;
    }

    // Calculate volume based on distance
    float DistanceVolume = FMath::Clamp(1.0f - (Distance / MaxAudioDistance), 0.0f, 1.0f);
    float FinalVolume = SoundData->Volume * DistanceVolume * GetCategoryVolume(EAudio_SoundCategory::Dinosaur);

    PlaySoundAtLocation(SoundData->SoundCue.Get(), Location, FinalVolume, SoundData->Pitch);
}

void UAudio_SoundManager::PlayFootstepSound(const FString& SurfaceType, FVector Location, float Weight)
{
    FString SoundKey = FString::Printf(TEXT("Footstep_%s"), *SurfaceType);
    FAudio_SoundData* SoundData = EnvironmentSounds.Find(SoundKey);
    
    if (!SoundData || !SoundData->SoundCue.IsValid())
    {
        return;
    }

    // Adjust pitch and volume based on weight
    float WeightPitch = FMath::Lerp(1.2f, 0.8f, Weight);
    float WeightVolume = FMath::Lerp(0.5f, 1.0f, Weight);

    PlaySoundAtLocation(
        SoundData->SoundCue.Get(),
        Location,
        SoundData->Volume * WeightVolume * GetCategoryVolume(EAudio_SoundCategory::Environment),
        SoundData->Pitch * WeightPitch
    );
}

void UAudio_SoundManager::CleanupFinishedComponents()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; --i)
    {
        UAudioComponent* Component = ActiveAudioComponents[i];
        if (!IsValid(Component) || !Component->IsPlaying())
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

void UAudio_SoundManager::InitializeSoundLibraries()
{
    // Initialize with placeholder sound data
    // In a real implementation, these would be loaded from data tables or config files
    
    // Dinosaur sounds
    FAudio_SoundData TRexRoar;
    TRexRoar.Category = EAudio_SoundCategory::Dinosaur;
    TRexRoar.Volume = 1.0f;
    TRexRoar.Pitch = 0.9f;
    DinosaurSounds.Add(TEXT("TRex_Roar"), TRexRoar);

    FAudio_SoundData RaptorCall;
    RaptorCall.Category = EAudio_SoundCategory::Dinosaur;
    RaptorCall.Volume = 0.8f;
    RaptorCall.Pitch = 1.1f;
    DinosaurSounds.Add(TEXT("Raptor_Call"), RaptorCall);

    // Environment sounds
    FAudio_SoundData GrassFootstep;
    GrassFootstep.Category = EAudio_SoundCategory::Environment;
    GrassFootstep.Volume = 0.6f;
    EnvironmentSounds.Add(TEXT("Footstep_Grass"), GrassFootstep);

    FAudio_SoundData StoneFootstep;
    StoneFootstep.Category = EAudio_SoundCategory::Environment;
    StoneFootstep.Volume = 0.8f;
    EnvironmentSounds.Add(TEXT("Footstep_Stone"), StoneFootstep);

    // Ambient sounds
    FAudio_SoundData ForestAmbient;
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.5f;
    ForestAmbient.bLooping = true;
    AmbientSounds.Add(TEXT("Forest_Day"), ForestAmbient);

    FAudio_SoundData SwampAmbient;
    SwampAmbient.Category = EAudio_SoundCategory::Ambient;
    SwampAmbient.Volume = 0.6f;
    SwampAmbient.bLooping = true;
    AmbientSounds.Add(TEXT("Swamp_Night"), SwampAmbient);
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(USoundCue* SoundCue, float Volume, float Pitch)
{
    UWorld* World = GetWorld();
    if (!World || !IsValid(SoundCue))
    {
        return nullptr;
    }

    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(this);
    if (AudioComponent)
    {
        AudioComponent->SetSound(SoundCue);
        AudioComponent->SetVolumeMultiplier(Volume * MasterVolume);
        AudioComponent->SetPitchMultiplier(Pitch);
        AudioComponent->bAutoDestroy = true;
        AudioComponent->bAllowSpatialization = false; // 2D sound
    }

    return AudioComponent;
}