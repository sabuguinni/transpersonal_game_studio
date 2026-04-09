#include "PropPlacementSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

APropPlacementSystem::APropPlacementSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    PropDensity = 1.0f;
    MinDistanceBetweenProps = 200.0f;
    bEnableStorytellingClusters = true;
    StorytellingClusterChance = 0.15f;

    // Initialize prop types and storytelling clusters
    InitializePropTypes();
    InitializeStorytellingClusters();
}

void APropPlacementSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Initialized with %d prop types and %d storytelling clusters"), 
           EnvironmentProps.Num(), StorytellingClusters.Num());
}

void APropPlacementSystem::InitializePropTypes()
{
    EnvironmentProps.Empty();

    // Small Rocks - common ground scatter
    FEnvironmentPropData SmallRock;
    SmallRock.PropType = EEnvironmentPropType::Rock_Small;
    SmallRock.MinScale = 0.5f;
    SmallRock.MaxScale = 1.0f;
    SmallRock.SpawnWeight = 5.0f; // Very common
    SmallRock.StorytellingValue = 0.1f;
    EnvironmentProps.Add(SmallRock);

    // Medium Rocks - moderate scatter
    FEnvironmentPropData MediumRock;
    MediumRock.PropType = EEnvironmentPropType::Rock_Medium;
    MediumRock.MinScale = 1.0f;
    MediumRock.MaxScale = 2.0f;
    MediumRock.SpawnWeight = 2.0f;
    MediumRock.StorytellingValue = 0.3f;
    EnvironmentProps.Add(MediumRock);

    // Large Boulders - rare but impactful
    FEnvironmentPropData LargeBoulder;
    LargeBoulder.PropType = EEnvironmentPropType::Rock_Large;
    LargeBoulder.MinScale = 3.0f;
    LargeBoulder.MaxScale = 6.0f;
    LargeBoulder.SpawnWeight = 0.3f; // Rare
    LargeBoulder.StorytellingValue = 0.8f;
    EnvironmentProps.Add(LargeBoulder);

    // Fallen Logs - prehistoric wood
    FEnvironmentPropData FallenLog;
    FallenLog.PropType = EEnvironmentPropType::Log_Fallen;
    FallenLog.MinScale = 2.0f;
    FallenLog.MaxScale = 4.0f;
    FallenLog.SpawnWeight = 1.0f;
    FallenLog.StorytellingValue = 0.6f;
    EnvironmentProps.Add(FallenLog);

    // Dinosaur Skulls - high storytelling value
    FEnvironmentPropData DinosaurSkull;
    DinosaurSkull.PropType = EEnvironmentPropType::Bone_Skull;
    DinosaurSkull.MinScale = 1.5f;
    DinosaurSkull.MaxScale = 3.0f;
    DinosaurSkull.SpawnWeight = 0.1f; // Very rare
    DinosaurSkull.StorytellingValue = 1.0f; // Maximum storytelling impact
    EnvironmentProps.Add(DinosaurSkull);

    // Ribcages - medium storytelling value
    FEnvironmentPropData Ribcage;
    Ribcage.PropType = EEnvironmentPropType::Bone_Ribcage;
    Ribcage.MinScale = 2.0f;
    Ribcage.MaxScale = 4.0f;
    Ribcage.SpawnWeight = 0.2f; // Rare
    Ribcage.StorytellingValue = 0.9f;
    EnvironmentProps.Add(Ribcage);

    // Small Crystals - mysterious elements
    FEnvironmentPropData SmallCrystal;
    SmallCrystal.PropType = EEnvironmentPropType::Crystal_Small;
    SmallCrystal.MinScale = 0.3f;
    SmallCrystal.MaxScale = 0.8f;
    SmallCrystal.SpawnWeight = 0.5f;
    SmallCrystal.StorytellingValue = 0.7f;
    EnvironmentProps.Add(SmallCrystal);

    // Large Crystal Formations - rare magical elements
    FEnvironmentPropData LargeCrystal;
    LargeCrystal.PropType = EEnvironmentPropType::Crystal_Large;
    LargeCrystal.MinScale = 2.0f;
    LargeCrystal.MaxScale = 5.0f;
    LargeCrystal.SpawnWeight = 0.05f; // Extremely rare
    LargeCrystal.StorytellingValue = 1.0f;
    EnvironmentProps.Add(LargeCrystal);

    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Initialized %d environment prop types"), EnvironmentProps.Num());
}

void APropPlacementSystem::InitializeStorytellingClusters()
{
    StorytellingClusters.Empty();

    // Dinosaur Death Site
    FStorytellingCluster DeathSite;
    DeathSite.ClusterName = TEXT("Dinosaur Death Site");
    DeathSite.RequiredProps = {EEnvironmentPropType::Bone_Skull, EEnvironmentPropType::Bone_Ribcage, EEnvironmentPropType::Rock_Medium};
    DeathSite.ClusterRadius = 300.0f;
    DeathSite.StoryHint = TEXT("A mighty creature fell here long ago...");
    StorytellingClusters.Add(DeathSite);

    // Crystal Grove
    FStorytellingCluster CrystalGrove;
    CrystalGrove.ClusterName = TEXT("Crystal Grove");
    CrystalGrove.RequiredProps = {EEnvironmentPropType::Crystal_Large, EEnvironmentPropType::Crystal_Small, EEnvironmentPropType::Rock_Small};
    CrystalGrove.ClusterRadius = 400.0f;
    CrystalGrove.StoryHint = TEXT("Strange energies emanate from these formations...");
    StorytellingClusters.Add(CrystalGrove);

    // Ancient Rockfall
    FStorytellingCluster Rockfall;
    Rockfall.ClusterName = TEXT("Ancient Rockfall");
    Rockfall.RequiredProps = {EEnvironmentPropType::Rock_Large, EEnvironmentPropType::Rock_Medium, EEnvironmentPropType::Rock_Small};
    Rockfall.ClusterRadius = 600.0f;
    Rockfall.StoryHint = TEXT("A landslide from ages past blocks the path...");
    StorytellingClusters.Add(Rockfall);

    // Fallen Forest
    FStorytellingCluster FallenForest;
    FallenForest.ClusterName = TEXT("Fallen Forest");
    FallenForest.RequiredProps = {EEnvironmentPropType::Log_Fallen, EEnvironmentPropType::Rock_Medium};
    FallenForest.ClusterRadius = 500.0f;
    FallenForest.StoryHint = TEXT("Ancient trees toppled by some great force...");
    StorytellingClusters.Add(FallenForest);

    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Initialized %d storytelling clusters"), StorytellingClusters.Num());
}

void APropPlacementSystem::PlacePropsInArea(FVector Center, float Radius, const FString& BiomeType)
{
    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Placing props in radius %.2f around (%s) for biome %s"), 
           Radius, *Center.ToString(), *BiomeType);

    // Get appropriate props for this biome
    TArray<FEnvironmentPropData> BiomeProps = GetPropsForBiome(BiomeType);
    
    // Calculate number of props to place based on density and area
    float Area = PI * Radius * Radius;
    int32 BasePropsToPlace = FMath::RoundToInt((Area / 1000000.0f) * PropDensity * 10.0f); // Base density

    // Randomly decide if we should create a storytelling cluster
    bool bCreateCluster = bEnableStorytellingClusters && 
                         FMath::RandRange(0.0f, 1.0f) < StorytellingClusterChance &&
                         StorytellingClusters.Num() > 0;

    if (bCreateCluster)
    {
        // Choose random storytelling cluster
        int32 ClusterIndex = FMath::RandRange(0, StorytellingClusters.Num() - 1);
        const FStorytellingCluster& SelectedCluster = StorytellingClusters[ClusterIndex];
        
        // Find a good location for the cluster (not too close to center)
        FVector ClusterLocation = Center + FMath::RandPointInCircle(Radius * 0.7f).ToVector3D();
        CreateStorytellingCluster(ClusterLocation, SelectedCluster);
        
        UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Created storytelling cluster '%s' at %s"), 
               *SelectedCluster.ClusterName, *ClusterLocation.ToString());
    }

    // Place regular props
    int32 PropsPlaced = 0;
    int32 Attempts = 0;
    const int32 MaxAttempts = BasePropsToPlace * 3; // Prevent infinite loops

    while (PropsPlaced < BasePropsToPlace && Attempts < MaxAttempts)
    {
        Attempts++;

        // Generate random position within radius
        FVector2D RandomCircle = FMath::RandPointInCircle(Radius);
        FVector PropLocation = Center + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
        
        // Check if location is valid (not too close to other props)
        if (!IsLocationValid(PropLocation, MinDistanceBetweenProps))
        {
            continue;
        }

        // Get surface location
        FVector SurfaceLocation = GetSurfaceLocation(PropLocation);
        if (SurfaceLocation.Z < -10000.0f) // Invalid surface
        {
            continue;
        }

        // Choose random prop type based on weights
        float TotalWeight = 0.0f;
        for (const auto& PropData : BiomeProps)
        {
            TotalWeight += PropData.SpawnWeight;
        }

        float RandomWeight = FMath::RandRange(0.0f, TotalWeight);
        float CurrentWeight = 0.0f;
        
        for (const auto& PropData : BiomeProps)
        {
            CurrentWeight += PropData.SpawnWeight;
            if (RandomWeight <= CurrentWeight)
            {
                // Generate random rotation
                FRotator PropRotation = FRotator::ZeroRotator;
                if (PropData.bCanRotateRandomly)
                {
                    PropRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                }
                
                if (PropData.bAlignToSurface)
                {
                    PropRotation = GetSurfaceRotation(SurfaceLocation);
                }

                // Generate random scale
                float RandomScale = FMath::RandRange(PropData.MinScale, PropData.MaxScale);
                FVector PropScale = FVector(RandomScale);

                // Spawn the prop
                AActor* SpawnedProp = SpawnProp(PropData, SurfaceLocation, PropRotation, PropScale);
                if (SpawnedProp)
                {
                    SpawnedProps.Add(SpawnedProp);
                    PropsPlaced++;
                }
                break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Placed %d props in %d attempts"), PropsPlaced, Attempts);
}

void APropPlacementSystem::CreateStorytellingCluster(FVector Location, const FStorytellingCluster& ClusterData)
{
    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Creating storytelling cluster '%s' at %s"), 
           *ClusterData.ClusterName, *Location.ToString());

    // Place required props in a cluster pattern
    for (EEnvironmentPropType PropType : ClusterData.RequiredProps)
    {
        // Find the prop data for this type
        const FEnvironmentPropData* PropData = EnvironmentProps.FindByPredicate(
            [PropType](const FEnvironmentPropData& Data) { return Data.PropType == PropType; });

        if (!PropData)
        {
            continue;
        }

        // Place 1-3 instances of this prop type in the cluster
        int32 InstanceCount = FMath::RandRange(1, 3);
        for (int32 i = 0; i < InstanceCount; ++i)
        {
            // Generate position within cluster radius
            FVector2D RandomOffset = FMath::RandPointInCircle(ClusterData.ClusterRadius);
            FVector PropLocation = Location + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
            
            // Get surface location
            FVector SurfaceLocation = GetSurfaceLocation(PropLocation);
            if (SurfaceLocation.Z < -10000.0f)
            {
                continue;
            }

            // Generate rotation and scale
            FRotator PropRotation = FRotator::ZeroRotator;
            if (PropData->bCanRotateRandomly)
            {
                PropRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
            }

            float RandomScale = FMath::RandRange(PropData->MinScale, PropData->MaxScale);
            FVector PropScale = FVector(RandomScale);

            // Spawn the prop
            AActor* SpawnedProp = SpawnProp(*PropData, SurfaceLocation, PropRotation, PropScale);
            if (SpawnedProp)
            {
                SpawnedProps.Add(SpawnedProp);
            }
        }
    }

    // Broadcast event
    OnStorytellingClusterCreated(ClusterData.ClusterName, Location);
}

void APropPlacementSystem::ClearPropsInArea(FVector Center, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Clearing props in radius %.2f around (%s)"), 
           Radius, *Center.ToString());

    int32 PropsRemoved = 0;
    for (int32 i = SpawnedProps.Num() - 1; i >= 0; --i)
    {
        if (IsValid(SpawnedProps[i]))
        {
            float Distance = FVector::Dist(SpawnedProps[i]->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                SpawnedProps[i]->Destroy();
                SpawnedProps.RemoveAt(i);
                PropsRemoved++;
            }
        }
        else
        {
            SpawnedProps.RemoveAt(i);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Removed %d props"), PropsRemoved);
}

void APropPlacementSystem::UpdatePropDensity(float NewDensity)
{
    PropDensity = FMath::Clamp(NewDensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("PropPlacementSystem: Updated prop density to %.2f"), PropDensity);
}

AActor* APropPlacementSystem::SpawnProp(const FEnvironmentPropData& PropData, FVector Location, FRotator Rotation, FVector Scale)
{
    // Create a basic static mesh actor for the prop
    AActor* PropActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
    if (!PropActor)
    {
        return nullptr;
    }

    // Add static mesh component
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(PropActor);
    PropActor->SetRootComponent(MeshComponent);
    
    // Set scale
    PropActor->SetActorScale3D(Scale);
    
    // Set a descriptive name
    FString PropName = FString::Printf(TEXT("Prop_%s_%d"), 
                                      *UEnum::GetValueAsString(PropData.PropType), 
                                      FMath::RandRange(1000, 9999));
    PropActor->SetActorLabel(PropName);

    UE_LOG(LogTemp, VeryVerbose, TEXT("PropPlacementSystem: Spawned prop %s at %s"), 
           *PropName, *Location.ToString());

    return PropActor;
}

TArray<FEnvironmentPropData> APropPlacementSystem::GetPropsForBiome(const FString& BiomeType) const
{
    TArray<FEnvironmentPropData> BiomeProps;

    if (BiomeType == TEXT("DenseForest"))
    {
        // Dense forest: logs, rocks, some bones
        for (const auto& PropData : EnvironmentProps)
        {
            if (PropData.PropType == EEnvironmentPropType::Log_Fallen ||
                PropData.PropType == EEnvironmentPropType::Rock_Small ||
                PropData.PropType == EEnvironmentPropType::Rock_Medium ||
                PropData.PropType == EEnvironmentPropType::Bone_Skull)
            {
                BiomeProps.Add(PropData);
            }
        }
    }
    else if (BiomeType == TEXT("Wetland"))
    {
        // Wetland: mostly rocks and occasional bones
        for (const auto& PropData : EnvironmentProps)
        {
            if (PropData.PropType == EEnvironmentPropType::Rock_Small ||
                PropData.PropType == EEnvironmentPropType::Rock_Medium ||
                PropData.PropType == EEnvironmentPropType::Bone_Ribcage)
            {
                BiomeProps.Add(PropData);
            }
        }
    }
    else if (BiomeType == TEXT("Highland"))
    {
        // Highland: rocks and crystals
        for (const auto& PropData : EnvironmentProps)
        {
            if (PropData.PropType == EEnvironmentPropType::Rock_Medium ||
                PropData.PropType == EEnvironmentPropType::Rock_Large ||
                PropData.PropType == EEnvironmentPropType::Crystal_Small ||
                PropData.PropType == EEnvironmentPropType::Crystal_Large)
            {
                BiomeProps.Add(PropData);
            }
        }
    }
    else
    {
        // Default: all props
        BiomeProps = EnvironmentProps;
    }

    return BiomeProps;
}

bool APropPlacementSystem::IsLocationValid(FVector Location, float MinDistance) const
{
    for (const auto& ExistingProp : SpawnedProps)
    {
        if (IsValid(ExistingProp))
        {
            float Distance = FVector::Dist(ExistingProp->GetActorLocation(), Location);
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }
    return true;
}

FVector APropPlacementSystem::GetSurfaceLocation(FVector Location) const
{
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        return HitResult.Location;
    }
    
    return FVector(Location.X, Location.Y, -10000.0f); // Invalid marker
}

FRotator APropPlacementSystem::GetSurfaceRotation(FVector Location) const
{
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 100);
    FVector TraceEnd = Location - FVector(0, 0, 100);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        return HitResult.Normal.Rotation();
    }
    
    return FRotator::ZeroRotator;
}