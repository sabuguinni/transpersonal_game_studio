#include "Audio_EnvironmentalTriggers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UAudio_EnvironmentalTriggerComponent::UAudio_EnvironmentalTriggerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    TriggerRadius = 500.0f;
    bTriggerOnce = false;
    bHasTriggered = false;
    
    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudio"));
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(0.8f);
    }
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->bAutoActivate = false;
        NarrativeAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UAudio_EnvironmentalTriggerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Attach audio components to owner
    if (AActor* Owner = GetOwner())
    {
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
        }
        
        if (NarrativeAudioComponent)
        {
            NarrativeAudioComponent->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
        }
    }
}

void UAudio_EnvironmentalTriggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopEnvironmentalAudio();
    Super::EndPlay(EndPlayReason);
}

void UAudio_EnvironmentalTriggerComponent::TriggerEnvironmentalAudio(AActor* TriggeringActor)
{
    // Check if we should trigger only once
    if (bTriggerOnce && bHasTriggered)
    {
        return;
    }
    
    // Verify triggering actor is a character (player)
    if (!TriggeringActor || !TriggeringActor->IsA<ACharacter>())
    {
        return;
    }
    
    bHasTriggered = true;
    
    // Play ambient sound
    PlayAmbientSound();
    
    // Play narrative voiceline after a short delay
    if (GetWorld())
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, 
            &UAudio_EnvironmentalTriggerComponent::PlayNarrativeVoiceline, 1.5f, false);
    }
    
    // Update player fear level based on trigger type
    UpdatePlayerFearLevel(TriggerData.FearImpact);
    
    UE_LOG(LogTemp, Log, TEXT("Environmental audio triggered by %s"), 
        *TriggeringActor->GetName());
}

void UAudio_EnvironmentalTriggerComponent::StopEnvironmentalAudio()
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
}

void UAudio_EnvironmentalTriggerComponent::SetTriggerData(const FAudio_EnvironmentalTriggerData& NewTriggerData)
{
    TriggerData = NewTriggerData;
    
    // Update audio components with new data
    if (AmbientAudioComponent && TriggerData.AmbientSound.IsValid())
    {
        if (USoundBase* Sound = TriggerData.AmbientSound.LoadSynchronous())
        {
            AmbientAudioComponent->SetSound(Sound);
            AmbientAudioComponent->SetVolumeMultiplier(TriggerData.AudioIntensity);
        }
    }
    
    if (NarrativeAudioComponent && TriggerData.NarrativeVoiceline.IsValid())
    {
        if (USoundBase* Sound = TriggerData.NarrativeVoiceline.LoadSynchronous())
        {
            NarrativeAudioComponent->SetSound(Sound);
        }
    }
}

void UAudio_EnvironmentalTriggerComponent::PlayAmbientSound()
{
    if (!AmbientAudioComponent)
    {
        return;
    }
    
    // Load and play ambient sound
    if (TriggerData.AmbientSound.IsValid())
    {
        if (USoundBase* Sound = TriggerData.AmbientSound.LoadSynchronous())
        {
            AmbientAudioComponent->SetSound(Sound);
            AmbientAudioComponent->SetVolumeMultiplier(TriggerData.AudioIntensity);
            
            if (TriggerData.bLoopAmbientSound)
            {
                AmbientAudioComponent->Play();
            }
            else
            {
                AmbientAudioComponent->Play();
            }
            
            UE_LOG(LogTemp, Log, TEXT("Playing ambient sound for trigger type: %d"), 
                (int32)TriggerData.TriggerType);
        }
    }
}

void UAudio_EnvironmentalTriggerComponent::PlayNarrativeVoiceline()
{
    if (!NarrativeAudioComponent)
    {
        return;
    }
    
    // Load and play narrative voiceline
    if (TriggerData.NarrativeVoiceline.IsValid())
    {
        if (USoundBase* Sound = TriggerData.NarrativeVoiceline.LoadSynchronous())
        {
            NarrativeAudioComponent->SetSound(Sound);
            NarrativeAudioComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("Playing narrative voiceline for trigger type: %d"), 
                (int32)TriggerData.TriggerType);
        }
    }
}

void UAudio_EnvironmentalTriggerComponent::UpdatePlayerFearLevel(float FearDelta)
{
    // Find player character and update fear level
    if (UWorld* World = GetWorld())
    {
        if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0))
        {
            // Try to find a fear component or property on the player
            // This would integrate with the survival system
            UE_LOG(LogTemp, Log, TEXT("Updating player fear level by %f"), FearDelta);
            
            // TODO: Integrate with actual player survival stats component
            // For now, just log the fear impact
        }
    }
}

// AAudio_EnvironmentalTriggerActor Implementation

AAudio_EnvironmentalTriggerActor::AAudio_EnvironmentalTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create environmental trigger component
    EnvironmentalTriggerComponent = CreateDefaultSubobject<UAudio_EnvironmentalTriggerComponent>(TEXT("EnvironmentalTriggerComponent"));
    
    bActivateOnEntry = true;
    bDeactivateOnExit = false;
    
    // Set up collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAudio_EnvironmentalTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &AAudio_EnvironmentalTriggerActor::OnActorBeginOverlap);
    OnActorEndOverlap.AddDynamic(this, &AAudio_EnvironmentalTriggerActor::OnActorEndOverlap);
}

void AAudio_EnvironmentalTriggerActor::OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (bActivateOnEntry && EnvironmentalTriggerComponent)
    {
        EnvironmentalTriggerComponent->TriggerEnvironmentalAudio(OtherActor);
    }
}

void AAudio_EnvironmentalTriggerActor::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (bDeactivateOnExit && EnvironmentalTriggerComponent)
    {
        EnvironmentalTriggerComponent->StopEnvironmentalAudio();
    }
}