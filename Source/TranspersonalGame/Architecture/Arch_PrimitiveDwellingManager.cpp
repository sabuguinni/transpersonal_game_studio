#include "Arch_PrimitiveDwellingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AArch_PrimitiveDwellingManager::AArch_PrimitiveDwellingManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default dwelling configs for each biome
    FArch_DwellingConfig SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.BaseSize = FVector(500.0f, 700.0f, 350.0f);
    SavannaConfig.bHasFirePit = true;
    SavannaConfig.NumRooms = 2;
    DwellingConfigs.Add(SavannaConfig);

    FArch_DwellingConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.BaseSize = FVector(400.0f, 600.0f, 300.0f);
    ForestConfig.bHasFirePit = true;
    ForestConfig.NumRooms = 1;
    DwellingConfigs.Add(ForestConfig);

    FArch_DwellingConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.BaseSize = FVector(600.0f, 800.0f, 400.0f);
    DesertConfig.bHasFirePit = false;
    DesertConfig.NumRooms = 3;
    DwellingConfigs.Add(DesertConfig);

    FArch_DwellingConfig SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.BaseSize = FVector(350.0f, 500.0f, 250.0f);
    SwampConfig.bHasFirePit = true;
    SwampConfig.NumRooms = 1;
    DwellingConfigs.Add(SwampConfig);

    FArch_DwellingConfig MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.BaseSize = FVector(450.0f, 650.0f, 380.0f);
    MountainConfig.bHasFirePit = true;
    MountainConfig.NumRooms = 2;
    DwellingConfigs.Add(MountainConfig);
}

void AArch_PrimitiveDwellingManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveDwellingManager initialized in world"));
    }
}

void AArch_PrimitiveDwellingManager::GenerateDwellingsInBiomes()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No world context for dwelling generation"));
        return;
    }

    for (const FArch_DwellingConfig& Config : DwellingConfigs)
    {
        FVector BiomeCenter = GetBiomeCenter(Config.BiomeType);
        
        // Generate 3-5 dwellings per biome
        int32 DwellingCount = FMath::RandRange(3, 5);
        for (int32 i = 0; i < DwellingCount; i++)
        {
            // Scatter dwellings around biome center
            FVector Offset = FVector(
                FMath::RandRange(-8000.0f, 8000.0f),
                FMath::RandRange(-8000.0f, 8000.0f),
                0.0f
            );
            FVector DwellingLocation = BiomeCenter + Offset;
            DwellingLocation.Z = 100.0f; // Ground level

            AActor* NewDwelling = CreateDwelling(DwellingLocation, Config);
            if (NewDwelling)
            {
                PopulateInterior(NewDwelling, Config);
                UE_LOG(LogTemp, Warning, TEXT("Created dwelling %d in biome %d"), i + 1, (int32)Config.BiomeType);
            }
        }
    }
}

AActor* AArch_PrimitiveDwellingManager::CreateDwelling(const FVector& Location, const FArch_DwellingConfig& Config)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Create main dwelling actor
    AStaticMeshActor* DwellingActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!DwellingActor)
    {
        return nullptr;
    }

    // Set dwelling label
    FString BiomeName = UEnum::GetValueAsString(Config.BiomeType);
    BiomeName = BiomeName.Right(BiomeName.Len() - BiomeName.Find("::") - 2);
    DwellingActor->SetActorLabel(FString::Printf(TEXT("Dwelling_%s_%d"), *BiomeName, FMath::RandRange(1, 99)));

    // Create walls, roof, and door
    CreateWalls(DwellingActor, Location, Config);
    CreateRoof(DwellingActor, Location, Config);
    CreateDoor(DwellingActor, Location, Config);

    return DwellingActor;
}

void AArch_PrimitiveDwellingManager::PopulateInterior(AActor* DwellingActor, const FArch_DwellingConfig& Config)
{
    if (!DwellingActor || !GetWorld())
    {
        return;
    }

    FVector DwellingLocation = DwellingActor->GetActorLocation();
    
    // Add fire pit if configured
    if (Config.bHasFirePit)
    {
        FVector FirePitLocation = DwellingLocation + FVector(0.0f, 0.0f, 50.0f);
        AddFirePit(FirePitLocation, DwellingActor);
    }

    // Add interior props for each room
    for (int32 Room = 0; Room < Config.NumRooms; Room++)
    {
        FVector RoomOffset = FVector(
            (Room % 2) * (Config.BaseSize.X * 0.4f),
            (Room / 2) * (Config.BaseSize.Y * 0.4f),
            0.0f
        );
        FVector RoomCenter = DwellingLocation + RoomOffset;
        FVector RoomSize = Config.BaseSize / FMath::Max(1.0f, (float)Config.NumRooms);
        
        AddInteriorProps(RoomCenter, RoomSize, DwellingActor);
    }
}

void AArch_PrimitiveDwellingManager::AddFirePit(const FVector& Location, AActor* ParentActor)
{
    if (!GetWorld())
    {
        return;
    }

    // Create fire pit using basic cylinder
    AStaticMeshActor* FirePit = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (FirePit)
    {
        UStaticMeshComponent* MeshComp = FirePit->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Try to load basic cylinder mesh
            UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            if (CylinderMesh)
            {
                MeshComp->SetStaticMesh(CylinderMesh);
                FirePit->SetActorScale3d(FVector(1.0f, 1.0f, 0.3f));
                FirePit->SetActorLabel(TEXT("FirePit"));
            }
        }
    }
}

void AArch_PrimitiveDwellingManager::AddInteriorProps(const FVector& RoomCenter, const FVector& RoomSize, AActor* ParentActor)
{
    if (!GetWorld())
    {
        return;
    }

    // Add basic props using primitive shapes
    TArray<FString> PropTypes = {TEXT("Sleeping_Area"), TEXT("Storage_Area"), TEXT("Work_Area")};
    
    for (int32 i = 0; i < FMath::Min(3, PropTypes.Num()); i++)
    {
        FVector PropLocation = RoomCenter + FVector(
            FMath::RandRange(-RoomSize.X * 0.3f, RoomSize.X * 0.3f),
            FMath::RandRange(-RoomSize.Y * 0.3f, RoomSize.Y * 0.3f),
            25.0f
        );

        AStaticMeshActor* Prop = GetWorld()->SpawnActor<AStaticMeshActor>(PropLocation, FRotator::ZeroRotator);
        if (Prop)
        {
            UStaticMeshComponent* MeshComp = Prop->GetStaticMeshComponent();
            if (MeshComp)
            {
                UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
                if (CubeMesh)
                {
                    MeshComp->SetStaticMesh(CubeMesh);
                    Prop->SetActorScale3d(FVector(0.5f, 1.0f, 0.3f));
                    Prop->SetActorLabel(PropTypes[i]);
                }
            }
        }
    }
}

FVector AArch_PrimitiveDwellingManager::GetBiomeCenter(EBiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return FVector(0.0f, 0.0f, 0.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 0.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 0.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 0.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 0.0f);
        default:
            return FVector::ZeroVector;
    }
}

class UMaterialInterface* AArch_PrimitiveDwellingManager::GetBiomeMaterial(EBiomeType BiomeType) const
{
    // Return appropriate material based on biome
    // For now, return nullptr and let UE5 use default materials
    return nullptr;
}

void AArch_PrimitiveDwellingManager::CreateWalls(AActor* DwellingActor, const FVector& Location, const FArch_DwellingConfig& Config)
{
    // Implementation for wall creation using basic shapes
    // This would be expanded with proper mesh loading in production
}

void AArch_PrimitiveDwellingManager::CreateRoof(AActor* DwellingActor, const FVector& Location, const FArch_DwellingConfig& Config)
{
    // Implementation for roof creation
    // This would be expanded with proper mesh loading in production
}

void AArch_PrimitiveDwellingManager::CreateDoor(AActor* DwellingActor, const FVector& Location, const FArch_DwellingConfig& Config)
{
    // Implementation for door creation
    // This would be expanded with proper mesh loading in production
}