#include "StoryEventManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UStoryEventManager::UStoryEventManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    TriggerDistance = 500.0f;
    PlayerCharacter = nullptr;
}

void UStoryEventManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character
    PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    
    // Initialize default story events
    InitializeDefaultEvents();
}

void UStoryEventManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (PlayerCharacter)
    {
        CheckProximityTriggers();
    }
}

void UStoryEventManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : ActiveStoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            ProcessStoryEvent(Event);
            break;
        }
    }
}

void UStoryEventManager::AddStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    ActiveStoryEvents.Add(NewEvent);
}

bool UStoryEventManager::IsEventCompleted(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : CompletedStoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return true;
        }
    }
    return false;
}

FNarr_StoryEvent UStoryEventManager::GetStoryEvent(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : ActiveStoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event;
        }
    }
    
    for (const FNarr_StoryEvent& Event : CompletedStoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event;
        }
    }
    
    return FNarr_StoryEvent();
}

void UStoryEventManager::InitializeDefaultEvents()
{
    // First encounter story event
    FNarr_StoryEvent FirstEncounter;
    FirstEncounter.EventID = TEXT("FirstDinosaurEncounter");
    FirstEncounter.DialogueText = TEXT("The ground shakes... something massive approaches. Stay low and move carefully.");
    FirstEncounter.CharacterName = TEXT("InnerVoice");
    FirstEncounter.TriggerType = ENarr_StoryTriggerType::ProximityTrigger;
    AddStoryEvent(FirstEncounter);
    
    // Safe camp discovery
    FNarr_StoryEvent SafeCamp;
    SafeCamp.EventID = TEXT("SafeCampDiscovery");
    SafeCamp.DialogueText = TEXT("Finally, a place to rest. The fire will keep the predators at bay through the night.");
    SafeCamp.CharacterName = TEXT("InnerVoice");
    SafeCamp.TriggerType = ENarr_StoryTriggerType::ProximityTrigger;
    AddStoryEvent(SafeCamp);
    
    // Ancient ruins discovery
    FNarr_StoryEvent AncientRuins;
    AncientRuins.EventID = TEXT("AncientRuinsDiscovery");
    AncientRuins.DialogueText = TEXT("These stones... they were placed by hands like mine. Others have walked this path before.");
    AncientRuins.CharacterName = TEXT("InnerVoice");
    AncientRuins.TriggerType = ENarr_StoryTriggerType::ProximityTrigger;
    AddStoryEvent(AncientRuins);
    
    // Survival milestone
    FNarr_StoryEvent SurvivalMilestone;
    SurvivalMilestone.EventID = TEXT("FirstDaySurvival");
    SurvivalMilestone.DialogueText = TEXT("The sun sets on my first day in this ancient world. I am still alive. That is enough for now.");
    SurvivalMilestone.CharacterName = TEXT("InnerVoice");
    SurvivalMilestone.TriggerType = ENarr_StoryTriggerType::TimedTrigger;
    AddStoryEvent(SurvivalMilestone);
}

void UStoryEventManager::CheckProximityTriggers()
{
    if (!PlayerCharacter)
        return;
        
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Check proximity to story trigger locations
    TArray<FVector> TriggerLocations = {
        FVector(2000, 0, 100),    // First encounter
        FVector(-1500, 1000, 150), // Safe camp
        FVector(0, -2000, 200)     // Ancient ruins
    };
    
    TArray<FString> TriggerEventIDs = {
        TEXT("FirstDinosaurEncounter"),
        TEXT("SafeCampDiscovery"),
        TEXT("AncientRuinsDiscovery")
    };
    
    for (int32 i = 0; i < TriggerLocations.Num(); i++)
    {
        float Distance = FVector::Dist(PlayerLocation, TriggerLocations[i]);
        if (Distance <= TriggerDistance)
        {
            if (!IsEventCompleted(TriggerEventIDs[i]))
            {
                TriggerStoryEvent(TriggerEventIDs[i]);
            }
        }
    }
}

void UStoryEventManager::ProcessStoryEvent(FNarr_StoryEvent& Event)
{
    // Mark event as completed
    Event.bIsCompleted = true;
    
    // Move to completed events
    CompletedStoryEvents.Add(Event);
    ActiveStoryEvents.RemoveAll([&Event](const FNarr_StoryEvent& E) {
        return E.EventID == Event.EventID;
    });
    
    // Display dialogue (in a real implementation, this would trigger UI)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *Event.CharacterName, *Event.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Story Event Triggered: %s - %s"), *Event.EventID, *Event.DialogueText);
}