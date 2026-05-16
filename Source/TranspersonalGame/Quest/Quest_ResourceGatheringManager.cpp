#include "Quest_ResourceGatheringManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UQuest_ResourceGatheringManager::UQuest_ResourceGatheringManager()
{
    NextMissionID = 1;
}

void UQuest_ResourceGatheringManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceGatheringManager initialized"));
    
    // Create initial gathering missions
    CreateStoneGatheringMission(FVector(1000, 0, 100));
    CreateWoodGatheringMission(FVector(-1000, 1000, 100));
    CreatePlantGatheringMission(FVector(0, -1000, 100));
}

void UQuest_ResourceGatheringManager::Deinitialize()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    Super::Deinitialize();
}

void UQuest_ResourceGatheringManager::StartGatheringMission(const FString& MissionID)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EMissionStatus::Available)
        {
            Mission.Status = EMissionStatus::InProgress;
            Mission.ElapsedTime = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("Started gathering mission: %s"), *Mission.MissionName);
            break;
        }
    }
}

void UQuest_ResourceGatheringManager::CompleteGatheringMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].Status = EMissionStatus::Completed;
            CompletedMissions.Add(ActiveMissions[i]);
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Completed gathering mission: %s"), *ActiveMissions[i].MissionName);
            break;
        }
    }
}

void UQuest_ResourceGatheringManager::UpdateResourceProgress(const FString& MissionID, EQuest_ResourceType ResourceType, int32 Amount)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EMissionStatus::InProgress)
        {
            for (FQuest_ResourceTarget& Target : Mission.ResourceTargets)
            {
                if (Target.ResourceType == ResourceType)
                {
                    Target.CurrentAmount = FMath::Min(Target.CurrentAmount + Amount, Target.TargetAmount);
                    
                    // Check if all targets are complete
                    bool bAllComplete = true;
                    for (const FQuest_ResourceTarget& CheckTarget : Mission.ResourceTargets)
                    {
                        if (CheckTarget.CurrentAmount < CheckTarget.TargetAmount)
                        {
                            bAllComplete = false;
                            break;
                        }
                    }
                    
                    if (bAllComplete)
                    {
                        CompleteGatheringMission(MissionID);
                    }
                    break;
                }
            }
            break;
        }
    }
}

FQuest_GatheringMission UQuest_ResourceGatheringManager::GetGatheringMission(const FString& MissionID)
{
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    return FQuest_GatheringMission();
}

TArray<FQuest_GatheringMission> UQuest_ResourceGatheringManager::GetActiveGatheringMissions()
{
    return ActiveMissions;
}

void UQuest_ResourceGatheringManager::CreateStoneGatheringMission(FVector Location)
{
    FQuest_GatheringMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Gather Stone Tools");
    NewMission.Description = TEXT("Collect stones to craft basic tools. Look for rocky outcrops and riverbanks.");
    NewMission.Priority = EMissionPriority::High;
    NewMission.Status = EMissionStatus::Available;
    NewMission.TimeLimit = 300.0f;
    NewMission.MissionLocation = Location;
    
    FQuest_ResourceTarget StoneTarget;
    StoneTarget.ResourceType = EQuest_ResourceType::Stone;
    StoneTarget.TargetAmount = 5;
    StoneTarget.CurrentAmount = 0;
    StoneTarget.SearchLocation = Location;
    StoneTarget.SearchRadius = 2000.0f;
    
    NewMission.ResourceTargets.Add(StoneTarget);
    ActiveMissions.Add(NewMission);
    
    // Spawn stone pickups in the area
    SpawnResourcePickups(EQuest_ResourceType::Stone, Location, 8);
}

void UQuest_ResourceGatheringManager::CreateWoodGatheringMission(FVector Location)
{
    FQuest_GatheringMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Collect Fallen Branches");
    NewMission.Description = TEXT("Gather wood from fallen branches and dead trees for fire and shelter.");
    NewMission.Priority = EMissionPriority::Medium;
    NewMission.Status = EMissionStatus::Available;
    NewMission.TimeLimit = 400.0f;
    NewMission.MissionLocation = Location;
    
    FQuest_ResourceTarget WoodTarget;
    WoodTarget.ResourceType = EQuest_ResourceType::Wood;
    WoodTarget.TargetAmount = 10;
    WoodTarget.CurrentAmount = 0;
    WoodTarget.SearchLocation = Location;
    WoodTarget.SearchRadius = 3000.0f;
    
    NewMission.ResourceTargets.Add(WoodTarget);
    ActiveMissions.Add(NewMission);
    
    // Spawn wood pickups in the area
    SpawnResourcePickups(EQuest_ResourceType::Wood, Location, 12);
}

void UQuest_ResourceGatheringManager::CreatePlantGatheringMission(FVector Location)
{
    FQuest_GatheringMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Forage for Plants");
    NewMission.Description = TEXT("Search for edible plants, berries, and medicinal herbs in the wilderness.");
    NewMission.Priority = EMissionPriority::Medium;
    NewMission.Status = EMissionStatus::Available;
    NewMission.TimeLimit = 500.0f;
    NewMission.MissionLocation = Location;
    
    FQuest_ResourceTarget PlantTarget;
    PlantTarget.ResourceType = EQuest_ResourceType::Plant;
    PlantTarget.TargetAmount = 15;
    PlantTarget.CurrentAmount = 0;
    PlantTarget.SearchLocation = Location;
    PlantTarget.SearchRadius = 2500.0f;
    
    NewMission.ResourceTargets.Add(PlantTarget);
    ActiveMissions.Add(NewMission);
    
    // Spawn plant pickups in the area
    SpawnResourcePickups(EQuest_ResourceType::Plant, Location, 20);
}

void UQuest_ResourceGatheringManager::CreateWaterGatheringMission(FVector Location)
{
    FQuest_GatheringMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Find Fresh Water");
    NewMission.Description = TEXT("Locate a clean water source. Look for streams, springs, or collect rainwater.");
    NewMission.Priority = EMissionPriority::Critical;
    NewMission.Status = EMissionStatus::Available;
    NewMission.TimeLimit = 200.0f;
    NewMission.MissionLocation = Location;
    
    FQuest_ResourceTarget WaterTarget;
    WaterTarget.ResourceType = EQuest_ResourceType::Water;
    WaterTarget.TargetAmount = 3;
    WaterTarget.CurrentAmount = 0;
    WaterTarget.SearchLocation = Location;
    WaterTarget.SearchRadius = 4000.0f;
    
    NewMission.ResourceTargets.Add(WaterTarget);
    ActiveMissions.Add(NewMission);
}

void UQuest_ResourceGatheringManager::CreateHuntingMission(FVector Location)
{
    FQuest_GatheringMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Hunt for Meat");
    NewMission.Description = TEXT("Track and hunt small prey for meat and hide. Be cautious of larger predators.");
    NewMission.Priority = EMissionPriority::High;
    NewMission.Status = EMissionStatus::Available;
    NewMission.TimeLimit = 800.0f;
    NewMission.MissionLocation = Location;
    
    FQuest_ResourceTarget MeatTarget;
    MeatTarget.ResourceType = EQuest_ResourceType::Meat;
    MeatTarget.TargetAmount = 2;
    MeatTarget.CurrentAmount = 0;
    MeatTarget.SearchLocation = Location;
    MeatTarget.SearchRadius = 5000.0f;
    
    FQuest_ResourceTarget HideTarget;
    HideTarget.ResourceType = EQuest_ResourceType::Hide;
    HideTarget.TargetAmount = 1;
    HideTarget.CurrentAmount = 0;
    HideTarget.SearchLocation = Location;
    HideTarget.SearchRadius = 5000.0f;
    
    NewMission.ResourceTargets.Add(MeatTarget);
    NewMission.ResourceTargets.Add(HideTarget);
    ActiveMissions.Add(NewMission);
}

bool UQuest_ResourceGatheringManager::IsResourceInRange(EQuest_ResourceType ResourceType, FVector PlayerLocation, float Range)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for resource actors in range
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetActorLabel().Contains(GetResourceTypeName(ResourceType)))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            if (Distance <= Range)
            {
                return true;
            }
        }
    }
    
    return false;
}

FVector UQuest_ResourceGatheringManager::FindNearestResourceLocation(EQuest_ResourceType ResourceType, FVector PlayerLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }
    
    FVector NearestLocation = FVector::ZeroVector;
    float NearestDistance = FLT_MAX;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetActorLabel().Contains(GetResourceTypeName(ResourceType)))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestLocation = Actor->GetActorLocation();
            }
        }
    }
    
    return NearestLocation;
}

void UQuest_ResourceGatheringManager::SpawnResourcePickups(EQuest_ResourceType ResourceType, FVector Location, int32 Count)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FString ResourceName = GetResourceTypeName(ResourceType);
    
    for (int32 i = 0; i < Count; i++)
    {
        // Random offset from base location
        FVector SpawnLocation = Location + FVector(
            FMath::RandRange(-1500.0f, 1500.0f),
            FMath::RandRange(-1500.0f, 1500.0f),
            FMath::RandRange(0.0f, 200.0f)
        );
        
        // Spawn a simple static mesh actor as resource pickup
        AStaticMeshActor* ResourceActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
        if (ResourceActor)
        {
            ResourceActor->SetActorLabel(FString::Printf(TEXT("%s_Pickup_%d"), *ResourceName, i));
            
            // Set a basic mesh component
            UStaticMeshComponent* MeshComp = ResourceActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                // Try to load a basic cube mesh as placeholder
                UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
                if (CubeMesh)
                {
                    MeshComp->SetStaticMesh(CubeMesh);
                    
                    // Scale based on resource type
                    FVector Scale = FVector(0.3f, 0.3f, 0.3f);
                    if (ResourceType == EQuest_ResourceType::Stone)
                    {
                        Scale = FVector(0.5f, 0.5f, 0.3f);
                    }
                    else if (ResourceType == EQuest_ResourceType::Wood)
                    {
                        Scale = FVector(0.2f, 0.2f, 1.0f);
                    }
                    
                    ResourceActor->SetActorScale3D(Scale);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d %s pickups near %s"), Count, *ResourceName, *Location.ToString());
}

void UQuest_ResourceGatheringManager::GenerateRandomGatheringMission()
{
    TArray<EQuest_ResourceType> ResourceTypes = {
        EQuest_ResourceType::Stone,
        EQuest_ResourceType::Wood,
        EQuest_ResourceType::Plant,
        EQuest_ResourceType::Water
    };
    
    EQuest_ResourceType RandomType = ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
    FVector RandomLocation = FVector(
        FMath::RandRange(-5000.0f, 5000.0f),
        FMath::RandRange(-5000.0f, 5000.0f),
        100.0f
    );
    
    switch (RandomType)
    {
        case EQuest_ResourceType::Stone:
            CreateStoneGatheringMission(RandomLocation);
            break;
        case EQuest_ResourceType::Wood:
            CreateWoodGatheringMission(RandomLocation);
            break;
        case EQuest_ResourceType::Plant:
            CreatePlantGatheringMission(RandomLocation);
            break;
        case EQuest_ResourceType::Water:
            CreateWaterGatheringMission(RandomLocation);
            break;
        default:
            break;
    }
}

void UQuest_ResourceGatheringManager::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.Status == EMissionStatus::InProgress)
        {
            Mission.ElapsedTime += DeltaTime;
            
            if (Mission.ElapsedTime >= Mission.TimeLimit)
            {
                Mission.Status = EMissionStatus::Failed;
                UE_LOG(LogTemp, Warning, TEXT("Mission failed due to timeout: %s"), *Mission.MissionName);
            }
        }
    }
}

FString UQuest_ResourceGatheringManager::GenerateMissionID()
{
    return FString::Printf(TEXT("GATHER_%d"), NextMissionID++);
}

FString UQuest_ResourceGatheringManager::GetResourceTypeName(EQuest_ResourceType ResourceType)
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone: return TEXT("Stone");
        case EQuest_ResourceType::Wood: return TEXT("Wood");
        case EQuest_ResourceType::Plant: return TEXT("Plant");
        case EQuest_ResourceType::Water: return TEXT("Water");
        case EQuest_ResourceType::Meat: return TEXT("Meat");
        case EQuest_ResourceType::Hide: return TEXT("Hide");
        case EQuest_ResourceType::Bone: return TEXT("Bone");
        default: return TEXT("Unknown");
    }
}