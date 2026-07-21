#include "Narr_StoryTrigger.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ANarr_StoryTrigger::ANarr_StoryTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create dialogue component
    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueSystem>(TEXT("DialogueComponent"));

    // Set default values
    TriggerEventType = ENarr_TriggerEvent::FirstEncounter;
    TriggerName = TEXT("Story Trigger");
    bOneTimeUse = true;
    bHasBeenTriggered = false;
    DelayBeforeDialogue = 1.0f;

    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &ANarr_StoryTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &ANarr_StoryTrigger::OnOverlapEnd);
}

void ANarr_StoryTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup dialogue based on trigger type
    SetupDialogueForEvent(TriggerEventType);
    
    UE_LOG(LogTemp, Log, TEXT("Story Trigger '%s' initialized with event type %d"), 
        *TriggerName, (int32)TriggerEventType);
}

void ANarr_StoryTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
    // Check if the overlapping actor is the player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    // Check if this is a one-time trigger that has already been used
    if (bOneTimeUse && bHasBeenTriggered)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Player entered story trigger: %s"), *TriggerName);
    
    // Trigger the story event with a delay
    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &ANarr_StoryTrigger::TriggerStoryEvent, DelayBeforeDialogue, false);
}

void ANarr_StoryTrigger::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Player left story trigger: %s"), *TriggerName);
}

void ANarr_StoryTrigger::TriggerStoryEvent()
{
    if (bOneTimeUse && bHasBeenTriggered)
    {
        return;
    }

    bHasBeenTriggered = true;

    if (DialogueComponent)
    {
        DialogueComponent->StartDialogue();
    }

    // Log the event
    FString EventName = TEXT("Unknown");
    switch (TriggerEventType)
    {
        case ENarr_TriggerEvent::FirstEncounter:
            EventName = TEXT("First Encounter");
            break;
        case ENarr_TriggerEvent::DangerWarning:
            EventName = TEXT("Danger Warning");
            break;
        case ENarr_TriggerEvent::ResourceDiscovery:
            EventName = TEXT("Resource Discovery");
            break;
        case ENarr_TriggerEvent::TerritoryEntry:
            EventName = TEXT("Territory Entry");
            break;
        case ENarr_TriggerEvent::CombatStart:
            EventName = TEXT("Combat Start");
            break;
    }

    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Story Event Triggered: %s - %s"), *TriggerName, *EventName);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Message);
    }

    UE_LOG(LogTemp, Log, TEXT("Story Event Triggered: %s - %s"), *TriggerName, *EventName);
}

void ANarr_StoryTrigger::SetupDialogueForEvent(ENarr_TriggerEvent EventType)
{
    if (!DialogueComponent)
    {
        return;
    }

    // Clear existing dialogue
    DialogueComponent->DialogueEntries.Empty();

    switch (EventType)
    {
        case ENarr_TriggerEvent::FirstEncounter:
        {
            FNarr_DialogueEntry Entry;
            Entry.SpeakerName = TEXT("Ancient Voice");
            Entry.DialogueText = FText::FromString(TEXT("You have entered the primordial lands. Here, only the strong survive."));
            Entry.DialogueType = ENarr_DialogueType::Information;
            Entry.DisplayDuration = 4.0f;
            DialogueComponent->AddDialogueEntry(Entry);
            break;
        }
        case ENarr_TriggerEvent::DangerWarning:
        {
            FNarr_DialogueEntry Entry;
            Entry.SpeakerName = TEXT("Tribal Scout");
            Entry.DialogueText = FText::FromString(TEXT("Danger stalks these paths! The great hunters are near!"));
            Entry.DialogueType = ENarr_DialogueType::Warning;
            Entry.DisplayDuration = 3.0f;
            DialogueComponent->AddDialogueEntry(Entry);
            break;
        }
        case ENarr_TriggerEvent::ResourceDiscovery:
        {
            FNarr_DialogueEntry Entry;
            Entry.SpeakerName = TEXT("Elder Gatherer");
            Entry.DialogueText = FText::FromString(TEXT("Rich resources lie hidden here. Gather what you can before the predators return."));
            Entry.DialogueType = ENarr_DialogueType::Survival;
            Entry.DisplayDuration = 4.5f;
            DialogueComponent->AddDialogueEntry(Entry);
            break;
        }
        case ENarr_TriggerEvent::TerritoryEntry:
        {
            FNarr_DialogueEntry Entry;
            Entry.SpeakerName = TEXT("Territory Guardian");
            Entry.DialogueText = FText::FromString(TEXT("You trespass on sacred hunting grounds. Prove your worth or face the consequences."));
            Entry.DialogueType = ENarr_DialogueType::Quest;
            Entry.DisplayDuration = 5.0f;
            DialogueComponent->AddDialogueEntry(Entry);
            break;
        }
        case ENarr_TriggerEvent::CombatStart:
        {
            FNarr_DialogueEntry Entry;
            Entry.SpeakerName = TEXT("Battle Cry");
            Entry.DialogueText = FText::FromString(TEXT("The hunt begins! Strike fast and true, or become prey yourself!"));
            Entry.DialogueType = ENarr_DialogueType::Combat;
            Entry.DisplayDuration = 2.5f;
            DialogueComponent->AddDialogueEntry(Entry);
            break;
        }
    }
}

void ANarr_StoryTrigger::ResetTrigger()
{
    bHasBeenTriggered = false;
    UE_LOG(LogTemp, Log, TEXT("Story Trigger '%s' has been reset"), *TriggerName);
}