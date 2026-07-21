#include "Narr_StoryProgression.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarr_StoryComponent::UNarr_StoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentPhase = ENarr_StoryPhase::Awakening;
    StoryProgressPercentage = 0.0f;
}

void UNarr_StoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default story events
    FNarr_StoryEvent FirstKillEvent;
    FirstKillEvent.EventID = TEXT("FirstDinosaurKill");
    FirstKillEvent.EventTitle = FText::FromString(TEXT("First Blood"));
    FirstKillEvent.EventDescription = FText::FromString(TEXT("Defeat your first dinosaur to prove your hunting prowess"));
    FirstKillEvent.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstKillEvent.CompletionReward = 100.0f;
    
    FNarr_StoryEvent TribalMeetingEvent;
    TribalMeetingEvent.EventID = TEXT("MeetTribalElder");
    TribalMeetingEvent.EventTitle = FText::FromString(TEXT("Elder's Wisdom"));
    TribalMeetingEvent.EventDescription = FText::FromString(TEXT("Speak with the tribal elder to learn ancient hunting techniques"));
    TribalMeetingEvent.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    TribalMeetingEvent.Prerequisites.Add(TEXT("FirstDinosaurKill"));
    TribalMeetingEvent.CompletionReward = 200.0f;
    
    StoryEvents.Add(FirstKillEvent);
    StoryEvents.Add(TribalMeetingEvent);
    
    // Initialize character arcs
    FNarr_CharacterArc ElderArc;
    ElderArc.CharacterName = TEXT("TribalElder");
    ElderArc.IntroductionPhase = ENarr_StoryPhase::FirstHunt;
    ElderArc.RelatedEvents.Add(TEXT("MeetTribalElder"));
    
    CharacterArcs.Add(ElderArc);
}

void UNarr_StoryComponent::AdvanceToPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase > CurrentPhase)
    {
        CurrentPhase = NewPhase;
        StoryProgressPercentage = CalculateStoryProgress();
        
        // Notify story manager
        if (UNarr_StoryManager* StoryManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_StoryManager>())
        {
            StoryManager->SetGlobalPhase(NewPhase);
        }
    }
}

void UNarr_StoryComponent::CompleteStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            Event.bIsCompleted = true;
            StoryProgressPercentage = CalculateStoryProgress();
            
            // Register with global manager
            if (UNarr_StoryManager* StoryManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_StoryManager>())
            {
                StoryManager->RegisterGlobalEvent(EventID);
            }
            break;
        }
    }
}

bool UNarr_StoryComponent::CanAccessEvent(const FString& EventID)
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            // Check phase requirement
            if (CurrentPhase < Event.RequiredPhase)
            {
                return false;
            }
            
            // Check prerequisites
            for (const FString& Prerequisite : Event.Prerequisites)
            {
                bool PrereqMet = false;
                for (const FNarr_StoryEvent& CheckEvent : StoryEvents)
                {
                    if (CheckEvent.EventID == Prerequisite && CheckEvent.bIsCompleted)
                    {
                        PrereqMet = true;
                        break;
                    }
                }
                if (!PrereqMet)
                {
                    return false;
                }
            }
            
            return !Event.bIsCompleted;
        }
    }
    
    return false;
}

TArray<FNarr_StoryEvent> UNarr_StoryComponent::GetAvailableEvents()
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (CanAccessEvent(Event.EventID))
        {
            AvailableEvents.Add(Event);
        }
    }
    
    return AvailableEvents;
}

void UNarr_StoryComponent::IntroduceCharacter(const FString& CharacterName)
{
    for (FNarr_CharacterArc& Arc : CharacterArcs)
    {
        if (Arc.CharacterName == CharacterName && !Arc.bIsMet)
        {
            Arc.bIsMet = true;
            Arc.RelationshipLevel = 1;
            break;
        }
    }
}

void UNarr_StoryComponent::UpdateRelationship(const FString& CharacterName, int32 Delta)
{
    for (FNarr_CharacterArc& Arc : CharacterArcs)
    {
        if (Arc.CharacterName == CharacterName && Arc.bIsMet)
        {
            Arc.RelationshipLevel = FMath::Clamp(Arc.RelationshipLevel + Delta, 0, 100);
            
            // Update global relationship
            if (UNarr_StoryManager* StoryManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_StoryManager>())
            {
                StoryManager->UpdateGlobalRelationship(CharacterName, Arc.RelationshipLevel);
            }
            break;
        }
    }
}

float UNarr_StoryComponent::CalculateStoryProgress()
{
    if (StoryEvents.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedEvents = 0;
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.bIsCompleted)
        {
            CompletedEvents++;
        }
    }
    
    return (float)CompletedEvents / (float)StoryEvents.Num() * 100.0f;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    GlobalStoryPhase = ENarr_StoryPhase::Awakening;
}

void UNarr_StoryManager::SetGlobalPhase(ENarr_StoryPhase NewPhase)
{
    GlobalStoryPhase = NewPhase;
}

ENarr_StoryPhase UNarr_StoryManager::GetGlobalPhase()
{
    return GlobalStoryPhase;
}

void UNarr_StoryManager::RegisterGlobalEvent(const FString& EventID)
{
    if (!EventID.IsEmpty() && !GlobalCompletedEvents.Contains(EventID))
    {
        GlobalCompletedEvents.Add(EventID);
    }
}

bool UNarr_StoryManager::IsEventGloballyCompleted(const FString& EventID)
{
    return GlobalCompletedEvents.Contains(EventID);
}

void UNarr_StoryManager::UpdateGlobalRelationship(const FString& CharacterName, int32 NewLevel)
{
    if (!CharacterName.IsEmpty())
    {
        GlobalCharacterRelationships.Add(CharacterName, NewLevel);
    }
}

int32 UNarr_StoryManager::GetGlobalRelationship(const FString& CharacterName)
{
    if (GlobalCharacterRelationships.Contains(CharacterName))
    {
        return GlobalCharacterRelationships[CharacterName];
    }
    
    return 0;
}