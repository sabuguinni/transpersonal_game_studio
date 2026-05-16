#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second for performance
    
    // Initialize default character data
    CharacterData = FNarr_CharacterArchetype();
    TriggerConditions = FNarr_DialogueTrigger();
    CurrentEmotion = ENarr_EmotionalState::Calm;
    LastTriggerTime = 0.0f;
    CurrentDialogue = FNarr_DialogueEntry();
    
    // Create audio component for voice playback
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
    if (AudioComponent)
    {
        AudioComponent->bAutoActivate = false;
        AudioComponent->SetVolumeMultiplier(0.8f);
        AudioComponent->SetPitchMultiplier(1.0f);
    }
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue entries if none exist
    if (CharacterData.DialogueEntries.Num() == 0)
    {
        // Warning dialogue
        FNarr_DialogueEntry WarningEntry;
        WarningEntry.DialogueText = TEXT("Danger stalks these lands. Keep your spear ready.");
        WarningEntry.DialogueType = ENarr_DialogueType::Warning;
        WarningEntry.TriggerContext = ENarr_SurvivalContext::Danger;
        WarningEntry.RequiredEmotion = ENarr_EmotionalState::Fearful;
        WarningEntry.TriggerDistance = 20.0f;
        WarningEntry.Priority = 8;
        CharacterData.DialogueEntries.Add(WarningEntry);
        
        // Hunting instruction
        FNarr_DialogueEntry HuntingEntry;
        HuntingEntry.DialogueText = TEXT("Follow the tracks. The herd moves toward water at dawn.");
        HuntingEntry.DialogueType = ENarr_DialogueType::Instruction;
        HuntingEntry.TriggerContext = ENarr_SurvivalContext::Hunting;
        HuntingEntry.RequiredEmotion = ENarr_EmotionalState::Calm;
        HuntingEntry.TriggerDistance = 15.0f;
        HuntingEntry.Priority = 6;
        CharacterData.DialogueEntries.Add(HuntingEntry);
        
        // Greeting
        FNarr_DialogueEntry GreetingEntry;
        GreetingEntry.DialogueText = TEXT("Hunter. The spirits watch over your path.");
        GreetingEntry.DialogueType = ENarr_DialogueType::Greeting;
        GreetingEntry.TriggerContext = ENarr_SurvivalContext::Rest;
        GreetingEntry.RequiredEmotion = ENarr_EmotionalState::Calm;
        GreetingEntry.TriggerDistance = 10.0f;
        GreetingEntry.Priority = 3;
        CharacterData.DialogueEntries.Add(GreetingEntry);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Component initialized for %s with %d dialogue entries"), 
           *CharacterData.CharacterName, CharacterData.DialogueEntries.Num());
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Get player actor
    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerActor)
    {
        return;
    }
    
    // Update emotional state based on environment
    float PlayerDistance = GetDistanceToPlayer();
    int32 ThreatLevel = GetNearbyThreatLevel();
    bool IsNearPredator = ThreatLevel >= 3;
    
    // Simple health simulation (would integrate with actual player health system)
    float PlayerHealth = 0.8f; // Placeholder
    UpdateEmotionalState(PlayerHealth, ThreatLevel, IsNearPredator);
    
    // Check if dialogue should trigger
    ENarr_SurvivalContext CurrentContext = GetPlayerSurvivalContext();
    if (ShouldTriggerDialogue(PlayerActor, CurrentContext))
    {
        FNarr_DialogueEntry SelectedDialogue = SelectDialogue(CurrentContext, CurrentEmotion);
        if (!SelectedDialogue.DialogueText.IsEmpty())
        {
            PlayDialogue(SelectedDialogue);
            LastTriggerTime = GetWorld()->GetTimeSeconds();
        }
    }
}

bool UNarr_DialogueComponent::ShouldTriggerDialogue(AActor* PlayerActor, ENarr_SurvivalContext Context)
{
    if (!PlayerActor)
    {
        return false;
    }
    
    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTriggerTime < TriggerConditions.MinTimeBetweenTriggers)
    {
        return false;
    }
    
    // Check distance
    float PlayerDistance = GetDistanceToPlayer();
    if (PlayerDistance > TriggerConditions.ProximityDistance)
    {
        return false;
    }
    
    // Check if we have dialogue for this context
    if (!HasDialogueForContext(Context))
    {
        return false;
    }
    
    // Check threat level
    int32 CurrentThreatLevel = GetNearbyThreatLevel();
    if (CurrentThreatLevel > TriggerConditions.ThreatLevel && Context != ENarr_SurvivalContext::Danger)
    {
        return false;
    }
    
    return true;
}

FNarr_DialogueEntry UNarr_DialogueComponent::SelectDialogue(ENarr_SurvivalContext Context, ENarr_EmotionalState Emotion)
{
    TArray<FNarr_DialogueEntry> CandidateDialogues;
    
    // Filter dialogues by context and emotion
    for (const FNarr_DialogueEntry& Entry : CharacterData.DialogueEntries)
    {
        if (Entry.TriggerContext == Context || Context == ENarr_SurvivalContext::Exploration)
        {
            if (Entry.RequiredEmotion == Emotion || Entry.RequiredEmotion == ENarr_EmotionalState::Calm)
            {
                CandidateDialogues.Add(Entry);
            }
        }
    }
    
    // If no exact matches, try broader search
    if (CandidateDialogues.Num() == 0)
    {
        for (const FNarr_DialogueEntry& Entry : CharacterData.DialogueEntries)
        {
            if (Entry.RequiredEmotion == ENarr_EmotionalState::Calm)
            {
                CandidateDialogues.Add(Entry);
            }
        }
    }
    
    // Select highest priority dialogue
    if (CandidateDialogues.Num() > 0)
    {
        FNarr_DialogueEntry BestDialogue = CandidateDialogues[0];
        for (const FNarr_DialogueEntry& Entry : CandidateDialogues)
        {
            if (Entry.Priority > BestDialogue.Priority)
            {
                BestDialogue = Entry;
            }
        }
        return BestDialogue;
    }
    
    return FNarr_DialogueEntry();
}

void UNarr_DialogueComponent::PlayDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    CurrentDialogue = DialogueEntry;
    
    // Log dialogue to screen and console
    UE_LOG(LogTemp, Log, TEXT("%s: %s"), *CharacterData.CharacterName, *DialogueEntry.DialogueText);
    
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *CharacterData.CharacterName, 
                                            *DialogueEntry.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }
    
    // Play audio if available and audio component exists
    if (AudioComponent && !DialogueEntry.AudioURL.IsEmpty())
    {
        // In a full implementation, this would load and play the audio from the URL
        // For now, we'll use a placeholder sound or text-to-speech integration
        UE_LOG(LogTemp, Log, TEXT("Playing audio from URL: %s"), *DialogueEntry.AudioURL);
    }
}

void UNarr_DialogueComponent::UpdateEmotionalState(float PlayerHealth, float ThreatLevel, bool IsNearPredator)
{
    // Simple emotional state logic based on survival conditions
    if (IsNearPredator || ThreatLevel >= 4)
    {
        CurrentEmotion = ENarr_EmotionalState::Fearful;
    }
    else if (ThreatLevel >= 2)
    {
        CurrentEmotion = ENarr_EmotionalState::Aggressive;
    }
    else if (PlayerHealth < 0.3f)
    {
        CurrentEmotion = ENarr_EmotionalState::Exhausted;
    }
    else
    {
        CurrentEmotion = CharacterData.DefaultEmotion;
    }
}

void UNarr_DialogueComponent::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    CharacterData.DialogueEntries.Add(NewEntry);
    UE_LOG(LogTemp, Log, TEXT("Added new dialogue entry: %s"), *NewEntry.DialogueText);
}

TArray<FNarr_DialogueEntry> UNarr_DialogueComponent::GetDialoguesByType(ENarr_DialogueType Type)
{
    TArray<FNarr_DialogueEntry> FilteredDialogues;
    
    for (const FNarr_DialogueEntry& Entry : CharacterData.DialogueEntries)
    {
        if (Entry.DialogueType == Type)
        {
            FilteredDialogues.Add(Entry);
        }
    }
    
    return FilteredDialogues;
}

bool UNarr_DialogueComponent::HasDialogueForContext(ENarr_SurvivalContext Context)
{
    for (const FNarr_DialogueEntry& Entry : CharacterData.DialogueEntries)
    {
        if (Entry.TriggerContext == Context)
        {
            return true;
        }
    }
    return false;
}

float UNarr_DialogueComponent::GetDistanceToPlayer() const
{
    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerActor || !GetOwner())
    {
        return 9999.0f;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
}

float UNarr_DialogueComponent::GetCurrentTimeOfDay() const
{
    // Placeholder - would integrate with actual day/night cycle
    return 0.5f; // Noon
}

int32 UNarr_DialogueComponent::GetNearbyThreatLevel() const
{
    // Simple threat detection - would integrate with actual dinosaur AI system
    UWorld* World = GetWorld();
    if (!World || !GetOwner())
    {
        return 0;
    }
    
    // Check for actors with "Dinosaur" or "Predator" in their name within 100 meters
    TArray<AActor*> NearbyActors;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= 10000.0f) // 100 meters
            {
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")))
                {
                    return 4; // High threat
                }
                else if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("predator")))
                {
                    return 2; // Medium threat
                }
            }
        }
    }
    
    return 1; // Low/no threat
}

ENarr_SurvivalContext UNarr_DialogueComponent::GetPlayerSurvivalContext() const
{
    // Placeholder logic - would integrate with actual player state system
    int32 ThreatLevel = GetNearbyThreatLevel();
    
    if (ThreatLevel >= 3)
    {
        return ENarr_SurvivalContext::Danger;
    }
    else if (ThreatLevel >= 2)
    {
        return ENarr_SurvivalContext::Combat;
    }
    else
    {
        // Default to exploration when no immediate threats
        return ENarr_SurvivalContext::Exploration;
    }
}