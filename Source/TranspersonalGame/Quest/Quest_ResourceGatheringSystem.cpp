#include "Quest_ResourceGatheringSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// AQuest_ResourceNode Implementation
AQuest_ResourceNode::AQuest_ResourceNode()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);

    // Default values
    ResourceType = EQuest_ResourceType::Stone;
    MaxResources = 10;
    CurrentResources = MaxResources;
    RegenerationTime = 60.0f;
    bCanRegenerate = true;

    // Set default mesh (cube)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMesh.Object);
    }
}

void AQuest_ResourceNode::BeginPlay()
{
    Super::BeginPlay();
    
    if (bCanRegenerate && RegenerationTime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(RegenerationTimer, this, &AQuest_ResourceNode::RegenerateResources, RegenerationTime, true);
    }
}

bool AQuest_ResourceNode::CanGatherResource() const
{
    return CurrentResources > 0;
}

int32 AQuest_ResourceNode::GatherResource(int32 AmountToGather)
{
    if (!CanGatherResource())
    {
        return 0;
    }

    int32 ActualGathered = FMath::Min(AmountToGather, CurrentResources);
    CurrentResources -= ActualGathered;

    UE_LOG(LogTemp, Log, TEXT("Gathered %d resources from node. Remaining: %d"), ActualGathered, CurrentResources);

    return ActualGathered;
}

void AQuest_ResourceNode::RegenerateResources()
{
    if (CurrentResources < MaxResources)
    {
        CurrentResources = FMath::Min(CurrentResources + 1, MaxResources);
        UE_LOG(LogTemp, Log, TEXT("Resource node regenerated. Current: %d/%d"), CurrentResources, MaxResources);
    }
}

// UQuest_ResourceGatheringSystem Implementation
UQuest_ResourceGatheringSystem::UQuest_ResourceGatheringSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;

    DetectionRadius = 1000.0f;
    GatheringTime = 3.0f;
    bIsGathering = false;
    CurrentGatheringNode = nullptr;
    GatheringProgress = 0.0f;
}

void UQuest_ResourceGatheringSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Scan for resource nodes in the world
    ScanForResourceNodes();
}

void UQuest_ResourceGatheringSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsGathering)
    {
        ProcessGathering(DeltaTime);
    }

    // Periodically scan for new resource nodes
    static float ScanTimer = 0.0f;
    ScanTimer += DeltaTime;
    if (ScanTimer >= 5.0f)
    {
        ScanForResourceNodes();
        ScanTimer = 0.0f;
    }
}

void UQuest_ResourceGatheringSystem::StartResourceGatheringQuest(EQuest_ResourceType ResourceType, int32 RequiredAmount, FVector TargetLocation)
{
    // Check if quest already exists
    for (FQuest_ResourceGatheringData& Quest : ActiveGatheringQuests)
    {
        if (Quest.ResourceType == ResourceType && !Quest.bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Resource gathering quest for %d already active"), (int32)ResourceType);
            return;
        }
    }

    // Create new quest
    FQuest_ResourceGatheringData NewQuest;
    NewQuest.ResourceType = ResourceType;
    NewQuest.RequiredAmount = RequiredAmount;
    NewQuest.CurrentAmount = 0;
    NewQuest.ResourceLocation = TargetLocation;
    NewQuest.GatherRadius = 500.0f;
    NewQuest.bIsCompleted = false;

    ActiveGatheringQuests.Add(NewQuest);

    UE_LOG(LogTemp, Log, TEXT("Started resource gathering quest: Type %d, Amount %d"), (int32)ResourceType, RequiredAmount);
}

bool UQuest_ResourceGatheringSystem::CompleteResourceGatheringQuest(EQuest_ResourceType ResourceType)
{
    for (FQuest_ResourceGatheringData& Quest : ActiveGatheringQuests)
    {
        if (Quest.ResourceType == ResourceType && Quest.CurrentAmount >= Quest.RequiredAmount)
        {
            Quest.bIsCompleted = true;
            OnQuestCompleted(ResourceType);
            UE_LOG(LogTemp, Log, TEXT("Completed resource gathering quest for type %d"), (int32)ResourceType);
            return true;
        }
    }
    return false;
}

void UQuest_ResourceGatheringSystem::UpdateResourceProgress(EQuest_ResourceType ResourceType, int32 AmountGathered)
{
    for (FQuest_ResourceGatheringData& Quest : ActiveGatheringQuests)
    {
        if (Quest.ResourceType == ResourceType && !Quest.bIsCompleted)
        {
            Quest.CurrentAmount += AmountGathered;
            OnResourceGathered(ResourceType, AmountGathered);
            
            UE_LOG(LogTemp, Log, TEXT("Updated resource progress: %d/%d"), Quest.CurrentAmount, Quest.RequiredAmount);
            
            if (Quest.CurrentAmount >= Quest.RequiredAmount)
            {
                CompleteResourceGatheringQuest(ResourceType);
            }
            break;
        }
    }
}

TArray<AQuest_ResourceNode*> UQuest_ResourceGatheringSystem::FindNearbyResourceNodes(EQuest_ResourceType ResourceType, float SearchRadius)
{
    TArray<AQuest_ResourceNode*> NearbyNodes;
    
    if (!GetOwner())
    {
        return NearbyNodes;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AQuest_ResourceNode* Node : DiscoveredResourceNodes)
    {
        if (Node && Node->ResourceType == ResourceType)
        {
            float Distance = FVector::Dist(OwnerLocation, Node->GetActorLocation());
            if (Distance <= SearchRadius && Node->CanGatherResource())
            {
                NearbyNodes.Add(Node);
            }
        }
    }

    return NearbyNodes;
}

bool UQuest_ResourceGatheringSystem::StartGathering(AQuest_ResourceNode* TargetNode)
{
    if (!TargetNode || bIsGathering || !TargetNode->CanGatherResource())
    {
        return false;
    }

    CurrentGatheringNode = TargetNode;
    bIsGathering = true;
    GatheringProgress = 0.0f;

    OnGatheringStarted(TargetNode);
    UE_LOG(LogTemp, Log, TEXT("Started gathering from resource node"));

    return true;
}

void UQuest_ResourceGatheringSystem::StopGathering()
{
    bIsGathering = false;
    CurrentGatheringNode = nullptr;
    GatheringProgress = 0.0f;
    GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
}

FQuest_ResourceGatheringData* UQuest_ResourceGatheringSystem::GetActiveQuest(EQuest_ResourceType ResourceType)
{
    for (FQuest_ResourceGatheringData& Quest : ActiveGatheringQuests)
    {
        if (Quest.ResourceType == ResourceType && !Quest.bIsCompleted)
        {
            return &Quest;
        }
    }
    return nullptr;
}

int32 UQuest_ResourceGatheringSystem::GetTotalResourcesGathered(EQuest_ResourceType ResourceType) const
{
    int32 Total = 0;
    for (const FQuest_ResourceGatheringData& Quest : ActiveGatheringQuests)
    {
        if (Quest.ResourceType == ResourceType)
        {
            Total += Quest.CurrentAmount;
        }
    }
    return Total;
}

float UQuest_ResourceGatheringSystem::GetGatheringProgress() const
{
    return GatheringProgress;
}

void UQuest_ResourceGatheringSystem::ScanForResourceNodes()
{
    if (!GetWorld())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest_ResourceNode::StaticClass(), FoundActors);

    DiscoveredResourceNodes.Empty();
    for (AActor* Actor : FoundActors)
    {
        if (AQuest_ResourceNode* ResourceNode = Cast<AQuest_ResourceNode>(Actor))
        {
            DiscoveredResourceNodes.Add(ResourceNode);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Discovered %d resource nodes"), DiscoveredResourceNodes.Num());
}

void UQuest_ResourceGatheringSystem::ProcessGathering(float DeltaTime)
{
    if (!CurrentGatheringNode || !bIsGathering)
    {
        StopGathering();
        return;
    }

    GatheringProgress += DeltaTime / GatheringTime;

    if (GatheringProgress >= 1.0f)
    {
        // Complete gathering
        int32 GatheredAmount = CurrentGatheringNode->GatherResource(1);
        if (GatheredAmount > 0)
        {
            UpdateResourceProgress(CurrentGatheringNode->ResourceType, GatheredAmount);
        }

        StopGathering();
    }
}