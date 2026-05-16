#include "Quest_ResourceGatheringSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UQuest_ResourceGatheringSystem::UQuest_ResourceGatheringSystem()
{
    ResourceRespawnMultiplier = 1.0f;
}

void UQuest_ResourceGatheringSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Initializing resource gathering system"));
    
    CreateDefaultResourceNodes();
    
    // Set up timer for resource updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            FTimerDelegate::CreateUObject(this, &UQuest_ResourceGatheringSystem::UpdateResourceNodes, 1.0f),
            1.0f,
            true
        );
    }
}

void UQuest_ResourceGatheringSystem::Deinitialize()
{
    ResourceNodes.Empty();
    ActiveMissions.Empty();
    PlayerInventory.Empty();
    
    Super::Deinitialize();
}

void UQuest_ResourceGatheringSystem::SpawnResourceNode(EQuest_ResourceType ResourceType, FVector Location, int32 Quantity)
{
    FQuest_ResourceNode NewNode;
    NewNode.ResourceType = ResourceType;
    NewNode.Location = Location;
    NewNode.Quantity = Quantity;
    NewNode.bIsActive = true;
    NewNode.LastHarvestTime = 0.0f;
    
    // Set difficulty based on resource type
    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone:
        case EQuest_ResourceType::Wood:
            NewNode.Difficulty = EQuest_GatheringDifficulty::Easy;
            break;
        case EQuest_ResourceType::Plant:
        case EQuest_ResourceType::Water:
            NewNode.Difficulty = EQuest_GatheringDifficulty::Medium;
            break;
        case EQuest_ResourceType::Bone:
        case EQuest_ResourceType::Hide:
        case EQuest_ResourceType::Meat:
            NewNode.Difficulty = EQuest_GatheringDifficulty::Hard;
            break;
        default:
            NewNode.Difficulty = EQuest_GatheringDifficulty::Dangerous;
            break;
    }
    
    ResourceNodes.Add(NewNode);
    SpawnResourceActor(NewNode);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Spawned %s resource at %s"), 
           *GetResourceTypeName(ResourceType), *Location.ToString());
}

void UQuest_ResourceGatheringSystem::RemoveResourceNode(FVector Location)
{
    for (int32 i = ResourceNodes.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(ResourceNodes[i].Location, Location) < 100.0f)
        {
            ResourceNodes.RemoveAt(i);
            break;
        }
    }
}

bool UQuest_ResourceGatheringSystem::HarvestResource(FVector Location, int32& OutQuantity, EQuest_ResourceType& OutResourceType)
{
    FQuest_ResourceNode* Node = FindResourceNodeAtLocation(Location);
    if (!Node || !Node->bIsActive || Node->Quantity <= 0)
    {
        OutQuantity = 0;
        return false;
    }
    
    OutQuantity = Node->Quantity;
    OutResourceType = Node->ResourceType;
    
    // Add to player inventory
    if (PlayerInventory.Contains(OutResourceType))
    {
        PlayerInventory[OutResourceType] += OutQuantity;
    }
    else
    {
        PlayerInventory.Add(OutResourceType, OutQuantity);
    }
    
    // Mark as harvested
    Node->bIsActive = false;
    Node->LastHarvestTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Harvested %d %s"), 
           OutQuantity, *GetResourceTypeName(OutResourceType));
    
    return true;
}

TArray<FQuest_ResourceNode> UQuest_ResourceGatheringSystem::GetNearbyResources(FVector PlayerLocation, float SearchRadius)
{
    TArray<FQuest_ResourceNode> NearbyNodes;
    
    for (const FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (Node.bIsActive && FVector::Dist(Node.Location, PlayerLocation) <= SearchRadius)
        {
            NearbyNodes.Add(Node);
        }
    }
    
    return NearbyNodes;
}

void UQuest_ResourceGatheringSystem::StartGatheringMission(const FQuest_GatheringMission& Mission)
{
    FQuest_GatheringMission NewMission = Mission;
    NewMission.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewMission.bIsCompleted = false;
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Started gathering mission: %s"), 
           *Mission.MissionName);
}

bool UQuest_ResourceGatheringSystem::CheckMissionProgress(const FString& MissionName, TMap<EQuest_ResourceType, int32>& OutProgress)
{
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName)
        {
            OutProgress.Empty();
            bool bMissionComplete = true;
            
            for (const auto& RequiredResource : Mission.RequiredResources)
            {
                int32 PlayerAmount = PlayerInventory.Contains(RequiredResource.Key) ? 
                                   PlayerInventory[RequiredResource.Key] : 0;
                OutProgress.Add(RequiredResource.Key, PlayerAmount);
                
                if (PlayerAmount < RequiredResource.Value)
                {
                    bMissionComplete = false;
                }
            }
            
            return bMissionComplete;
        }
    }
    
    return false;
}

void UQuest_ResourceGatheringSystem::CompleteMission(const FString& MissionName)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (ActiveMissions[i].MissionName == MissionName)
        {
            ActiveMissions[i].bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Completed mission: %s"), *MissionName);
            break;
        }
    }
}

void UQuest_ResourceGatheringSystem::PopulateBiomeWithResources(EBiomeType BiomeType, FVector BiomeCenter, float BiomeRadius)
{
    int32 ResourceCount = 0;
    
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            // Savanna: Stone, Wood, some plants
            for (int32 i = 0; i < 20; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Stone, RandomLocation, FMath::RandRange(1, 3));
                ResourceCount++;
            }
            for (int32 i = 0; i < 15; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Wood, RandomLocation, FMath::RandRange(2, 5));
                ResourceCount++;
            }
            break;
            
        case EBiomeType::Forest:
            // Forest: Lots of wood, plants, some water
            for (int32 i = 0; i < 30; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Wood, RandomLocation, FMath::RandRange(3, 7));
                ResourceCount++;
            }
            for (int32 i = 0; i < 25; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Plant, RandomLocation, FMath::RandRange(1, 4));
                ResourceCount++;
            }
            break;
            
        case EBiomeType::Desert:
            // Desert: Stone, rare water
            for (int32 i = 0; i < 25; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Stone, RandomLocation, FMath::RandRange(2, 4));
                ResourceCount++;
            }
            for (int32 i = 0; i < 5; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Water, RandomLocation, 1);
                ResourceCount++;
            }
            break;
            
        case EBiomeType::Swamp:
            // Swamp: Water, plants, some wood
            for (int32 i = 0; i < 20; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Water, RandomLocation, FMath::RandRange(2, 4));
                ResourceCount++;
            }
            for (int32 i = 0; i < 20; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    0.0f
                );
                SpawnResourceNode(EQuest_ResourceType::Plant, RandomLocation, FMath::RandRange(2, 5));
                ResourceCount++;
            }
            break;
            
        case EBiomeType::Mountain:
            // Mountain: Lots of stone, some rare resources
            for (int32 i = 0; i < 35; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(0.0f, 500.0f)
                );
                SpawnResourceNode(EQuest_ResourceType::Stone, RandomLocation, FMath::RandRange(3, 6));
                ResourceCount++;
            }
            for (int32 i = 0; i < 10; i++)
            {
                FVector RandomLocation = BiomeCenter + FVector(
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(-BiomeRadius, BiomeRadius),
                    FMath::RandRange(0.0f, 500.0f)
                );
                SpawnResourceNode(EQuest_ResourceType::Bone, RandomLocation, FMath::RandRange(1, 2));
                ResourceCount++;
            }
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Populated %s biome with %d resource nodes"), 
           *UEnum::GetValueAsString(BiomeType), ResourceCount);
}

FString UQuest_ResourceGatheringSystem::GetResourceTypeName(EQuest_ResourceType ResourceType)
{
    return UEnum::GetValueAsString(ResourceType);
}

void UQuest_ResourceGatheringSystem::UpdateResourceNodes(float DeltaTime)
{
    if (!GetWorld()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (!Node.bIsActive && Node.LastHarvestTime > 0.0f)
        {
            float TimeSinceHarvest = CurrentTime - Node.LastHarvestTime;
            if (TimeSinceHarvest >= Node.RespawnTime * ResourceRespawnMultiplier)
            {
                Node.bIsActive = true;
                Node.LastHarvestTime = 0.0f;
                SpawnResourceActor(Node);
            }
        }
    }
    
    CheckMissionTimeouts();
}

void UQuest_ResourceGatheringSystem::CreateDefaultResourceNodes()
{
    // Create some default resource nodes in the savanna biome (center of map)
    SpawnResourceNode(EQuest_ResourceType::Stone, FVector(1000.0f, 1000.0f, 100.0f), 2);
    SpawnResourceNode(EQuest_ResourceType::Wood, FVector(1500.0f, 500.0f, 100.0f), 3);
    SpawnResourceNode(EQuest_ResourceType::Plant, FVector(500.0f, 1500.0f, 100.0f), 1);
    SpawnResourceNode(EQuest_ResourceType::Water, FVector(2000.0f, 2000.0f, 50.0f), 1);
}

void UQuest_ResourceGatheringSystem::SpawnResourceActor(const FQuest_ResourceNode& ResourceNode)
{
    if (!GetWorld()) return;
    
    // This would spawn a visual representation of the resource
    // For now, we just log that it should be spawned
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Should spawn visual actor for %s at %s"), 
           *GetResourceTypeName(ResourceNode.ResourceType), *ResourceNode.Location.ToString());
}

FQuest_ResourceNode* UQuest_ResourceGatheringSystem::FindResourceNodeAtLocation(FVector Location)
{
    for (FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (FVector::Dist(Node.Location, Location) < 100.0f)
        {
            return &Node;
        }
    }
    return nullptr;
}

void UQuest_ResourceGatheringSystem::CheckMissionTimeouts()
{
    if (!GetWorld()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (!Mission.bIsCompleted && Mission.TimeLimit > 0.0f)
        {
            float ElapsedTime = CurrentTime - Mission.StartTime;
            if (ElapsedTime >= Mission.TimeLimit)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringSystem: Mission %s timed out"), 
                       *Mission.MissionName);
                Mission.bIsCompleted = true; // Mark as failed/completed
            }
        }
    }
}