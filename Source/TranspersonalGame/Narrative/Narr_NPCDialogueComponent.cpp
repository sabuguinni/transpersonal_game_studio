#include "Narr_NPCDialogueComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNarr_NPCDialogueComponent::UNarr_NPCDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    NPCName = "Unknown NPC";
    NPCRole = ENarr_NPCRole::Villager;
    InteractionRange = 200.0f;
    bCanInteract = true;
    CurrentDialogueID = "greeting_01";
    TimesSpokenTo = 0;
    
    // Initialize default personality
    Personality.Friendliness = 0.7f;
    Personality.Caution = 0.5f;
    Personality.Knowledge = 0.6f;
    Personality.Aggression = 0.2f;
    Personality.PreferredTopics.Add("Weather");
    Personality.PreferredTopics.Add("Hunting");
    Personality.PreferredTopics.Add("Survival");
}

void UNarr_NPCDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDialogue();
}

void UNarr_NPCDialogueComponent::InitializeDialogue()
{
    // Set up default dialogue entries based on NPC role
    DialogueEntries.Empty();
    
    FNarr_DialogueEntry GreetingEntry;
    GreetingEntry.DialogueID = "greeting_01";
    GreetingEntry.SpeakerName = NPCName;
    
    switch (NPCRole)
    {
        case ENarr_NPCRole::Elder:
            GreetingEntry.DialogueText = "Welcome, young one. The winds carry news of your journey.";
            GreetingEntry.PlayerResponses.Add("Tell me about this place");
            GreetingEntry.PlayerResponses.Add("I need guidance");
            GreetingEntry.PlayerResponses.Add("Farewell");
            break;
            
        case ENarr_NPCRole::Hunter:
            GreetingEntry.DialogueText = "Another survivor. The beasts grow bolder each day.";
            GreetingEntry.PlayerResponses.Add("What beasts?");
            GreetingEntry.PlayerResponses.Add("Can you teach me to hunt?");
            GreetingEntry.PlayerResponses.Add("I must go");
            break;
            
        case ENarr_NPCRole::Crafter:
            GreetingEntry.DialogueText = "These hands have shaped stone and bone for many seasons.";
            GreetingEntry.PlayerResponses.Add("Can you craft something for me?");
            GreetingEntry.PlayerResponses.Add("What materials do you need?");
            GreetingEntry.PlayerResponses.Add("Maybe later");
            break;
            
        default:
            GreetingEntry.DialogueText = "The day grows long, and dangers lurk in shadows.";
            GreetingEntry.PlayerResponses.Add("What dangers?");
            GreetingEntry.PlayerResponses.Add("Stay safe");
            break;
    }
    
    GreetingEntry.NextDialogueID = "response_01";
    GreetingEntry.bIsQuestRelated = false;
    
    DialogueEntries.Add(GreetingEntry);
}

bool UNarr_NPCDialogueComponent::CanStartDialogue(AActor* PlayerActor)
{
    if (!bCanInteract || !PlayerActor)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= InteractionRange;
}

FNarr_DialogueEntry UNarr_NPCDialogueComponent::GetCurrentDialogue()
{
    FNarr_DialogueEntry* Entry = FindDialogueEntry(CurrentDialogueID);
    if (Entry)
    {
        return *Entry;
    }
    
    // Return default entry if not found
    FNarr_DialogueEntry DefaultEntry;
    DefaultEntry.DialogueID = "default";
    DefaultEntry.SpeakerName = NPCName;
    DefaultEntry.DialogueText = "I have nothing more to say.";
    return DefaultEntry;
}

void UNarr_NPCDialogueComponent::AdvanceDialogue(const FString& PlayerChoice)
{
    FNarr_DialogueEntry* CurrentEntry = FindDialogueEntry(CurrentDialogueID);
    if (CurrentEntry && !CurrentEntry->NextDialogueID.IsEmpty())
    {
        CurrentDialogueID = CurrentEntry->NextDialogueID;
        TimesSpokenTo++;
        UpdatePersonalityBasedOnInteraction();
    }
}

void UNarr_NPCDialogueComponent::ResetDialogue()
{
    CurrentDialogueID = "greeting_01";
    TimesSpokenTo = 0;
}

void UNarr_NPCDialogueComponent::SetDialogueAvailable(bool bAvailable)
{
    bCanInteract = bAvailable;
}

TArray<FString> UNarr_NPCDialogueComponent::GetAvailableResponses()
{
    FNarr_DialogueEntry* Entry = FindDialogueEntry(CurrentDialogueID);
    if (Entry)
    {
        return Entry->PlayerResponses;
    }
    
    return TArray<FString>();
}

float UNarr_NPCDialogueComponent::GetPersonalityTrait(const FString& TraitName)
{
    if (TraitName == "Friendliness")
        return Personality.Friendliness;
    else if (TraitName == "Caution")
        return Personality.Caution;
    else if (TraitName == "Knowledge")
        return Personality.Knowledge;
    else if (TraitName == "Aggression")
        return Personality.Aggression;
    
    return 0.5f; // Default neutral value
}

void UNarr_NPCDialogueComponent::ModifyPersonalityTrait(const FString& TraitName, float Delta)
{
    if (TraitName == "Friendliness")
        Personality.Friendliness = FMath::Clamp(Personality.Friendliness + Delta, 0.0f, 1.0f);
    else if (TraitName == "Caution")
        Personality.Caution = FMath::Clamp(Personality.Caution + Delta, 0.0f, 1.0f);
    else if (TraitName == "Knowledge")
        Personality.Knowledge = FMath::Clamp(Personality.Knowledge + Delta, 0.0f, 1.0f);
    else if (TraitName == "Aggression")
        Personality.Aggression = FMath::Clamp(Personality.Aggression + Delta, 0.0f, 1.0f);
}

FNarr_DialogueEntry* UNarr_NPCDialogueComponent::FindDialogueEntry(const FString& DialogueID)
{
    for (FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

void UNarr_NPCDialogueComponent::UpdatePersonalityBasedOnInteraction()
{
    // Slightly increase friendliness with repeated positive interactions
    if (TimesSpokenTo > 0 && TimesSpokenTo % 3 == 0)
    {
        ModifyPersonalityTrait("Friendliness", 0.05f);
        ModifyPersonalityTrait("Caution", -0.02f);
    }
}