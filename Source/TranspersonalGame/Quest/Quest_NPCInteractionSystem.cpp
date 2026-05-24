#include "Quest_NPCInteractionSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UQuest_NPCInteractionSystem::UQuest_NPCInteractionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentState = EQuest_NPCState::Idle;
    bIsInteractable = true;
    InteractionCooldown = 2.0f;
    InteractionPrompt = TEXT("Press E to interact");
    bHasActiveQuest = false;
    
    LastInteractionTime = 0.0f;
    CurrentInteractingActor = nullptr;
    bIsCurrentlyInteracting = false;
    
    // Setup default NPC profile
    NPCProfile.NPCName = TEXT("Prehistoric Survivor");
    NPCProfile.Role = EQuest_NPCRole::Survivor;
    NPCProfile.Description = TEXT("A fellow human trying to survive in this dangerous prehistoric world.");
    NPCProfile.InteractionRange = 300.0f;
    NPCProfile.bCanGiveQuests = true;
    
    // Add some default dialogues
    FQuest_NPCDialogue defaultDialogue;
    defaultDialogue.DialogueText = TEXT("The dinosaurs are getting more aggressive lately. We need to be careful.");
    defaultDialogue.SpeakerName = NPCProfile.NPCName;
    defaultDialogue.DialogueDuration = 4.0f;
    defaultDialogue.bIsQuestRelated = false;
    NPCProfile.AvailableDialogues.Add(defaultDialogue);
    
    FQuest_NPCDialogue questDialogue;
    questDialogue.DialogueText = TEXT("I need someone brave enough to gather resources from the dangerous areas. Are you up for it?");
    questDialogue.SpeakerName = NPCProfile.NPCName;
    questDialogue.DialogueDuration = 5.0f;
    questDialogue.bIsQuestRelated = true;
    NPCProfile.AvailableDialogues.Add(questDialogue);
}

void UQuest_NPCInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_NPCInteractionSystem: BeginPlay for NPC '%s'"), *NPCProfile.NPCName);
    
    // Initialize available quests based on NPC role
    if (NPCProfile.bCanGiveQuests)
    {
        switch (NPCProfile.Role)
        {
            case EQuest_NPCRole::Hunter:
                AvailableQuestIDs.Add(TEXT("hunt_raptor_pack"));
                AvailableQuestIDs.Add(TEXT("track_trex_territory"));
                break;
            case EQuest_NPCRole::Crafter:
                AvailableQuestIDs.Add(TEXT("gather_crafting_materials"));
                AvailableQuestIDs.Add(TEXT("build_advanced_tools"));
                break;
            case EQuest_NPCRole::Guide:
                AvailableQuestIDs.Add(TEXT("explore_new_biome"));
                AvailableQuestIDs.Add(TEXT("map_safe_routes"));
                break;
            default:
                AvailableQuestIDs.Add(TEXT("basic_survival_tutorial"));
                break;
        }
    }
}

void UQuest_NPCInteractionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateNPCBehavior(DeltaTime);
    CheckForNearbyPlayers();
}

bool UQuest_NPCInteractionSystem::CanInteract() const
{
    if (!bIsInteractable || bIsCurrentlyInteracting)
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastInteractionTime) >= InteractionCooldown;
}

void UQuest_NPCInteractionSystem::StartInteraction(AActor* InteractingActor)
{
    if (!CanInteract() || !InteractingActor)
    {
        return;
    }
    
    CurrentInteractingActor = InteractingActor;
    bIsCurrentlyInteracting = true;
    LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_NPCInteractionSystem: Started interaction with %s"), 
           InteractingActor ? *InteractingActor->GetName() : TEXT("Unknown Actor"));
    
    // Change state to talking
    SetNPCState(EQuest_NPCState::Talking);
    
    // Get appropriate dialogue
    FQuest_NPCDialogue dialogue = GetRandomDialogue();
    
    // Log the dialogue (in a real game, this would trigger UI)
    UE_LOG(LogTemp, Warning, TEXT("NPC %s says: %s"), *NPCProfile.NPCName, *dialogue.DialogueText);
    
    // Check if we have quests to offer
    if (HasAvailableQuests())
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC %s has quests available!"), *NPCProfile.NPCName);
    }
}

void UQuest_NPCInteractionSystem::EndInteraction()
{
    if (!bIsCurrentlyInteracting)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_NPCInteractionSystem: Ended interaction"));
    
    CurrentInteractingActor = nullptr;
    bIsCurrentlyInteracting = false;
    
    // Return to idle state
    SetNPCState(EQuest_NPCState::Idle);
}

FQuest_NPCDialogue UQuest_NPCInteractionSystem::GetRandomDialogue() const
{
    if (NPCProfile.AvailableDialogues.Num() == 0)
    {
        // Return default dialogue if none available
        FQuest_NPCDialogue defaultDialogue;
        defaultDialogue.DialogueText = TEXT("...");
        defaultDialogue.SpeakerName = NPCProfile.NPCName;
        return defaultDialogue;
    }
    
    // Get state-based dialogue or random
    if (bIsCurrentlyInteracting && HasAvailableQuests())
    {
        // Prefer quest-related dialogues
        for (const FQuest_NPCDialogue& dialogue : NPCProfile.AvailableDialogues)
        {
            if (dialogue.bIsQuestRelated)
            {
                return dialogue;
            }
        }
    }
    
    // Return random dialogue
    int32 randomIndex = FMath::RandRange(0, NPCProfile.AvailableDialogues.Num() - 1);
    return NPCProfile.AvailableDialogues[randomIndex];
}

void UQuest_NPCInteractionSystem::SetNPCState(EQuest_NPCState NewState)
{
    if (CurrentState != NewState)
    {
        EQuest_NPCState PreviousState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Quest_NPCInteractionSystem: NPC %s state changed from %d to %d"), 
               *NPCProfile.NPCName, (int32)PreviousState, (int32)NewState);
    }
}

bool UQuest_NPCInteractionSystem::HasAvailableQuests() const
{
    return NPCProfile.bCanGiveQuests && AvailableQuestIDs.Num() > 0;
}

void UQuest_NPCInteractionSystem::AddAvailableQuest(const FString& QuestID)
{
    if (!QuestID.IsEmpty() && !AvailableQuestIDs.Contains(QuestID))
    {
        AvailableQuestIDs.Add(QuestID);
        UE_LOG(LogTemp, Log, TEXT("Quest_NPCInteractionSystem: Added quest '%s' to NPC %s"), 
               *QuestID, *NPCProfile.NPCName);
    }
}

void UQuest_NPCInteractionSystem::CompleteQuest(const FString& QuestID)
{
    if (AvailableQuestIDs.Contains(QuestID))
    {
        AvailableQuestIDs.Remove(QuestID);
        CompletedQuestIDs.Add(QuestID);
        
        UE_LOG(LogTemp, Log, TEXT("Quest_NPCInteractionSystem: Completed quest '%s' for NPC %s"), 
               *QuestID, *NPCProfile.NPCName);
    }
}

bool UQuest_NPCInteractionSystem::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

void UQuest_NPCInteractionSystem::UpdateNPCBehavior(float DeltaTime)
{
    // Simple state-based behavior
    switch (CurrentState)
    {
        case EQuest_NPCState::Idle:
            // Maybe occasionally change to working or gathering
            if (FMath::RandRange(0, 1000) < 1) // Very low chance per tick
            {
                if (NPCProfile.Role == EQuest_NPCRole::Crafter)
                {
                    SetNPCState(EQuest_NPCState::Crafting);
                }
                else if (NPCProfile.Role == EQuest_NPCRole::Hunter)
                {
                    SetNPCState(EQuest_NPCState::Hunting);
                }
                else
                {
                    SetNPCState(EQuest_NPCState::Gathering);
                }
            }
            break;
            
        case EQuest_NPCState::Working:
        case EQuest_NPCState::Crafting:
        case EQuest_NPCState::Gathering:
        case EQuest_NPCState::Hunting:
            // Return to idle after some time
            if (FMath::RandRange(0, 1000) < 5) // Higher chance to return to idle
            {
                SetNPCState(EQuest_NPCState::Idle);
            }
            break;
            
        case EQuest_NPCState::Talking:
            // Handled by interaction system
            break;
            
        default:
            break;
    }
}

void UQuest_NPCInteractionSystem::CheckForNearbyPlayers()
{
    if (!GetOwner() || bIsCurrentlyInteracting)
    {
        return;
    }
    
    // Get player controller
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    // Check distance to player
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PC->GetPawn()->GetActorLocation());
    
    if (Distance <= NPCProfile.InteractionRange)
    {
        // Player is in range - could show interaction prompt here
        // For now, just log it occasionally
        static float LastPromptTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastPromptTime > 3.0f) // Show prompt every 3 seconds
        {
            UE_LOG(LogTemp, Warning, TEXT("NPC %s: %s (Distance: %.1f)"), 
                   *NPCProfile.NPCName, *InteractionPrompt, Distance);
            LastPromptTime = CurrentTime;
        }
    }
}

FString UQuest_NPCInteractionSystem::GetStateBasedDialogue() const
{
    switch (CurrentState)
    {
        case EQuest_NPCState::Working:
            return TEXT("I'm busy working right now, but I can spare a moment.");
        case EQuest_NPCState::Crafting:
            return TEXT("Just finishing up some tools. These primitive materials are tricky to work with.");
        case EQuest_NPCState::Gathering:
            return TEXT("I've been gathering resources all morning. This world is harsh but provides.");
        case EQuest_NPCState::Hunting:
            return TEXT("The hunt was successful today. We'll eat well tonight.");
        case EQuest_NPCState::Sleeping:
            return TEXT("*yawns* Oh, you woke me up. What do you need?");
        default:
            return TEXT("Hello there, fellow survivor.");
    }
}