#include "WorldArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BiomeSystemManager.h"

AWorldArchitectureManager::AWorldArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MinElementSpacing = 2000.0f;
    MaxElementsPerBiome = 50;
}

void AWorldArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitectureManager: Starting architectural initialization"));
    InitializeArchitecturalElements();
}

void AWorldArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorldArchitectureManager::InitializeArchitecturalElements()
{
    ArchitecturalElements.Empty();
    
    // Create architecture for each biome
    CreateBiomeSpecificArchitecture(EBiomeType::Savana);
    CreateBiomeSpecificArchitecture(EBiomeType::Forest);
    CreateBiomeSpecificArchitecture(EBiomeType::Desert);
    CreateBiomeSpecificArchitecture(EBiomeType::Swamp);
    CreateBiomeSpecificArchitecture(EBiomeType::Mountain);
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitectureManager: Initialized %d architectural elements"), ArchitecturalElements.Num());
}

void AWorldArchitectureManager::SpawnArchitecturalElement(const FEng_ArchitecturalElement& Element)
{
    if (!ValidateArchitecturalPlacement(Element.Location, MinElementSpacing))
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldArchitectureManager: Invalid placement for %s"), *Element.ElementName);
        return;
    }

    // Spawn the architectural element
    SpawnRockFormationMesh(Element.Location, Element.Rotation, Element.Scale);
    
    // Add to our tracking array
    ArchitecturalElements.Add(Element);
    
    UE_LOG(LogTemp, Log, TEXT("WorldArchitectureManager: Spawned %s at %s"), *Element.ElementName, *Element.Location.ToString());
}

void AWorldArchitectureManager::CreateCaveSystem(EBiomeType BiomeType, const FVector& Location)
{
    FEng_ArchitecturalElement CaveElement;
    CaveElement.ElementName = FString::Printf(TEXT("Cave_%s"), *UEnum::GetValueAsString(BiomeType));
    CaveElement.Location = Location;
    CaveElement.Rotation = FRotator(0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f), 0.0f);
    CaveElement.Scale = FVector(2.0f, 2.0f, 1.5f);
    CaveElement.AssociatedBiome = BiomeType;
    CaveElement.bIsInteractable = true;
    CaveElement.StructuralIntegrity = 100.0f;
    
    SpawnArchitecturalElement(CaveElement);
}

void AWorldArchitectureManager::CreateRockFormation(EBiomeType BiomeType, const FVector& Location, float Scale)
{
    FEng_ArchitecturalElement RockElement;
    RockElement.ElementName = FString::Printf(TEXT("RockFormation_%s_%d"), *UEnum::GetValueAsString(BiomeType), FMath::RandRange(1, 999));
    RockElement.Location = Location;
    RockElement.Rotation = FRotator(
        UKismetMathLibrary::RandomFloatInRange(-15.0f, 15.0f),
        UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f),
        UKismetMathLibrary::RandomFloatInRange(-10.0f, 10.0f)
    );
    RockElement.Scale = FVector(Scale, Scale, Scale * UKismetMathLibrary::RandomFloatInRange(0.8f, 1.2f));
    RockElement.AssociatedBiome = BiomeType;
    RockElement.bIsInteractable = false;
    RockElement.StructuralIntegrity = 100.0f;
    
    SpawnArchitecturalElement(RockElement);
}

void AWorldArchitectureManager::CreateTribalStructure(const FVector& Location, const FString& StructureType)
{
    FEng_ArchitecturalElement TribalElement;
    TribalElement.ElementName = FString::Printf(TEXT("TribalStructure_%s"), *StructureType);
    TribalElement.Location = Location;
    TribalElement.Rotation = FRotator(0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f), 0.0f);
    TribalElement.Scale = FVector(1.0f, 1.0f, 1.0f);
    TribalElement.AssociatedBiome = EBiomeType::Savana; // Default to savana
    TribalElement.bIsInteractable = true;
    TribalElement.StructuralIntegrity = 75.0f;
    
    SpawnArchitecturalElement(TribalElement);
}

TArray<FEng_ArchitecturalElement> AWorldArchitectureManager::GetArchitecturalElementsInRadius(const FVector& Center, float Radius) const
{
    TArray<FEng_ArchitecturalElement> ElementsInRadius;
    
    for (const FEng_ArchitecturalElement& Element : ArchitecturalElements)
    {
        float Distance = FVector::Dist(Center, Element.Location);
        if (Distance <= Radius)
        {
            ElementsInRadius.Add(Element);
        }
    }
    
    return ElementsInRadius;
}

bool AWorldArchitectureManager::ValidateArchitecturalPlacement(const FVector& Location, float MinDistance) const
{
    for (const FEng_ArchitecturalElement& Element : ArchitecturalElements)
    {
        float Distance = FVector::Dist(Location, Element.Location);
        if (Distance < MinDistance)
        {
            return false;
        }
    }
    return true;
}

void AWorldArchitectureManager::GenerateDefaultArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitectureManager: Generating default architecture"));
    InitializeArchitecturalElements();
}

void AWorldArchitectureManager::CreateBiomeSpecificArchitecture(EBiomeType BiomeType)
{
    // Get biome center from BiomeSystemManager
    FVector BiomeCenter;
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
            break;
        case EBiomeType::Forest:
            BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
            break;
        case EBiomeType::Desert:
            BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
            break;
        case EBiomeType::Swamp:
            BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
            break;
        case EBiomeType::Mountain:
            BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
            break;
        default:
            BiomeCenter = FVector::ZeroVector;
            break;
    }
    
    // Create 10-15 rock formations per biome
    int32 NumRockFormations = FMath::RandRange(10, 15);
    for (int32 i = 0; i < NumRockFormations; i++)
    {
        FVector SpawnLocation = GetValidSpawnLocation(BiomeType);
        float RockScale = UKismetMathLibrary::RandomFloatInRange(0.5f, 3.0f);
        CreateRockFormation(BiomeType, SpawnLocation, RockScale);
    }
    
    // Create 2-3 cave systems per biome
    int32 NumCaves = FMath::RandRange(2, 3);
    for (int32 i = 0; i < NumCaves; i++)
    {
        FVector CaveLocation = GetValidSpawnLocation(BiomeType);
        CreateCaveSystem(BiomeType, CaveLocation);
    }
}

FVector AWorldArchitectureManager::GetValidSpawnLocation(EBiomeType BiomeType) const
{
    FVector BiomeCenter;
    float BiomeRadius;
    
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
            BiomeRadius = 40000.0f;
            break;
        case EBiomeType::Forest:
            BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
            BiomeRadius = 35000.0f;
            break;
        case EBiomeType::Desert:
            BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
            BiomeRadius = 30000.0f;
            break;
        case EBiomeType::Swamp:
            BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
            BiomeRadius = 25000.0f;
            break;
        case EBiomeType::Mountain:
            BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
            BiomeRadius = 20000.0f;
            break;
        default:
            BiomeCenter = FVector::ZeroVector;
            BiomeRadius = 10000.0f;
            break;
    }
    
    // Generate random point within biome radius
    float RandomAngle = UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f);
    float RandomDistance = UKismetMathLibrary::RandomFloatInRange(0.0f, BiomeRadius * 0.8f);
    
    float X = BiomeCenter.X + RandomDistance * FMath::Cos(FMath::DegreesToRadians(RandomAngle));
    float Y = BiomeCenter.Y + RandomDistance * FMath::Sin(FMath::DegreesToRadians(RandomAngle));
    float Z = BiomeCenter.Z + UKismetMathLibrary::RandomFloatInRange(0.0f, 500.0f);
    
    return FVector(X, Y, Z);
}

void AWorldArchitectureManager::SpawnRockFormationMesh(const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    // Spawn a basic cube as placeholder for rock formation
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (RockActor)
    {
        UStaticMeshComponent* MeshComp = RockActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Try to load a basic cube mesh as placeholder
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                RockActor->SetActorScale3D(Scale);
                RockActor->SetActorLabel(FString::Printf(TEXT("ArchRock_%d"), FMath::RandRange(1000, 9999)));
            }
        }
    }
}