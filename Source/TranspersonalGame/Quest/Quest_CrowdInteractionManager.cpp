#include "Quest_CrowdInteractionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AQuest_CrowdInteractionManager::AQuest_CrowdInteractionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize quest system properties
    bQuestSystemActive = true;
    QuestUpdateInterval = 1.0f;
    CrowdDetectionRadius = 2000.0f;
    MaxCrowdEntitiesTracked = 500;
    CurrentCrowdCount = 0;
    LastQuestUpdateTime = 0.0f;
    
    // Set default root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("QuestManagerRoot"));
}

void AQuest_CrowdInteractionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionManager: System initialized"));
    
    // Initialize default quest objectives
    FQuest_CrowdObjective DefaultObjective;
    DefaultObjective.ObjectiveName = TEXT("Find Tribal Elder");
    DefaultObjective.InteractionType = EQuest_CrowdInteractionType::FindNPC;
    DefaultObjective.TargetLocation = FVector(0, 0, 100);
    DefaultObjective.CrowdDensityThreshold = 30.0f;
    DefaultObjective.bIsCompleted = false;
    DefaultObjective.CompletionReward = 250.0f;
    
    ActiveObjectives.Add(DefaultObjective);
    
    // Start quest system
    LastQuestUpdateTime = GetWorld()->GetTimeSeconds();
}

void AQuest_CrowdInteractionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bQuestSystemActive)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastQuestUpdateTime >= QuestUpdateInterval)
    {
        UpdateQuestProgress();
        ScanForNearbyEntities();
        UpdateCrowdDensityMap();
        LastQuestUpdateTime = CurrentTime;
    }
}

void AQuest_CrowdInteractionManager::StartCrowdQuest(const FString& QuestName, EQuest_CrowdInteractionType InteractionType, FVector TargetLoc)
{
    FQuest_CrowdObjective NewObjective;
    NewObjective.ObjectiveName = QuestName;
    NewObjective.InteractionType = InteractionType;
    NewObjective.TargetLocation = TargetLoc;
    NewObjective.CrowdDensityThreshold = 25.0f;
    NewObjective.bIsCompleted = false;
    NewObjective.CompletionReward = 150.0f;
    
    ActiveObjectives.Add(NewObjective);
    QuestStartTimes.Add(QuestName, GetWorld()->GetTimeSeconds());
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionManager: Started quest '%s' at location %s"), 
           *QuestName, *TargetLoc.ToString());
}

void AQuest_CrowdInteractionManager::CompleteObjective(const FString& ObjectiveName)
{
    for (int32 i = 0; i < ActiveObjectives.Num(); i++)
    {
        if (ActiveObjectives[i].ObjectiveName == ObjectiveName && !ActiveObjectives[i].bIsCompleted)
        {
            ActiveObjectives[i].bIsCompleted = true;
            float Reward = CalculateQuestReward(ObjectiveName);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionManager: Completed objective '%s' - Reward: %.1f"), 
                   *ObjectiveName, Reward);
            break;
        }
    }
}

bool AQuest_CrowdInteractionManager::CheckCrowdDensityAtLocation(FVector Location, float Radius)
{
    if (!GetWorld())
        return false;
    
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Location,
        Radius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        FoundActors
    );
    
    // Count crowd entities (actors with "Crowd" or "NPC" in their name)
    int32 CrowdCount = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor && (Actor->GetName().Contains(TEXT("Crowd")) || Actor->GetName().Contains(TEXT("NPC"))))
        {
            CrowdCount++;
        }
    }
    
    CurrentCrowdCount = CrowdCount;
    return CrowdCount > 10; // Threshold for "crowded" area
}

void AQuest_CrowdInteractionManager::UpdateQuestProgress()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
        return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (FQuest_CrowdObjective& Objective : ActiveObjectives)
    {
        if (Objective.bIsCompleted)
            continue;
        
        float DistanceToTarget = FVector::Dist(PlayerLocation, Objective.TargetLocation);
        bool bInCrowdedArea = CheckCrowdDensityAtLocation(PlayerLocation, CrowdDetectionRadius);
        
        // Check completion conditions based on interaction type
        switch (Objective.InteractionType)
        {
            case EQuest_CrowdInteractionType::FindNPC:
                if (DistanceToTarget < 500.0f && bInCrowdedArea)
                {
                    CompleteObjective(Objective.ObjectiveName);
                }
                break;
                
            case EQuest_CrowdInteractionType::AvoidDetection:
                if (DistanceToTarget < 300.0f && !IsPlayerDetectedByCrowd(PlayerLocation))
                {
                    CompleteObjective(Objective.ObjectiveName);
                }
                break;
                
            case EQuest_CrowdInteractionType::EscortThroughCrowd:
                if (DistanceToTarget < 200.0f)
                {
                    CompleteObjective(Objective.ObjectiveName);
                }
                break;
        }
    }
}

TArray<FVector> AQuest_CrowdInteractionManager::GetNearbyNPCLocations(FVector PlayerLocation, float SearchRadius)
{
    TArray<FVector> NPCLocations;
    
    if (!GetWorld())
        return NPCLocations;
    
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        PlayerLocation,
        SearchRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        FoundActors
    );
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("NPC")))
        {
            NPCLocations.Add(Actor->GetActorLocation());
        }
    }
    
    return NPCLocations;
}

bool AQuest_CrowdInteractionManager::IsPlayerDetectedByCrowd(FVector PlayerLocation)
{
    // Simple detection logic based on proximity to crowd entities
    bool bDetected = CheckCrowdDensityAtLocation(PlayerLocation, 800.0f);
    
    if (bDetected)
    {
        TriggerCrowdReaction(PlayerLocation, 1000.0f);
    }
    
    return bDetected;
}

void AQuest_CrowdInteractionManager::TriggerCrowdReaction(FVector ReactionCenter, float ReactionRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionManager: Crowd reaction triggered at %s"), 
           *ReactionCenter.ToString());
    
    // Visual debug for crowd reaction
    if (GetWorld())
    {
        DrawDebugSphere(GetWorld(), ReactionCenter, ReactionRadius, 12, FColor::Red, false, 2.0f);
    }
}

FVector AQuest_CrowdInteractionManager::FindSafestPathThroughCrowd(FVector StartLocation, FVector EndLocation)
{
    // Simple pathfinding that avoids high-density crowd areas
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    FVector SafePath = StartLocation + (Direction * 1000.0f);
    
    // Check if the direct path has low crowd density
    if (!CheckCrowdDensityAtLocation(SafePath, 500.0f))
    {
        return SafePath;
    }
    
    // Try alternative paths (left and right)
    FVector LeftPath = StartLocation + FVector::CrossProduct(Direction, FVector::UpVector) * 500.0f;
    FVector RightPath = StartLocation + FVector::CrossProduct(FVector::UpVector, Direction) * 500.0f;
    
    if (!CheckCrowdDensityAtLocation(LeftPath, 500.0f))
    {
        return LeftPath;
    }
    else if (!CheckCrowdDensityAtLocation(RightPath, 500.0f))
    {
        return RightPath;
    }
    
    return SafePath; // Fallback to direct path
}

bool AQuest_CrowdInteractionManager::ValidateQuestCompletion(const FString& QuestName)
{
    for (const FQuest_CrowdObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveName == QuestName)
        {
            return Objective.bIsCompleted;
        }
    }
    return false;
}

float AQuest_CrowdInteractionManager::CalculateQuestReward(const FString& QuestName)
{
    for (const FQuest_CrowdObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveName == QuestName)
        {
            float BaseReward = Objective.CompletionReward;
            
            // Bonus for completing in crowded areas
            if (CurrentCrowdCount > 50)
            {
                BaseReward *= 1.5f;
            }
            
            // Time bonus
            if (QuestStartTimes.Contains(QuestName))
            {
                float CompletionTime = GetWorld()->GetTimeSeconds() - QuestStartTimes[QuestName];
                if (CompletionTime < 300.0f) // 5 minutes
                {
                    BaseReward *= 1.2f;
                }
            }
            
            return BaseReward;
        }
    }
    return 0.0f;
}

void AQuest_CrowdInteractionManager::ScanForNearbyEntities()
{
    if (!GetWorld())
        return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
        return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    TrackedNPCLocations = GetNearbyNPCLocations(PlayerLocation, CrowdDetectionRadius);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdInteractionManager: Tracking %d NPCs near player"), 
           TrackedNPCLocations.Num());
}

void AQuest_CrowdInteractionManager::UpdateCrowdDensityMap()
{
    if (!GetWorld())
        return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
        return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // Update crowd density at player location
    bool bInCrowdedArea = CheckCrowdDensityAtLocation(PlayerLocation, CrowdDetectionRadius);
    
    if (bInCrowdedArea)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_CrowdInteractionManager: Player in crowded area - %d entities nearby"), 
               CurrentCrowdCount);
    }
}

bool AQuest_CrowdInteractionManager::IsLocationSafeFromCrowd(FVector Location)
{
    return !CheckCrowdDensityAtLocation(Location, 1000.0f);
}