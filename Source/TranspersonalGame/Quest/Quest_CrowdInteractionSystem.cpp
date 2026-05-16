#include "Quest_CrowdInteractionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

// Quest Crowd Interaction Component Implementation
UQuest_CrowdInteractionComponent::UQuest_CrowdInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CrowdManager = nullptr;
    CrowdCheckInterval = 1.0f;
    LastCrowdCheck = 0.0f;
}

void UQuest_CrowdInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    FindCrowdManager();
}

void UQuest_CrowdInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastCrowdCheck += DeltaTime;
    if (LastCrowdCheck >= CrowdCheckInterval)
    {
        UpdateCrowdQuestProgress(DeltaTime);
        ProcessCrowdTriggers();
        LastCrowdCheck = 0.0f;
    }
}

void UQuest_CrowdInteractionComponent::StartCrowdQuest(const FQuest_CrowdInteractionData& QuestData)
{
    FQuest_CrowdInteractionData NewQuest = QuestData;
    NewQuest.ElapsedTime = 0.0f;
    NewQuest.bIsCompleted = false;
    
    ActiveCrowdQuests.Add(NewQuest);
    
    // Register with global subsystem
    if (UQuest_CrowdInteractionSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuest_CrowdInteractionSubsystem>())
    {
        Subsystem->RegisterCrowdQuest(NewQuest);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Started crowd quest: %s"), *QuestData.QuestID);
}

void UQuest_CrowdInteractionComponent::CompleteCrowdQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveCrowdQuests.Num(); i++)
    {
        if (ActiveCrowdQuests[i].QuestID == QuestID)
        {
            ActiveCrowdQuests[i].bIsCompleted = true;
            
            // Unregister from global subsystem
            if (UQuest_CrowdInteractionSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuest_CrowdInteractionSubsystem>())
            {
                Subsystem->UnregisterCrowdQuest(QuestID);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Completed crowd quest: %s"), *QuestID);
            ActiveCrowdQuests.RemoveAt(i);
            break;
        }
    }
}

bool UQuest_CrowdInteractionComponent::CheckCrowdDensityTrigger(const FQuest_CrowdInteractionData& Quest)
{
    if (!CrowdManager)
    {
        return false;
    }
    
    int32 NearbyEntities = GetNearbyEntityCount(Quest.TargetLocation, Quest.InteractionRadius);
    return NearbyEntities >= Quest.TriggerThreshold;
}

bool UQuest_CrowdInteractionComponent::CheckEntityProximityTrigger(const FQuest_CrowdInteractionData& Quest)
{
    if (!CrowdManager)
    {
        return false;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float DistanceToTarget = FVector::Dist(PlayerLocation, Quest.TargetLocation);
    
    return DistanceToTarget <= Quest.InteractionRadius;
}

void UQuest_CrowdInteractionComponent::UpdateCrowdQuestProgress(float DeltaTime)
{
    for (FQuest_CrowdInteractionData& Quest : ActiveCrowdQuests)
    {
        if (Quest.bIsCompleted)
        {
            continue;
        }
        
        Quest.ElapsedTime += DeltaTime;
        
        // Check for time limit expiration
        if (Quest.TimeLimit > 0.0f && Quest.ElapsedTime >= Quest.TimeLimit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd quest %s expired"), *Quest.QuestID);
            CompleteCrowdQuest(Quest.QuestID);
            continue;
        }
        
        // Check quest completion conditions
        bool bQuestCompleted = false;
        
        switch (Quest.ObjectiveType)
        {
            case EQuest_CrowdObjective::FollowCrowd:
                bQuestCompleted = CheckEntityProximityTrigger(Quest);
                break;
                
            case EQuest_CrowdObjective::AvoidCrowd:
                bQuestCompleted = !CheckCrowdDensityTrigger(Quest);
                break;
                
            case EQuest_CrowdObjective::InteractWithEntity:
                bQuestCompleted = CheckEntityProximityTrigger(Quest) && CheckCrowdDensityTrigger(Quest);
                break;
                
            case EQuest_CrowdObjective::InfluenceBehavior:
                bQuestCompleted = CrowdManager && CrowdManager->GetCrowdDensityMultiplier() > 1.0f;
                break;
                
            case EQuest_CrowdObjective::EscortEntity:
                bQuestCompleted = CheckEntityProximityTrigger(Quest);
                break;
        }
        
        if (bQuestCompleted)
        {
            CompleteCrowdQuest(Quest.QuestID);
        }
    }
}

int32 UQuest_CrowdInteractionComponent::GetNearbyEntityCount(const FVector& Location, float Radius)
{
    if (!CrowdManager)
    {
        return 0;
    }
    
    // Simplified entity count - in real implementation would query Mass Entity system
    return FMath::RandRange(0, 20);
}

void UQuest_CrowdInteractionComponent::FindCrowdManager()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (TActorIterator<ACrowd_MassEntityManager> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        CrowdManager = *ActorIterator;
        UE_LOG(LogTemp, Warning, TEXT("Found crowd manager for quest system"));
        break;
    }
}

void UQuest_CrowdInteractionComponent::ProcessCrowdTriggers()
{
    for (const FQuest_CrowdInteractionData& Quest : ActiveCrowdQuests)
    {
        if (Quest.bIsCompleted)
        {
            continue;
        }
        
        bool bTriggerActivated = false;
        
        switch (Quest.TriggerType)
        {
            case EQuest_CrowdTriggerType::DensityThreshold:
                bTriggerActivated = CheckCrowdDensityTrigger(Quest);
                break;
                
            case EQuest_CrowdTriggerType::EntityProximity:
                bTriggerActivated = CheckEntityProximityTrigger(Quest);
                break;
                
            case EQuest_CrowdTriggerType::EntityBehaviorChange:
                bTriggerActivated = CrowdManager && CrowdManager->GetCrowdDensityMultiplier() != 1.0f;
                break;
                
            case EQuest_CrowdTriggerType::CrowdMovement:
                bTriggerActivated = true; // Simplified - always active for movement tracking
                break;
                
            case EQuest_CrowdTriggerType::CrowdDispersion:
                bTriggerActivated = !CheckCrowdDensityTrigger(Quest);
                break;
        }
        
        if (bTriggerActivated)
        {
            UE_LOG(LogTemp, Log, TEXT("Crowd trigger activated for quest: %s"), *Quest.QuestID);
        }
    }
}

// Quest Crowd Interaction Subsystem Implementation
void UQuest_CrowdInteractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CompletedCrowdQuests = 0;
    TotalCrowdInteractionTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Crowd Interaction Subsystem initialized"));
}

void UQuest_CrowdInteractionSubsystem::RegisterCrowdQuest(const FQuest_CrowdInteractionData& QuestData)
{
    GlobalCrowdQuests.Add(QuestData);
    UE_LOG(LogTemp, Warning, TEXT("Registered global crowd quest: %s"), *QuestData.QuestID);
}

void UQuest_CrowdInteractionSubsystem::UnregisterCrowdQuest(const FString& QuestID)
{
    for (int32 i = 0; i < GlobalCrowdQuests.Num(); i++)
    {
        if (GlobalCrowdQuests[i].QuestID == QuestID)
        {
            TotalCrowdInteractionTime += GlobalCrowdQuests[i].ElapsedTime;
            CompletedCrowdQuests++;
            GlobalCrowdQuests.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Unregistered global crowd quest: %s"), *QuestID);
            break;
        }
    }
}

FQuest_CrowdInteractionData UQuest_CrowdInteractionSubsystem::GetCrowdQuestByID(const FString& QuestID)
{
    for (const FQuest_CrowdInteractionData& Quest : GlobalCrowdQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    
    return FQuest_CrowdInteractionData();
}

TArray<FQuest_CrowdInteractionData> UQuest_CrowdInteractionSubsystem::GetActiveCrowdQuests()
{
    return GlobalCrowdQuests;
}

bool UQuest_CrowdInteractionSubsystem::HasActiveCrowdQuest(const FString& QuestID)
{
    for (const FQuest_CrowdInteractionData& Quest : GlobalCrowdQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return true;
        }
    }
    
    return false;
}

FQuest_CrowdInteractionData UQuest_CrowdInteractionSubsystem::CreateFollowCrowdQuest(const FVector& StartLocation, float Duration)
{
    FQuest_CrowdInteractionData NewQuest;
    NewQuest.QuestID = FString::Printf(TEXT("FollowCrowd_%d"), FMath::RandRange(1000, 9999));
    NewQuest.TriggerType = EQuest_CrowdTriggerType::EntityProximity;
    NewQuest.ObjectiveType = EQuest_CrowdObjective::FollowCrowd;
    NewQuest.TargetLocation = StartLocation;
    NewQuest.InteractionRadius = 500.0f;
    NewQuest.TimeLimit = Duration;
    NewQuest.RequiredEntityCount = 3;
    
    return NewQuest;
}

FQuest_CrowdInteractionData UQuest_CrowdInteractionSubsystem::CreateAvoidCrowdQuest(const FVector& DangerZone, float Radius)
{
    FQuest_CrowdInteractionData NewQuest;
    NewQuest.QuestID = FString::Printf(TEXT("AvoidCrowd_%d"), FMath::RandRange(1000, 9999));
    NewQuest.TriggerType = EQuest_CrowdTriggerType::DensityThreshold;
    NewQuest.ObjectiveType = EQuest_CrowdObjective::AvoidCrowd;
    NewQuest.TargetLocation = DangerZone;
    NewQuest.InteractionRadius = Radius;
    NewQuest.TriggerThreshold = 10.0f;
    NewQuest.TimeLimit = 180.0f;
    
    return NewQuest;
}

FQuest_CrowdInteractionData UQuest_CrowdInteractionSubsystem::CreateEntityInteractionQuest(const FVector& TargetLocation, int32 EntityCount)
{
    FQuest_CrowdInteractionData NewQuest;
    NewQuest.QuestID = FString::Printf(TEXT("EntityInteraction_%d"), FMath::RandRange(1000, 9999));
    NewQuest.TriggerType = EQuest_CrowdTriggerType::EntityProximity;
    NewQuest.ObjectiveType = EQuest_CrowdObjective::InteractWithEntity;
    NewQuest.TargetLocation = TargetLocation;
    NewQuest.InteractionRadius = 300.0f;
    NewQuest.RequiredEntityCount = EntityCount;
    NewQuest.TimeLimit = 240.0f;
    
    return NewQuest;
}