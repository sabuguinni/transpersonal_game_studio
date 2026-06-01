#include "Quest_ResourcePickupActor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

AQuest_ResourcePickupActor::AQuest_ResourcePickupActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(150.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Default values
    ResourceType = EQuest_ResourceType::Rock;
    ResourceAmount = 1;
    PickupRange = 150.0f;
    bAutoPickup = true;
    MeshScale = FVector(1.0f, 1.0f, 1.0f);

    // Set default tags
    Tags.Add(TEXT("Resource"));
    Tags.Add(TEXT("Pickup"));
}

void AQuest_ResourcePickupActor::BeginPlay()
{
    Super::BeginPlay();
    
    SetupMeshForResourceType();
    ConfigureCollision();
    
    // Bind overlap event
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickupActor::OnOverlapBegin);
    
    UE_LOG(LogTemp, Warning, TEXT("ResourcePickup: %s spawned at %s"), 
           *UEnum::GetValueAsString(ResourceType), 
           *GetActorLocation().ToString());
}

void AQuest_ResourcePickupActor::SetResourceType(EQuest_ResourceType NewResourceType)
{
    ResourceType = NewResourceType;
    SetupMeshForResourceType();
    
    // Update actor label and tags
    FString ResourceName = UEnum::GetValueAsString(ResourceType);
    SetActorLabel(FString::Printf(TEXT("Resource_%s"), *ResourceName));
    
    Tags.Empty();
    Tags.Add(TEXT("Resource"));
    Tags.Add(TEXT("Pickup"));
    Tags.Add(*ResourceName);
}

EQuest_ResourceType AQuest_ResourcePickupActor::GetResourceType() const
{
    return ResourceType;
}

int32 AQuest_ResourcePickupActor::GetResourceAmount() const
{
    return ResourceAmount;
}

bool AQuest_ResourcePickupActor::CanBePickedUp() const
{
    return ResourceAmount > 0 && IsValidLowLevel() && !IsActorBeingDestroyed();
}

void AQuest_ResourcePickupActor::PickupResource(AActor* PickupActor)
{
    if (!CanBePickedUp() || !PickupActor)
    {
        return;
    }
    
    // Find crafting system component on the pickup actor
    UQuest_CraftingSystem* CraftingSystem = PickupActor->FindComponentByClass<UQuest_CraftingSystem>();
    if (CraftingSystem)
    {
        CraftingSystem->AddResource(ResourceType, ResourceAmount);
        
        UE_LOG(LogTemp, Warning, TEXT("ResourcePickup: %s picked up %d %s"), 
               *PickupActor->GetName(), 
               ResourceAmount, 
               *UEnum::GetValueAsString(ResourceType));
        
        // Destroy this pickup
        Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ResourcePickup: %s has no CraftingSystem component"), *PickupActor->GetName());
    }
}

void AQuest_ResourcePickupActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoPickup || !OtherActor || OtherActor == this)
    {
        return;
    }
    
    // Check if the overlapping actor has a crafting system
    UQuest_CraftingSystem* CraftingSystem = OtherActor->FindComponentByClass<UQuest_CraftingSystem>();
    if (CraftingSystem)
    {
        PickupResource(OtherActor);
    }
}

void AQuest_ResourcePickupActor::SetupMeshForResourceType()
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Set scale based on resource type
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:
            MeshScale = FVector(0.8f, 0.8f, 0.6f);
            ResourceAmount = FMath::RandRange(1, 3);
            break;
            
        case EQuest_ResourceType::Stick:
            MeshScale = FVector(0.3f, 2.0f, 0.3f);
            ResourceAmount = FMath::RandRange(1, 2);
            break;
            
        case EQuest_ResourceType::Leaf:
            MeshScale = FVector(1.2f, 1.2f, 0.1f);
            ResourceAmount = FMath::RandRange(2, 5);
            break;
            
        case EQuest_ResourceType::Fiber:
            MeshScale = FVector(0.5f, 0.5f, 1.5f);
            ResourceAmount = FMath::RandRange(1, 3);
            break;
            
        case EQuest_ResourceType::Bone:
            MeshScale = FVector(0.4f, 1.8f, 0.4f);
            ResourceAmount = 1;
            break;
            
        case EQuest_ResourceType::Hide:
            MeshScale = FVector(1.5f, 1.5f, 0.2f);
            ResourceAmount = 1;
            break;
            
        case EQuest_ResourceType::Stone:
            MeshScale = FVector(0.6f, 0.6f, 0.8f);
            ResourceAmount = FMath::RandRange(1, 2);
            break;
            
        case EQuest_ResourceType::Wood:
            MeshScale = FVector(0.8f, 0.8f, 1.2f);
            ResourceAmount = FMath::RandRange(1, 2);
            break;
            
        default:
            MeshScale = FVector(1.0f, 1.0f, 1.0f);
            ResourceAmount = 1;
            break;
    }
    
    MeshComponent->SetWorldScale3D(MeshScale);
    
    // Try to load appropriate mesh (fallback to basic shapes)
    UStaticMesh* DefaultMesh = nullptr;
    
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:
        case EQuest_ResourceType::Stone:
            // Try to load a rock mesh, fallback to sphere
            DefaultMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
            break;
            
        case EQuest_ResourceType::Stick:
        case EQuest_ResourceType::Wood:
        case EQuest_ResourceType::Bone:
            // Try to load a cylinder for stick-like objects
            DefaultMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
            break;
            
        case EQuest_ResourceType::Leaf:
        case EQuest_ResourceType::Hide:
            // Try to load a plane for flat objects
            DefaultMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
            break;
            
        default:
            DefaultMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            break;
    }
    
    if (DefaultMesh)
    {
        MeshComponent->SetStaticMesh(DefaultMesh);
    }
}

void AQuest_ResourcePickupActor::ConfigureCollision()
{
    if (CollisionSphere)
    {
        CollisionSphere->SetSphereRadius(PickupRange);
    }
}