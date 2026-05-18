#include "Quest_ResourceGatheringSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

UQuest_ResourceGatheringSystem::UQuest_ResourceGatheringSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    ResourceScanRadius = 5000.0f;
    MaxResourcesPerBiome = 50;
    MaxConcurrentMissions = 3;
    MissionGenerationInterval = 180.0f;
    LastMissionGenerationTime = 0.0f;
}

void UQuest_ResourceGatheringSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize resource system
    SpawnResourceNodes();
    
    // Generate initial gathering mission
    GenerateGatheringMission();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Resource Gathering System initialized"));
}

void UQuest_ResourceGatheringSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMissionTimers(DeltaTime);
    UpdateResourceRespawn(DeltaTime);
    CheckMissionCompletion();
    
    // Generate new missions periodically
    if (GetWorld()->GetTimeSeconds() - LastMissionGenerationTime > MissionGenerationInterval)
    {
        if (ActiveMissions.Num() < MaxConcurrentMissions)
        {
            GenerateGatheringMission();
            LastMissionGenerationTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UQuest_ResourceGatheringSystem::SpawnResourceNodes()
{
    TArray<EBiomeType> Biomes = {EBiomeType::Savana, EBiomeType::Forest, EBiomeType::Swamp, EBiomeType::Desert, EBiomeType::Mountain};
    TArray<FString> ResourceTypes = GetAvailableResourceTypes();
    
    for (EBiomeType Biome : Biomes)
    {
        FVector BiomeCenter = GetBiomeCenter(Biome);
        
        for (int32 i = 0; i < MaxResourcesPerBiome; i++)
        {
            FString ResourceType = ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
            
            // Random location within biome radius
            float Angle = FMath::RandRange(0.0f, 360.0f);
            float Distance = FMath::RandRange(1000.0f, 8000.0f);
            FVector Offset = FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
                0.0f
            );
            
            FVector SpawnLocation = BiomeCenter + Offset;
            SpawnLocation.Z = 100.0f; // Ground level
            
            int32 Quantity = FMath::RandRange(1, 3);
            SpawnResourceNodeAtLocation(ResourceType, SpawnLocation, Quantity);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d resource nodes across all biomes"), ResourceNodes.Num());
}

void UQuest_ResourceGatheringSystem::SpawnResourceNodeAtLocation(const FString& ResourceType, const FVector& Location, int32 Quantity)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create resource node data
    FQuest_ResourceNode NewNode;
    NewNode.ResourceType = ResourceType;
    NewNode.Quantity = Quantity;
    NewNode.Location = Location;
    NewNode.bIsActive = true;
    NewNode.RespawnTime = FMath::RandRange(300.0f, 600.0f);
    
    // Spawn visual actor
    AStaticMeshActor* ResourceActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (ResourceActor)
    {
        ResourceActor->SetActorLabel(FString::Printf(TEXT("Resource_%s_%d"), *ResourceType, ResourceNodes.Num()));
        
        // Set a basic cube mesh as placeholder
        UStaticMeshComponent* MeshComp = ResourceActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                
                // Scale based on resource type
                FVector Scale = FVector(0.5f, 0.5f, 0.5f);
                if (ResourceType == TEXT("Stone")) Scale = FVector(0.8f, 0.8f, 0.6f);
                else if (ResourceType == TEXT("Wood")) Scale = FVector(0.3f, 0.3f, 1.2f);
                else if (ResourceType == TEXT("Plant")) Scale = FVector(0.4f, 0.4f, 0.8f);
                
                ResourceActor->SetActorScale3D(Scale);
            }
        }
        
        NewNode.NodeActor = ResourceActor;
    }
    
    ResourceNodes.Add(NewNode);
}

bool UQuest_ResourceGatheringSystem::GatherResource(const FVector& Location, const FString& ResourceType, int32 Quantity)
{
    // Find nearby resource node
    for (int32 i = 0; i < ResourceNodes.Num(); i++)
    {
        FQuest_ResourceNode& Node = ResourceNodes[i];
        
        if (Node.bIsActive && Node.ResourceType == ResourceType)
        {
            float Distance = FVector::Dist(Location, Node.Location);
            if (Distance <= 200.0f) // Gathering range
            {
                int32 GatheredAmount = FMath::Min(Quantity, Node.Quantity);
                Node.Quantity -= GatheredAmount;
                
                // Add to player inventory
                AddToPlayerInventory(ResourceType, GatheredAmount);
                
                // Update mission progress
                UpdateMissionProgress(ResourceType, GatheredAmount);
                
                // Deactivate node if depleted
                if (Node.Quantity <= 0)
                {
                    Node.bIsActive = false;
                    if (Node.NodeActor)
                    {
                        Node.NodeActor->SetActorHiddenInGame(true);
                    }
                }
                
                UE_LOG(LogTemp, Warning, TEXT("Gathered %d %s at location %s"), GatheredAmount, *ResourceType, *Location.ToString());
                return true;
            }
        }
    }
    
    return false;
}

int32 UQuest_ResourceGatheringSystem::GetPlayerResourceCount(const FString& ResourceType)
{
    if (PlayerInventory.Contains(ResourceType))
    {
        return PlayerInventory[ResourceType];
    }
    return 0;
}

void UQuest_ResourceGatheringSystem::AddToPlayerInventory(const FString& ResourceType, int32 Quantity)
{
    if (PlayerInventory.Contains(ResourceType))
    {
        PlayerInventory[ResourceType] += Quantity;
    }
    else
    {
        PlayerInventory.Add(ResourceType, Quantity);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Added %d %s to inventory. Total: %d"), Quantity, *ResourceType, PlayerInventory[ResourceType]);
}

TArray<FQuest_ResourceNode> UQuest_ResourceGatheringSystem::GetNearbyResources(const FVector& Location, float Radius)
{
    TArray<FQuest_ResourceNode> NearbyNodes;
    
    for (const FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (Node.bIsActive)
        {
            float Distance = FVector::Dist(Location, Node.Location);
            if (Distance <= Radius)
            {
                NearbyNodes.Add(Node);
            }
        }
    }
    
    return NearbyNodes;
}

void UQuest_ResourceGatheringSystem::GenerateGatheringMission()
{
    TArray<FString> ResourceTypes = GetAvailableResourceTypes();
    FString TargetResource = ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
    
    FQuest_GatheringMission NewMission;
    NewMission.MissionName = FString::Printf(TEXT("Gather %s Resources"), *TargetResource);
    NewMission.TargetResource = TargetResource;
    NewMission.RequiredQuantity = FMath::RandRange(3, 8);
    NewMission.CurrentQuantity = 0;
    NewMission.TimeLimit = FMath::RandRange(300.0f, 900.0f);
    NewMission.TimeRemaining = NewMission.TimeLimit;
    NewMission.bIsActive = false;
    NewMission.bIsCompleted = false;
    NewMission.RewardExperience = NewMission.RequiredQuantity * 20.0f;
    
    // Add hint locations (nearby resource nodes)
    for (const FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (Node.bIsActive && Node.ResourceType == TargetResource)
        {
            NewMission.HintLocations.Add(Node.Location);
            if (NewMission.HintLocations.Num() >= 3) break;
        }
    }
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Generated gathering mission: %s (Collect %d %s)"), 
        *NewMission.MissionName, NewMission.RequiredQuantity, *TargetResource);
}

void UQuest_ResourceGatheringSystem::StartMission(const FString& MissionName)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && !Mission.bIsActive)
        {
            Mission.bIsActive = true;
            UE_LOG(LogTemp, Warning, TEXT("Started gathering mission: %s"), *MissionName);
            break;
        }
    }
}

void UQuest_ResourceGatheringSystem::UpdateMissionProgress(const FString& ResourceType, int32 Quantity)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted && Mission.TargetResource == ResourceType)
        {
            Mission.CurrentQuantity += Quantity;
            UE_LOG(LogTemp, Warning, TEXT("Mission progress: %s (%d/%d)"), 
                *Mission.MissionName, Mission.CurrentQuantity, Mission.RequiredQuantity);
        }
    }
}

void UQuest_ResourceGatheringSystem::CompleteMission(const FString& MissionName)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_GatheringMission& Mission = ActiveMissions[i];
        if (Mission.MissionName == MissionName)
        {
            Mission.bIsCompleted = true;
            Mission.bIsActive = false;
            
            UE_LOG(LogTemp, Warning, TEXT("Completed gathering mission: %s (Reward: %.1f XP)"), 
                *MissionName, Mission.RewardExperience);
            
            // Remove completed mission after a delay
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

bool UQuest_ResourceGatheringSystem::IsMissionActive(const FString& MissionName)
{
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && Mission.bIsActive)
        {
            return true;
        }
    }
    return false;
}

TArray<FQuest_GatheringMission> UQuest_ResourceGatheringSystem::GetActiveMissions()
{
    TArray<FQuest_GatheringMission> Result;
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive)
        {
            Result.Add(Mission);
        }
    }
    return Result;
}

FVector UQuest_ResourceGatheringSystem::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(2000.0f, 8000.0f);
    
    FVector RandomLocation = BiomeCenter + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        100.0f
    );
    
    return RandomLocation;
}

FString UQuest_ResourceGatheringSystem::GetRandomResourceType()
{
    TArray<FString> ResourceTypes = GetAvailableResourceTypes();
    return ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
}

void UQuest_ResourceGatheringSystem::CleanupExpiredMissions()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_GatheringMission& Mission = ActiveMissions[i];
        if (Mission.bIsActive && Mission.TimeRemaining <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission expired: %s"), *Mission.MissionName);
            ActiveMissions.RemoveAt(i);
        }
    }
}

void UQuest_ResourceGatheringSystem::RespawnResourceNode(int32 NodeIndex)
{
    if (ResourceNodes.IsValidIndex(NodeIndex))
    {
        FQuest_ResourceNode& Node = ResourceNodes[NodeIndex];
        if (!Node.bIsActive)
        {
            Node.bIsActive = true;
            Node.Quantity = FMath::RandRange(1, 3);
            
            if (Node.NodeActor)
            {
                Node.NodeActor->SetActorHiddenInGame(false);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Respawned resource node: %s"), *Node.ResourceType);
        }
    }
}

void UQuest_ResourceGatheringSystem::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive)
        {
            Mission.TimeRemaining -= DeltaTime;
        }
    }
}

void UQuest_ResourceGatheringSystem::UpdateResourceRespawn(float DeltaTime)
{
    for (int32 i = 0; i < ResourceNodes.Num(); i++)
    {
        FQuest_ResourceNode& Node = ResourceNodes[i];
        if (!Node.bIsActive)
        {
            Node.RespawnTime -= DeltaTime;
            if (Node.RespawnTime <= 0.0f)
            {
                RespawnResourceNode(i);
                Node.RespawnTime = FMath::RandRange(300.0f, 600.0f);
            }
        }
    }
}

void UQuest_ResourceGatheringSystem::CheckMissionCompletion()
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted)
        {
            if (Mission.CurrentQuantity >= Mission.RequiredQuantity)
            {
                CompleteMission(Mission.MissionName);
            }
        }
    }
}

FVector UQuest_ResourceGatheringSystem::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savana: return FVector(0.0f, 0.0f, 0.0f);
        case EBiomeType::Forest: return FVector(-45000.0f, 40000.0f, 0.0f);
        case EBiomeType::Swamp: return FVector(-50000.0f, -45000.0f, 0.0f);
        case EBiomeType::Desert: return FVector(55000.0f, 0.0f, 0.0f);
        case EBiomeType::Mountain: return FVector(40000.0f, 50000.0f, 0.0f);
        default: return FVector::ZeroVector;
    }
}

TArray<FString> UQuest_ResourceGatheringSystem::GetAvailableResourceTypes()
{
    return {
        TEXT("Stone"),
        TEXT("Wood"),
        TEXT("Plant"),
        TEXT("Bone"),
        TEXT("Hide"),
        TEXT("Flint"),
        TEXT("Clay"),
        TEXT("Water")
    };
}