#include "Quest_ResourcePickup.h"
#include "TranspersonalCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create pickup collision sphere
    PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
    PickupCollision->SetupAttachment(RootComponent);
    PickupCollision->SetSphereRadius(150.0f);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap event
    PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnPickupOverlapBegin);

    // Default values
    ResourceType = EResourceType::Stone;
    ResourceAmount = 1;
    RespawnTime = 30.0f;
    bIsPickedUp = false;
    RespawnTimer = 0.0f;

    // Set default mesh based on resource type
    UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
    if (MeshComp)
    {
        // Try to load basic cube mesh as fallback
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMeshAsset.Succeeded())
        {
            MeshComp->SetStaticMesh(CubeMeshAsset.Object);
            MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
        }
    }
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    // Set mesh color based on resource type
    UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
    if (MeshComp)
    {
        UMaterialInterface* Material = nullptr;
        
        switch (ResourceType)
        {
            case EResourceType::Stone:
                // Gray for stone
                break;
            case EResourceType::Wood:
                // Brown for wood
                break;
            case EResourceType::Fiber:
                // Green for fiber
                break;
            default:
                break;
        }
    }
}

void AQuest_ResourcePickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle respawn timer
    if (bIsPickedUp && RespawnTimer > 0.0f)
    {
        RespawnTimer -= DeltaTime;
        if (RespawnTimer <= 0.0f)
        {
            RespawnResource();
        }
    }
}

void AQuest_ResourcePickup::OnPickupOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsPickedUp)
    {
        return;
    }

    ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(OtherActor);
    if (Character)
    {
        PickupResource(Character);
    }
}

void AQuest_ResourcePickup::PickupResource(ATranspersonalCharacter* Character)
{
    if (!Character || bIsPickedUp)
    {
        return;
    }

    // Add resource to character's inventory (placeholder - inventory system needed)
    UE_LOG(LogTemp, Warning, TEXT("Player picked up %d %s"), ResourceAmount, 
           ResourceType == EResourceType::Stone ? TEXT("Stone") :
           ResourceType == EResourceType::Wood ? TEXT("Wood") :
           ResourceType == EResourceType::Fiber ? TEXT("Fiber") : TEXT("Unknown"));

    // Hide the pickup
    bIsPickedUp = true;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // Start respawn timer
    RespawnTimer = RespawnTime;

    // Show pickup message to player
    if (GEngine)
    {
        FString ResourceName = ResourceType == EResourceType::Stone ? TEXT("Stone") :
                              ResourceType == EResourceType::Wood ? TEXT("Wood") :
                              ResourceType == EResourceType::Fiber ? TEXT("Fiber") : TEXT("Resource");
        
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("Picked up %d %s"), ResourceAmount, *ResourceName));
    }
}

void AQuest_ResourcePickup::RespawnResource()
{
    bIsPickedUp = false;
    RespawnTimer = 0.0f;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    UE_LOG(LogTemp, Log, TEXT("Resource respawned at %s"), *GetActorLocation().ToString());
}