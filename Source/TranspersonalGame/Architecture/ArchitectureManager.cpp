#include "ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UArchitectureManager::UArchitectureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    MaxStructuresPerBiome = 20;
    MinDistanceBetweenStructures = 1000.0f;
    bAutoSpawnOnBeginPlay = false;
}

void UArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoSpawnOnBeginPlay)
    {
        // Spawn structures in all biomes
        SpawnStructuresInBiome(EBiomeType::Savana, 5);
        SpawnStructuresInBiome(EBiomeType::Pantano, 3);
        SpawnStructuresInBiome(EBiomeType::Floresta, 4);
        SpawnStructuresInBiome(EBiomeType::Deserto, 6);
        SpawnStructuresInBiome(EBiomeType::Montanha, 8);
    }
}

void UArchitectureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structural integrity over time
    UpdateStructuralIntegrity(DeltaTime);
}

void UArchitectureManager::SpawnStructuresInBiome(EBiomeType BiomeType, int32 Count)
{
    FVector BiomeCenter = GetBiomeBaseLocation(BiomeType);
    
    for (int32 i = 0; i < Count; i++)
    {
        // Generate random offset within biome area
        float OffsetX = FMath::RandRange(-5000.0f, 5000.0f);
        float OffsetY = FMath::RandRange(-5000.0f, 5000.0f);
        FVector SpawnLocation = BiomeCenter + FVector(OffsetX, OffsetY, 0.0f);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            EArch_StructureType StructureType = static_cast<EArch_StructureType>(FMath::RandRange(0, 4));
            SpawnStructureAtLocation(SpawnLocation, StructureType, BiomeType);
        }
    }
}

void UArchitectureManager::SpawnStructureAtLocation(const FVector& Location, EArch_StructureType StructureType, EBiomeType BiomeType)
{
    if (!GetWorld())
    {
        return;
    }
    
    FArch_StructureData StructureData;
    StructureData.Location = Location;
    StructureData.BiomeType = BiomeType;
    StructureData.StructuralIntegrity = FMath::RandRange(60.0f, 100.0f);
    StructureData.bIsRuin = StructureData.StructuralIntegrity < 80.0f;
    
    // Set scale based on structure type
    switch (StructureType)
    {
        case EArch_StructureType::StonePillar:
            StructureData.Scale = FVector(1.5f, 1.5f, 4.0f);
            StructureData.StructureName = TEXT("Stone_Pillar");
            break;
        case EArch_StructureType::CaveEntrance:
            StructureData.Scale = FVector(3.0f, 2.0f, 2.5f);
            StructureData.StructureName = TEXT("Cave_Entrance");
            break;
        case EArch_StructureType::RockFormation:
            StructureData.Scale = FVector(2.5f, 2.5f, 1.8f);
            StructureData.StructureName = TEXT("Rock_Formation");
            break;
        case EArch_StructureType::AncientRuin:
            StructureData.Scale = FVector(4.0f, 3.0f, 2.0f);
            StructureData.StructureName = TEXT("Ancient_Ruin");
            StructureData.bIsRuin = true;
            break;
        case EArch_StructureType::NaturalArch:
            StructureData.Scale = FVector(3.5f, 1.0f, 3.0f);
            StructureData.StructureName = TEXT("Natural_Arch");
            break;
    }
    
    // Random rotation for natural variation
    StructureData.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    
    AStaticMeshActor* StructureActor = CreateStructureActor(StructureData, StructureType);
    if (StructureActor)
    {
        SpawnedStructures.Add(StructureData);
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("Architecture: Spawned %s in %s at %s"), 
               *StructureData.StructureName, *BiomeName, *Location.ToString());
    }
}

void UArchitectureManager::RemoveStructuresInRadius(const FVector& Center, float Radius)
{
    for (int32 i = SpawnedStructures.Num() - 1; i >= 0; i--)
    {
        float Distance = FVector::Dist(SpawnedStructures[i].Location, Center);
        if (Distance <= Radius)
        {
            SpawnedStructures.RemoveAt(i);
        }
    }
}

TArray<FArch_StructureData> UArchitectureManager::GetStructuresInBiome(EBiomeType BiomeType) const
{
    TArray<FArch_StructureData> BiomeStructures;
    
    for (const FArch_StructureData& Structure : SpawnedStructures)
    {
        if (Structure.BiomeType == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

void UArchitectureManager::UpdateStructuralIntegrity(float DeltaTime)
{
    // Slowly degrade structures over time (very slow for gameplay)
    float DegradationRate = 0.001f; // 0.1% per second
    
    for (FArch_StructureData& Structure : SpawnedStructures)
    {
        if (Structure.StructuralIntegrity > 0.0f)
        {
            Structure.StructuralIntegrity -= DegradationRate * DeltaTime;
            Structure.StructuralIntegrity = FMath::Max(0.0f, Structure.StructuralIntegrity);
            
            // Mark as ruin when integrity drops below 50%
            if (Structure.StructuralIntegrity < 50.0f)
            {
                Structure.bIsRuin = true;
            }
        }
    }
}

void UArchitectureManager::SpawnTestStructures()
{
    // Editor-only function to test structure spawning
    SpawnStructuresInBiome(EBiomeType::Savana, 3);
    UE_LOG(LogTemp, Warning, TEXT("Architecture: Test structures spawned"));
}

FVector UArchitectureManager::GetBiomeBaseLocation(EBiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Montanha:
            return FVector(40000.0f, 50000.0f, 200.0f);
        default:
            return FVector::ZeroVector;
    }
}

bool UArchitectureManager::IsValidSpawnLocation(const FVector& Location) const
{
    // Check minimum distance from existing structures
    for (const FArch_StructureData& ExistingStructure : SpawnedStructures)
    {
        float Distance = FVector::Dist(Location, ExistingStructure.Location);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    return true;
}

AStaticMeshActor* UArchitectureManager::CreateStructureActor(const FArch_StructureData& StructureData, EArch_StructureType StructureType)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Spawn basic cube actor as placeholder
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* StructureActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        StructureData.Location,
        StructureData.Rotation,
        SpawnParams
    );
    
    if (StructureActor)
    {
        StructureActor->SetActorScale3D(StructureData.Scale);
        StructureActor->SetActorLabel(StructureData.StructureName);
        
        // Set collision for interaction
        if (UStaticMeshComponent* MeshComp = StructureActor->GetStaticMeshComponent())
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
    
    return StructureActor;
}