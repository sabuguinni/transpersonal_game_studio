#include "Arch_CaveSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

AArch_CaveSystemManager::AArch_CaveSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize components
    InitializeCaveComponents();
}

void AArch_CaveSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup materials and generate cave system
    SetupCaveMaterials();
    GenerateCaveSystem();
}

void AArch_CaveSystemManager::InitializeCaveComponents()
{
    // Root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CaveRoot"));

    // Cave entrance mesh
    CaveEntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveEntranceMesh"));
    CaveEntranceMesh->SetupAttachment(RootComponent);
    
    // Cave interior mesh
    CaveInteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveInteriorMesh"));
    CaveInteriorMesh->SetupAttachment(RootComponent);
    CaveInteriorMesh->SetRelativeLocation(FVector(-500.0f, 0.0f, 0.0f));

    // Fire pit mesh
    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(CaveInteriorMesh);
    FirePitMesh->SetRelativeLocation(FVector(-200.0f, 0.0f, -50.0f));

    // Cave interior volume for interactions
    CaveInteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CaveInteriorVolume"));
    CaveInteriorVolume->SetupAttachment(CaveInteriorMesh);
    CaveInteriorVolume->SetBoxExtent(FVector(400.0f, 300.0f, 200.0f));
}

void AArch_CaveSystemManager::SetupCaveMaterials()
{
    // Set default materials if not assigned
    if (!CaveWallMaterial)
    {
        // Use basic material for now - can be replaced with proper cave material
        if (CaveEntranceMesh && CaveEntranceMesh->GetStaticMesh())
        {
            CaveWallMaterial = CaveEntranceMesh->GetMaterial(0);
        }
    }

    if (!CaveFloorMaterial)
    {
        CaveFloorMaterial = CaveWallMaterial;
    }

    // Apply materials to meshes
    if (CaveWallMaterial)
    {
        if (CaveEntranceMesh)
        {
            CaveEntranceMesh->SetMaterial(0, CaveWallMaterial);
        }
        if (CaveInteriorMesh)
        {
            CaveInteriorMesh->SetMaterial(0, CaveWallMaterial);
        }
    }
}

void AArch_CaveSystemManager::GenerateCaveSystem()
{
    // Generate cave based on configuration
    SetupCaveInterior();
    
    if (CaveConfig.bHasFirePit)
    {
        PlaceFirePit();
    }
    
    if (CaveConfig.bHasShelterArea)
    {
        AddShelterElements();
    }

    UE_LOG(LogTemp, Warning, TEXT("Cave system generated with depth: %f, width: %f, height: %f"), 
           CaveConfig.CaveDepth, CaveConfig.CaveWidth, CaveConfig.CaveHeight);
}

void AArch_CaveSystemManager::SetupCaveInterior()
{
    if (!CaveInteriorMesh)
    {
        return;
    }

    // Scale cave interior based on configuration
    FVector CaveScale = FVector(
        CaveConfig.CaveDepth / 1000.0f,
        CaveConfig.CaveWidth / 800.0f,
        CaveConfig.CaveHeight / 400.0f
    );
    
    CaveInteriorMesh->SetRelativeScale3D(CaveScale);

    // Update interior volume to match cave size
    if (CaveInteriorVolume)
    {
        FVector VolumeExtent = FVector(
            CaveConfig.CaveDepth * 0.4f,
            CaveConfig.CaveWidth * 0.375f,
            CaveConfig.CaveHeight * 0.5f
        );
        CaveInteriorVolume->SetBoxExtent(VolumeExtent);
    }
}

void AArch_CaveSystemManager::PlaceFirePit()
{
    if (!FirePitMesh)
    {
        return;
    }

    // Position fire pit in the cave interior
    FVector FirePitLocation = FVector(
        -CaveConfig.CaveDepth * 0.3f,
        0.0f,
        -50.0f
    );
    
    FirePitMesh->SetRelativeLocation(FirePitLocation);
    FirePitMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.5f));

    UE_LOG(LogTemp, Log, TEXT("Fire pit placed at location: %s"), *FirePitLocation.ToString());
}

void AArch_CaveSystemManager::AddShelterElements()
{
    // Add primitive shelter elements like sleeping areas, storage
    // This would typically spawn additional mesh components or actors
    
    UE_LOG(LogTemp, Log, TEXT("Shelter elements added to cave system"));
}

bool AArch_CaveSystemManager::IsLocationSuitableForCave(const FVector& Location) const
{
    // Check if location is suitable for cave placement
    // Consider terrain slope, biome type, distance from other structures
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Simple check - ensure we're not too close to other cave systems
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AArch_CaveSystemManager::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != this)
        {
            float Distance = FVector::Dist(Location, Actor->GetActorLocation());
            if (Distance < 2000.0f) // Minimum 20m distance between caves
            {
                return false;
            }
        }
    }

    return true;
}

void AArch_CaveSystemManager::SpawnCaveAtLocation(const FVector& Location, const FRotator& Rotation)
{
    if (!IsLocationSuitableForCave(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("Location not suitable for cave placement"));
        return;
    }

    SetActorLocation(Location);
    SetActorRotation(Rotation);
    
    GenerateCaveSystem();
    
    UE_LOG(LogTemp, Log, TEXT("Cave spawned at location: %s"), *Location.ToString());
}

void AArch_CaveSystemManager::GenerateTestCave()
{
    // Editor-only function to generate a test cave
    FVector TestLocation = GetActorLocation();
    FRotator TestRotation = GetActorRotation();
    
    SpawnCaveAtLocation(TestLocation, TestRotation);
    
    UE_LOG(LogTemp, Warning, TEXT("Test cave generated at current actor location"));
}

FVector AArch_CaveSystemManager::CalculateOptimalCaveLocation() const
{
    // Calculate optimal location based on biome preference and terrain
    FVector OptimalLocation = GetActorLocation();
    
    // Adjust based on preferred biome
    switch (CaveConfig.PreferredBiome)
    {
        case EBiomeType::Mountain:
            OptimalLocation = FVector(40000.0f, 50000.0f, 500.0f);
            break;
        case EBiomeType::Forest:
            OptimalLocation = FVector(-45000.0f, 40000.0f, 200.0f);
            break;
        case EBiomeType::Desert:
            OptimalLocation = FVector(55000.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Swamp:
            OptimalLocation = FVector(-50000.0f, -45000.0f, 50.0f);
            break;
        default:
            OptimalLocation = FVector(0.0f, 0.0f, 100.0f);
            break;
    }
    
    return OptimalLocation;
}