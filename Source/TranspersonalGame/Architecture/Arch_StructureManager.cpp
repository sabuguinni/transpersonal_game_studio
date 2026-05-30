#include "Arch_StructureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create primary structure mesh
    PrimaryStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryStructureMesh"));
    PrimaryStructureMesh->SetupAttachment(RootComponent);

    // Initialize biome locations (from memory ID 709)
    BiomeLocations.Empty();
    BiomeLocations.Add(FVector(0.0f, 0.0f, 100.0f)); // Savana
    BiomeLocations.Add(FVector(-50000.0f, -45000.0f, 100.0f)); // Pantano
    BiomeLocations.Add(FVector(-45000.0f, 40000.0f, 100.0f)); // Floresta
    BiomeLocations.Add(FVector(55000.0f, 0.0f, 100.0f)); // Deserto
    BiomeLocations.Add(FVector(40000.0f, 50000.0f, 100.0f)); // Montanha

    // Initialize structure data
    StructureData.StructureType = EArch_StructureType::Ruins;
    StructureData.Location = FVector::ZeroVector;
    StructureData.Rotation = FRotator::ZeroRotator;
    StructureData.Scale = FVector::OneVector;
    StructureData.WearLevel = 0.5f;
    StructureData.bHasMossGrowth = true;
    StructureData.bHasCarvedSymbols = false;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateStructureMesh();
    
    if (StructureData.bHasMossGrowth)
    {
        ApplyMossEffect();
    }
    
    if (StructureData.bHasCarvedSymbols)
    {
        ApplyCarvedSymbols();
    }
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArch_StructureManager::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;
    UpdateStructureMesh();
}

void AArch_StructureManager::ApplyWeatheringEffect(float WeatheringAmount)
{
    StructureData.WearLevel = FMath::Clamp(StructureData.WearLevel + WeatheringAmount, 0.0f, 1.0f);
    
    // Apply visual weathering effects
    if (PrimaryStructureMesh && PrimaryStructureMesh->GetMaterial(0))
    {
        // Create dynamic material instance to modify weathering parameters
        UMaterialInstanceDynamic* DynamicMaterial = PrimaryStructureMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), StructureData.WearLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), StructureData.bHasMossGrowth ? StructureData.WearLevel * 0.8f : 0.0f);
        }
    }
}

void AArch_StructureManager::SpawnStructureAtBiome(int32 BiomeIndex, EArch_StructureType StructureType)
{
    if (BiomeIndex >= 0 && BiomeIndex < BiomeLocations.Num())
    {
        FVector BiomeLocation = BiomeLocations[BiomeIndex];
        
        // Add random offset within biome area
        FVector RandomOffset = FVector(
            FMath::RandRange(-15000.0f, 15000.0f),
            FMath::RandRange(-15000.0f, 15000.0f),
            0.0f
        );
        
        FVector SpawnLocation = BiomeLocation + RandomOffset;
        
        // Set structure data
        StructureData.StructureType = StructureType;
        StructureData.Location = SpawnLocation;
        StructureData.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        // Move actor to new location
        SetActorLocation(SpawnLocation);
        SetActorRotation(StructureData.Rotation);
        
        UpdateStructureMesh();
        
        UE_LOG(LogTemp, Warning, TEXT("AArch_StructureManager: Spawned %s structure at biome %d: %s"), 
               *UEnum::GetValueAsString(StructureType), BiomeIndex, *SpawnLocation.ToString());
    }
}

void AArch_StructureManager::DistributeStructuresAcrossBiomes()
{
    for (int32 i = 0; i < BiomeLocations.Num(); ++i)
    {
        EArch_StructureType StructureType = static_cast<EArch_StructureType>(FMath::RandRange(0, 5));
        SpawnStructureAtBiome(i, StructureType);
    }
}

void AArch_StructureManager::GenerateRandomStructures()
{
    // Generate 3-5 random structures across biomes
    int32 NumStructures = FMath::RandRange(3, 5);
    
    for (int32 i = 0; i < NumStructures; ++i)
    {
        int32 RandomBiome = FMath::RandRange(0, BiomeLocations.Num() - 1);
        EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 5));
        
        SpawnStructureAtBiome(RandomBiome, RandomType);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AArch_StructureManager: Generated %d random structures"), NumStructures);
}

void AArch_StructureManager::SetStructureData(const FArch_StructureData& NewData)
{
    StructureData = NewData;
    UpdateStructureMesh();
    
    if (StructureData.bHasMossGrowth)
    {
        ApplyMossEffect();
    }
    
    if (StructureData.bHasCarvedSymbols)
    {
        ApplyCarvedSymbols();
    }
}

void AArch_StructureManager::UpdateStructureMesh()
{
    UStaticMesh* NewMesh = GetMeshForStructureType(StructureData.StructureType);
    
    if (NewMesh && PrimaryStructureMesh)
    {
        PrimaryStructureMesh->SetStaticMesh(NewMesh);
        PrimaryStructureMesh->SetWorldScale3D(StructureData.Scale);
        
        // Apply weathering effect
        ApplyWeatheringEffect(0.0f); // Refresh current weathering level
    }
}

void AArch_StructureManager::ApplyMossEffect()
{
    if (PrimaryStructureMesh && PrimaryStructureMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = PrimaryStructureMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            float MossAmount = StructureData.bHasMossGrowth ? FMath::RandRange(0.3f, 0.8f) : 0.0f;
            DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), MossAmount);
            DynamicMaterial->SetVectorParameterValue(TEXT("MossColor"), FLinearColor(0.2f, 0.6f, 0.3f, 1.0f));
        }
    }
}

void AArch_StructureManager::ApplyCarvedSymbols()
{
    if (PrimaryStructureMesh && PrimaryStructureMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = PrimaryStructureMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            float SymbolVisibility = StructureData.bHasCarvedSymbols ? 1.0f : 0.0f;
            DynamicMaterial->SetScalarParameterValue(TEXT("SymbolVisibility"), SymbolVisibility);
        }
    }
}

UStaticMesh* AArch_StructureManager::GetMeshForStructureType(EArch_StructureType Type)
{
    // Return appropriate mesh based on structure type
    // This would normally load from the mesh arrays, but for now return nullptr
    // The actual meshes would be set in Blueprint or loaded from content
    
    switch (Type)
    {
        case EArch_StructureType::Ruins:
            return RuinsMeshes.Num() > 0 ? RuinsMeshes[0] : nullptr;
        case EArch_StructureType::CaveDwelling:
            return CaveDwellingMeshes.Num() > 0 ? CaveDwellingMeshes[0] : nullptr;
        case EArch_StructureType::StonePillar:
            return PillarMeshes.Num() > 0 ? PillarMeshes[0] : nullptr;
        case EArch_StructureType::Archway:
            return RuinsMeshes.Num() > 1 ? RuinsMeshes[1] : nullptr;
        case EArch_StructureType::Platform:
            return RuinsMeshes.Num() > 2 ? RuinsMeshes[2] : nullptr;
        case EArch_StructureType::Shelter:
            return CaveDwellingMeshes.Num() > 1 ? CaveDwellingMeshes[1] : nullptr;
        default:
            return nullptr;
    }
}