#include "Quest_NPCInteractionManager.h"
#include "../TranspersonalCharacter.h"
#include "Quest_DialogueSystem.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AQuest_NPCInteractionManager::AQuest_NPCInteractionManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    InteractionRange = 300.0f;
    DialogueDisplayTime = 5.0f;
    bIsInDialogue = false;
    CurrentInteractingNPC = nullptr;
    CurrentPlayer = nullptr;
}

void AQuest_NPCInteractionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: System initialized"));
    
    // Find all NPCs in the level
    TArray<AActor*> FoundNPCs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundNPCs);
    
    for (AActor* Actor : FoundNPCs)
    {
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("NPC")) || ActorName.Contains(TEXT("Elder")) || 
            ActorName.Contains(TEXT("Scout")) || ActorName.Contains(TEXT("Craft")))
        {
            ActiveNPCs.Add(Actor);
            UE_LOG(LogTemp, Warning, TEXT("Registered NPC: %s"), *ActorName);
        }
    }
}

void AQuest_NPCInteractionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsInDialogue)
    {
        UpdateNPCInteractions();
        CheckPlayerProximity();
    }
}

void AQuest_NPCInteractionManager::InitiateNPCDialogue(AActor* NPCActor, ATranspersonalCharacter* Player)
{
    if (!NPCActor || !Player || bIsInDialogue)
    {
        return;
    }

    CurrentInteractingNPC = NPCActor;
    CurrentPlayer = Player;
    bIsInDialogue = true;

    // Generate contextual dialogue
    CurrentDialogue = GenerateContextualDialogue(NPCActor, Player);
    
    // Play the dialogue
    PlayNPCDialogue(CurrentDialogue);
    
    UE_LOG(LogTemp, Warning, TEXT("Initiated dialogue with %s"), *NPCActor->GetName());
}

void AQuest_NPCInteractionManager::EndNPCDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    bIsInDialogue = false;
    CurrentInteractingNPC = nullptr;
    CurrentPlayer = nullptr;
    CurrentDialogue = FQuest_NPCDialogue();
    
    UE_LOG(LogTemp, Warning, TEXT("Ended NPC dialogue"));
}

bool AQuest_NPCInteractionManager::IsPlayerInInteractionRange(AActor* NPCActor, ATranspersonalCharacter* Player)
{
    if (!NPCActor || !Player)
    {
        return false;
    }

    float Distance = FVector::Dist(NPCActor->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange;
}

void AQuest_NPCInteractionManager::RegisterNPCQuestGiver(AActor* NPCActor, const FQuest_NPCQuestData& QuestData)
{
    if (!NPCActor)
    {
        return;
    }

    RegisteredNPCs.Add(NPCActor, QuestData);
    AvailableQuests.Add(QuestData.QuestID, QuestData);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered quest giver: %s with quest: %s"), 
           *NPCActor->GetName(), *QuestData.QuestTitle);
}

void AQuest_NPCInteractionManager::AssignQuestToPlayer(ATranspersonalCharacter* Player, const FQuest_NPCQuestData& QuestData)
{
    if (!Player || !CanAssignQuest(Player, QuestData))
    {
        return;
    }

    // Add quest to player's active quests
    if (!PlayerActiveQuests.Contains(Player))
    {
        PlayerActiveQuests.Add(Player, TArray<FString>());
    }
    
    PlayerActiveQuests[Player].Add(QuestData.QuestID);
    
    // Display quest assignment message
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Quest Assigned: %s"), *QuestData.QuestTitle);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned quest %s to player"), *QuestData.QuestTitle);
}

bool AQuest_NPCInteractionManager::CanAssignQuest(ATranspersonalCharacter* Player, const FQuest_NPCQuestData& QuestData)
{
    if (!Player)
    {
        return false;
    }

    // Check if player already has this quest
    if (PlayerActiveQuests.Contains(Player))
    {
        if (PlayerActiveQuests[Player].Contains(QuestData.QuestID))
        {
            return false;
        }
    }

    // Validate quest requirements
    return ValidateQuestRequirements(Player, QuestData);
}

void AQuest_NPCInteractionManager::CompleteQuestForPlayer(ATranspersonalCharacter* Player, const FString& QuestID)
{
    if (!Player || !PlayerActiveQuests.Contains(Player))
    {
        return;
    }

    TArray<FString>& ActiveQuests = PlayerActiveQuests[Player];
    if (ActiveQuests.Contains(QuestID))
    {
        ActiveQuests.Remove(QuestID);
        
        // Award experience if quest data available
        if (AvailableQuests.Contains(QuestID))
        {
            const FQuest_NPCQuestData& QuestData = AvailableQuests[QuestID];
            
            if (GEngine)
            {
                FString Message = FString::Printf(TEXT("Quest Completed: %s (+%.0f XP)"), 
                                                *QuestData.QuestTitle, QuestData.RewardExperience);
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Gold, Message);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Completed quest %s for player"), *QuestID);
    }
}

void AQuest_NPCInteractionManager::PlayNPCDialogue(const FQuest_NPCDialogue& DialogueData)
{
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("%s: %s"), *DialogueData.NPCName, *DialogueData.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueData.DialogueDuration, FColor::Cyan, Message);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Playing dialogue: %s"), *DialogueData.DialogueText);
}

void AQuest_NPCInteractionManager::ProcessPlayerResponse(int32 ResponseIndex)
{
    if (!bIsInDialogue || ResponseIndex < 0 || ResponseIndex >= CurrentDialogue.PlayerResponses.Num())
    {
        return;
    }

    FString SelectedResponse = CurrentDialogue.PlayerResponses[ResponseIndex];
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("You: %s"), *SelectedResponse);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, Message);
    }
    
    // Process response logic here
    if (SelectedResponse.Contains(TEXT("Accept")) && CurrentInteractingNPC)
    {
        // Check if NPC has a quest to assign
        if (RegisteredNPCs.Contains(CurrentInteractingNPC))
        {
            const FQuest_NPCQuestData& QuestData = RegisteredNPCs[CurrentInteractingNPC];
            AssignQuestToPlayer(CurrentPlayer, QuestData);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Processed player response: %s"), *SelectedResponse);
}

TArray<FString> AQuest_NPCInteractionManager::GetAvailableResponses()
{
    if (bIsInDialogue)
    {
        return CurrentDialogue.PlayerResponses;
    }
    
    return TArray<FString>();
}

void AQuest_NPCInteractionManager::UpdateNPCInteractions()
{
    // Find the player character
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player)
    {
        return;
    }

    // Check interactions with all active NPCs
    for (AActor* NPC : ActiveNPCs)
    {
        if (IsPlayerInInteractionRange(NPC, Player))
        {
            // Show interaction prompt
            if (GEngine)
            {
                FString Message = FString::Printf(TEXT("Press E to talk to %s"), *NPC->GetActorLabel());
                GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, Message);
            }
        }
    }
}

void AQuest_NPCInteractionManager::CheckPlayerProximity()
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player)
    {
        return;
    }

    // Auto-initiate dialogue if player gets very close to an NPC
    for (AActor* NPC : ActiveNPCs)
    {
        float Distance = FVector::Dist(NPC->GetActorLocation(), Player->GetActorLocation());
        if (Distance <= 150.0f && !bIsInDialogue)
        {
            InitiateNPCDialogue(NPC, Player);
            break;
        }
    }
}

FQuest_NPCDialogue AQuest_NPCInteractionManager::GenerateContextualDialogue(AActor* NPCActor, ATranspersonalCharacter* Player)
{
    FQuest_NPCDialogue Dialogue;
    
    if (!NPCActor || !Player)
    {
        return Dialogue;
    }

    FString NPCName = NPCActor->GetActorLabel();
    Dialogue.NPCName = NPCName;
    Dialogue.DialogueDuration = 6.0f;
    Dialogue.bIsQuestRelated = true;

    // Generate contextual dialogue based on NPC type
    if (NPCName.Contains(TEXT("Elder")))
    {
        Dialogue.DialogueText = TEXT("Young one, the tribe needs your strength. Are you ready for a dangerous mission?");
        Dialogue.PlayerResponses.Add(TEXT("I accept the mission"));
        Dialogue.PlayerResponses.Add(TEXT("Tell me more about the danger"));
        Dialogue.PlayerResponses.Add(TEXT("I need more time to prepare"));
    }
    else if (NPCName.Contains(TEXT("Scout")))
    {
        Dialogue.DialogueText = TEXT("The hunting grounds beyond the ridge are rich with prey, but predators lurk there too.");
        Dialogue.PlayerResponses.Add(TEXT("Lead me to the hunting grounds"));
        Dialogue.PlayerResponses.Add(TEXT("What kind of predators?"));
        Dialogue.PlayerResponses.Add(TEXT("I'll scout alone"));
    }
    else if (NPCName.Contains(TEXT("Craft")))
    {
        Dialogue.DialogueText = TEXT("I can teach you to craft better tools, but you'll need to gather the materials first.");
        Dialogue.PlayerResponses.Add(TEXT("What materials do I need?"));
        Dialogue.PlayerResponses.Add(TEXT("Show me the crafting techniques"));
        Dialogue.PlayerResponses.Add(TEXT("I'll return when I'm ready"));
    }
    else
    {
        Dialogue.DialogueText = TEXT("Greetings, survivor. These are dangerous times for our people.");
        Dialogue.PlayerResponses.Add(TEXT("How can I help?"));
        Dialogue.PlayerResponses.Add(TEXT("What dangers do you speak of?"));
        Dialogue.PlayerResponses.Add(TEXT("Farewell"));
    }

    return Dialogue;
}

bool AQuest_NPCInteractionManager::ValidateQuestRequirements(ATranspersonalCharacter* Player, const FQuest_NPCQuestData& QuestData)
{
    if (!Player)
    {
        return false;
    }

    // Basic validation - can be extended with inventory checks, level requirements, etc.
    
    // Check if player has too many active quests
    if (PlayerActiveQuests.Contains(Player))
    {
        if (PlayerActiveQuests[Player].Num() >= 5) // Max 5 active quests
        {
            return false;
        }
    }

    // Check urgency level requirements
    if (QuestData.UrgencyLevel == ESurvivalUrgencyLevel::LifeThreatening)
    {
        // Only assign life-threatening quests if player has survival skills
        // This would check player stats in a full implementation
        return true;
    }

    return true;
}