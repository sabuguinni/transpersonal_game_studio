#include "Quest_NPCInteractionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "TranspersonalCharacter.h"

UQuest_NPCInteractionManager::UQuest_NPCInteractionManager()
{
    NPCUpdateInterval = 2.0f;
    CurrentInteractingNPC = TEXT("");
}

void UQuest_NPCInteractionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeTribalNPCDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest NPC Interaction Manager initialized"));
}

void UQuest_NPCInteractionManager::Deinitialize()
{
    RegisteredNPCs.Empty();
    NPCBehaviorStates.Empty();
    NPCDialogues.Empty();
    CurrentInteractingNPC = TEXT("");
    
    Super::Deinitialize();
}

void UQuest_NPCInteractionManager::RegisterNPC(const FString& NPCID, AActor* NPCActor, const FQuest_NPCBehaviorState& InitialState)
{
    if (!NPCActor || NPCID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register NPC: Invalid actor or ID"));
        return;
    }

    RegisteredNPCs.Add(NPCID, NPCActor);
    NPCBehaviorStates.Add(NPCID, InitialState);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered NPC: %s"), *NPCID);
}

void UQuest_NPCInteractionManager::UnregisterNPC(const FString& NPCID)
{
    RegisteredNPCs.Remove(NPCID);
    NPCBehaviorStates.Remove(NPCID);
    NPCDialogues.Remove(NPCID);
    
    if (CurrentInteractingNPC == NPCID)
    {
        CurrentInteractingNPC = TEXT("");
    }
}

bool UQuest_NPCInteractionManager::StartNPCInteraction(const FString& NPCID, ATranspersonalCharacter* Player)
{
    if (!RegisteredNPCs.Contains(NPCID) || !Player)
    {
        return false;
    }

    FQuest_NPCBehaviorState* NPCState = NPCBehaviorStates.Find(NPCID);
    if (!NPCState || !NPCState->bIsInteractable)
    {
        return false;
    }

    CurrentInteractingNPC = NPCID;
    
    // Stop NPC movement during interaction
    NPCState->ActivityTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Started interaction with NPC: %s"), *NPCID);
    return true;
}

void UQuest_NPCInteractionManager::EndNPCInteraction(const FString& NPCID)
{
    if (CurrentInteractingNPC == NPCID)
    {
        CurrentInteractingNPC = TEXT("");
        
        // Resume NPC activity
        FQuest_NPCBehaviorState* NPCState = NPCBehaviorStates.Find(NPCID);
        if (NPCState)
        {
            NPCState->ActivityTimer = FMath::RandRange(5.0f, 15.0f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Ended interaction with NPC: %s"), *NPCID);
    }
}

FQuest_NPCDialogue UQuest_NPCInteractionManager::GetNPCDialogue(const FString& NPCID)
{
    if (NPCDialogues.Contains(NPCID))
    {
        return NPCDialogues[NPCID];
    }
    
    // Return default dialogue for unregistered NPCs
    FQuest_NPCDialogue DefaultDialogue;
    DefaultDialogue.NPCName = TEXT("Tribal Hunter");
    DefaultDialogue.DialogueText = TEXT("Greetings, survivor. The land is dangerous, but rich with resources.");
    DefaultDialogue.PlayerResponses.Add(TEXT("What dangers should I watch for?"));
    DefaultDialogue.PlayerResponses.Add(TEXT("Where can I find good hunting grounds?"));
    DefaultDialogue.PlayerResponses.Add(TEXT("Farewell."));
    
    return DefaultDialogue;
}

void UQuest_NPCInteractionManager::UpdateNPCBehavior(const FString& NPCID, const FQuest_NPCBehaviorState& NewState)
{
    if (NPCBehaviorStates.Contains(NPCID))
    {
        NPCBehaviorStates[NPCID] = NewState;
    }
}

TArray<FString> UQuest_NPCInteractionManager::GetNearbyInteractableNPCs(const FVector& PlayerLocation, float InteractionRadius)
{
    TArray<FString> NearbyNPCs;
    
    for (const auto& NPCPair : RegisteredNPCs)
    {
        AActor* NPCActor = NPCPair.Value;
        if (!NPCActor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, NPCActor->GetActorLocation());
        if (Distance <= InteractionRadius)
        {
            const FQuest_NPCBehaviorState* NPCState = NPCBehaviorStates.Find(NPCPair.Key);
            if (NPCState && NPCState->bIsInteractable)
            {
                NearbyNPCs.Add(NPCPair.Key);
            }
        }
    }
    
    return NearbyNPCs;
}

void UQuest_NPCInteractionManager::SetNPCQuestAvailability(const FString& NPCID, const FString& QuestID, bool bIsAvailable)
{
    FQuest_NPCBehaviorState* NPCState = NPCBehaviorStates.Find(NPCID);
    if (!NPCState)
    {
        return;
    }
    
    if (bIsAvailable)
    {
        NPCState->AvailableQuests.AddUnique(QuestID);
    }
    else
    {
        NPCState->AvailableQuests.Remove(QuestID);
    }
}

bool UQuest_NPCInteractionManager::CanNPCGiveQuest(const FString& NPCID, const FString& QuestID)
{
    const FQuest_NPCBehaviorState* NPCState = NPCBehaviorStates.Find(NPCID);
    if (!NPCState)
    {
        return false;
    }
    
    return NPCState->AvailableQuests.Contains(QuestID);
}

void UQuest_NPCInteractionManager::ProcessNPCDailyRoutines(float DeltaTime)
{
    for (auto& NPCPair : NPCBehaviorStates)
    {
        UpdateNPCActivity(NPCPair.Key, DeltaTime);
    }
}

void UQuest_NPCInteractionManager::SpawnTribalNPCs()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn tribal NPCs at key locations
    TArray<FVector> SpawnLocations = {
        FVector(2000.0f, 2000.0f, 100.0f),    // Near player start
        FVector(-1000.0f, 3000.0f, 100.0f),   // Hunting grounds
        FVector(4000.0f, -1000.0f, 100.0f),   // Resource area
        FVector(-2000.0f, -2000.0f, 100.0f),  // Safe camp
        FVector(0.0f, 5000.0f, 100.0f)        // Trading post
    };
    
    TArray<FString> NPCNames = {
        TEXT("Kael_Hunter"),
        TEXT("Nara_Gatherer"),
        TEXT("Thok_Warrior"),
        TEXT("Yuki_Elder"),
        TEXT("Zara_Scout")
    };
    
    for (int32 i = 0; i < SpawnLocations.Num() && i < NPCNames.Num(); i++)
    {
        FQuest_NPCBehaviorState NPCState;
        NPCState.NPCID = NPCNames[i];
        NPCState.CurrentActivity = static_cast<EQuestObjectiveType>(i % 3); // Cycle through activities
        NPCState.TargetLocation = SpawnLocations[i];
        NPCState.ActivityTimer = FMath::RandRange(10.0f, 30.0f);
        NPCState.bIsInteractable = true;
        
        // Add quest availability based on NPC type
        if (NPCNames[i].Contains(TEXT("Hunter")))
        {
            NPCState.AvailableQuests.Add(TEXT("Hunt_Raptor_Pack"));
            NPCState.AvailableQuests.Add(TEXT("Track_Predator"));
        }
        else if (NPCNames[i].Contains(TEXT("Gatherer")))
        {
            NPCState.AvailableQuests.Add(TEXT("Gather_Rare_Plants"));
            NPCState.AvailableQuests.Add(TEXT("Find_Water_Source"));
        }
        
        // Register the NPC (actor will be spawned by UE5 command)
        NPCBehaviorStates.Add(NPCNames[i], NPCState);
        
        UE_LOG(LogTemp, Warning, TEXT("Prepared tribal NPC: %s at location %s"), *NPCNames[i], *SpawnLocations[i].ToString());
    }
}

void UQuest_NPCInteractionManager::InitializeTribalNPCDialogues()
{
    // Hunter NPC
    FQuest_NPCDialogue HunterDialogue;
    HunterDialogue.NPCName = TEXT("Kael the Hunter");
    HunterDialogue.DialogueText = TEXT("The raptors have been more aggressive lately. We need skilled hunters to thin their numbers.");
    HunterDialogue.PlayerResponses.Add(TEXT("I can help hunt the raptors."));
    HunterDialogue.PlayerResponses.Add(TEXT("What weapons work best against them?"));
    HunterDialogue.PlayerResponses.Add(TEXT("Where do they usually hunt?"));
    HunterDialogue.bIsQuestGiver = true;
    HunterDialogue.QuestID = TEXT("Hunt_Raptor_Pack");
    NPCDialogues.Add(TEXT("Kael_Hunter"), HunterDialogue);
    
    // Gatherer NPC
    FQuest_NPCDialogue GathererDialogue;
    GathererDialogue.NPCName = TEXT("Nara the Gatherer");
    GathererDialogue.DialogueText = TEXT("The healing plants near the river are running low. We need someone to venture into dangerous territory to find more.");
    GathererDialogue.PlayerResponses.Add(TEXT("I'll search for the healing plants."));
    GathererDialogue.PlayerResponses.Add(TEXT("What do these plants look like?"));
    GathererDialogue.PlayerResponses.Add(TEXT("How dangerous is the area?"));
    GathererDialogue.bIsQuestGiver = true;
    GathererDialogue.QuestID = TEXT("Gather_Rare_Plants");
    NPCDialogues.Add(TEXT("Nara_Gatherer"), GathererDialogue);
    
    // Warrior NPC
    FQuest_NPCDialogue WarriorDialogue;
    WarriorDialogue.NPCName = TEXT("Thok the Warrior");
    WarriorDialogue.DialogueText = TEXT("Our camp needs defending. The great beasts grow bolder each day.");
    WarriorDialogue.PlayerResponses.Add(TEXT("I'll help defend the camp."));
    WarriorDialogue.PlayerResponses.Add(TEXT("What threatens us most?"));
    WarriorDialogue.PlayerResponses.Add(TEXT("How can I prepare for battle?"));
    WarriorDialogue.bIsQuestGiver = true;
    WarriorDialogue.QuestID = TEXT("Defend_Camp");
    NPCDialogues.Add(TEXT("Thok_Warrior"), WarriorDialogue);
    
    // Elder NPC
    FQuest_NPCDialogue ElderDialogue;
    ElderDialogue.NPCName = TEXT("Yuki the Elder");
    ElderDialogue.DialogueText = TEXT("Young one, survival requires wisdom as much as strength. Learn the ways of this ancient land.");
    ElderDialogue.PlayerResponses.Add(TEXT("Teach me about this land."));
    ElderDialogue.PlayerResponses.Add(TEXT("What wisdom can you share?"));
    ElderDialogue.PlayerResponses.Add(TEXT("I seek knowledge of the old ways."));
    ElderDialogue.bIsQuestGiver = false;
    NPCDialogues.Add(TEXT("Yuki_Elder"), ElderDialogue);
    
    // Scout NPC
    FQuest_NPCDialogue ScoutDialogue;
    ScoutDialogue.NPCName = TEXT("Zara the Scout");
    ScoutDialogue.DialogueText = TEXT("I've seen strange movements in the distant valleys. Someone should investigate these new territories.");
    ScoutDialogue.PlayerResponses.Add(TEXT("I'll explore the distant valleys."));
    ScoutDialogue.PlayerResponses.Add(TEXT("What did you see out there?"));
    ScoutDialogue.PlayerResponses.Add(TEXT("Is it safe to travel alone?"));
    ScoutDialogue.bIsQuestGiver = true;
    ScoutDialogue.QuestID = TEXT("Explore_Valley");
    NPCDialogues.Add(TEXT("Zara_Scout"), ScoutDialogue);
}

void UQuest_NPCInteractionManager::UpdateNPCActivity(const FString& NPCID, float DeltaTime)
{
    FQuest_NPCBehaviorState* NPCState = NPCBehaviorStates.Find(NPCID);
    AActor* NPCActor = RegisteredNPCs.FindRef(NPCID);
    
    if (!NPCState || !NPCActor)
    {
        return;
    }
    
    // Skip update if NPC is currently being interacted with
    if (CurrentInteractingNPC == NPCID)
    {
        return;
    }
    
    NPCState->ActivityTimer -= DeltaTime;
    
    if (NPCState->ActivityTimer <= 0.0f)
    {
        // Change activity and set new target
        FVector CurrentLocation = NPCActor->GetActorLocation();
        
        switch (NPCState->CurrentActivity)
        {
            case EQuestObjectiveType::Hunt:
                NPCState->TargetLocation = GetRandomPatrolLocation(CurrentLocation, 1500.0f);
                NPCState->ActivityTimer = FMath::RandRange(15.0f, 25.0f);
                break;
                
            case EQuestObjectiveType::Gather:
                NPCState->TargetLocation = GetRandomPatrolLocation(CurrentLocation, 800.0f);
                NPCState->ActivityTimer = FMath::RandRange(20.0f, 35.0f);
                break;
                
            case EQuestObjectiveType::Explore:
                NPCState->TargetLocation = GetRandomPatrolLocation(CurrentLocation, 2000.0f);
                NPCState->ActivityTimer = FMath::RandRange(30.0f, 45.0f);
                break;
                
            default:
                NPCState->TargetLocation = CurrentLocation;
                NPCState->ActivityTimer = FMath::RandRange(10.0f, 20.0f);
                break;
        }
        
        // Cycle to next activity
        int32 NextActivity = (static_cast<int32>(NPCState->CurrentActivity) + 1) % 3;
        NPCState->CurrentActivity = static_cast<EQuestObjectiveType>(NextActivity);
    }
}

FVector UQuest_NPCInteractionManager::GetRandomPatrolLocation(const FVector& BaseLocation, float PatrolRadius)
{
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    FVector RandomDirection = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
        0.0f
    );
    
    return BaseLocation + (RandomDirection * RandomDistance);
}