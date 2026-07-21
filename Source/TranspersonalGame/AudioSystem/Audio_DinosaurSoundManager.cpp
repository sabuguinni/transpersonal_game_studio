#include "Audio_DinosaurSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UAudio_DinosaurSoundManager::UAudio_DinosaurSoundManager()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Default settings
    DinosaurType = EAudio_DinosaurType::TRex;
    CurrentBehavior = EAudio_DinosaurBehavior::Idle;
    MasterVolume = 1.0f;
    bEnableRandomVocalizations = true;
    bEnableFootstepSounds = true;

    VocalizationComponent = nullptr;
    FootstepComponent = nullptr;
}

void UAudio_DinosaurSoundManager::BeginPlay()
{
    Super::BeginPlay();

    // Create audio components
    if (AActor* Owner = GetOwner())
    {
        VocalizationComponent = NewObject<UAudioComponent>(Owner, TEXT("DinosaurVocalizationComponent"));
        VocalizationComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        VocalizationComponent->bAutoActivate = false;
        VocalizationComponent->RegisterComponent();

        FootstepComponent = NewObject<UAudioComponent>(Owner, TEXT("DinosaurFootstepComponent"));
        FootstepComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        FootstepComponent->bAutoActivate = false;
        FootstepComponent->RegisterComponent();
    }

    InitializeDinosaurSoundData();
    
    if (bEnableRandomVocalizations)
    {
        StartRandomVocalizationTimer();
    }
}

void UAudio_DinosaurSoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopRandomVocalizationTimer();
    StopFootstepLoop();
    StopAllSounds();
    Super::EndPlay(EndPlayReason);
}

void UAudio_DinosaurSoundManager::InitializeDinosaurSoundData()
{
    // T-Rex sound configuration
    FAudio_DinosaurSoundSet TRexSounds;
    TRexSounds.BaseVolume = 1.0f;
    TRexSounds.MaxAudibleDistance = 5000.0f;
    TRexSounds.VocalizationInterval = 20.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::TRex, TRexSounds);

    // Raptor sound configuration
    FAudio_DinosaurSoundSet RaptorSounds;
    RaptorSounds.BaseVolume = 0.7f;
    RaptorSounds.MaxAudibleDistance = 2000.0f;
    RaptorSounds.VocalizationInterval = 12.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Raptor, RaptorSounds);

    // Brachiosaurus sound configuration
    FAudio_DinosaurSoundSet BrachiosaurusSounds;
    BrachiosaurusSounds.BaseVolume = 1.2f;
    BrachiosaurusSounds.MaxAudibleDistance = 6000.0f;
    BrachiosaurusSounds.VocalizationInterval = 30.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Brachiosaurus, BrachiosaurusSounds);

    // Triceratops sound configuration
    FAudio_DinosaurSoundSet TriceratopsSounds;
    TriceratopsSounds.BaseVolume = 0.9f;
    TriceratopsSounds.MaxAudibleDistance = 3000.0f;
    TriceratopsSounds.VocalizationInterval = 25.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Triceratops, TriceratopsSounds);

    // Pteranodon sound configuration
    FAudio_DinosaurSoundSet PteranodonSounds;
    PteranodonSounds.BaseVolume = 0.6f;
    PteranodonSounds.MaxAudibleDistance = 4000.0f;
    PteranodonSounds.VocalizationInterval = 8.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Pteranodon, PteranodonSounds);

    // Stegosaurus sound configuration
    FAudio_DinosaurSoundSet StegosaurusSounds;
    StegosaurusSounds.BaseVolume = 0.8f;
    StegosaurusSounds.MaxAudibleDistance = 2500.0f;
    StegosaurusSounds.VocalizationInterval = 18.0f;
    DinosaurSoundMap.Add(EAudio_DinosaurType::Stegosaurus, StegosaurusSounds);
}

void UAudio_DinosaurSoundManager::SetDinosaurType(EAudio_DinosaurType NewType)
{
    if (DinosaurType != NewType)
    {
        DinosaurType = NewType;
        StopRandomVocalizationTimer();
        
        if (bEnableRandomVocalizations)
        {
            StartRandomVocalizationTimer();
        }
    }
}

void UAudio_DinosaurSoundManager::SetBehavior(EAudio_DinosaurBehavior NewBehavior)
{
    if (CurrentBehavior != NewBehavior)
    {
        CurrentBehavior = NewBehavior;
        
        // Play immediate vocalization for certain behavior changes
        if (NewBehavior == EAudio_DinosaurBehavior::Hunting || 
            NewBehavior == EAudio_DinosaurBehavior::Territorial ||
            NewBehavior == EAudio_DinosaurBehavior::Fleeing)
        {
            PlayVocalization(NewBehavior);
        }
    }
}

void UAudio_DinosaurSoundManager::PlayVocalization(EAudio_DinosaurBehavior BehaviorType)
{
    if (!VocalizationComponent)
    {
        return;
    }

    TArray<USoundCue*> SoundsToPlay = GetSoundsForBehavior(BehaviorType);
    
    if (SoundsToPlay.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, SoundsToPlay.Num() - 1);
        USoundCue* SelectedSound = SoundsToPlay[RandomIndex];
        
        if (SelectedSound && DinosaurSoundMap.Contains(DinosaurType))
        {
            const FAudio_DinosaurSoundSet& SoundSet = DinosaurSoundMap[DinosaurType];
            VocalizationComponent->SetSound(SelectedSound);
            VocalizationComponent->SetVolumeMultiplier(SoundSet.BaseVolume * MasterVolume);
            VocalizationComponent->Play();
        }
    }
}

void UAudio_DinosaurSoundManager::PlayFootstepSound()
{
    if (!FootstepComponent || !DinosaurSoundMap.Contains(DinosaurType))
    {
        return;
    }

    const FAudio_DinosaurSoundSet& SoundSet = DinosaurSoundMap[DinosaurType];
    
    if (SoundSet.FootstepSounds.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, SoundSet.FootstepSounds.Num() - 1);
        USoundCue* FootstepSound = SoundSet.FootstepSounds[RandomIndex];
        
        if (FootstepSound)
        {
            FootstepComponent->SetSound(FootstepSound);
            FootstepComponent->SetVolumeMultiplier(SoundSet.BaseVolume * MasterVolume * 0.8f);
            FootstepComponent->Play();
        }
    }
}

void UAudio_DinosaurSoundManager::StartFootstepLoop(float StepInterval)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FootstepTimer,
            this,
            &UAudio_DinosaurSoundManager::PlayFootstepSound,
            StepInterval,
            true
        );
    }
}

void UAudio_DinosaurSoundManager::StopFootstepLoop()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(FootstepTimer);
    }
}

void UAudio_DinosaurSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_DinosaurSoundManager::EnableRandomVocalizations(bool bEnable)
{
    bEnableRandomVocalizations = bEnable;
    
    if (bEnable)
    {
        StartRandomVocalizationTimer();
    }
    else
    {
        StopRandomVocalizationTimer();
    }
}

void UAudio_DinosaurSoundManager::StopAllSounds()
{
    if (VocalizationComponent && VocalizationComponent->IsPlaying())
    {
        VocalizationComponent->Stop();
    }
    
    if (FootstepComponent && FootstepComponent->IsPlaying())
    {
        FootstepComponent->Stop();
    }
}

void UAudio_DinosaurSoundManager::StartRandomVocalizationTimer()
{
    if (!DinosaurSoundMap.Contains(DinosaurType))
    {
        return;
    }

    const FAudio_DinosaurSoundSet& SoundSet = DinosaurSoundMap[DinosaurType];
    float TimerInterval = SoundSet.VocalizationInterval + FMath::RandRange(-5.0f, 10.0f);
    TimerInterval = FMath::Max(TimerInterval, 5.0f);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            VocalizationTimer,
            this,
            &UAudio_DinosaurSoundManager::PlayRandomVocalization,
            TimerInterval,
            false
        );
    }
}

void UAudio_DinosaurSoundManager::StopRandomVocalizationTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(VocalizationTimer);
    }
}

void UAudio_DinosaurSoundManager::PlayRandomVocalization()
{
    PlayVocalization(CurrentBehavior);
    
    // Restart timer for next vocalization
    if (bEnableRandomVocalizations)
    {
        StartRandomVocalizationTimer();
    }
}

TArray<USoundCue*> UAudio_DinosaurSoundManager::GetSoundsForBehavior(EAudio_DinosaurBehavior Behavior)
{
    if (!DinosaurSoundMap.Contains(DinosaurType))
    {
        return TArray<USoundCue*>();
    }

    const FAudio_DinosaurSoundSet& SoundSet = DinosaurSoundMap[DinosaurType];

    switch (Behavior)
    {
        case EAudio_DinosaurBehavior::Idle:
            return SoundSet.IdleSounds;
        case EAudio_DinosaurBehavior::Hunting:
            return SoundSet.HuntingSounds;
        case EAudio_DinosaurBehavior::Feeding:
            return SoundSet.FeedingSounds;
        case EAudio_DinosaurBehavior::Territorial:
            return SoundSet.TerritorialSounds;
        case EAudio_DinosaurBehavior::Fleeing:
            return SoundSet.FleeingSounds;
        case EAudio_DinosaurBehavior::Mating:
            return SoundSet.MatingSounds;
        case EAudio_DinosaurBehavior::Dying:
            return SoundSet.DyingSounds;
        default:
            return SoundSet.IdleSounds;
    }
}