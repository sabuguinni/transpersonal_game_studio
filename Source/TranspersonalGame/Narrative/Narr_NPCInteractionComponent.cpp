#include "Narr_NPCInteractionComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

UNarr_NPCInteractionComponent::UNarr_NPCInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    InteractionRange = 300.0f;
    bCanInteract = true;
    CurrentInteractingActor = nullptr;
    bIsInteracting = false;

    // Default personality
    Personality.NPCName = TEXT("Tribal Member");
    Personality.CurrentMood = ENarr_NPCMood::Neutral;
    Personality.TrustLevel = 0.5f;
    Personality.FearLevel = 0.3f;
    Personality.KnownConversations.Add(TEXT("Survivor_Tips"));
}

void UNarr_NPCInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateMoodBasedOnStats();
}

void UNarr_NPCInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update mood periodically
    static float MoodUpdateTimer = 0.0f;
    MoodUpdateTimer += DeltaTime;
    if (MoodUpdateTimer >= 5.0f)
    {
        UpdateMoodBasedOnStats();
        MoodUpdateTimer = 0.0f;
    }
}

void UNarr_NPCInteractionComponent::InitiateInteraction(AActor* InteractingActor)
{
    if (!CanInteract(InteractingActor))
    {
        return;
    }

    CurrentInteractingActor = InteractingActor;
    bIsInteracting = true;

    // Slight trust increase when player initiates peaceful interaction
    ModifyTrust(0.05f);

    UE_LOG(LogTemp, Warning, TEXT("NPC %s initiated interaction with %s"), 
           *Personality.NPCName, 
           *InteractingActor->GetName());
}

void UNarr_NPCInteractionComponent::EndInteraction()
{
    if (bIsInteracting)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC %s ended interaction"), *Personality.NPCName);
    }

    CurrentInteractingActor = nullptr;
    bIsInteracting = false;
}

bool UNarr_NPCInteractionComponent::CanInteract(AActor* InteractingActor) const
{
    if (!bCanInteract || !InteractingActor)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), InteractingActor->GetActorLocation());
    if (Distance > InteractionRange)
    {
        return false;
    }

    // Check mood - hostile NPCs won't interact peacefully
    if (Personality.CurrentMood == ENarr_NPCMood::Hostile)
    {
        return false;
    }

    return true;
}

FString UNarr_NPCInteractionComponent::GetGreetingDialogue() const
{
    switch (Personality.CurrentMood)
    {
        case ENarr_NPCMood::Friendly:
            return FString::Printf(TEXT("%s greets you warmly. 'Welcome, friend. The hunt has been good today.'"), *Personality.NPCName);
        
        case ENarr_NPCMood::Neutral:
            return FString::Printf(TEXT("%s nods acknowledgment. 'Stranger. What brings you to our territory?'"), *Personality.NPCName);
        
        case ENarr_NPCMood::Cautious:
            return FString::Printf(TEXT("%s watches you carefully. 'Stay where I can see you. These are dangerous times.'"), *Personality.NPCName);
        
        case ENarr_NPCMood::Fearful:
            return FString::Printf(TEXT("%s trembles slightly. 'Please... I mean no harm. The great beasts stalk these lands.'"), *Personality.NPCName);
        
        case ENarr_NPCMood::Hostile:
            return FString::Printf(TEXT("%s glares with suspicion. 'You are not welcome here. Leave, before trouble finds us both.'"), *Personality.NPCName);
        
        default:
            return FString::Printf(TEXT("%s remains silent."), *Personality.NPCName);
    }
}

void UNarr_NPCInteractionComponent::ModifyTrust(float TrustChange)
{
    Personality.TrustLevel = FMath::Clamp(Personality.TrustLevel + TrustChange, 0.0f, 1.0f);
    UpdateMoodBasedOnStats();
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s trust modified by %f, new trust: %f"), 
           *Personality.NPCName, TrustChange, Personality.TrustLevel);
}

void UNarr_NPCInteractionComponent::ModifyFear(float FearChange)
{
    Personality.FearLevel = FMath::Clamp(Personality.FearLevel + FearChange, 0.0f, 1.0f);
    UpdateMoodBasedOnStats();
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s fear modified by %f, new fear: %f"), 
           *Personality.NPCName, FearChange, Personality.FearLevel);
}

ENarr_NPCMood UNarr_NPCInteractionComponent::GetCurrentMood() const
{
    return Personality.CurrentMood;
}

void UNarr_NPCInteractionComponent::SetPersonality(const FNarr_NPCPersonality& NewPersonality)
{
    Personality = NewPersonality;
    UpdateMoodBasedOnStats();
}

FNarr_NPCPersonality UNarr_NPCInteractionComponent::GetPersonality() const
{
    return Personality;
}

void UNarr_NPCInteractionComponent::UpdateMoodBasedOnStats()
{
    // Determine mood based on trust and fear levels
    if (Personality.FearLevel > 0.7f)
    {
        Personality.CurrentMood = ENarr_NPCMood::Fearful;
    }
    else if (Personality.TrustLevel > 0.7f && Personality.FearLevel < 0.3f)
    {
        Personality.CurrentMood = ENarr_NPCMood::Friendly;
    }
    else if (Personality.TrustLevel < 0.3f && Personality.FearLevel > 0.5f)
    {
        Personality.CurrentMood = ENarr_NPCMood::Hostile;
    }
    else if (Personality.FearLevel > 0.4f || Personality.TrustLevel < 0.4f)
    {
        Personality.CurrentMood = ENarr_NPCMood::Cautious;
    }
    else
    {
        Personality.CurrentMood = ENarr_NPCMood::Neutral;
    }
}

FString UNarr_NPCInteractionComponent::GenerateContextualDialogue() const
{
    // Generate dialogue based on current game state and NPC personality
    TArray<FString> ContextualLines;
    
    switch (Personality.CurrentMood)
    {
        case ENarr_NPCMood::Friendly:
            ContextualLines.Add(TEXT("The hunting grounds are rich today. Share our fire if you wish."));
            ContextualLines.Add(TEXT("I've seen the great beasts move south. Safe travels, friend."));
            ContextualLines.Add(TEXT("Your presence brings comfort in these dangerous times."));
            break;
            
        case ENarr_NPCMood::Fearful:
            ContextualLines.Add(TEXT("Did you hear that roar? Something massive stalks these lands."));
            ContextualLines.Add(TEXT("The elders speak of omens. Dark times approach."));
            ContextualLines.Add(TEXT("Stay close to the fires. The shadows hide terrible things."));
            break;
            
        case ENarr_NPCMood::Cautious:
            ContextualLines.Add(TEXT("Keep your weapons ready. Predators hunt in packs here."));
            ContextualLines.Add(TEXT("I don't know you, but these lands test us all."));
            ContextualLines.Add(TEXT("Trust is earned through survival, stranger."));
            break;
            
        default:
            ContextualLines.Add(TEXT("The hunt continues. As it always has."));
            ContextualLines.Add(TEXT("Survival requires constant vigilance."));
            break;
    }
    
    if (ContextualLines.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, ContextualLines.Num() - 1);
        return ContextualLines[RandomIndex];
    }
    
    return TEXT("...");
}