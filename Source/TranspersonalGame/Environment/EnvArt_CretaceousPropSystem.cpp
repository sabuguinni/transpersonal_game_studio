#include "EnvArt_CretaceousPropSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UEnvArt_CretaceousPropSystem::UEnvArt_CretaceousPropSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Performance settings for Cretaceous environment
    MaxPropsPerBiome = 1000;
    PropCullingDistance = 5000.0f;
    bEnableLODSystem = true;
    
    SetupDefaultCretaceousProps();
}

void UEnvArt_CretaceousPropSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCretaceousProps();
}

void UEnvArt_CretaceousPropSystem::SetupDefaultCretaceousProps()
{
    // Fallen logs - tell stories of ancient forests
    FEnvArt_CretaceousPropData FallenLog;
    FallenLog.PropName = TEXT("FallenLog");
    FallenLog.MinScale = FVector(2.0f, 0.8f, 0.8f);
    FallenLog.MaxScale = FVector(4.0f, 1.2f, 1.2f);
    FallenLog.SpawnProbability = 0.3f;
    FallenLog.MaxInstancesPerBiome = 25;
    FallenLog.MinDistanceBetween = 800.0f;
    FallenLog.bAllowRotationVariation = true;
    FallenLog.bSnapToGround = true;
    CretaceousProps.Add(TEXT("FallenLog"), FallenLog);
    
    // Boulder clusters - geological storytelling
    FEnvArt_CretaceousPropData Boulder;
    Boulder.PropName = TEXT("CretaceousBoulder");
    Boulder.MinScale = FVector(1.5f, 1.5f, 1.5f);
    Boulder.MaxScale = FVector(3.0f, 3.0f, 2.5f);
    Boulder.SpawnProbability = 0.4f;
    Boulder.MaxInstancesPerBiome = 40;
    Boulder.MinDistanceBetween = 600.0f;
    Boulder.bAllowRotationVariation = true;
    Boulder.bSnapToGround = true;
    CretaceousProps.Add(TEXT("Boulder"), Boulder);
    
    // Fern clusters - Cretaceous undergrowth
    FEnvArt_CretaceousPropData FernCluster;
    FernCluster.PropName = TEXT("CretaceousFern");
    FernCluster.MinScale = FVector(0.8f, 0.8f, 0.8f);
    FernCluster.MaxScale = FVector(1.5f, 1.5f, 1.8f);
    FernCluster.SpawnProbability = 0.7f;
    FernCluster.MaxInstancesPerBiome = 150;
    FernCluster.MinDistanceBetween = 300.0f;
    FernCluster.bAllowRotationVariation = true;
    FernCluster.bSnapToGround = true;
    CretaceousProps.Add(TEXT("Fern"), FernCluster);
    
    // Cycad plants - distinctive Cretaceous flora
    FEnvArt_CretaceousPropData Cycad;
    Cycad.PropName = TEXT("CretaceousCycad");
    Cycad.MinScale = FVector(1.0f, 1.0f, 1.0f);
    Cycad.MaxScale = FVector(1.8f, 1.8f, 2.2f);
    Cycad.SpawnProbability = 0.5f;
    Cycad.MaxInstancesPerBiome = 80;
    Cycad.MinDistanceBetween = 500.0f;
    Cycad.bAllowRotationVariation = true;
    Cycad.bSnapToGround = true;
    CretaceousProps.Add(TEXT("Cycad"), Cycad);
    
    // Conifer stumps - evidence of ancient forests
    FEnvArt_CretaceousPropData Stump;
    Stump.PropName = TEXT("ConiferStump");
    Stump.MinScale = FVector(1.2f, 1.2f, 0.5f);
    Stump.MaxScale = FVector(2.0f, 2.0f, 1.0f);
    Stump.SpawnProbability = 0.2f;
    Stump.MaxInstancesPerBiome = 15;
    Stump.MinDistanceBetween = 1200.0f;
    Stump.bAllowRotationVariation = true;
    Stump.bSnapToGround = true;
    CretaceousProps.Add(TEXT("Stump"), Stump);
}

void UEnvArt_CretaceousPropSystem::InitializeCretaceousProps()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Cretaceous Prop System with %d prop types"), CretaceousProps.Num());
    
    // Clear any existing props
    for (AActor* PropActor : SpawnedProps)
    {
        if (IsValid(PropActor))
        {
            PropActor->Destroy();
        }
    }
    SpawnedProps.Empty();
    ActiveClusters.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous Prop System initialized successfully"));
}

bool UEnvArt_CretaceousPropSystem::CreatePropCluster(const FVector& Location, float Radius, EBiomeType BiomeType, int32 PropCount)
{
    if (PropCount <= 0 || Radius <= 0.0f)
    {
        return false;
    }
    
    // Check if we're within performance limits
    if (GetTotalPropCount() + PropCount > MaxPropsPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create prop cluster - would exceed performance limit"));
        return false;
    }
    
    // Create cluster data
    FEnvArt_PropClusterData NewCluster;
    NewCluster.ClusterCenter = Location;
    NewCluster.ClusterRadius = Radius;
    NewCluster.BiomeType = BiomeType;
    NewCluster.PropsInCluster = 0;
    
    // Get appropriate props for this biome
    TArray<FString> BiomeProps = GetPropsForBiome(BiomeType);
    if (BiomeProps.Num() == 0)
    {
        return false;
    }
    
    // Spawn props in cluster
    for (int32 i = 0; i < PropCount; ++i)
    {
        // Random position within cluster radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius);
        FVector PropLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Select random prop type for this biome
        FString PropType = BiomeProps[FMath::RandRange(0, BiomeProps.Num() - 1)];
        
        if (IsValidPropLocation(PropLocation, PropType))
        {
            FRotator PropRotation = GetRandomRotation(true);
            AActor* NewProp = SpawnEnvironmentalProp(PropType, PropLocation, PropRotation);
            if (NewProp)
            {
                NewCluster.PropsInCluster++;
                NewCluster.PropTypes.AddUnique(PropType);
            }
        }
    }
    
    if (NewCluster.PropsInCluster > 0)
    {
        ActiveClusters.Add(NewCluster);
        UE_LOG(LogTemp, Log, TEXT("Created prop cluster with %d props at %s"), NewCluster.PropsInCluster, *Location.ToString());
        return true;
    }
    
    return false;
}

AActor* UEnvArt_CretaceousPropSystem::SpawnEnvironmentalProp(const FString& PropType, const FVector& Location, const FRotator& Rotation)
{
    if (!CretaceousProps.Contains(PropType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown prop type: %s"), *PropType);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    const FEnvArt_CretaceousPropData& PropData = CretaceousProps[PropType];
    
    // Spawn static mesh actor
    AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation);
    if (!PropActor)
    {
        return nullptr;
    }
    
    // Set actor label for identification
    PropActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *PropData.PropName, SpawnedProps.Num()));
    
    // Apply scale variation
    FVector Scale = CalculateScaleVariation(PropData);
    PropActor->SetActorScale3D(Scale);
    
    // Snap to ground if required
    if (PropData.bSnapToGround)
    {
        FHitResult HitResult;
        FVector StartTrace = Location + FVector(0, 0, 1000);
        FVector EndTrace = Location - FVector(0, 0, 1000);
        
        if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic))
        {
            PropActor->SetActorLocation(HitResult.Location);
        }
    }
    
    SpawnedProps.Add(PropActor);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %s prop at %s"), *PropType, *Location.ToString());
    return PropActor;
}

void UEnvArt_CretaceousPropSystem::OptimizePropDistribution()
{
    // Remove props that exceed limits
    while (SpawnedProps.Num() > MaxPropsPerBiome)
    {
        AActor* OldestProp = SpawnedProps[0];
        if (IsValid(OldestProp))
        {
            OldestProp->Destroy();
        }
        SpawnedProps.RemoveAt(0);
    }
    
    // Clean up invalid references
    SpawnedProps.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    UE_LOG(LogTemp, Log, TEXT("Optimized prop distribution - %d props remaining"), SpawnedProps.Num());
}

TArray<FString> UEnvArt_CretaceousPropSystem::GetPropsForBiome(EBiomeType BiomeType) const
{
    TArray<FString> BiomeProps;
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            BiomeProps.Add(TEXT("FallenLog"));
            BiomeProps.Add(TEXT("Fern"));
            BiomeProps.Add(TEXT("Cycad"));
            BiomeProps.Add(TEXT("Stump"));
            break;
            
        case EBiomeType::Plains:
            BiomeProps.Add(TEXT("Boulder"));
            BiomeProps.Add(TEXT("Fern"));
            break;
            
        case EBiomeType::Desert:
            BiomeProps.Add(TEXT("Boulder"));
            BiomeProps.Add(TEXT("Stump"));
            break;
            
        case EBiomeType::Swamp:
            BiomeProps.Add(TEXT("FallenLog"));
            BiomeProps.Add(TEXT("Fern"));
            BiomeProps.Add(TEXT("Cycad"));
            break;
            
        case EBiomeType::Mountain:
            BiomeProps.Add(TEXT("Boulder"));
            BiomeProps.Add(TEXT("Stump"));
            break;
            
        default:
            BiomeProps.Add(TEXT("Boulder"));
            break;
    }
    
    return BiomeProps;
}

bool UEnvArt_CretaceousPropSystem::IsValidPropLocation(const FVector& Location, const FString& PropType) const
{
    if (!CretaceousProps.Contains(PropType))
    {
        return false;
    }
    
    const FEnvArt_CretaceousPropData& PropData = CretaceousProps[PropType];
    
    // Check minimum distance to other props of same type
    for (const AActor* ExistingProp : SpawnedProps)
    {
        if (IsValid(ExistingProp) && ExistingProp->GetActorLabel().Contains(PropData.PropName))
        {
            float Distance = FVector::Dist(Location, ExistingProp->GetActorLocation());
            if (Distance < PropData.MinDistanceBetween)
            {
                return false;
            }
        }
    }
    
    return ValidateTerrainPlacement(Location);
}

void UEnvArt_CretaceousPropSystem::UpdatePropLOD(const FVector& PlayerLocation)
{
    if (!bEnableLODSystem)
    {
        return;
    }
    
    for (AActor* PropActor : SpawnedProps)
    {
        if (!IsValid(PropActor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, PropActor->GetActorLocation());
        bool bShouldBeVisible = Distance <= PropCullingDistance;
        
        PropActor->SetActorHiddenInGame(!bShouldBeVisible);
    }
}

int32 UEnvArt_CretaceousPropSystem::GetTotalPropCount() const
{
    return SpawnedProps.Num();
}

void UEnvArt_CretaceousPropSystem::ClearBiomeProps(EBiomeType BiomeType)
{
    TArray<FString> BiomeProps = GetPropsForBiome(BiomeType);
    
    for (int32 i = SpawnedProps.Num() - 1; i >= 0; --i)
    {
        AActor* PropActor = SpawnedProps[i];
        if (IsValid(PropActor))
        {
            FString ActorLabel = PropActor->GetActorLabel();
            for (const FString& PropType : BiomeProps)
            {
                if (ActorLabel.Contains(PropType))
                {
                    PropActor->Destroy();
                    SpawnedProps.RemoveAt(i);
                    break;
                }
            }
        }
    }
    
    // Remove clusters for this biome
    ActiveClusters.RemoveAll([BiomeType](const FEnvArt_PropClusterData& Cluster) {
        return Cluster.BiomeType == BiomeType;
    });
}

void UEnvArt_CretaceousPropSystem::CreateStorytellingScene(const FVector& Location, const FString& SceneType)
{
    if (SceneType == TEXT("AbandonedCamp"))
    {
        // Create a scene that tells the story of an abandoned campsite
        SpawnEnvironmentalProp(TEXT("FallenLog"), Location, FRotator(0, 45, 0));
        SpawnEnvironmentalProp(TEXT("Boulder"), Location + FVector(200, 0, 0), FRotator::ZeroRotator);
        SpawnEnvironmentalProp(TEXT("Stump"), Location + FVector(-150, 100, 0), FRotator::ZeroRotator);
    }
    else if (SceneType == TEXT("DinosaurNest"))
    {
        // Create a scene suggesting dinosaur nesting area
        CreatePropCluster(Location, 800.0f, EBiomeType::Forest, 8);
        SpawnEnvironmentalProp(TEXT("Boulder"), Location, FRotator::ZeroRotator);
    }
    else if (SceneType == TEXT("AncientGrove"))
    {
        // Create an atmospheric ancient grove
        for (int32 i = 0; i < 5; ++i)
        {
            FVector StumpLocation = Location + FVector(
                FMath::RandRange(-500, 500),
                FMath::RandRange(-500, 500),
                0
            );
            SpawnEnvironmentalProp(TEXT("Stump"), StumpLocation, GetRandomRotation(true));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created storytelling scene '%s' at %s"), *SceneType, *Location.ToString());
}

bool UEnvArt_CretaceousPropSystem::ValidateTerrainPlacement(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Simple terrain validation - check if location is above ground
    FHitResult HitResult;
    FVector StartTrace = Location + FVector(0, 0, 1000);
    FVector EndTrace = Location - FVector(0, 0, 1000);
    
    return World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic);
}

FVector UEnvArt_CretaceousPropSystem::CalculateScaleVariation(const FEnvArt_CretaceousPropData& PropData) const
{
    return FVector(
        FMath::RandRange(PropData.MinScale.X, PropData.MaxScale.X),
        FMath::RandRange(PropData.MinScale.Y, PropData.MaxScale.Y),
        FMath::RandRange(PropData.MinScale.Z, PropData.MaxScale.Z)
    );
}

FRotator UEnvArt_CretaceousPropSystem::GetRandomRotation(bool bAllowVariation) const
{
    if (!bAllowVariation)
    {
        return FRotator::ZeroRotator;
    }
    
    return FRotator(
        FMath::RandRange(-15.0f, 15.0f),  // Slight pitch variation
        FMath::RandRange(0.0f, 360.0f),   // Full yaw rotation
        FMath::RandRange(-10.0f, 10.0f)   // Slight roll variation
    );
}