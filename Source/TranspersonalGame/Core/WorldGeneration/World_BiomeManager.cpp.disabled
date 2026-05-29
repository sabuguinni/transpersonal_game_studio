#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default biome definitions
    InitializeDefaultBiomes();
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoPopulateOnBeginPlay)
    {
        // Start population with a small delay to ensure world is fully loaded
        GetWorld()->GetTimerManager().SetTimer(
            PopulationTimerHandle,
            this,
            &AWorld_BiomeManager::PopulationTick,
            PopulationTickInterval,
            true,
            1.0f // Initial delay
        );
    }
}

void AWorld_BiomeManager::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana biome
    FWorld_BiomeDefinition Savana;
    Savana.BiomeType = EBiomeType::Savana;
    Savana.CenterLocation = FVector(0, 0, 100);
    Savana.Radius = 10000.0f;
    Savana.TargetActorCount = 500;
    Savana.VegetationDensity = 0.6f;
    Savana.RockDensity = 0.3f;
    Savana.DinosaurDensity = 0.15f;
    Savana.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"),
        TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops"),
        TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin")
    };
    BiomeDefinitions.Add(Savana);
    
    // Floresta biome
    FWorld_BiomeDefinition Floresta;
    Floresta.BiomeType = EBiomeType::Floresta;
    Floresta.CenterLocation = FVector(-45000, 40000, 100);
    Floresta.Radius = 8000.0f;
    Floresta.TargetActorCount = 600;
    Floresta.VegetationDensity = 0.9f;
    Floresta.RockDensity = 0.2f;
    Floresta.DinosaurDensity = 0.1f;
    Floresta.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus"),
        TEXT("/Game/Dinosaur_Pack/Parasaurolophus/Mesh/SKM_Parasaurolophus_Mesh")
    };
    BiomeDefinitions.Add(Floresta);
    
    // Deserto biome
    FWorld_BiomeDefinition Deserto;
    Deserto.BiomeType = EBiomeType::Deserto;
    Deserto.CenterLocation = FVector(55000, 0, 100);
    Deserto.Radius = 12000.0f;
    Deserto.TargetActorCount = 400;
    Deserto.VegetationDensity = 0.2f;
    Deserto.RockDensity = 0.6f;
    Deserto.DinosaurDensity = 0.08f;
    Deserto.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh"),
        TEXT("/Game/Dinosaur_Pack/Protoceratops/Mesh/SKM_Protoceratops_Skin")
    };
    BiomeDefinitions.Add(Deserto);
    
    // Pantano biome
    FWorld_BiomeDefinition Pantano;
    Pantano.BiomeType = EBiomeType::Pantano;
    Pantano.CenterLocation = FVector(-50000, -45000, 100);
    Pantano.Radius = 6000.0f;
    Pantano.TargetActorCount = 450;
    Pantano.VegetationDensity = 0.8f;
    Pantano.RockDensity = 0.4f;
    Pantano.DinosaurDensity = 0.12f;
    Pantano.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Tsintaosaurus/Mesh/SKM_Tsintaosaurus_Mesh"),
        TEXT("/Game/Dinosaur_Pack/Pachycephalo/Mesh/SKM_Pachycephalo")
    };
    BiomeDefinitions.Add(Pantano);
    
    // Montanha biome
    FWorld_BiomeDefinition Montanha;
    Montanha.BiomeType = EBiomeType::Montanha;
    Montanha.CenterLocation = FVector(40000, 50000, 200);
    Montanha.Radius = 7000.0f;
    Montanha.TargetActorCount = 350;
    Montanha.VegetationDensity = 0.4f;
    Montanha.RockDensity = 0.8f;
    Montanha.DinosaurDensity = 0.06f;
    Montanha.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh")
    };
    BiomeDefinitions.Add(Montanha);
}

void AWorld_BiomeManager::PopulateAllBiomes()
{
    for (const FWorld_BiomeDefinition& BiomeDefinition : BiomeDefinitions)
    {
        PopulateBiome(BiomeDefinition);
    }
    UpdateBiomeStatistics();
}

void AWorld_BiomeManager::PopulateBiome(const FWorld_BiomeDefinition& BiomeDefinition)
{
    int32 CurrentActorCount = GetActorCountInBiome(BiomeDefinition.BiomeType);
    int32 ActorsToSpawn = FMath::Max(0, BiomeDefinition.TargetActorCount - CurrentActorCount);
    
    if (ActorsToSpawn == 0)
    {
        return;
    }
    
    // Calculate spawn counts for each category
    int32 VegetationToSpawn = FMath::RoundToInt(ActorsToSpawn * BiomeDefinition.VegetationDensity);
    int32 RocksToSpawn = FMath::RoundToInt(ActorsToSpawn * BiomeDefinition.RockDensity);
    int32 DinosaursToSpawn = FMath::RoundToInt(ActorsToSpawn * BiomeDefinition.DinosaurDensity);
    
    // Spawn dinosaurs (highest priority)
    for (int32 i = 0; i < DinosaursToSpawn && i < BiomeDefinition.DinosaurAssets.Num(); ++i)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeDefinition);
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        if (IsLocationValidForSpawn(SpawnLocation))
        {
            AActor* SpawnedActor = SpawnActorInBiome(
                BiomeDefinition.DinosaurAssets[i % BiomeDefinition.DinosaurAssets.Num()],
                SpawnLocation,
                SpawnRotation
            );
            
            if (SpawnedActor)
            {
                FString BiomeName = UEnum::GetValueAsString(BiomeDefinition.BiomeType);
                BiomeName = BiomeName.Replace(TEXT("EBiomeType::"), TEXT(""));
                SpawnedActor->SetActorLabel(FString::Printf(TEXT("Dino_%s_%d"), *BiomeName, TotalSpawnedActors));
                TotalSpawnedActors++;
            }
        }
    }
}

void AWorld_BiomeManager::ClearBiome(EBiomeType BiomeType)
{
    TArray<AActor*> ActorsInBiome = GetActorsInBiome(BiomeType);
    
    for (AActor* Actor : ActorsInBiome)
    {
        if (IsValid(Actor) && !Actor->IsA<APawn>()) // Don't destroy player or important pawns
        {
            Actor->Destroy();
        }
    }
    
    UpdateBiomeStatistics();
}

void AWorld_BiomeManager::ClearAllBiomes()
{
    for (const FWorld_BiomeDefinition& BiomeDefinition : BiomeDefinitions)
    {
        ClearBiome(BiomeDefinition.BiomeType);
    }
}

int32 AWorld_BiomeManager::GetActorCountInBiome(EBiomeType BiomeType) const
{
    return ActorsPerBiome.Contains(BiomeType) ? ActorsPerBiome[BiomeType] : 0;
}

TArray<AActor*> AWorld_BiomeManager::GetActorsInBiome(EBiomeType BiomeType) const
{
    TArray<AActor*> Result;
    
    // Find biome definition
    const FWorld_BiomeDefinition* BiomeDefinition = nullptr;
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            BiomeDefinition = &Biome;
            break;
        }
    }
    
    if (!BiomeDefinition)
    {
        return Result;
    }
    
    // Get all actors in world
    UWorld* World = GetWorld();
    if (!World)
    {
        return Result;
    }
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (IsValid(Actor))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), BiomeDefinition->CenterLocation);
            if (Distance <= BiomeDefinition->Radius)
            {
                Result.Add(Actor);
            }
        }
    }
    
    return Result;
}

void AWorld_BiomeManager::ValidateBiomeAssets()
{
    for (FWorld_BiomeDefinition& BiomeDefinition : BiomeDefinitions)
    {
        // Validate dinosaur assets
        for (int32 i = BiomeDefinition.DinosaurAssets.Num() - 1; i >= 0; --i)
        {
            const FString& AssetPath = BiomeDefinition.DinosaurAssets[i];
            UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
            if (!Asset)
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid dinosaur asset: %s"), *AssetPath);
                BiomeDefinition.DinosaurAssets.RemoveAt(i);
            }
        }
    }
}

void AWorld_BiomeManager::PopulationTick()
{
    if (CurrentBiomeIndex >= BiomeDefinitions.Num())
    {
        // Population complete
        GetWorld()->GetTimerManager().ClearTimer(PopulationTimerHandle);
        UpdateBiomeStatistics();
        UE_LOG(LogTemp, Log, TEXT("Biome population completed. Total actors: %d"), TotalSpawnedActors);
        return;
    }
    
    const FWorld_BiomeDefinition& CurrentBiome = BiomeDefinitions[CurrentBiomeIndex];
    int32 CurrentActorCount = GetActorCountInBiome(CurrentBiome.BiomeType);
    
    if (CurrentActorCount >= CurrentBiome.TargetActorCount)
    {
        // Move to next biome
        CurrentBiomeIndex++;
        CurrentActorIndex = 0;
        return;
    }
    
    // Spawn a batch of actors
    for (int32 i = 0; i < MaxActorsPerFrame; ++i)
    {
        if (CurrentActorCount + i >= CurrentBiome.TargetActorCount)
        {
            break;
        }
        
        // Spawn dinosaur if available
        if (CurrentActorIndex < CurrentBiome.DinosaurAssets.Num())
        {
            FVector SpawnLocation = GetRandomLocationInBiome(CurrentBiome);
            FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
            
            if (IsLocationValidForSpawn(SpawnLocation))
            {
                AActor* SpawnedActor = SpawnActorInBiome(
                    CurrentBiome.DinosaurAssets[CurrentActorIndex % CurrentBiome.DinosaurAssets.Num()],
                    SpawnLocation,
                    SpawnRotation
                );
                
                if (SpawnedActor)
                {
                    FString BiomeName = UEnum::GetValueAsString(CurrentBiome.BiomeType);
                    BiomeName = BiomeName.Replace(TEXT("EBiomeType::"), TEXT(""));
                    SpawnedActor->SetActorLabel(FString::Printf(TEXT("Dino_%s_%d"), *BiomeName, TotalSpawnedActors));
                    TotalSpawnedActors++;
                }
            }
        }
        
        CurrentActorIndex++;
    }
}

AActor* AWorld_BiomeManager::SpawnActorInBiome(const FString& AssetPath, const FVector& Location, const FRotator& Rotation)
{
    UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
    if (!Asset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load asset: %s"), *AssetPath);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Try to spawn as static mesh actor
    if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
    {
        AStaticMeshActor* StaticMeshActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (StaticMeshActor && StaticMeshActor->GetStaticMeshComponent())
        {
            StaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
            return StaticMeshActor;
        }
    }
    
    // Try to spawn as skeletal mesh actor (for dinosaurs)
    if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Asset))
    {
        // Create a basic pawn for skeletal meshes
        APawn* SkeletalMeshPawn = World->SpawnActor<APawn>(Location, Rotation);
        if (SkeletalMeshPawn)
        {
            USkeletalMeshComponent* SkeletalMeshComponent = NewObject<USkeletalMeshComponent>(SkeletalMeshPawn);
            SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
            SkeletalMeshPawn->SetRootComponent(SkeletalMeshComponent);
            SkeletalMeshComponent->AttachToComponent(SkeletalMeshPawn->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            return SkeletalMeshPawn;
        }
    }
    
    return nullptr;
}

FVector AWorld_BiomeManager::GetRandomLocationInBiome(const FWorld_BiomeDefinition& BiomeDefinition) const
{
    // Generate random point within circle
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeDefinition.Radius * 0.9f); // Stay within 90% of radius
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        0.0f
    );
    
    return BiomeDefinition.CenterLocation + Offset;
}

bool AWorld_BiomeManager::IsLocationValidForSpawn(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Simple validation - check if location is not too close to other actors
    FVector Start = Location + FVector(0, 0, 1000);
    FVector End = Location - FVector(0, 0, 1000);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    // Trace to find ground
    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        return true; // Found ground
    }
    
    return false; // No ground found
}

void AWorld_BiomeManager::UpdateBiomeStatistics()
{
    ActorsPerBiome.Empty();
    
    for (const FWorld_BiomeDefinition& BiomeDefinition : BiomeDefinitions)
    {
        int32 ActorCount = 0;
        UWorld* World = GetWorld();
        
        if (World)
        {
            for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
            {
                AActor* Actor = *ActorIterator;
                if (IsValid(Actor))
                {
                    float Distance = FVector::Dist(Actor->GetActorLocation(), BiomeDefinition.CenterLocation);
                    if (Distance <= BiomeDefinition.Radius)
                    {
                        ActorCount++;
                    }
                }
            }
        }
        
        ActorsPerBiome.Add(BiomeDefinition.BiomeType, ActorCount);
    }
}