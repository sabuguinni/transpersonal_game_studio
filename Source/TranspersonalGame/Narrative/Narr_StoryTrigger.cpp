#include "Narr_StoryTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ANarr_StoryTrigger::ANarr_StoryTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize trigger properties
    TriggerType = ENarr_TriggerType::FirstDinosaur;
    bIsActive = true;
    bHasBeenTriggered = false;
    ThreatLevel = 0.5f;
    bIncreaseFearOnTrigger = false;
    FearIncreaseAmount = 10.0f;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Set default trigger box size
    UBoxComponent* BoxComp = GetCollisionComponent();
    if (BoxComp)
    {
        BoxComp->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    }

    // Initialize default story event
    StoryEvent.EventID = TEXT("DefaultEvent");
    StoryEvent.NarrativeText = FText::FromString(TEXT("Something important happened here..."));
    StoryEvent.VoiceOverCue = nullptr;
    StoryEvent.DisplayDuration = 5.0f;
    StoryEvent.bRequiresPlayerInput = false;
    StoryEvent.bOnlyTriggerOnce = true;
}

void ANarr_StoryTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &ANarr_StoryTrigger::OnTriggerEnter);
    OnActorEndOverlap.AddDynamic(this, &ANarr_StoryTrigger::OnTriggerExit);

    UE_LOG(LogTemp, Log, TEXT("Narr_StoryTrigger initialized: %s"), *StoryEvent.EventID);
}

void ANarr_StoryTrigger::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
                                       bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if it's the player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter || !bIsActive)
    {
        return;
    }

    // Check if this trigger should only fire once
    if (StoryEvent.bOnlyTriggerOnce && bHasBeenTriggered)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Player entered story trigger: %s"), *StoryEvent.EventID);

    // Trigger the story event
    TriggerStoryEvent();

    // Call Blueprint implementable event
    OnPlayerEnterTrigger();
}

void ANarr_StoryTrigger::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Player exited story trigger: %s"), *StoryEvent.EventID);

    // Call Blueprint implementable event
    OnPlayerExitTrigger();
}

void ANarr_StoryTrigger::TriggerStoryEvent()
{
    if (!bIsActive)
    {
        return;
    }

    // Mark as triggered
    bHasBeenTriggered = true;

    // Play voice-over if available
    PlayVoiceOver();

    // Update player fear if configured
    if (bIncreaseFearOnTrigger)
    {
        UpdatePlayerFear();
    }

    // Display narrative text (this would typically be handled by UI system)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[NARRATIVE] %s"), 
                                            *StoryEvent.NarrativeText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, StoryEvent.DisplayDuration, FColor::Yellow, DisplayText);
    }

    // Log the event
    UE_LOG(LogTemp, Warning, TEXT("Story Event Triggered: %s - %s"), 
           *StoryEvent.EventID, *StoryEvent.NarrativeText.ToString());

    // Call Blueprint implementable event
    OnStoryEventTriggered(StoryEvent);
}

void ANarr_StoryTrigger::ResetTrigger()
{
    bHasBeenTriggered = false;
    bIsActive = true;
    
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    UE_LOG(LogTemp, Log, TEXT("Story trigger reset: %s"), *StoryEvent.EventID);
}

void ANarr_StoryTrigger::SetTriggerActive(bool bActive)
{
    bIsActive = bActive;
    
    if (!bActive && AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    UE_LOG(LogTemp, Log, TEXT("Story trigger %s set to: %s"), 
           *StoryEvent.EventID, bActive ? TEXT("Active") : TEXT("Inactive"));
}

void ANarr_StoryTrigger::PlayVoiceOver()
{
    if (StoryEvent.VoiceOverCue && AudioComponent)
    {
        AudioComponent->SetSound(StoryEvent.VoiceOverCue);
        AudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing voice-over for story event: %s"), *StoryEvent.EventID);
    }
}

void ANarr_StoryTrigger::UpdatePlayerFear()
{
    // Find the player character and update fear
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        // This would typically interface with the player's survival stats system
        // For now, we'll just log the fear increase
        UE_LOG(LogTemp, Warning, TEXT("Player fear increased by %.1f due to story trigger: %s"), 
               FearIncreaseAmount, *StoryEvent.EventID);
        
        // In a real implementation, this would call something like:
        // PlayerCharacter->GetSurvivalComponent()->IncreaseFear(FearIncreaseAmount);
    }
}