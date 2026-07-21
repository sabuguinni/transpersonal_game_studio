#include "Audio_DynamicAudioTrigger.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AAudio_DynamicAudioTrigger::AAudio_DynamicAudioTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Default values
    TriggerType = EAudio_TriggerType::Ambient;
    FadeInTime = 2.0f;
    FadeOutTime = 1.5f;
    VolumeMultiplier = 1.0f;
    bLoopAudio = true;
    bPlayOnce = false;
    bHasTriggered = false;

    // Set up collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AAudio_DynamicAudioTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &AAudio_DynamicAudioTrigger::OnTriggerEnter);
    OnActorEndOverlap.AddDynamic(this, &AAudio_DynamicAudioTrigger::OnTriggerExit);

    // Configure audio component
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(0.0f);
        AudioComponent->bAutoActivate = false;
    }
}

void AAudio_DynamicAudioTrigger::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a character (player)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (bPlayOnce && bHasTriggered)
        {
            return;
        }

        PlayAudioForTriggerType();
        bHasTriggered = true;

        UE_LOG(LogTemp, Warning, TEXT("Audio Trigger Activated: %s"), *GetActorLabel());
    }
}

void AAudio_DynamicAudioTrigger::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (!bPlayOnce)
        {
            StopAudioWithFade();
        }

        UE_LOG(LogTemp, Warning, TEXT("Audio Trigger Deactivated: %s"), *GetActorLabel());
    }
}

void AAudio_DynamicAudioTrigger::PlayAudioForTriggerType()
{
    if (!AudioComponent)
    {
        return;
    }

    USoundCue* SoundToPlay = nullptr;

    switch (TriggerType)
    {
        case EAudio_TriggerType::Ambient:
        case EAudio_TriggerType::Discovery:
            SoundToPlay = AmbientSound;
            break;
        case EAudio_TriggerType::Narrative:
            SoundToPlay = NarrativeSound;
            break;
        case EAudio_TriggerType::Danger:
        case EAudio_TriggerType::Combat:
            SoundToPlay = AmbientSound; // Can be customized for danger sounds
            break;
    }

    if (SoundToPlay)
    {
        AudioComponent->SetSound(SoundToPlay);
        AudioComponent->SetVolumeMultiplier(0.0f);
        AudioComponent->Play();

        // Fade in
        AudioComponent->FadeIn(FadeInTime, VolumeMultiplier);
    }
}

void AAudio_DynamicAudioTrigger::StopAudioWithFade()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->FadeOut(FadeOutTime, 0.0f);
    }
}