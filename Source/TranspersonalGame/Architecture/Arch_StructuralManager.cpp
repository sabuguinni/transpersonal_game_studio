#include "Arch_StructuralManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AArch_StructuralManager::AArch_StructuralManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    StructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(StructureMesh);
    InteractionVolume->SetBoxExtent(FVector(300.0f, 300.0f, 500.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Bind overlap events
    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_StructuralManager::OnInteractionVolumeBeginOverlap);

    // Initialize default structure config
    StructureConfig.StructureType = EArch_StructureType::StonePillar;
    StructureConfig.Dimensions = FVector(200.0f, 200.0f, 400.0f);
    StructureConfig.WeatheringLevel = 0.5f;
    StructureConfig.bHasMossGrowth = true;
    StructureConfig.bHasCarvings = false;
    StructureConfig.StructuralIntegrity = 1.0f;

    // Try to load default materials
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (StoneMat.Succeeded())
    {
        StoneMaterial = StoneMat.Object;
        WeatheredStoneMaterial = StoneMat.Object;
        MossyStoneMaterial = StoneMat.Object;
    }

    // Set default mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded() && StructureMesh)
    {
        StructureMesh->SetStaticMesh(CubeMesh.Object);
    }
}

void AArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structure with current config
    InitializeStructure(StructureConfig.StructureType, GetActorLocation(), GetActorRotation());
    
    // Apply initial weathering and moss
    ApplyWeathering(StructureConfig.WeatheringLevel);
    SetMossGrowth(StructureConfig.bHasMossGrowth);
    AddCarvings(StructureConfig.bHasCarvings);
}

void AArch_StructuralManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gradual weathering over time (very slow)
    if (StructureConfig.WeatheringLevel < 1.0f)
    {
        float WeatheringRate = 0.001f; // Very slow weathering
        StructureConfig.WeatheringLevel = FMath::Clamp(StructureConfig.WeatheringLevel + (WeatheringRate * DeltaTime), 0.0f, 1.0f);
        
        // Update material every 10 seconds to avoid constant updates
        static float MaterialUpdateTimer = 0.0f;
        MaterialUpdateTimer += DeltaTime;
        if (MaterialUpdateTimer >= 10.0f)
        {
            UpdateStructureMaterial();
            MaterialUpdateTimer = 0.0f;
        }
    }
}

void AArch_StructuralManager::InitializeStructure(EArch_StructureType Type, FVector Location, FRotator Rotation)
{
    StructureConfig.StructureType = Type;
    
    // Set location and rotation
    SetActorLocation(Location);
    SetActorRotation(Rotation);
    
    // Update mesh based on structure type
    UStaticMesh* NewMesh = GetMeshForStructureType(Type);
    if (NewMesh && StructureMesh)
    {
        StructureMesh->SetStaticMesh(NewMesh);
    }
    
    // Set dimensions based on structure type
    SetStructureDimensions();
    
    // Update material
    UpdateStructureMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture: Initialized %s structure at location %s"), 
           *UEnum::GetValueAsString(Type), *Location.ToString());
}

void AArch_StructuralManager::ApplyWeathering(float WeatheringAmount)
{
    StructureConfig.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    
    // Reduce structural integrity based on weathering
    StructureConfig.StructuralIntegrity = FMath::Clamp(1.0f - (StructureConfig.WeatheringLevel * 0.3f), 0.1f, 1.0f);
    
    UpdateStructureMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture: Applied weathering level %.2f, integrity now %.2f"), 
           WeatheringAmount, StructureConfig.StructuralIntegrity);
}

void AArch_StructuralManager::SetMossGrowth(bool bEnableMoss)
{
    StructureConfig.bHasMossGrowth = bEnableMoss;
    UpdateStructureMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture: Moss growth %s"), bEnableMoss ? TEXT("enabled") : TEXT("disabled"));
}

void AArch_StructuralManager::AddCarvings(bool bEnableCarvings)
{
    StructureConfig.bHasCarvings = bEnableCarvings;
    UpdateStructureMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture: Carvings %s"), bEnableCarvings ? TEXT("enabled") : TEXT("disabled"));
}

void AArch_StructuralManager::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        // Check if it's a player character
        if (OtherActor->IsA<APawn>())
        {
            OnStructureInteraction(OtherActor);
            UE_LOG(LogTemp, Log, TEXT("Architecture: Player interacted with %s structure"), 
                   *UEnum::GetValueAsString(StructureConfig.StructureType));
        }
    }
}

void AArch_StructuralManager::UpdateStructureMaterial()
{
    if (!StructureMesh)
        return;

    UMaterialInterface* MaterialToUse = StoneMaterial;

    // Choose material based on weathering and moss
    if (StructureConfig.bHasMossGrowth && StructureConfig.WeatheringLevel > 0.3f)
    {
        MaterialToUse = MossyStoneMaterial ? MossyStoneMaterial : StoneMaterial;
    }
    else if (StructureConfig.WeatheringLevel > 0.6f)
    {
        MaterialToUse = WeatheredStoneMaterial ? WeatheredStoneMaterial : StoneMaterial;
    }

    if (MaterialToUse)
    {
        StructureMesh->SetMaterial(0, MaterialToUse);
    }
}

void AArch_StructuralManager::SetStructureDimensions()
{
    if (!StructureMesh)
        return;

    FVector Scale = FVector(1.0f);

    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::StonePillar:
            Scale = FVector(0.5f, 0.5f, 2.0f); // Tall and narrow
            StructureConfig.Dimensions = FVector(100.0f, 100.0f, 400.0f);
            break;
        case EArch_StructureType::StoneWall:
            Scale = FVector(4.0f, 0.5f, 1.5f); // Long and wide
            StructureConfig.Dimensions = FVector(800.0f, 100.0f, 300.0f);
            break;
        case EArch_StructureType::StoneArchway:
            Scale = FVector(1.5f, 0.5f, 2.5f); // Archway proportions
            StructureConfig.Dimensions = FVector(300.0f, 100.0f, 500.0f);
            break;
        case EArch_StructureType::CaveEntrance:
            Scale = FVector(2.0f, 1.0f, 1.5f); // Cave opening
            StructureConfig.Dimensions = FVector(400.0f, 200.0f, 300.0f);
            break;
        case EArch_StructureType::RockFormation:
            Scale = FVector(1.5f, 1.5f, 1.0f); // Natural rock cluster
            StructureConfig.Dimensions = FVector(300.0f, 300.0f, 200.0f);
            break;
        case EArch_StructureType::AncientRuin:
            Scale = FVector(2.0f, 2.0f, 1.0f); // Ruined structure
            StructureConfig.Dimensions = FVector(400.0f, 400.0f, 200.0f);
            break;
    }

    StructureMesh->SetWorldScale3D(Scale);
}

UStaticMesh* AArch_StructuralManager::GetMeshForStructureType(EArch_StructureType Type)
{
    // For now, return basic cube mesh - in production this would load specific meshes
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    return CubeMesh;
}