#include "EnvArt_CretaceousRockSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

UEnvArt_CretaceousRockSystem::UEnvArt_CretaceousRockSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    MaxRocksPerBiome = 200.0f;
    MinDistanceBetweenRocks = 150.0f;
    bAutoGenerateOnBeginPlay = false;
    
    InitializeRockVariations();
}

void UEnvArt_CretaceousRockSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateOnBeginPlay)
    {
        // Generate rocks for all biome types
        GenerateRocksForBiome(EBiomeType::Temperate, FVector(0, 0, 100), 10000.0f);
        GenerateRocksForBiome(EBiomeType::Desert, FVector(20000, 0, 100), 8000.0f);
        GenerateRocksForBiome(EBiomeType::Tropical, FVector(-20000, 0, 100), 12000.0f);
        GenerateRocksForBiome(EBiomeType::Volcanic, FVector(0, 20000, 100), 6000.0f);
        GenerateRocksForBiome(EBiomeType::Swamp, FVector(0, -20000, 100), 9000.0f);
    }
}

void UEnvArt_CretaceousRockSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic validation of rock distribution
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;
    
    if (ValidationTimer >= 30.0f) // Validate every 30 seconds
    {
        ValidateRockDistribution();
        ValidationTimer = 0.0f;
    }
}

void UEnvArt_CretaceousRockSystem::GenerateRocksForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("UEnvArt_CretaceousRockSystem: No valid world for rock generation"));
        return;
    }
    
    int32 RocksToGenerate = FMath::RandRange(50, FMath::FloorToInt(MaxRocksPerBiome * 0.5f));
    int32 ClustersToCreate = FMath::RandRange(3, 8);
    
    UE_LOG(LogTemp, Log, TEXT("Generating %d rocks in %d clusters for biome %d"), RocksToGenerate, ClustersToCreate, (int32)BiomeType);
    
    // Create rock clusters
    for (int32 i = 0; i < ClustersToCreate; i++)
    {
        FVector ClusterCenter = GetRandomLocationInRadius(BiomeCenter, BiomeRadius * 0.8f);
        float ClusterRadius = FMath::RandRange(300.0f, 800.0f);
        int32 RocksInCluster = FMath::RandRange(5, 15);
        
        CreateRockCluster(ClusterCenter, ClusterRadius, RocksInCluster, BiomeType);
    }
    
    // Generate scattered individual rocks
    int32 ScatteredRocks = RocksToGenerate - (ClustersToCreate * 10);
    for (int32 i = 0; i < ScatteredRocks; i++)
    {
        FVector RockLocation = GetRandomLocationInRadius(BiomeCenter, BiomeRadius);
        
        if (IsValidRockLocation(RockLocation, MinDistanceBetweenRocks))
        {
            FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
            FEnvArt_RockVariation SelectedVariation = SelectRandomRockVariation();
            
            AStaticMeshActor* NewRock = SpawnSingleRock(RockLocation, RandomRotation, SelectedVariation);
            if (NewRock)
            {
                ApplyBiomeSpecificMaterial(NewRock, BiomeType);
            }
        }
    }
}

void UEnvArt_CretaceousRockSystem::CreateRockCluster(const FVector& ClusterCenter, float Radius, int32 RockCount, EBiomeType BiomeType)
{
    FEnvArt_RockCluster NewCluster;
    NewCluster.CenterLocation = ClusterCenter;
    NewCluster.ClusterRadius = Radius;
    NewCluster.RockCount = RockCount;
    NewCluster.BiomeType = BiomeType;
    
    ActiveClusters.Add(NewCluster);
    
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RockLocation = GetRandomLocationInRadius(ClusterCenter, Radius);
        FRotator RandomRotation = FRotator(
            FMath::RandRange(-15.0f, 15.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );
        
        FEnvArt_RockVariation SelectedVariation = SelectRandomRockVariation();
        
        AStaticMeshActor* NewRock = SpawnSingleRock(RockLocation, RandomRotation, SelectedVariation);
        if (NewRock)
        {
            ApplyBiomeSpecificMaterial(NewRock, BiomeType);
            
            // Add cluster-specific properties
            NewRock->SetActorLabel(FString::Printf(TEXT("ClusterRock_%s_%d"), *UEnum::GetValueAsString(BiomeType), i));
        }
    }
}

AStaticMeshActor* UEnvArt_CretaceousRockSystem::SpawnSingleRock(const FVector& Location, const FRotator& Rotation, const FEnvArt_RockVariation& Variation)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Check total actor count limit
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() >= 8000)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor limit reached (%d) - not spawning more rocks"), AllActors.Num());
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* NewRock = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    
    if (NewRock)
    {
        // Apply random scale variation
        FVector RandomScale = FVector(
            FMath::RandRange(Variation.MinScale.X, Variation.MaxScale.X),
            FMath::RandRange(Variation.MinScale.Y, Variation.MaxScale.Y),
            FMath::RandRange(Variation.MinScale.Z, Variation.MaxScale.Z)
        );
        NewRock->SetActorScale3D(RandomScale);
        
        // Set rock name and properties
        NewRock->SetActorLabel(FString::Printf(TEXT("CretaceousRock_%s"), *Variation.RockName));
        
        // Enable collision for gameplay interaction
        if (UStaticMeshComponent* MeshComp = NewRock->GetStaticMeshComponent())
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        }
        
        SpawnedRocks.Add(NewRock);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned rock '%s' at location %s"), *Variation.RockName, *Location.ToString());
    }
    
    return NewRock;
}

void UEnvArt_CretaceousRockSystem::ClearAllRocks()
{
    for (AStaticMeshActor* Rock : SpawnedRocks)
    {
        if (IsValid(Rock))
        {
            Rock->Destroy();
        }
    }
    
    SpawnedRocks.Empty();
    ActiveClusters.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Cleared all rocks from Cretaceous rock system"));
}

int32 UEnvArt_CretaceousRockSystem::GetTotalRockCount() const
{
    int32 ValidRockCount = 0;
    for (const AStaticMeshActor* Rock : SpawnedRocks)
    {
        if (IsValid(Rock))
        {
            ValidRockCount++;
        }
    }
    return ValidRockCount;
}

void UEnvArt_CretaceousRockSystem::ValidateRockDistribution()
{
    // Remove invalid rock references
    SpawnedRocks.RemoveAll([](const AStaticMeshActor* Rock) {
        return !IsValid(Rock);
    });
    
    int32 CurrentRockCount = GetTotalRockCount();
    UE_LOG(LogTemp, Log, TEXT("Rock validation: %d valid rocks, %d clusters"), CurrentRockCount, ActiveClusters.Num());
    
    // Check if we need to clean up excess rocks due to actor limits
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() > 7500) // Start cleanup before hitting hard limit
    {
        int32 RocksToRemove = FMath::Min(SpawnedRocks.Num() / 4, 50); // Remove 25% or max 50 rocks
        
        for (int32 i = 0; i < RocksToRemove && SpawnedRocks.Num() > 0; i++)
        {
            int32 RandomIndex = FMath::RandRange(0, SpawnedRocks.Num() - 1);
            if (IsValid(SpawnedRocks[RandomIndex]))
            {
                SpawnedRocks[RandomIndex]->Destroy();
            }
            SpawnedRocks.RemoveAt(RandomIndex);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Removed %d rocks due to actor count limit"), RocksToRemove);
    }
}

void UEnvArt_CretaceousRockSystem::InitializeRockVariations()
{
    RockVariations.Empty();
    
    // Limestone formations
    FEnvArt_RockVariation Limestone;
    Limestone.RockName = TEXT("Limestone");
    Limestone.MinScale = FVector(0.6f, 0.6f, 0.4f);
    Limestone.MaxScale = FVector(1.4f, 1.4f, 1.0f);
    Limestone.SpawnWeight = 3.0f;
    Limestone.bCanCluster = true;
    RockVariations.Add(Limestone);
    
    // Sandstone outcrops
    FEnvArt_RockVariation Sandstone;
    Sandstone.RockName = TEXT("Sandstone");
    Sandstone.MinScale = FVector(0.8f, 0.8f, 0.6f);
    Sandstone.MaxScale = FVector(1.6f, 1.6f, 1.2f);
    Sandstone.SpawnWeight = 2.5f;
    Sandstone.bCanCluster = true;
    RockVariations.Add(Sandstone);
    
    // Volcanic basalt
    FEnvArt_RockVariation Basalt;
    Basalt.RockName = TEXT("Basalt");
    Basalt.MinScale = FVector(0.5f, 0.5f, 0.8f);
    Basalt.MaxScale = FVector(1.2f, 1.2f, 2.0f);
    Basalt.SpawnWeight = 1.5f;
    Basalt.bCanCluster = false;
    RockVariations.Add(Basalt);
    
    // Granite boulders
    FEnvArt_RockVariation Granite;
    Granite.RockName = TEXT("Granite");
    Granite.MinScale = FVector(1.0f, 1.0f, 1.0f);
    Granite.MaxScale = FVector(2.0f, 2.0f, 1.8f);
    Granite.SpawnWeight = 1.0f;
    Granite.bCanCluster = false;
    RockVariations.Add(Granite);
    
    // Shale formations
    FEnvArt_RockVariation Shale;
    Shale.RockName = TEXT("Shale");
    Shale.MinScale = FVector(0.4f, 0.4f, 0.2f);
    Shale.MaxScale = FVector(1.0f, 1.0f, 0.6f);
    Shale.SpawnWeight = 2.0f;
    Shale.bCanCluster = true;
    RockVariations.Add(Shale);
}

FEnvArt_RockVariation UEnvArt_CretaceousRockSystem::SelectRandomRockVariation() const
{
    if (RockVariations.Num() == 0)
    {
        return FEnvArt_RockVariation(); // Return default if no variations
    }
    
    float TotalWeight = 0.0f;
    for (const FEnvArt_RockVariation& Variation : RockVariations)
    {
        TotalWeight += Variation.SpawnWeight;
    }
    
    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const FEnvArt_RockVariation& Variation : RockVariations)
    {
        CurrentWeight += Variation.SpawnWeight;
        if (RandomValue <= CurrentWeight)
        {
            return Variation;
        }
    }
    
    return RockVariations[0]; // Fallback to first variation
}

bool UEnvArt_CretaceousRockSystem::IsValidRockLocation(const FVector& Location, float MinDistance) const
{
    for (const AStaticMeshActor* ExistingRock : SpawnedRocks)
    {
        if (IsValid(ExistingRock))
        {
            float Distance = FVector::Dist(Location, ExistingRock->GetActorLocation());
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }
    return true;
}

FVector UEnvArt_CretaceousRockSystem::GetRandomLocationInRadius(const FVector& Center, float Radius) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        FMath::RandRange(-50.0f, 50.0f) // Small height variation
    );
    
    return Center + Offset;
}

void UEnvArt_CretaceousRockSystem::ApplyBiomeSpecificMaterial(AStaticMeshActor* RockActor, EBiomeType BiomeType)
{
    if (!IsValid(RockActor))
    {
        return;
    }
    
    // This would normally load biome-specific materials
    // For now, just set appropriate tags for material assignment
    switch (BiomeType)
    {
        case EBiomeType::Desert:
            RockActor->Tags.Add(TEXT("DesertRock"));
            break;
        case EBiomeType::Tropical:
            RockActor->Tags.Add(TEXT("TropicalRock"));
            break;
        case EBiomeType::Volcanic:
            RockActor->Tags.Add(TEXT("VolcanicRock"));
            break;
        case EBiomeType::Swamp:
            RockActor->Tags.Add(TEXT("SwampRock"));
            break;
        default:
            RockActor->Tags.Add(TEXT("TemperateRock"));
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied biome material tag for %s rock"), *UEnum::GetValueAsString(BiomeType));
}