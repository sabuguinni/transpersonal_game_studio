#include "Quest_ResourceGatheringSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

AQuest_ResourceNode::AQuest_ResourceNode()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mesh component
    ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
    ResourceMesh->SetupAttachment(RootComponent);
    ResourceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ResourceMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    bIsHarvestable = true;
    HarvestTime = 3.0f;
    bIsBeingHarvested = false;

    // Set default resource data
    ResourceData.ResourceType = EQuest_ResourceType::Stone;
    ResourceData.Quantity = 1;
    ResourceData.ResourceName = TEXT("Stone");
    ResourceData.RespawnTime = 30.0f;
}

void AQuest_ResourceNode::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourceNode::OnInteractionSphereBeginOverlap);
    }

    SetupResourceMesh();
}

void AQuest_ResourceNode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle harvest progress visualization here if needed
    if (bIsBeingHarvested && ResourceMesh)
    {
        // Could add visual feedback like scaling or material changes
        float ScaleFactor = FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.1f + 0.9f;
        ResourceMesh->SetWorldScale3D(FVector(ScaleFactor));
    }
}

bool AQuest_ResourceNode::CanHarvest() const
{
    return bIsHarvestable && !bIsBeingHarvested;
}

void AQuest_ResourceNode::StartHarvest(AActor* Harvester)
{
    if (!CanHarvest() || !Harvester)
    {
        return;
    }

    bIsBeingHarvested = true;

    // Start harvest timer
    FTimerDelegate HarvestDelegate;
    HarvestDelegate.BindUFunction(this, FName("CompleteHarvest"));
    GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, HarvestDelegate, HarvestTime, false);

    UE_LOG(LogTemp, Warning, TEXT("Starting harvest of %s"), *ResourceData.ResourceName);
}

void AQuest_ResourceNode::CompleteHarvest()
{
    if (!bIsBeingHarvested)
    {
        return;
    }

    bIsBeingHarvested = false;
    bIsHarvestable = false;

    // Hide the mesh
    if (ResourceMesh)
    {
        ResourceMesh->SetVisibility(false);
        ResourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Trigger harvest event
    OnResourceHarvested(nullptr, ResourceData);

    // Start respawn timer
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindUFunction(this, FName("RespawnResource"));
    GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, ResourceData.RespawnTime, false);

    UE_LOG(LogTemp, Warning, TEXT("Harvested %s x%d"), *ResourceData.ResourceName, ResourceData.Quantity);
}

void AQuest_ResourceNode::SetResourceType(EQuest_ResourceType NewType, int32 NewQuantity)
{
    ResourceData.ResourceType = NewType;
    ResourceData.Quantity = NewQuantity;

    switch (NewType)
    {
        case EQuest_ResourceType::Stone:
            ResourceData.ResourceName = TEXT("Stone");
            break;
        case EQuest_ResourceType::Wood:
            ResourceData.ResourceName = TEXT("Wood");
            break;
        case EQuest_ResourceType::Plant:
            ResourceData.ResourceName = TEXT("Plant Fiber");
            break;
        case EQuest_ResourceType::Water:
            ResourceData.ResourceName = TEXT("Water");
            break;
        case EQuest_ResourceType::Bone:
            ResourceData.ResourceName = TEXT("Bone");
            break;
    }

    SetupResourceMesh();
}

void AQuest_ResourceNode::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !CanHarvest())
    {
        return;
    }

    // Check if overlapping actor is a character
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player can harvest %s"), *ResourceData.ResourceName);
        // Could trigger UI prompt here
    }
}

void AQuest_ResourceNode::RespawnResource()
{
    bIsHarvestable = true;
    bIsBeingHarvested = false;

    // Show the mesh again
    if (ResourceMesh)
    {
        ResourceMesh->SetVisibility(true);
        ResourceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ResourceMesh->SetWorldScale3D(FVector(1.0f));
    }

    OnResourceRespawned();
    UE_LOG(LogTemp, Warning, TEXT("Resource %s respawned"), *ResourceData.ResourceName);
}

void AQuest_ResourceNode::SetupResourceMesh()
{
    if (!ResourceMesh)
    {
        return;
    }

    // Set different meshes based on resource type
    // For now, use basic shapes - in production these would be proper assets
    UStaticMesh* MeshToUse = nullptr;
    
    switch (ResourceData.ResourceType)
    {
        case EQuest_ResourceType::Stone:
            // Use cube for stone
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
        case EQuest_ResourceType::Wood:
            // Use cylinder for wood
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
        case EQuest_ResourceType::Plant:
            // Use sphere for plants
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
            break;
        default:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
    }

    if (MeshToUse)
    {
        ResourceMesh->SetStaticMesh(MeshToUse);
    }
}

// Resource Gathering System Implementation
UQuest_ResourceGatheringSystem::UQuest_ResourceGatheringSystem()
{
    bQuestCompleted = false;
    QuestTitle = TEXT("Gather Resources");
    QuestDescription = TEXT("Collect the required resources to complete this quest.");
}

void UQuest_ResourceGatheringSystem::InitializeGatheringQuest(const TArray<FQuest_ResourceData>& Resources)
{
    RequiredResources = Resources;
    CollectedResources.Empty();
    bQuestCompleted = false;

    // Initialize collected resources map
    for (const FQuest_ResourceData& Resource : RequiredResources)
    {
        CollectedResources.Add(Resource.ResourceType, 0);
    }

    UE_LOG(LogTemp, Warning, TEXT("Initialized gathering quest with %d resource types"), RequiredResources.Num());
}

bool UQuest_ResourceGatheringSystem::AddResource(EQuest_ResourceType ResourceType, int32 Quantity)
{
    if (!CollectedResources.Contains(ResourceType))
    {
        return false;
    }

    int32 CurrentAmount = CollectedResources[ResourceType];
    int32 NewAmount = CurrentAmount + Quantity;

    // Find required amount
    int32 RequiredAmount = 0;
    for (const FQuest_ResourceData& Resource : RequiredResources)
    {
        if (Resource.ResourceType == ResourceType)
        {
            RequiredAmount = Resource.Quantity;
            break;
        }
    }

    // Cap at required amount
    NewAmount = FMath::Min(NewAmount, RequiredAmount);
    CollectedResources[ResourceType] = NewAmount;

    OnResourceAdded(ResourceType, NewAmount, RequiredAmount);
    CheckQuestCompletion();

    UE_LOG(LogTemp, Warning, TEXT("Added resource: %d, New total: %d/%d"), (int32)ResourceType, NewAmount, RequiredAmount);
    return true;
}

bool UQuest_ResourceGatheringSystem::IsQuestComplete() const
{
    for (const FQuest_ResourceData& Resource : RequiredResources)
    {
        int32 CollectedAmount = CollectedResources.FindRef(Resource.ResourceType);
        if (CollectedAmount < Resource.Quantity)
        {
            return false;
        }
    }
    return true;
}

float UQuest_ResourceGatheringSystem::GetQuestProgress() const
{
    if (RequiredResources.Num() == 0)
    {
        return 1.0f;
    }

    float TotalProgress = 0.0f;
    for (const FQuest_ResourceData& Resource : RequiredResources)
    {
        int32 CollectedAmount = CollectedResources.FindRef(Resource.ResourceType);
        float ResourceProgress = FMath::Clamp((float)CollectedAmount / (float)Resource.Quantity, 0.0f, 1.0f);
        TotalProgress += ResourceProgress;
    }

    return TotalProgress / RequiredResources.Num();
}

FString UQuest_ResourceGatheringSystem::GetProgressText() const
{
    FString ProgressText = TEXT("Progress:\n");
    
    for (const FQuest_ResourceData& Resource : RequiredResources)
    {
        int32 CollectedAmount = CollectedResources.FindRef(Resource.ResourceType);
        ProgressText += FString::Printf(TEXT("%s: %d/%d\n"), *Resource.ResourceName, CollectedAmount, Resource.Quantity);
    }

    return ProgressText;
}

void UQuest_ResourceGatheringSystem::SpawnResourceNodes(UWorld* World, const TArray<FVector>& Locations)
{
    if (!World || RequiredResources.Num() == 0)
    {
        return;
    }

    for (int32 i = 0; i < Locations.Num() && i < RequiredResources.Num() * 3; ++i)
    {
        FVector SpawnLocation = Locations[i];
        EQuest_ResourceType ResourceType = RequiredResources[i % RequiredResources.Num()].ResourceType;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AQuest_ResourceNode* ResourceNode = World->SpawnActor<AQuest_ResourceNode>(AQuest_ResourceNode::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        
        if (ResourceNode)
        {
            ResourceNode->SetResourceType(ResourceType, 1);
            ResourceNode->SetActorLabel(FString::Printf(TEXT("ResourceNode_%s_%d"), *RequiredResources[i % RequiredResources.Num()].ResourceName, i));
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned resource node: %s at %s"), *ResourceNode->GetActorLabel(), *SpawnLocation.ToString());
        }
    }
}

void UQuest_ResourceGatheringSystem::CheckQuestCompletion()
{
    if (!bQuestCompleted && IsQuestComplete())
    {
        bQuestCompleted = true;
        OnQuestCompleted();
        UE_LOG(LogTemp, Warning, TEXT("Quest completed!"));
    }
}