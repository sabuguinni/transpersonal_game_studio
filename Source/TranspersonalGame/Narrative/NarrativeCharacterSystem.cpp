#include "NarrativeCharacterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

UNarrativeCharacterComponent::UNarrativeCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize default values
    Background.CharacterName = TEXT("Unknown Survivor");
    Background.Role = ENPCRole::Survivor;
    Background.OriginTribe = TEXT("Wanderer");
    Background.CurrentGoal = TEXT("Survive another day");

    PlayerRelationship.NPCName = Background.CharacterName;
    PlayerRelationship.TrustLevel = 0.0f;
    PlayerRelationship.RespectLevel = 0.0f;
    PlayerRelationship.InteractionCount = 0;
    PlayerRelationship.bHasMetPlayer = false;

    CurrentMood = TEXT("Cautious");
    bCanInteract = true;
    CurrentStoryArc = TEXT("FirstContact");

    // Add default dialogue options
    AvailableDialogues.Add(TEXT("Greeting_Cautious"));
    AvailableDialogues.Add(TEXT("Warning_General"));
    AvailableDialogues.Add(TEXT("Survival_Tips"));
}

void UNarrativeCharacterComponent::BeginPlay()
{
    Super::BeginPlay();

    // Register with the narrative subsystem
    if (UNarrativeCharacterSubsystem* NarrativeSystem = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeCharacterSubsystem>())
    {
        FString CharacterID = FString::Printf(TEXT("%s_%s"), *Background.CharacterName, *GetOwner()->GetName());
        NarrativeSystem->RegisterCharacter(CharacterID, this);
    }
}

void UNarrativeCharacterComponent::UpdateRelationship(float TrustChange, float RespectChange)
{
    PlayerRelationship.TrustLevel = FMath::Clamp(PlayerRelationship.TrustLevel + TrustChange, -100.0f, 100.0f);
    PlayerRelationship.RespectLevel = FMath::Clamp(PlayerRelationship.RespectLevel + RespectChange, -100.0f, 100.0f);
    PlayerRelationship.InteractionCount++;
    PlayerRelationship.LastInteraction = FDateTime::Now();

    if (!PlayerRelationship.bHasMetPlayer)
    {
        PlayerRelationship.bHasMetPlayer = true;
        UE_LOG(LogTemp, Log, TEXT("First meeting with %s"), *Background.CharacterName);
    }

    // Update mood based on relationship
    if (PlayerRelationship.TrustLevel > 50.0f)
    {
        CurrentMood = TEXT("Friendly");
    }
    else if (PlayerRelationship.TrustLevel < -25.0f)
    {
        CurrentMood = TEXT("Hostile");
    }
    else if (PlayerRelationship.RespectLevel > 30.0f)
    {
        CurrentMood = TEXT("Respectful");
    }
    else
    {
        CurrentMood = TEXT("Neutral");
    }

    // Notify subsystem of relationship change
    if (UNarrativeCharacterSubsystem* NarrativeSystem = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeCharacterSubsystem>())
    {
        FString CharacterID = FString::Printf(TEXT("%s_%s"), *Background.CharacterName, *GetOwner()->GetName());
        NarrativeSystem->UpdateGlobalRelationship(CharacterID, TrustChange, RespectChange);
    }
}

FString UNarrativeCharacterComponent::GetContextualDialogue(const FString& Context)
{
    FString DialogueKey = FString::Printf(TEXT("%s_%s_%s"), *Context, *CurrentMood, *Background.Role.ToString());
    
    // Generate contextual dialogue based on personality and relationship
    if (Context == TEXT("Danger"))
    {
        if (Background.Personality.Courage > 70.0f)
        {
            return TEXT("Stand your ground! We face this threat together!");
        }
        else if (Background.Personality.Courage < 30.0f)
        {
            return TEXT("We should retreat! This is too dangerous!");
        }
        else
        {
            return TEXT("Be careful. Danger approaches.");
        }
    }
    else if (Context == TEXT("Discovery"))
    {
        if (Background.Personality.Curiosity > 60.0f)
        {
            return TEXT("Fascinating! I've never seen anything like this before!");
        }
        else if (Background.Personality.Wisdom > 70.0f)
        {
            return TEXT("This reminds me of the old stories. We should be cautious.");
        }
        else
        {
            return TEXT("What do you make of this?");
        }
    }
    else if (Context == TEXT("Greeting"))
    {
        if (PlayerRelationship.TrustLevel > 50.0f)
        {
            return TEXT("Good to see you again, friend!");
        }
        else if (PlayerRelationship.bHasMetPlayer)
        {
            return TEXT("We meet again.");
        }
        else
        {
            return TEXT("I don't know you. State your business.");
        }
    }

    return TEXT("...");
}

bool UNarrativeCharacterComponent::CanDiscussStoryBeat(const FString& StoryBeat) const
{
    // Check if character has the knowledge/experience for this story beat
    if (Background.Role == ENPCRole::TribalElder)
    {
        return true; // Elders know most stories
    }
    
    if (Background.Role == ENPCRole::Scout && StoryBeat.Contains(TEXT("Location")))
    {
        return true; // Scouts know about locations
    }
    
    if (Background.Role == ENPCRole::Hunter && StoryBeat.Contains(TEXT("Dinosaur")))
    {
        return true; // Hunters know about dinosaurs
    }

    return CompletedStoryBeats.Contains(StoryBeat);
}

void UNarrativeCharacterComponent::MarkStoryBeatCompleted(const FString& StoryBeat)
{
    if (!CompletedStoryBeats.Contains(StoryBeat))
    {
        CompletedStoryBeats.Add(StoryBeat);
        UE_LOG(LogTemp, Log, TEXT("%s completed story beat: %s"), *Background.CharacterName, *StoryBeat);
    }
}

float UNarrativeCharacterComponent::GetPersonalityTrait(const FString& TraitName) const
{
    if (TraitName == TEXT("Courage"))
    {
        return Background.Personality.Courage;
    }
    else if (TraitName == TEXT("Wisdom"))
    {
        return Background.Personality.Wisdom;
    }
    else if (TraitName == TEXT("Aggression"))
    {
        return Background.Personality.Aggression;
    }
    else if (TraitName == TEXT("Trust"))
    {
        return Background.Personality.Trust;
    }
    else if (TraitName == TEXT("Curiosity"))
    {
        return Background.Personality.Curiosity;
    }
    
    return 50.0f; // Default neutral value
}

// Subsystem Implementation
void UNarrativeCharacterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentMainStoryArc = TEXT("Awakening");
    
    // Initialize core story beats
    GlobalStoryBeats.Add(TEXT("FirstAwakening"));
    GlobalStoryBeats.Add(TEXT("FirstDinosaur"));
    GlobalStoryBeats.Add(TEXT("FirstTribe"));
    GlobalStoryBeats.Add(TEXT("FirstCraft"));
    GlobalStoryBeats.Add(TEXT("FirstHunt"));
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeCharacterSubsystem initialized"));
}

void UNarrativeCharacterSubsystem::RegisterCharacter(const FString& CharacterID, UNarrativeCharacterComponent* Character)
{
    if (Character && !CharacterID.IsEmpty())
    {
        RegisteredCharacters.Add(CharacterID, Character);
        
        // Initialize relationship tracking
        FNarr_NPCRelationship& Relationship = PlayerRelationships.FindOrAdd(CharacterID);
        Relationship.NPCName = Character->Background.CharacterName;
        
        UE_LOG(LogTemp, Log, TEXT("Registered narrative character: %s"), *CharacterID);
    }
}

UNarrativeCharacterComponent* UNarrativeCharacterSubsystem::GetCharacter(const FString& CharacterID)
{
    if (UNarrativeCharacterComponent** Found = RegisteredCharacters.Find(CharacterID))
    {
        return *Found;
    }
    return nullptr;
}

TArray<UNarrativeCharacterComponent*> UNarrativeCharacterSubsystem::GetCharactersByRole(ENPCRole Role)
{
    TArray<UNarrativeCharacterComponent*> Result;
    
    for (auto& CharacterPair : RegisteredCharacters)
    {
        if (CharacterPair.Value && CharacterPair.Value->Background.Role == Role)
        {
            Result.Add(CharacterPair.Value);
        }
    }
    
    return Result;
}

void UNarrativeCharacterSubsystem::UpdateGlobalRelationship(const FString& CharacterID, float TrustChange, float RespectChange)
{
    if (FNarr_NPCRelationship* Relationship = PlayerRelationships.Find(CharacterID))
    {
        Relationship->TrustLevel = FMath::Clamp(Relationship->TrustLevel + TrustChange, -100.0f, 100.0f);
        Relationship->RespectLevel = FMath::Clamp(Relationship->RespectLevel + RespectChange, -100.0f, 100.0f);
        Relationship->InteractionCount++;
        Relationship->LastInteraction = FDateTime::Now();
    }
}

float UNarrativeCharacterSubsystem::GetRelationshipLevel(const FString& CharacterID, const FString& RelationType)
{
    if (const FNarr_NPCRelationship* Relationship = PlayerRelationships.Find(CharacterID))
    {
        if (RelationType == TEXT("Trust"))
        {
            return Relationship->TrustLevel;
        }
        else if (RelationType == TEXT("Respect"))
        {
            return Relationship->RespectLevel;
        }
    }
    return 0.0f;
}

void UNarrativeCharacterSubsystem::AdvanceStoryBeat(const FString& StoryBeat)
{
    if (!GlobalStoryBeats.Contains(StoryBeat))
    {
        GlobalStoryBeats.Add(StoryBeat);
        UE_LOG(LogTemp, Log, TEXT("Advanced story beat: %s"), *StoryBeat);
        
        // Notify all characters of story progression
        for (auto& CharacterPair : RegisteredCharacters)
        {
            if (CharacterPair.Value)
            {
                CharacterPair.Value->MarkStoryBeatCompleted(StoryBeat);
            }
        }
    }
}

bool UNarrativeCharacterSubsystem::IsStoryBeatCompleted(const FString& StoryBeat) const
{
    return GlobalStoryBeats.Contains(StoryBeat);
}

void UNarrativeCharacterSubsystem::SetMainStoryArc(const FString& NewArc)
{
    CurrentMainStoryArc = NewArc;
    UE_LOG(LogTemp, Log, TEXT("Main story arc changed to: %s"), *NewArc);
}

TArray<FString> UNarrativeCharacterSubsystem::GetRelevantCharactersForLocation(const FVector& Location, float Radius)
{
    TArray<FString> RelevantCharacters;
    
    for (auto& CharacterPair : RegisteredCharacters)
    {
        if (CharacterPair.Value && CharacterPair.Value->GetOwner())
        {
            float Distance = FVector::Dist(CharacterPair.Value->GetOwner()->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                RelevantCharacters.Add(CharacterPair.Key);
            }
        }
    }
    
    return RelevantCharacters;
}

FString UNarrativeCharacterSubsystem::GenerateContextualNarration(const FString& Context, const FVector& Location)
{
    TArray<FString> NearbyCharacters = GetRelevantCharactersForLocation(Location, 2000.0f);
    
    if (NearbyCharacters.Num() > 0)
    {
        // Get the most relevant character for narration
        UNarrativeCharacterComponent* NarrativeChar = GetCharacter(NearbyCharacters[0]);
        if (NarrativeChar)
        {
            return NarrativeChar->GetContextualDialogue(Context);
        }
    }
    
    // Default contextual narration
    if (Context == TEXT("Exploration"))
    {
        return TEXT("The prehistoric world stretches endlessly before you, full of danger and wonder.");
    }
    else if (Context == TEXT("Danger"))
    {
        return TEXT("Something moves in the shadows. Your survival instincts scream danger.");
    }
    else if (Context == TEXT("Discovery"))
    {
        return TEXT("You've found something that could change everything.");
    }
    
    return TEXT("The ancient world holds many secrets.");
}