#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxStructuresPerBiome = 10;
    MinDistanceBetweenStructures = 5000.0f;
    WeatheringRate = 0.1f;
    bEnableWeatheringSystem = true;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Structure Manager initialized"));
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableWeatheringSystem)
    {
        ApplyWeathering(DeltaTime);
    }
}

bool AArch_StructureManager::SpawnStructure(EArch_StructureType StructureType, FVector Location, EBiomeType BiomeType)
{
    if (!IsLocationValidForStructure(Location, StructureType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid location for structure spawn"));
        return false;
    }

    // Check if we've reached the limit for this biome
    TArray<FArch_StructureData> BiomeStructures = GetStructuresInBiome(BiomeType);
    if (BiomeStructures.Num() >= MaxStructuresPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max structures reached for biome"));
        return false;
    }

    // Create the structure based on type
    switch (StructureType)
    {
        case EArch_StructureType::StoneCircle:
            CreateStoneCircle(Location, BiomeType);
            break;
        case EArch_StructureType::CaveEntrance:
            CreateCaveEntrance(Location, BiomeType);
            break;
        case EArch_StructureType::RockFormation:
            CreateRockFormation(Location, BiomeType);
            break;
        case EArch_StructureType::AncientRuin:
            CreateAncientRuin(Location, BiomeType);
            break;
        case EArch_StructureType::NaturalArch:
            CreateNaturalArch(Location, BiomeType);
            break;
    }

    // Add to managed structures
    FArch_StructureData NewStructure;
    NewStructure.StructureType = StructureType;
    NewStructure.Location = Location;
    NewStructure.AssociatedBiome = BiomeType;
    NewStructure.WeatheringLevel = FMath::RandRange(0.3f, 0.7f);
    ManagedStructures.Add(NewStructure);

    UE_LOG(LogTemp, Warning, TEXT("Structure spawned successfully"));
    return true;
}

void AArch_StructureManager::RemoveStructure(int32 StructureIndex)
{
    if (ManagedStructures.IsValidIndex(StructureIndex))
    {
        ManagedStructures.RemoveAt(StructureIndex);
        UE_LOG(LogTemp, Warning, TEXT("Structure removed"));
    }
}

TArray<FArch_StructureData> AArch_StructureManager::GetStructuresInBiome(EBiomeType BiomeType)
{
    TArray<FArch_StructureData> BiomeStructures;
    
    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        if (Structure.AssociatedBiome == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

void AArch_StructureManager::ApplyWeathering(float DeltaTime)
{
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        Structure.WeatheringLevel += WeatheringRate * DeltaTime * 0.01f; // Very slow weathering
        Structure.WeatheringLevel = FMath::Clamp(Structure.WeatheringLevel, 0.0f, 1.0f);
    }
}

void AArch_StructureManager::GenerateStructuresForAllBiomes()
{
    TArray<EBiomeType> BiomeTypes = {
        EBiomeType::Forest,
        EBiomeType::Savanna,
        EBiomeType::Swamp,
        EBiomeType::Mountain,
        EBiomeType::Desert
    };

    for (EBiomeType BiomeType : BiomeTypes)
    {
        for (int32 i = 0; i < 3; i++) // Generate 3 structures per biome
        {
            FVector RandomLocation = GetRandomLocationInBiome(BiomeType);
            EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 4));
            SpawnStructure(RandomType, RandomLocation, BiomeType);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated structures for all biomes"));
}

bool AArch_StructureManager::IsLocationValidForStructure(FVector Location, EArch_StructureType StructureType)
{
    // Check minimum distance from existing structures
    for (const FArch_StructureData& ExistingStructure : ManagedStructures)
    {
        float Distance = FVector::Dist(Location, ExistingStructure.Location);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }

    // Additional validation based on structure type
    switch (StructureType)
    {
        case EArch_StructureType::CaveEntrance:
            // Cave entrances should be near elevated terrain
            return Location.Z > 100.0f;
        case EArch_StructureType::StoneCircle:
            // Stone circles should be on relatively flat ground
            return Location.Z < 300.0f;
        default:
            return true;
    }
}

FVector AArch_StructureManager::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    // Define biome boundaries (simplified)
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            return FVector(FMath::RandRange(-10000, 20000), FMath::RandRange(-10000, 20000), 100);
        case EBiomeType::Savanna:
            return FVector(FMath::RandRange(30000, 70000), FMath::RandRange(30000, 70000), 100);
        case EBiomeType::Swamp:
            return FVector(FMath::RandRange(-50000, -20000), FMath::RandRange(20000, 50000), 50);
        case EBiomeType::Mountain:
            return FVector(FMath::RandRange(60000, 100000), FMath::RandRange(-40000, 0), 500);
        case EBiomeType::Desert:
            return FVector(FMath::RandRange(-70000, -30000), FMath::RandRange(-50000, -10000), 20);
        default:
            return FVector::ZeroVector;
    }
}

void AArch_StructureManager::CreateStoneCircle(FVector CenterLocation, EBiomeType BiomeType)
{
    const int32 NumStones = 5;
    const float Radius = 800.0f;
    
    for (int32 i = 0; i < NumStones; i++)
    {
        float Angle = (i * 72.0f) * PI / 180.0f; // 72 degrees apart
        FVector StoneLocation = CenterLocation + FVector(
            Radius * FMath::Cos(Angle),
            Radius * FMath::Sin(Angle),
            0
        );
        
        FRotator StoneRotation = FRotator(0, FMath::RandRange(0, 360), FMath::RandRange(-5, 5));
        FVector StoneScale = FVector(0.8f, 0.4f, 3.5f);
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        FString StoneLabel = FString::Printf(TEXT("Stone_%s_%03d"), *BiomeName, i + 1);
        
        SpawnStructureActor(StoneLocation, StoneRotation, StoneScale, StoneLabel);
    }
}

void AArch_StructureManager::CreateCaveEntrance(FVector Location, EBiomeType BiomeType)
{
    FRotator CaveRotation = FRotator(0, FMath::RandRange(0, 360), 0);
    FVector CaveScale = FVector(4.0f, 2.0f, 2.0f);
    
    FString BiomeName = UEnum::GetValueAsString(BiomeType);
    FString CaveLabel = FString::Printf(TEXT("Cave_%s_001"), *BiomeName);
    
    SpawnStructureActor(Location, CaveRotation, CaveScale, CaveLabel);
}

void AArch_StructureManager::CreateRockFormation(FVector Location, EBiomeType BiomeType)
{
    const int32 NumRocks = FMath::RandRange(3, 7);
    
    for (int32 i = 0; i < NumRocks; i++)
    {
        FVector RockOffset = FVector(
            FMath::RandRange(-500, 500),
            FMath::RandRange(-500, 500),
            0
        );
        
        FVector RockLocation = Location + RockOffset;
        FRotator RockRotation = FRotator(
            FMath::RandRange(-10, 10),
            FMath::RandRange(0, 360),
            FMath::RandRange(-10, 10)
        );
        
        FVector RockScale = FVector(
            FMath::RandRange(1.5f, 3.0f),
            FMath::RandRange(1.5f, 3.0f),
            FMath::RandRange(2.0f, 4.0f)
        );
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        FString RockLabel = FString::Printf(TEXT("Rock_%s_%03d"), *BiomeName, i + 1);
        
        SpawnStructureActor(RockLocation, RockRotation, RockScale, RockLabel);
    }
}

void AArch_StructureManager::CreateAncientRuin(FVector Location, EBiomeType BiomeType)
{
    // Create a simple ruined structure with multiple pieces
    TArray<FVector> RuinPieces = {
        FVector(0, 0, 0),      // Center piece
        FVector(300, 0, 0),    // East wall
        FVector(-300, 0, 0),   // West wall
        FVector(0, 300, 0),    // North wall
        FVector(0, -300, 0)    // South wall
    };
    
    for (int32 i = 0; i < RuinPieces.Num(); i++)
    {
        FVector PieceLocation = Location + RuinPieces[i];
        FRotator PieceRotation = FRotator(0, FMath::RandRange(0, 360), FMath::RandRange(-15, 15));
        FVector PieceScale = FVector(2.0f, 0.5f, 2.5f);
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        FString RuinLabel = FString::Printf(TEXT("Ruin_%s_%03d"), *BiomeName, i + 1);
        
        SpawnStructureActor(PieceLocation, PieceRotation, PieceScale, RuinLabel);
    }
}

void AArch_StructureManager::CreateNaturalArch(FVector Location, EBiomeType BiomeType)
{
    // Create arch supports and top piece
    FVector LeftSupport = Location + FVector(-200, 0, 0);
    FVector RightSupport = Location + FVector(200, 0, 0);
    FVector ArchTop = Location + FVector(0, 0, 400);
    
    TArray<FVector> ArchPieces = { LeftSupport, RightSupport, ArchTop };
    TArray<FVector> ArchScales = {
        FVector(1.0f, 1.0f, 4.0f),  // Left support
        FVector(1.0f, 1.0f, 4.0f),  // Right support
        FVector(4.0f, 1.0f, 0.5f)   // Top piece
    };
    
    for (int32 i = 0; i < ArchPieces.Num(); i++)
    {
        FRotator ArchRotation = FRotator(0, 0, 0);
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        FString ArchLabel = FString::Printf(TEXT("Arch_%s_%03d"), *BiomeName, i + 1);
        
        SpawnStructureActor(ArchPieces[i], ArchRotation, ArchScales[i], ArchLabel);
    }
}

AActor* AArch_StructureManager::SpawnStructureActor(FVector Location, FRotator Rotation, FVector Scale, const FString& Label)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Spawn a static mesh actor as placeholder
    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (StructureActor)
    {
        StructureActor->SetActorScale3D(Scale);
        StructureActor->SetActorLabel(Label);
        
        UE_LOG(LogTemp, Warning, TEXT("Structure actor spawned: %s"), *Label);
        return StructureActor;
    }
    
    return nullptr;
}

void AArch_StructureManager::ApplyWeatheringToActor(AActor* StructureActor, float WeatheringLevel)
{
    if (!StructureActor)
    {
        return;
    }

    // Apply weathering effects to the actor's materials
    AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(StructureActor);
    if (MeshActor && MeshActor->GetStaticMeshComponent())
    {
        UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
        
        // Create dynamic material instance if needed
        UMaterialInterface* CurrentMaterial = MeshComp->GetMaterial(0);
        if (CurrentMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, this);
            if (DynamicMaterial)
            {
                // Apply weathering parameters
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), WeatheringLevel);
                DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), WeatheringLevel * 0.5f);
                MeshComp->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}