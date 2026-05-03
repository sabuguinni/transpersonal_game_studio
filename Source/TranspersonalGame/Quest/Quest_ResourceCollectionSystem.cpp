#include "Quest_ResourceCollectionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogResourceCollection, Log, All);

UQuest_ResourceCollectionSystem::UQuest_ResourceCollectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize resource collection goals
    ResourceCollectionGoals.Add(EResourceType::Stone, 10);
    ResourceCollectionGoals.Add(EResourceType::Wood, 15);
    ResourceCollectionGoals.Add(EResourceType::Food, 5);
    ResourceCollectionGoals.Add(EResourceType::Water, 3);
    ResourceCollectionGoals.Add(EResourceType::Fiber, 8);
    
    // Initialize collected amounts
    ResourcesCollected.Add(EResourceType::Stone, 0);
    ResourcesCollected.Add(EResourceType::Wood, 0);
    ResourcesCollected.Add(EResourceType::Food, 0);
    ResourcesCollected.Add(EResourceType::Water, 0);
    ResourcesCollected.Add(EResourceType::Fiber, 0);
    
    CollectionRadius = 200.0f;
    bAutoCollectNearbyResources = true;
    TotalResourcesCollected = 0;
}

void UQuest_ResourceCollectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogResourceCollection, Log, TEXT("Resource Collection System initialized"));
    
    // Spawn initial resource nodes across biomes
    SpawnResourceNodes();
    
    // Display initial collection goals
    DisplayCollectionGoals();
}

void UQuest_ResourceCollectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
        return;
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;
    
    // Auto-collect nearby resources if enabled
    if (bAutoCollectNearbyResources)
    {
        CheckForNearbyResources(PlayerPawn->GetActorLocation());
    }
    
    // Check collection progress
    CheckCollectionProgress();
}

void UQuest_ResourceCollectionSystem::SpawnResourceNodes()
{
    if (!GetWorld())
        return;
    
    UE_LOG(LogResourceCollection, Log, TEXT("Spawning resource nodes across biomes"));
    
    // Define resource spawn locations for each biome
    TArray<FResourceSpawnData> SpawnLocations = {
        // Savana - balanced resources
        {FVector(2000, 1500, 100), EResourceType::Stone, TEXT("SavanaStone1")},
        {FVector(-1000, 2000, 100), EResourceType::Wood, TEXT("SavanaWood1")},
        {FVector(1500, -1800, 100), EResourceType::Fiber, TEXT("SavanaFiber1")},
        {FVector(-500, 500, 100), EResourceType::Food, TEXT("SavanaFood1")},
        
        // Pântano - water and fiber rich
        {FVector(-50000, -45000, 50), EResourceType::Water, TEXT("SwampWater1")},
        {FVector(-48000, -43000, 50), EResourceType::Fiber, TEXT("SwampFiber1")},
        {FVector(-52000, -47000, 50), EResourceType::Food, TEXT("SwampFood1")},
        
        // Floresta - wood and food rich
        {FVector(-45000, 40000, 100), EResourceType::Wood, TEXT("ForestWood1")},
        {FVector(-43000, 42000, 100), EResourceType::Wood, TEXT("ForestWood2")},
        {FVector(-47000, 38000, 100), EResourceType::Food, TEXT("ForestFood1")},
        {FVector(-44000, 41000, 100), EResourceType::Fiber, TEXT("ForestFiber1")},
        
        // Deserto - stone rich but scarce resources
        {FVector(55000, 0, 100), EResourceType::Stone, TEXT("DesertStone1")},
        {FVector(57000, 2000, 100), EResourceType::Stone, TEXT("DesertStone2")},
        {FVector(53000, -1500, 100), EResourceType::Water, TEXT("DesertWater1")},
        
        // Montanha - stone and rare resources
        {FVector(40000, 50000, 600), EResourceType::Stone, TEXT("MountainStone1")},
        {FVector(42000, 52000, 650), EResourceType::Stone, TEXT("MountainStone2")},
        {FVector(38000, 48000, 580), EResourceType::Water, TEXT("MountainWater1")}
    };
    
    for (const FResourceSpawnData& SpawnData : SpawnLocations)
    {
        SpawnResourceNode(SpawnData.Location, SpawnData.ResourceType, SpawnData.NodeName);
    }
}

void UQuest_ResourceCollectionSystem::SpawnResourceNode(const FVector& Location, EResourceType ResourceType, const FString& NodeName)
{
    if (!GetWorld())
        return;
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* ResourceNode = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (ResourceNode)
    {
        ResourceNode->SetActorLabel(NodeName);
        ResourceNode->SetFolderPath(TEXT("/Quest/Resources"));
        
        // Add sphere collision for interaction detection
        USphereComponent* SphereComponent = NewObject<USphereComponent>(ResourceNode);
        if (SphereComponent)
        {
            SphereComponent->SetSphereRadius(CollectionRadius);
            SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
            SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
            ResourceNode->SetRootComponent(SphereComponent);
        }
        
        // Store resource data
        ResourceNodes.Add(ResourceNode, ResourceType);
        
        FString ResourceTypeName = UEnum::GetValueAsString(ResourceType);
        UE_LOG(LogResourceCollection, Log, TEXT("Spawned %s resource node: %s"), *ResourceTypeName, *NodeName);
    }
}

void UQuest_ResourceCollectionSystem::CheckForNearbyResources(const FVector& PlayerLocation)
{
    TArray<AActor*> NodesToRemove;
    
    for (auto& NodePair : ResourceNodes)
    {
        AActor* ResourceNode = NodePair.Key;
        EResourceType ResourceType = NodePair.Value;
        
        if (!IsValid(ResourceNode))
        {
            NodesToRemove.Add(ResourceNode);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, ResourceNode->GetActorLocation());
        if (Distance <= CollectionRadius)
        {
            // Collect the resource
            CollectResource(ResourceType, ResourceNode);
            NodesToRemove.Add(ResourceNode);
        }
    }
    
    // Remove collected nodes
    for (AActor* NodeToRemove : NodesToRemove)
    {
        ResourceNodes.Remove(NodeToRemove);
        if (IsValid(NodeToRemove))
        {
            NodeToRemove->Destroy();
        }
    }
}

void UQuest_ResourceCollectionSystem::CollectResource(EResourceType ResourceType, AActor* ResourceNode)
{
    // Increase collected amount
    int32& CollectedAmount = ResourcesCollected[ResourceType];
    CollectedAmount++;
    TotalResourcesCollected++;
    
    FString ResourceTypeName = UEnum::GetValueAsString(ResourceType);
    FString NodeName = ResourceNode ? ResourceNode->GetActorLabel() : TEXT("Unknown");
    
    UE_LOG(LogResourceCollection, Log, TEXT("Collected %s from %s (Total %s: %d)"), 
           *ResourceTypeName, *NodeName, *ResourceTypeName, CollectedAmount);
    
    // Display collection notification
    if (GEngine)
    {
        int32 Goal = ResourceCollectionGoals[ResourceType];
        FString Message = FString::Printf(TEXT("Collected %s (%d/%d)"), 
                                        *ResourceTypeName, CollectedAmount, Goal);
        FColor NotificationColor = GetResourceColor(ResourceType);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, NotificationColor, Message);
    }
    
    // Check if resource goal completed
    CheckResourceGoalCompletion(ResourceType);
}

void UQuest_ResourceCollectionSystem::CheckResourceGoalCompletion(EResourceType ResourceType)
{
    int32 CollectedAmount = ResourcesCollected[ResourceType];
    int32 Goal = ResourceCollectionGoals[ResourceType];
    
    if (CollectedAmount >= Goal)
    {
        FString ResourceTypeName = UEnum::GetValueAsString(ResourceType);
        UE_LOG(LogResourceCollection, Warning, TEXT("Resource collection goal completed: %s!"), *ResourceTypeName);
        
        if (GEngine)
        {
            FString Message = FString::Printf(TEXT("GOAL COMPLETED: %s collection finished! (+50 XP)"), *ResourceTypeName);
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Gold, Message);
        }
        
        // Award completion XP
        AwardResourceCollectionXP(50);
    }
}

void UQuest_ResourceCollectionSystem::CheckCollectionProgress()
{
    // Check if all resource goals are completed
    bool bAllGoalsCompleted = true;
    for (const auto& GoalPair : ResourceCollectionGoals)
    {
        EResourceType ResourceType = GoalPair.Key;
        int32 Goal = GoalPair.Value;
        int32 Collected = ResourcesCollected[ResourceType];
        
        if (Collected < Goal)
        {
            bAllGoalsCompleted = false;
            break;
        }
    }
    
    if (bAllGoalsCompleted && !bAllResourceGoalsCompleted)
    {
        bAllResourceGoalsCompleted = true;
        OnAllResourceGoalsCompleted();
    }
}

void UQuest_ResourceCollectionSystem::OnAllResourceGoalsCompleted()
{
    UE_LOG(LogResourceCollection, Warning, TEXT("ALL RESOURCE COLLECTION GOALS COMPLETED!"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Gold, 
            TEXT("MASTER GATHERER! All resource collection goals completed! (+200 XP)"));
    }
    
    // Award master gatherer bonus
    AwardResourceCollectionXP(200);
    
    // Unlock advanced crafting recipes
    UnlockAdvancedCraftingRecipes();
}

void UQuest_ResourceCollectionSystem::DisplayCollectionGoals()
{
    if (!GEngine)
        return;
    
    UE_LOG(LogResourceCollection, Log, TEXT("=== RESOURCE COLLECTION GOALS ==="));
    
    for (const auto& GoalPair : ResourceCollectionGoals)
    {
        EResourceType ResourceType = GoalPair.Key;
        int32 Goal = GoalPair.Value;
        int32 Collected = ResourcesCollected[ResourceType];
        
        FString ResourceTypeName = UEnum::GetValueAsString(ResourceType);
        FString ProgressText = FString::Printf(TEXT("%s: %d/%d"), *ResourceTypeName, Collected, Goal);
        
        UE_LOG(LogResourceCollection, Log, TEXT("  %s"), *ProgressText);
        
        FColor ProgressColor = (Collected >= Goal) ? FColor::Green : FColor::White;
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, ProgressColor, ProgressText);
    }
}

FColor UQuest_ResourceCollectionSystem::GetResourceColor(EResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EResourceType::Stone: return FColor::Silver;
        case EResourceType::Wood: return FColor::Orange;
        case EResourceType::Food: return FColor::Green;
        case EResourceType::Water: return FColor::Blue;
        case EResourceType::Fiber: return FColor::Yellow;
        default: return FColor::White;
    }
}

void UQuest_ResourceCollectionSystem::AwardResourceCollectionXP(int32 XPAmount)
{
    // TODO: Integrate with character progression system
    UE_LOG(LogResourceCollection, Log, TEXT("Awarded %d resource collection XP"), XPAmount);
}

void UQuest_ResourceCollectionSystem::UnlockAdvancedCraftingRecipes()
{
    UE_LOG(LogResourceCollection, Warning, TEXT("Advanced crafting recipes unlocked!"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, 
            TEXT("Advanced crafting recipes unlocked! Check your crafting menu."));
    }
}

int32 UQuest_ResourceCollectionSystem::GetResourceCount(EResourceType ResourceType) const
{
    const int32* Count = ResourcesCollected.Find(ResourceType);
    return Count ? *Count : 0;
}

bool UQuest_ResourceCollectionSystem::IsResourceGoalCompleted(EResourceType ResourceType) const
{
    int32 Collected = GetResourceCount(ResourceType);
    const int32* Goal = ResourceCollectionGoals.Find(ResourceType);
    return Goal ? (Collected >= *Goal) : false;
}

float UQuest_ResourceCollectionSystem::GetOverallCollectionProgress() const
{
    int32 CompletedGoals = 0;
    int32 TotalGoals = ResourceCollectionGoals.Num();
    
    for (const auto& GoalPair : ResourceCollectionGoals)
    {
        if (IsResourceGoalCompleted(GoalPair.Key))
        {
            CompletedGoals++;
        }
    }
    
    return TotalGoals > 0 ? (float)CompletedGoals / (float)TotalGoals : 0.0f;
}