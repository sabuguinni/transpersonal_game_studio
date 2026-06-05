#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    InteractionRange = 300.0f;
    bCanInitiateDialogue = true;
    CurrentConversation = nullptr;
    CurrentLineIndex = 0;
    ConversationPartner = nullptr;
}

bool UNarr_DialogueComponent::StartConversation(const FString& ConversationID, AActor* Initiator)
{
    if (IsInConversation())
    {
        return false;
    }

    for (FNarr_DialogueConversation& Conversation : AvailableConversations)
    {
        if (Conversation.ConversationID == ConversationID)
        {
            CurrentConversation = &Conversation;
            CurrentLineIndex = 0;
            ConversationPartner = Initiator;
            
            UE_LOG(LogTemp, Log, TEXT("Started conversation: %s"), *ConversationID);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Conversation not found: %s"), *ConversationID);
    return false;
}

void UNarr_DialogueComponent::EndConversation()
{
    if (CurrentConversation)
    {
        UE_LOG(LogTemp, Log, TEXT("Ended conversation: %s"), *CurrentConversation->ConversationID);
        
        CurrentConversation = nullptr;
        CurrentLineIndex = 0;
        ConversationPartner = nullptr;
    }
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentDialogueLine() const
{
    if (CurrentConversation && CurrentLineIndex < CurrentConversation->DialogueLines.Num())
    {
        return CurrentConversation->DialogueLines[CurrentLineIndex];
    }
    
    return FNarr_DialogueLine();
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!CurrentConversation)
    {
        return false;
    }

    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentConversation->DialogueLines.Num())
    {
        EndConversation();
        return false;
    }

    return true;
}

bool UNarr_DialogueComponent::IsInConversation() const
{
    return CurrentConversation != nullptr;
}

void UNarr_DialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Subsystem initialized"));
}

void UNarr_DialogueSubsystem::RegisterDialogueActor(AActor* Actor, UNarr_DialogueComponent* DialogueComponent)
{
    if (Actor && DialogueComponent)
    {
        RegisteredDialogueActors.Add(Actor, DialogueComponent);
        UE_LOG(LogTemp, Log, TEXT("Registered dialogue actor: %s"), *Actor->GetName());
    }
}

void UNarr_DialogueSubsystem::UnregisterDialogueActor(AActor* Actor)
{
    if (Actor && RegisteredDialogueActors.Contains(Actor))
    {
        RegisteredDialogueActors.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("Unregistered dialogue actor: %s"), *Actor->GetName());
    }
}

TArray<AActor*> UNarr_DialogueSubsystem::GetNearbyDialogueActors(const FVector& Location, float Radius) const
{
    TArray<AActor*> NearbyActors;
    
    for (const auto& ActorPair : RegisteredDialogueActors)
    {
        AActor* Actor = ActorPair.Key;
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Location) <= Radius)
        {
            NearbyActors.Add(Actor);
        }
    }
    
    return NearbyActors;
}

void UNarr_DialogueSubsystem::PlayDialogueAudio(const FString& AudioFilePath)
{
    if (!AudioFilePath.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioFilePath);
        
        // Load and play audio file
        USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *AudioFilePath);
        if (DialogueSound)
        {
            UGameplayStatics::PlaySound2D(this, DialogueSound);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load dialogue audio: %s"), *AudioFilePath);
        }
    }
}