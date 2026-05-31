#include "ArchitecturalManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UArchitecturalManager::UArchitecturalManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;

    MaxStructuresPerBiome = 50.0f;
    StructureSpawnRadius = 15000.0f;
    bAutoSpawnStructures = false;
    MinDistanceBetweenStructures = 2000.0f;

    InitializeBiomeCenters();
}

void UArchitecturalManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSpawnStructures)
    {
        DistributeStructuresEvenly();
    }
}

void UArchitecturalManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Periodic cleanup of excess structures
    CleanupExcessStructures();
}

void UArchitecturalManager::InitializeBiomeCenters()
{
    BiomeCenters.Empty();
    BiomeCenters.Add(EBiomeType::Savanna, FVector(0.0f, 0.0f, 100.0f));
    BiomeCenters.Add(EBiomeType::Swamp, FVector(-50000.0f, -45000.0f, 100.0f));
    BiomeCenters.Add(EBiomeType::Forest, FVector(-45000.0f, 40000.0f, 100.0f));
    BiomeCenters.Add(EBiomeType::Desert, FVector(55000.0f, 0.0f, 100.0f));
    BiomeCenters.Add(EBiomeType::Mountain, FVector(40000.0f, 50000.0f, 100.0f));
}

void UArchitecturalManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation, EBiomeType BiomeType)
{
    if (!IsLocationValid(Location, BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Invalid location for structure spawn"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn a basic static mesh actor as structure placeholder
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    
    if (StructureActor)
    {
        // Set actor label based on structure type
        FString StructureName;
        switch (StructureType)
        {
            case EArch_StructureType::Pillar:
                StructureName = TEXT("StonePillar");
                break;
            case EArch_StructureType::Temple:
                StructureName = TEXT("AncientTemple");
                break;
            case EArch_StructureType::Dwelling:
                StructureName = TEXT("PrimitiveDwelling");
                break;
            case EArch_StructureType::Ruin:
                StructureName = TEXT("AncientRuin");
                break;
            default:
                StructureName = TEXT("Structure");
                break;
        }

        StructureActor->SetActorLabel(FString::Printf(TEXT("%s_%s_%d"), *StructureName, *UEnum::GetValueAsString(BiomeType), FMath::RandRange(1000, 9999)));
        
        RegisterStructure(StructureActor, StructureType, BiomeType);
        SetupInteriorProps(StructureActor, StructureType);

        UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Spawned %s at %s"), *StructureName, *Location.ToString());
    }
}

void UArchitecturalManager::SpawnStructuresAcrossBiomes(EArch_StructureType StructureType, int32 CountPerBiome)
{
    for (const auto& BiomePair : BiomeCenters)
    {
        EBiomeType BiomeType = BiomePair.Key;
        
        for (int32 i = 0; i < CountPerBiome; ++i)
        {
            FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
            FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            
            SpawnStructureAtLocation(StructureType, SpawnLocation, SpawnRotation, BiomeType);
        }
    }
}

TArray<AActor*> UArchitecturalManager::GetAllStructuresInBiome(EBiomeType BiomeType)
{
    TArray<AActor*> StructuresInBiome;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        return StructuresInBiome;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);

    FVector BiomeCenter = BiomeCenters.FindRef(BiomeType);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), BiomeCenter) <= StructureSpawnRadius)
        {
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("Pillar")) || ActorLabel.Contains(TEXT("Temple")) || 
                ActorLabel.Contains(TEXT("Dwelling")) || ActorLabel.Contains(TEXT("Ruin")) ||
                ActorLabel.Contains(TEXT("Structure")))
            {
                StructuresInBiome.Add(Actor);
            }
        }
    }

    return StructuresInBiome;
}

void UArchitecturalManager::UpdateStructuralIntegrity(AActor* StructureActor, float NewIntegrity)
{
    if (!StructureActor)
    {
        return;
    }

    for (FArch_StructureData& StructureData : ManagedStructures)
    {
        if (FVector::Dist(StructureData.Location, StructureActor->GetActorLocation()) < 100.0f)
        {
            StructureData.StructuralIntegrity = FMath::Clamp(NewIntegrity, 0.0f, 100.0f);
            
            if (StructureData.StructuralIntegrity <= 25.0f && !StructureData.bIsRuin)
            {
                ConvertToRuin(StructureActor);
            }
            break;
        }
    }
}

void UArchitecturalManager::ConvertToRuin(AActor* StructureActor)
{
    if (!StructureActor)
    {
        return;
    }

    // Update actor label to indicate ruin status
    FString CurrentLabel = StructureActor->GetActorLabel();
    if (!CurrentLabel.Contains(TEXT("Ruin")))
    {
        StructureActor->SetActorLabel(FString::Printf(TEXT("Ruin_%s"), *CurrentLabel));
    }

    // Update managed structure data
    for (FArch_StructureData& StructureData : ManagedStructures)
    {
        if (FVector::Dist(StructureData.Location, StructureActor->GetActorLocation()) < 100.0f)
        {
            StructureData.bIsRuin = true;
            StructureData.StructuralIntegrity = FMath::RandRange(5.0f, 25.0f);
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Converted structure to ruin: %s"), *StructureActor->GetActorLabel());
}

void UArchitecturalManager::SpawnRuinsInBiome(EBiomeType BiomeType, int32 RuinCount)
{
    for (int32 i = 0; i < RuinCount; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        SpawnStructureAtLocation(EArch_StructureType::Ruin, SpawnLocation, SpawnRotation, BiomeType);
    }
}

void UArchitecturalManager::SetupInteriorProps(AActor* StructureActor, EArch_StructureType StructureType)
{
    if (!StructureActor)
    {
        return;
    }

    // Add primitive artifacts and interior elements based on structure type
    switch (StructureType)
    {
        case EArch_StructureType::Dwelling:
            AddPrimitiveArtifacts(StructureActor);
            break;
        case EArch_StructureType::Temple:
            // Add ceremonial elements
            break;
        case EArch_StructureType::Ruin:
            // Add scattered debris
            break;
        default:
            break;
    }
}

void UArchitecturalManager::AddPrimitiveArtifacts(AActor* StructureActor)
{
    if (!StructureActor)
    {
        return;
    }

    // This would spawn primitive tools, fire pits, sleeping areas, etc.
    // For now, just log the action
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Added primitive artifacts to %s"), *StructureActor->GetActorLabel());
}

FVector UArchitecturalManager::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector BiomeCenter = BiomeCenters.FindRef(BiomeType);
    
    float RandomX = FMath::RandRange(-StructureSpawnRadius, StructureSpawnRadius);
    float RandomY = FMath::RandRange(-StructureSpawnRadius, StructureSpawnRadius);
    
    return FVector(BiomeCenter.X + RandomX, BiomeCenter.Y + RandomY, BiomeCenter.Z);
}

void UArchitecturalManager::DistributeStructuresEvenly()
{
    // Spawn 2-3 structures per biome for balanced distribution
    SpawnStructuresAcrossBiomes(EArch_StructureType::Pillar, 2);
    SpawnStructuresAcrossBiomes(EArch_StructureType::Ruin, 1);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Distributed structures evenly across all biomes"));
}

bool UArchitecturalManager::IsLocationValid(FVector Location, EBiomeType BiomeType)
{
    FVector BiomeCenter = BiomeCenters.FindRef(BiomeType);
    float DistanceFromCenter = FVector::Dist(Location, BiomeCenter);
    
    return DistanceFromCenter <= StructureSpawnRadius;
}

void UArchitecturalManager::RegisterStructure(AActor* StructureActor, EArch_StructureType StructureType, EBiomeType BiomeType)
{
    if (!StructureActor)
    {
        return;
    }

    FArch_StructureData NewStructure;
    NewStructure.StructureName = StructureActor->GetActorLabel();
    NewStructure.Location = StructureActor->GetActorLocation();
    NewStructure.Rotation = StructureActor->GetActorRotation();
    NewStructure.BiomeType = BiomeType;
    NewStructure.StructuralIntegrity = 100.0f;
    NewStructure.bIsRuin = (StructureType == EArch_StructureType::Ruin);

    ManagedStructures.Add(NewStructure);
}

void UArchitecturalManager::CleanupExcessStructures()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Check total actor count and remove excess if needed
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() > 20000)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Actor count exceeds limit (%d/20000), cleanup needed"), AllActors.Num());
        
        // Remove oldest managed structures if necessary
        if (ManagedStructures.Num() > MaxStructuresPerBiome * 5)
        {
            int32 StructuresToRemove = ManagedStructures.Num() - (MaxStructuresPerBiome * 5);
            ManagedStructures.RemoveAt(0, StructuresToRemove);
        }
    }
}