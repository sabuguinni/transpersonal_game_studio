#include "Audio_NarrativeAudioSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UAudio_NarrativeAudioSystem::UAudio_NarrativeAudioSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize state
    bIsPlayingNarrative = false;
    CurrentPlaybackTime = 0.0f;
    TargetVolume = 1.0f;
    FadeStartVolume = 1.0f;
    FadeTimeRemaining = 0.0f;
    bIsFading = false;

    // Create audio components
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));

    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(1.0f);
        NarrativeAudioComponent->bAutoActivate = false;
    }

    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(0.3f);
        AmbienceAudioComponent->bAutoActivate = false;
    }
}

void UAudio_NarrativeAudioSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultClips();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: BeginPlay - System initialized with %d clips"), NarrativeClips.Num());
}

void UAudio_NarrativeAudioSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    if (AmbienceAudioComponent && AmbienceAudioComponent->IsPlaying())
    {
        AmbienceAudioComponent->Stop();
    }
    
    if (GetWorld() && FadeTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UAudio_NarrativeAudioSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsPlayingNarrative && NarrativeAudioComponent)
    {
        if (NarrativeAudioComponent->IsPlaying())
        {
            CurrentPlaybackTime += DeltaTime;
        }
        else if (CurrentPlaybackTime > 0.0f)
        {
            // Narrative finished playing
            OnNarrativePlaybackFinished();
        }
    }
}

void UAudio_NarrativeAudioSystem::PlayNarrativeClip(const FString& ClipName)
{
    FAudio_NarrativeClip* FoundClip = NarrativeClips.FindByPredicate([&ClipName](const FAudio_NarrativeClip& Clip)
    {
        return Clip.ClipName == ClipName;
    });

    if (FoundClip && NarrativeAudioComponent)
    {
        // Stop current narrative if playing
        if (bIsPlayingNarrative)
        {
            StopCurrentNarrative();
        }

        CurrentClip = *FoundClip;
        CurrentPlaybackTime = 0.0f;
        bIsPlayingNarrative = true;

        // Set volume
        NarrativeAudioComponent->SetVolumeMultiplier(CurrentClip.Volume);

        // For now, we'll use a placeholder sound since we have URLs but not loaded assets
        // In a full implementation, this would load the sound asset from the URL
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Playing narrative clip '%s' by %s"), 
               *CurrentClip.ClipName, 
               *UEnum::GetValueAsString(CurrentClip.NarratorType));

        // Broadcast event
        OnNarrativeStarted.Broadcast(CurrentClip);

        // Simulate playback duration for now
        if (GetWorld())
        {
            FTimerHandle PlaybackTimer;
            GetWorld()->GetTimerManager().SetTimer(PlaybackTimer, [this]()
            {
                OnNarrativePlaybackFinished();
            }, CurrentClip.Duration, false);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeAudioSystem: Clip '%s' not found"), *ClipName);
    }
}

void UAudio_NarrativeAudioSystem::PlayNarrativeByType(EAudio_NarrativeType NarrativeType, EAudio_NarratorType NarratorType)
{
    FAudio_NarrativeClip* FoundClip = NarrativeClips.FindByPredicate([NarrativeType, NarratorType](const FAudio_NarrativeClip& Clip)
    {
        return Clip.NarrativeType == NarrativeType && Clip.NarratorType == NarratorType;
    });

    if (FoundClip)
    {
        PlayNarrativeClip(FoundClip->ClipName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeAudioSystem: No clip found for type %s by narrator %s"), 
               *UEnum::GetValueAsString(NarrativeType),
               *UEnum::GetValueAsString(NarratorType));
    }
}

void UAudio_NarrativeAudioSystem::StopCurrentNarrative()
{
    if (bIsPlayingNarrative && NarrativeAudioComponent)
    {
        NarrativeAudioComponent->Stop();
        bIsPlayingNarrative = false;
        CurrentPlaybackTime = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Stopped narrative '%s'"), *CurrentClip.ClipName);
    }
}

void UAudio_NarrativeAudioSystem::PauseCurrentNarrative()
{
    if (bIsPlayingNarrative && NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->SetPaused(true);
        OnNarrativePaused.Broadcast(CurrentClip);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Paused narrative '%s'"), *CurrentClip.ClipName);
    }
}

void UAudio_NarrativeAudioSystem::ResumeCurrentNarrative()
{
    if (bIsPlayingNarrative && NarrativeAudioComponent && NarrativeAudioComponent->GetPaused())
    {
        NarrativeAudioComponent->SetPaused(false);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Resumed narrative '%s'"), *CurrentClip.ClipName);
    }
}

void UAudio_NarrativeAudioSystem::SetNarrativeVolume(float NewVolume)
{
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(FMath::Clamp(NewVolume, 0.0f, 1.0f));
    }
}

void UAudio_NarrativeAudioSystem::SetAmbienceVolume(float NewVolume)
{
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(FMath::Clamp(NewVolume, 0.0f, 1.0f));
    }
}

void UAudio_NarrativeAudioSystem::FadeInNarrative(float FadeTime)
{
    if (NarrativeAudioComponent && FadeTime > 0.0f)
    {
        FadeStartVolume = 0.0f;
        TargetVolume = CurrentClip.Volume;
        FadeTimeRemaining = FadeTime;
        bIsFading = true;

        NarrativeAudioComponent->SetVolumeMultiplier(0.0f);

        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UAudio_NarrativeAudioSystem::UpdateFade, 0.05f, true);
        }
    }
}

void UAudio_NarrativeAudioSystem::FadeOutNarrative(float FadeTime)
{
    if (NarrativeAudioComponent && FadeTime > 0.0f)
    {
        FadeStartVolume = NarrativeAudioComponent->VolumeMultiplier;
        TargetVolume = 0.0f;
        FadeTimeRemaining = FadeTime;
        bIsFading = true;

        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UAudio_NarrativeAudioSystem::UpdateFade, 0.05f, true);
        }
    }
}

void UAudio_NarrativeAudioSystem::AddNarrativeClip(const FAudio_NarrativeClip& NewClip)
{
    // Check if clip with same name already exists
    int32 ExistingIndex = NarrativeClips.IndexOfByPredicate([&NewClip](const FAudio_NarrativeClip& Clip)
    {
        return Clip.ClipName == NewClip.ClipName;
    });

    if (ExistingIndex != INDEX_NONE)
    {
        // Replace existing clip
        NarrativeClips[ExistingIndex] = NewClip;
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Updated existing clip '%s'"), *NewClip.ClipName);
    }
    else
    {
        // Add new clip
        NarrativeClips.Add(NewClip);
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Added new clip '%s'"), *NewClip.ClipName);
    }
}

void UAudio_NarrativeAudioSystem::RemoveNarrativeClip(const FString& ClipName)
{
    int32 RemovedCount = NarrativeClips.RemoveAll([&ClipName](const FAudio_NarrativeClip& Clip)
    {
        return Clip.ClipName == ClipName;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Removed clip '%s'"), *ClipName);
    }
}

FAudio_NarrativeClip UAudio_NarrativeAudioSystem::GetNarrativeClip(const FString& ClipName)
{
    FAudio_NarrativeClip* FoundClip = NarrativeClips.FindByPredicate([&ClipName](const FAudio_NarrativeClip& Clip)
    {
        return Clip.ClipName == ClipName;
    });

    return FoundClip ? *FoundClip : FAudio_NarrativeClip();
}

TArray<FAudio_NarrativeClip> UAudio_NarrativeAudioSystem::GetClipsByType(EAudio_NarrativeType NarrativeType)
{
    return NarrativeClips.FilterByPredicate([NarrativeType](const FAudio_NarrativeClip& Clip)
    {
        return Clip.NarrativeType == NarrativeType;
    });
}

TArray<FAudio_NarrativeClip> UAudio_NarrativeAudioSystem::GetClipsByNarrator(EAudio_NarratorType NarratorType)
{
    return NarrativeClips.FilterByPredicate([NarratorType](const FAudio_NarrativeClip& Clip)
    {
        return Clip.NarratorType == NarratorType;
    });
}

void UAudio_NarrativeAudioSystem::UpdateFade()
{
    if (!bIsFading || !NarrativeAudioComponent)
    {
        return;
    }

    FadeTimeRemaining -= 0.05f;

    if (FadeTimeRemaining <= 0.0f)
    {
        // Fade complete
        NarrativeAudioComponent->SetVolumeMultiplier(TargetVolume);
        bIsFading = false;

        if (GetWorld() && FadeTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
        }

        // If fading out to zero, stop playback
        if (TargetVolume == 0.0f)
        {
            StopCurrentNarrative();
        }
    }
    else
    {
        // Calculate current fade volume
        float FadeProgress = 1.0f - (FadeTimeRemaining / (FadeTimeRemaining + 0.05f));
        float CurrentVolume = FMath::Lerp(FadeStartVolume, TargetVolume, FadeProgress);
        NarrativeAudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
}

void UAudio_NarrativeAudioSystem::OnNarrativePlaybackFinished()
{
    if (bIsPlayingNarrative)
    {
        OnNarrativeFinished.Broadcast(CurrentClip);
        bIsPlayingNarrative = false;
        CurrentPlaybackTime = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Finished narrative '%s'"), *CurrentClip.ClipName);
    }
}

void UAudio_NarrativeAudioSystem::InitializeDefaultClips()
{
    // Initialize with the generated voice clips from previous agent
    FAudio_NarrativeClip TribalElderClip;
    TribalElderClip.ClipName = TEXT("TribalElder_AncientFire");
    TribalElderClip.NarrativeType = EAudio_NarrativeType::TribalLegend;
    TribalElderClip.NarratorType = EAudio_NarratorType::TribalElder;
    TribalElderClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781494513395_PrehistoricNarrator.mp3");
    TribalElderClip.Duration = 11.0f;
    TribalElderClip.Volume = 0.8f;
    NarrativeClips.Add(TribalElderClip);

    FAudio_NarrativeClip ScoutWarningClip;
    ScoutWarningClip.ClipName = TEXT("TribalScout_DangerWarning");
    ScoutWarningClip.NarrativeType = EAudio_NarrativeType::WarningStory;
    ScoutWarningClip.NarratorType = EAudio_NarratorType::ScoutWarrior;
    ScoutWarningClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781494517941_TribalScout.mp3");
    ScoutWarningClip.Duration = 9.0f;
    ScoutWarningClip.Volume = 0.9f;
    NarrativeClips.Add(ScoutWarningClip);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeAudioSystem: Initialized with %d default clips"), NarrativeClips.Num());
}