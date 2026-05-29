#include "Narr_StoryTriggerSystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ANarr_StoryTriggerSystem::ANarr_StoryTriggerSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Setup collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    bIsTriggered = false;
    LastTriggerTime = 0.0f;

    // Setup default trigger data
    TriggerData.TriggerName = TEXT("Story Location");
    TriggerData.TriggerType = ENarr_TriggerType::LocationDiscovery;
    TriggerData.NarrativeText = TEXT("You have discovered an important location.");
    TriggerData.bCanRetrigger = false;
    TriggerData.RetriggerCooldown = 60.0f;

    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &ANarr_StoryTriggerSystem::OnActorBeginOverlap);
    OnActorEndOverlap.AddDynamic(this, &ANarr_StoryTriggerSystem::OnActorEndOverlap);
}

void ANarr_StoryTriggerSystem::BeginPlay()
{
    Super::BeginPlay();
    
    LogTriggerEvent(TEXT("Story trigger initialized"));
}

void ANarr_StoryTriggerSystem::OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor)
        return;

    // Check if the overlapping actor is a character (player)
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && Character->IsPlayerControlled())
    {
        TriggerStoryEvent(Character);
    }
}

void ANarr_StoryTriggerSystem::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    // Optional: Handle exit events if needed
}

void ANarr_StoryTriggerSystem::TriggerStoryEvent(AActor* TriggeringActor)
{
    if (!CanTrigger())
    {
        LogTriggerEvent(TEXT("Trigger blocked - cooldown or already triggered"));
        return;
    }

    // Mark as triggered
    bIsTriggered = true;
    LastTriggerTime = GetWorld()->GetTimeSeconds();

    // Log the trigger event
    LogTriggerEvent(FString::Printf(TEXT("Story triggered: %s"), *TriggerData.TriggerName));

    // Play narrative audio if available
    PlayNarrativeAudio();

    // Fire Blueprint event
    OnStoryTriggered(TriggerData);

    // Display text on screen for debugging
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[STORY] %s: %s"), 
            *TriggerData.TriggerName, 
            *TriggerData.NarrativeText);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }
}

bool ANarr_StoryTriggerSystem::CanTrigger() const
{
    // If already triggered and can't retrigger
    if (bIsTriggered && !TriggerData.bCanRetrigger)
    {
        return false;
    }

    // If can retrigger, check cooldown
    if (bIsTriggered && TriggerData.bCanRetrigger)
    {
        float TimeSinceLastTrigger = GetWorld()->GetTimeSeconds() - LastTriggerTime;
        return TimeSinceLastTrigger >= TriggerData.RetriggerCooldown;
    }

    return true;
}

void ANarr_StoryTriggerSystem::ResetTrigger()
{
    bIsTriggered = false;
    LastTriggerTime = 0.0f;
    LogTriggerEvent(TEXT("Trigger reset"));
}

void ANarr_StoryTriggerSystem::PlayNarrativeAudio()
{
    if (!AudioComponent)
        return;

    // For now, just log that audio would play
    // In a full implementation, this would load and play the audio from AudioURL
    LogTriggerEvent(FString::Printf(TEXT("Playing narrative audio: %s (%.1fs)"), 
        *TriggerData.AudioURL, TriggerData.AudioDuration));

    // TODO: Implement actual audio loading and playback from URL
    // This would require additional systems for downloading and caching audio
}

void ANarr_StoryTriggerSystem::LogTriggerEvent(const FString& EventType)
{
    UE_LOG(LogTemp, Log, TEXT("[StoryTrigger] %s - %s"), *GetName(), *EventType);
}