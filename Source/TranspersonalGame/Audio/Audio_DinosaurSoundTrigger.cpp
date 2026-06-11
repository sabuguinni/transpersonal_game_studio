#include "Audio_DinosaurSoundTrigger.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AAudio_DinosaurSoundTrigger::AAudio_DinosaurSoundTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger sphere component
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    DinosaurAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurAudioComponent"));
    DinosaurAudioComponent->SetupAttachment(RootComponent);
    DinosaurAudioComponent->bAutoActivate = false;

    // Set default values
    DinosaurType = EAudio_DinosaurType::TRex;
    CurrentBehavior = EAudio_DinosaurBehavior::Idle;
    TriggerRadius = 1000.0f;
    MaxAudibleDistance = 2000.0f;
    bUse3DSound = true;
    PitchVariation = 0.1f;
    bPlayerTriggered = false;
    bIsPlayerInRange = false;

    // Initialize sound set defaults
    SoundSet.MinPlayInterval = 5.0f;
    SoundSet.MaxPlayInterval = 15.0f;
    SoundSet.VolumeMultiplier = 1.0f;
}

void AAudio_DinosaurSoundTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_DinosaurSoundTrigger::OnTriggerBeginOverlap);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_DinosaurSoundTrigger::OnTriggerEndOverlap);
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }

    UpdateAudioSettings();
    StartAutomaticSounds();
}

void AAudio_DinosaurSoundTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_DinosaurSoundTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                       bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        bIsPlayerInRange = true;
        bPlayerTriggered = true;

        // Play aggressive sound when player enters range
        if (CurrentBehavior == EAudio_DinosaurBehavior::Territorial || 
            CurrentBehavior == EAudio_DinosaurBehavior::Hunting)
        {
            PlayAggressiveSound();
        }
    }
}

void AAudio_DinosaurSoundTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        bIsPlayerInRange = false;
        bPlayerTriggered = false;
    }
}

void AAudio_DinosaurSoundTrigger::SetDinosaurBehavior(EAudio_DinosaurBehavior NewBehavior)
{
    if (CurrentBehavior != NewBehavior)
    {
        CurrentBehavior = NewBehavior;
        
        // Adjust sound intervals based on behavior
        switch (NewBehavior)
        {
            case EAudio_DinosaurBehavior::Hunting:
                SoundSet.MinPlayInterval = 3.0f;
                SoundSet.MaxPlayInterval = 8.0f;
                break;
            case EAudio_DinosaurBehavior::Territorial:
                SoundSet.MinPlayInterval = 2.0f;
                SoundSet.MaxPlayInterval = 6.0f;
                break;
            case EAudio_DinosaurBehavior::Fleeing:
                SoundSet.MinPlayInterval = 1.0f;
                SoundSet.MaxPlayInterval = 3.0f;
                break;
            default:
                SoundSet.MinPlayInterval = 5.0f;
                SoundSet.MaxPlayInterval = 15.0f;
                break;
        }
        
        // Reschedule next sound with new intervals
        ScheduleNextSound();
    }
}

void AAudio_DinosaurSoundTrigger::PlayRandomSound(EAudio_DinosaurBehavior BehaviorType)
{
    if (!DinosaurAudioComponent) return;

    TArray<USoundBase*>* SoundArray = nullptr;

    switch (BehaviorType)
    {
        case EAudio_DinosaurBehavior::Idle:
            SoundArray = &SoundSet.IdleSounds;
            break;
        case EAudio_DinosaurBehavior::Hunting:
        case EAudio_DinosaurBehavior::Territorial:
            SoundArray = &SoundSet.AggressiveSounds;
            break;
        case EAudio_DinosaurBehavior::Feeding:
            SoundArray = &SoundSet.BreathingSounds;
            break;
        default:
            SoundArray = &SoundSet.IdleSounds;
            break;
    }

    if (SoundArray && SoundArray->Num() > 0)
    {
        USoundBase* SelectedSound = GetRandomSoundFromArray(*SoundArray);
        if (SelectedSound)
        {
            DinosaurAudioComponent->SetSound(SelectedSound);
            
            // Add pitch variation
            float PitchMultiplier = 1.0f + FMath::RandRange(-PitchVariation, PitchVariation);
            DinosaurAudioComponent->SetPitchMultiplier(PitchMultiplier);
            
            DinosaurAudioComponent->Play();
        }
    }
}

void AAudio_DinosaurSoundTrigger::PlayFootstepSound()
{
    if (!DinosaurAudioComponent || SoundSet.FootstepSounds.Num() == 0) return;

    USoundBase* FootstepSound = GetRandomSoundFromArray(SoundSet.FootstepSounds);
    if (FootstepSound)
    {
        DinosaurAudioComponent->SetSound(FootstepSound);
        DinosaurAudioComponent->SetVolumeMultiplier(SoundSet.VolumeMultiplier * 0.8f); // Footsteps slightly quieter
        DinosaurAudioComponent->Play();
    }
}

void AAudio_DinosaurSoundTrigger::PlayAggressiveSound()
{
    if (!DinosaurAudioComponent || SoundSet.AggressiveSounds.Num() == 0) return;

    USoundBase* AggressiveSound = GetRandomSoundFromArray(SoundSet.AggressiveSounds);
    if (AggressiveSound)
    {
        DinosaurAudioComponent->SetSound(AggressiveSound);
        DinosaurAudioComponent->SetVolumeMultiplier(SoundSet.VolumeMultiplier * 1.2f); // Aggressive sounds louder
        
        // Lower pitch for more threatening sound
        DinosaurAudioComponent->SetPitchMultiplier(0.9f + FMath::RandRange(-0.1f, 0.1f));
        DinosaurAudioComponent->Play();
    }
}

void AAudio_DinosaurSoundTrigger::SetTriggerRadius(float NewRadius)
{
    TriggerRadius = NewRadius;
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }
}

void AAudio_DinosaurSoundTrigger::StartAutomaticSounds()
{
    ScheduleNextSound();
}

void AAudio_DinosaurSoundTrigger::StopAutomaticSounds()
{
    if (GetWorldTimerManager().IsTimerActive(SoundPlayTimer))
    {
        GetWorldTimerManager().ClearTimer(SoundPlayTimer);
    }
}

void AAudio_DinosaurSoundTrigger::ScheduleNextSound()
{
    if (!GetWorld()) return;

    float RandomInterval = FMath::RandRange(SoundSet.MinPlayInterval, SoundSet.MaxPlayInterval);
    
    // Increase frequency if player is in range
    if (bIsPlayerInRange)
    {
        RandomInterval *= 0.6f; // 40% faster when player nearby
    }

    GetWorldTimerManager().SetTimer(SoundPlayTimer, this, &AAudio_DinosaurSoundTrigger::PlayScheduledSound, 
                                   RandomInterval, false);
}

void AAudio_DinosaurSoundTrigger::PlayScheduledSound()
{
    // Play sound based on current behavior
    PlayRandomSound(CurrentBehavior);
    
    // Schedule the next sound
    ScheduleNextSound();
}

USoundBase* AAudio_DinosaurSoundTrigger::GetRandomSoundFromArray(const TArray<USoundBase*>& SoundArray)
{
    if (SoundArray.Num() == 0) return nullptr;
    
    int32 RandomIndex = FMath::RandRange(0, SoundArray.Num() - 1);
    return SoundArray[RandomIndex];
}

void AAudio_DinosaurSoundTrigger::UpdateAudioSettings()
{
    if (!DinosaurAudioComponent) return;

    DinosaurAudioComponent->SetVolumeMultiplier(SoundSet.VolumeMultiplier);
    
    if (bUse3DSound)
    {
        DinosaurAudioComponent->bAllowSpatialization = true;
        DinosaurAudioComponent->SetAttenuationSettings(nullptr); // Use default attenuation
    }
    else
    {
        DinosaurAudioComponent->bAllowSpatialization = false;
    }
}