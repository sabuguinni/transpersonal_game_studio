#include "ArchitecturalStructureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AArchitecturalStructureManager::AArchitecturalStructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize structure data
    StructureData = FArch_StructureData();

    // Set default values
    SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
    
    // Initialize structure meshes map
    InitializeStructureMeshes();
}

void AArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionVolume)
    {
        InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArchitecturalStructureManager::OnInteractionVolumeBeginOverlap);
    }

    // Apply initial structure setup
    UpdateStructureMesh();
    ApplyMaterialBasedOnWeathering();
    SetupInteractionVolume();
}

void AArchitecturalStructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gradual weathering over time (very slow)
    if (StructureData.WeatheringLevel < 1.0f)
    {
        float WeatheringRate = 0.00001f; // Very slow weathering
        StructureData.WeatheringLevel = FMath::Clamp(StructureData.WeatheringLevel + (WeatheringRate * DeltaTime), 0.0f, 1.0f);
        
        // Update materials every 10 seconds
        static float MaterialUpdateTimer = 0.0f;
        MaterialUpdateTimer += DeltaTime;
        if (MaterialUpdateTimer >= 10.0f)
        {
            ApplyMaterialBasedOnWeathering();
            MaterialUpdateTimer = 0.0f;
        }
    }
}

void AArchitecturalStructureManager::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;
    UpdateStructureMesh();
    SetupInteractionVolume();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Structure type changed to %d"), (int32)NewType);
}

void AArchitecturalStructureManager::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(StructureData.WeatheringLevel + WeatheringAmount, 0.0f, 1.0f);
    ApplyMaterialBasedOnWeathering();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Weathering applied, level now: %f"), StructureData.WeatheringLevel);
}

void AArchitecturalStructureManager::SetMossGrowth(bool bEnableMoss)
{
    StructureData.bHasMossGrowth = bEnableMoss;
    ApplyMaterialBasedOnWeathering();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Moss growth %s"), bEnableMoss ? TEXT("enabled") : TEXT("disabled"));
}

void AArchitecturalStructureManager::UpdateStructureMesh()
{
    if (!StructureMesh)
    {
        return;
    }

    // Get mesh for current structure type
    UStaticMesh* MeshToUse = nullptr;
    if (StructureMeshes.Contains(StructureData.StructureType))
    {
        MeshToUse = StructureMeshes[StructureData.StructureType];
    }

    if (MeshToUse)
    {
        StructureMesh->SetStaticMesh(MeshToUse);
    }
    else
    {
        // Use default cube mesh if no specific mesh is available
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            StructureMesh->SetStaticMesh(CubeMesh);
        }
    }

    // Apply scale based on structure dimensions
    FVector Scale = StructureData.Dimensions / 100.0f; // Convert to reasonable scale
    StructureMesh->SetRelativeScale3D(Scale);
}

void AArchitecturalStructureManager::RegenerateStructure()
{
    UpdateStructureMesh();
    ApplyMaterialBasedOnWeathering();
    SetupInteractionVolume();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Structure regenerated"));
}

void AArchitecturalStructureManager::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!StructureData.bIsInteractable)
    {
        return;
    }

    // Check if the overlapping actor is a player character
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        OnPlayerInteraction();
        UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Player interaction detected"));
    }
}

void AArchitecturalStructureManager::InitializeStructureMeshes()
{
    // Initialize with default meshes - these would be replaced with actual assets
    UStaticMesh* DefaultMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    
    if (DefaultMesh)
    {
        StructureMeshes.Add(EArch_StructureType::Archway, DefaultMesh);
        StructureMeshes.Add(EArch_StructureType::Pillar, DefaultMesh);
        StructureMeshes.Add(EArch_StructureType::Wall, DefaultMesh);
        StructureMeshes.Add(EArch_StructureType::Foundation, DefaultMesh);
        StructureMeshes.Add(EArch_StructureType::Ruin, DefaultMesh);
        StructureMeshes.Add(EArch_StructureType::Chamber, DefaultMesh);
    }
}

void AArchitecturalStructureManager::ApplyMaterialBasedOnWeathering()
{
    if (!StructureMesh)
    {
        return;
    }

    // Apply weathered material based on weathering level and moss growth
    UMaterialInterface* MaterialToApply = nullptr;

    if (StructureData.bHasMossGrowth && StructureData.WeatheringLevel > 0.3f)
    {
        MaterialToApply = MossyMaterial;
    }
    else
    {
        MaterialToApply = WeatheredStoneMaterial;
    }

    if (MaterialToApply)
    {
        StructureMesh->SetMaterial(0, MaterialToApply);
    }
    else
    {
        // Use default material if custom materials are not available
        UMaterialInterface* DefaultMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        if (DefaultMaterial)
        {
            StructureMesh->SetMaterial(0, DefaultMaterial);
        }
    }
}

void AArchitecturalStructureManager::SetupInteractionVolume()
{
    if (!InteractionVolume)
    {
        return;
    }

    // Set interaction volume size based on structure dimensions
    FVector BoxExtent = StructureData.Dimensions * 1.2f; // 20% larger than structure
    InteractionVolume->SetBoxExtent(BoxExtent);

    // Position slightly above ground
    InteractionVolume->SetRelativeLocation(FVector(0.0f, 0.0f, BoxExtent.Z * 0.5f));
}