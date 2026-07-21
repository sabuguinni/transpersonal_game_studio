#include "EnvArt_CretaceousEnvironmentSpawner.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/ExponentialHeightFog.h"

AEnvArt_CretaceousEnvironmentSpawner::AEnvArt_CretaceousEnvironmentSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default values
    SpawnRadius = 5000.0f;
    MaxPropsPerBiome = 25;
    bEnableAtmosphericFog = true;
    FogDensity = 0.02f;
    FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    
    // Initialize default biome spawn points
    FEnvArt_BiomeSpawnData ForestBiome;
    ForestBiome.SpawnLocation = FVector(50000.0f, 50000.0f, 100.0f);
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.PropDensity = 15;
    BiomeSpawnPoints.Add(ForestBiome);
    
    FEnvArt_BiomeSpawnData VolcanicBiome;
    VolcanicBiome.SpawnLocation = FVector(75000.0f, 75000.0f, 200.0f);
    VolcanicBiome.BiomeType = EBiomeType::Volcanic;
    VolcanicBiome.PropDensity = 8;
    BiomeSpawnPoints.Add(VolcanicBiome);
    
    InitializeDefaultProps();
}

void AEnvArt_CretaceousEnvironmentSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-spawn props on begin play if enabled
    if (BiomeSpawnPoints.Num() > 0)
    {
        SpawnAllBiomeProps();
    }
    
    // Create atmospheric fog if enabled
    if (bEnableAtmosphericFog)
    {
        CreateAtmosphericFog(GetActorLocation());
    }
}

void AEnvArt_CretaceousEnvironmentSpawner::InitializeDefaultProps()
{
    CretaceousProps.Empty();
    
    // Cretaceous fallen log
    FEnvArt_CretaceousPropData FallenLog;
    FallenLog.PropName = TEXT("CretaceousFallenLog");
    FallenLog.ScaleRange = FVector(0.8f, 1.5f, 1.0f);
    FallenLog.SpawnWeight = 3.0f;
    CretaceousProps.Add(FallenLog);
    
    // Cretaceous boulder
    FEnvArt_CretaceousPropData Boulder;
    Boulder.PropName = TEXT("CretaceousBoulder");
    Boulder.ScaleRange = FVector(0.6f, 2.0f, 1.0f);
    Boulder.SpawnWeight = 2.0f;
    CretaceousProps.Add(Boulder);
    
    // Cretaceous fern cluster
    FEnvArt_CretaceousPropData FernCluster;
    FernCluster.PropName = TEXT("CretaceousFerns");
    FernCluster.ScaleRange = FVector(0.5f, 1.2f, 1.0f);
    FernCluster.SpawnWeight = 4.0f;
    CretaceousProps.Add(FernCluster);
    
    // Volcanic rock formation
    FEnvArt_CretaceousPropData VolcanicRock;
    VolcanicRock.PropName = TEXT("VolcanicRockFormation");
    VolcanicRock.ScaleRange = FVector(0.7f, 1.8f, 1.0f);
    VolcanicRock.SpawnWeight = 2.5f;
    CretaceousProps.Add(VolcanicRock);
}

void AEnvArt_CretaceousEnvironmentSpawner::SpawnCretaceousPropsAtBiome(const FEnvArt_BiomeSpawnData& BiomeData)
{
    if (CretaceousProps.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Cretaceous props defined for spawning"));
        return;
    }
    
    int32 PropsToSpawn = FMath::Min(BiomeData.PropDensity, MaxPropsPerBiome);
    
    for (int32 i = 0; i < PropsToSpawn; i++)
    {
        // Select random prop based on weight
        float TotalWeight = 0.0f;
        for (const FEnvArt_CretaceousPropData& Prop : CretaceousProps)
        {
            TotalWeight += Prop.SpawnWeight;
        }
        
        float RandomWeight = FMath::FRandRange(0.0f, TotalWeight);
        float CurrentWeight = 0.0f;
        
        for (const FEnvArt_CretaceousPropData& Prop : CretaceousProps)
        {
            CurrentWeight += Prop.SpawnWeight;
            if (RandomWeight <= CurrentWeight)
            {
                // Spawn this prop
                FVector SpawnLocation = GetRandomLocationInRadius(BiomeData.SpawnLocation, SpawnRadius);
                FRotator SpawnRotation = GetRandomRotation();
                
                AActor* SpawnedProp = SpawnPropAtLocation(Prop, SpawnLocation, SpawnRotation);
                if (SpawnedProp)
                {
                    SpawnedProps.Add(SpawnedProp);
                    UE_LOG(LogTemp, Log, TEXT("Spawned %s at %s"), *Prop.PropName, *SpawnLocation.ToString());
                }
                break;
            }
        }
    }
}

void AEnvArt_CretaceousEnvironmentSpawner::SpawnAllBiomeProps()
{
    for (const FEnvArt_BiomeSpawnData& BiomeData : BiomeSpawnPoints)
    {
        SpawnCretaceousPropsAtBiome(BiomeData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d total Cretaceous props across %d biomes"), 
           SpawnedProps.Num(), BiomeSpawnPoints.Num());
}

AActor* AEnvArt_CretaceousEnvironmentSpawner::SpawnPropAtLocation(const FEnvArt_CretaceousPropData& PropData, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn static mesh actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    if (PropActor)
    {
        // Set actor label for identification
        PropActor->SetActorLabel(PropData.PropName);
        
        // Apply random scale within range
        float RandomScale = FMath::FRandRange(PropData.ScaleRange.X, PropData.ScaleRange.Y);
        PropActor->SetActorScale3D(FVector(RandomScale));
        
        // Configure static mesh component
        UStaticMeshComponent* MeshComp = PropActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Enable collision for gameplay interaction
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            
            // Set up for potential mesh assignment
            if (PropData.PropMesh.IsValid())
            {
                MeshComp->SetStaticMesh(PropData.PropMesh.Get());
            }
        }
        
        return PropActor;
    }
    
    return nullptr;
}

void AEnvArt_CretaceousEnvironmentSpawner::CreateAtmosphericFog(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Check if fog already exists
    if (AtmosphericFogActor)
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AtmosphericFogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (AtmosphericFogActor)
    {
        AtmosphericFogActor->SetActorLabel(TEXT("CretaceousAtmosphericFog"));
        
        // Configure fog properties
        UExponentialHeightFogComponent* FogComponent = AtmosphericFogActor->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(FogDensity);
            FogComponent->SetFogInscatteringColor(FogColor);
            FogComponent->SetFogMaxOpacity(0.8f);
            FogComponent->SetStartDistance(1000.0f);
            FogComponent->SetFogHeightFalloff(0.1f);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Created atmospheric fog at %s"), *Location.ToString());
    }
}

FVector AEnvArt_CretaceousEnvironmentSpawner::GetRandomLocationInRadius(const FVector& Center, float Radius) const
{
    FVector2D RandomCircle = FMath::RandPointInCircle(Radius);
    return Center + FVector(RandomCircle.X, RandomCircle.Y, FMath::FRandRange(-100.0f, 100.0f));
}

FRotator AEnvArt_CretaceousEnvironmentSpawner::GetRandomRotation() const
{
    return FRotator(
        FMath::FRandRange(-15.0f, 15.0f),  // Pitch
        FMath::FRandRange(0.0f, 360.0f),   // Yaw
        FMath::FRandRange(-10.0f, 10.0f)   // Roll
    );
}

void AEnvArt_CretaceousEnvironmentSpawner::SpawnBiomePropsEditor()
{
    // Clear existing spawned props
    for (AActor* Actor : SpawnedProps)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedProps.Empty();
    
    // Respawn all props
    SpawnAllBiomeProps();
    
    UE_LOG(LogTemp, Log, TEXT("Editor spawn complete: %d props spawned"), SpawnedProps.Num());
}