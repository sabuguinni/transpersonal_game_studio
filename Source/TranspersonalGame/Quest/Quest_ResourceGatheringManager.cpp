#include "Quest_ResourceGatheringManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

AQuest_ResourceGatheringManager::AQuest_ResourceGatheringManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bQuestActive = false;
    DetectionRadius = 500.0f;
    MaxActiveNodes = 20;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AQuest_ResourceGatheringManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeResourceNodes();
    SpawnResourceNodes();
}

void AQuest_ResourceGatheringManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bQuestActive)
    {
        CheckPlayerProximity();
        
        // Handle resource node respawning
        for (int32 i = 0; i < ResourceNodes.Num(); i++)
        {
            FQuest_ResourceNode& Node = ResourceNodes[i];
            if (!Node.bIsActive && Node.RespawnTime > 0.0f)
            {
                Node.RespawnTime -= DeltaTime;
                if (Node.RespawnTime <= 0.0f)
                {
                    RespawnResourceNode(i);
                }
            }
        }
    }
}

void AQuest_ResourceGatheringManager::StartGatheringQuest(EQuest_ResourceType ResourceType, int32 Amount)
{
    bQuestActive = true;
    
    // Clear existing objectives
    ActiveObjectives.Empty();
    
    // Create new objective
    FQuest_GatheringObjective NewObjective;
    NewObjective.TargetResource = ResourceType;
    NewObjective.RequiredAmount = Amount;
    NewObjective.CurrentAmount = 0;
    
    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone:
            NewObjective.ObjectiveText = FString::Printf(TEXT("Gather %d stones"), Amount);
            break;
        case EQuest_ResourceType::Wood:
            NewObjective.ObjectiveText = FString::Printf(TEXT("Collect %d pieces of wood"), Amount);
            break;
        case EQuest_ResourceType::Plant:
            NewObjective.ObjectiveText = FString::Printf(TEXT("Harvest %d plants"), Amount);
            break;
        case EQuest_ResourceType::Water:
            NewObjective.ObjectiveText = FString::Printf(TEXT("Collect %d water containers"), Amount);
            break;
        case EQuest_ResourceType::Bone:
            NewObjective.ObjectiveText = FString::Printf(TEXT("Gather %d bones"), Amount);
            break;
        case EQuest_ResourceType::Hide:
            NewObjective.ObjectiveText = FString::Printf(TEXT("Collect %d hides"), Amount);
            break;
    }
    
    ActiveObjectives.Add(NewObjective);
    
    UE_LOG(LogTemp, Warning, TEXT("Resource Gathering Quest Started: %s"), *NewObjective.ObjectiveText);
}

void AQuest_ResourceGatheringManager::CompleteGatheringQuest()
{
    bQuestActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Resource Gathering Quest Completed!"));
    
    // Reward logic would go here
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Resource Gathering Quest Complete!"));
    }
}

bool AQuest_ResourceGatheringManager::GatherResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (!bQuestActive)
    {
        return false;
    }
    
    bool bFoundObjective = false;
    for (FQuest_GatheringObjective& Objective : ActiveObjectives)
    {
        if (Objective.TargetResource == ResourceType)
        {
            Objective.CurrentAmount = FMath::Min(Objective.CurrentAmount + Amount, Objective.RequiredAmount);
            bFoundObjective = true;
            
            UE_LOG(LogTemp, Warning, TEXT("Resource gathered: %d/%d"), Objective.CurrentAmount, Objective.RequiredAmount);
            
            if (Objective.CurrentAmount >= Objective.RequiredAmount)
            {
                CompleteGatheringQuest();
            }
            break;
        }
    }
    
    return bFoundObjective;
}

void AQuest_ResourceGatheringManager::SpawnResourceNodes()
{
    // Clear existing nodes if any
    ResourceNodes.Empty();
    
    // Generate random resource nodes
    for (int32 i = 0; i < MaxActiveNodes; i++)
    {
        FQuest_ResourceNode NewNode;
        NewNode.Location = GetRandomResourceLocation();
        NewNode.ResourceType = static_cast<EQuest_ResourceType>(FMath::RandRange(0, 5));
        NewNode.Quantity = FMath::RandRange(1, 3);
        NewNode.bIsActive = true;
        NewNode.RespawnTime = FMath::RandRange(30.0f, 120.0f);
        
        ResourceNodes.Add(NewNode);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d resource nodes"), ResourceNodes.Num());
}

void AQuest_ResourceGatheringManager::UpdateObjectiveProgress(EQuest_ResourceType ResourceType, int32 Amount)
{
    GatherResource(ResourceType, Amount);
}

TArray<FVector> AQuest_ResourceGatheringManager::GetNearbyResourceLocations(EQuest_ResourceType ResourceType, FVector PlayerLocation)
{
    TArray<FVector> NearbyLocations;
    
    for (const FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (Node.ResourceType == ResourceType && Node.bIsActive)
        {
            float Distance = FVector::Dist(PlayerLocation, Node.Location);
            if (Distance <= DetectionRadius)
            {
                NearbyLocations.Add(Node.Location);
            }
        }
    }
    
    return NearbyLocations;
}

bool AQuest_ResourceGatheringManager::IsQuestComplete()
{
    if (!bQuestActive || ActiveObjectives.Num() == 0)
    {
        return false;
    }
    
    for (const FQuest_GatheringObjective& Objective : ActiveObjectives)
    {
        if (Objective.CurrentAmount < Objective.RequiredAmount)
        {
            return false;
        }
    }
    
    return true;
}

FString AQuest_ResourceGatheringManager::GetQuestProgressText()
{
    if (!bQuestActive || ActiveObjectives.Num() == 0)
    {
        return TEXT("No active gathering quest");
    }
    
    FString ProgressText;
    for (const FQuest_GatheringObjective& Objective : ActiveObjectives)
    {
        ProgressText += FString::Printf(TEXT("%s: %d/%d\n"), 
            *Objective.ObjectiveText, 
            Objective.CurrentAmount, 
            Objective.RequiredAmount);
    }
    
    return ProgressText;
}

void AQuest_ResourceGatheringManager::RespawnResourceNode(int32 NodeIndex)
{
    if (ResourceNodes.IsValidIndex(NodeIndex))
    {
        FQuest_ResourceNode& Node = ResourceNodes[NodeIndex];
        Node.bIsActive = true;
        Node.RespawnTime = FMath::RandRange(30.0f, 120.0f);
        Node.Quantity = FMath::RandRange(1, 3);
        
        UE_LOG(LogTemp, Log, TEXT("Resource node respawned at location: %s"), *Node.Location.ToString());
    }
}

void AQuest_ResourceGatheringManager::InitializeResourceNodes()
{
    // Initialize with default settings
    ResourceNodes.Empty();
}

void AQuest_ResourceGatheringManager::CheckPlayerProximity()
{
    // Get player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Check proximity to resource nodes
    for (FQuest_ResourceNode& Node : ResourceNodes)
    {
        if (Node.bIsActive)
        {
            float Distance = FVector::Dist(PlayerLocation, Node.Location);
            if (Distance <= 200.0f) // Close proximity for auto-gathering
            {
                // Auto-gather resource when player is close
                GatherResource(Node.ResourceType, Node.Quantity);
                Node.bIsActive = false;
                Node.RespawnTime = FMath::RandRange(60.0f, 180.0f);
            }
        }
    }
}

FVector AQuest_ResourceGatheringManager::GetRandomResourceLocation()
{
    // Generate random location within a reasonable range
    float X = FMath::RandRange(-2000.0f, 2000.0f);
    float Y = FMath::RandRange(-2000.0f, 2000.0f);
    float Z = FMath::RandRange(0.0f, 200.0f);
    
    return FVector(X, Y, Z);
}

bool AQuest_ResourceGatheringManager::IsLocationValid(FVector Location)
{
    // Basic validation - could be expanded with terrain checks
    return Location.Z >= 0.0f && Location.Z <= 1000.0f;
}