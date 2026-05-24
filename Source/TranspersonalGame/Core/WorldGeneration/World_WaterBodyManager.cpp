#include "World_WaterBodyManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_WaterBodyManager::AWorld_WaterBodyManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxWaterBodies = 50;
    MinWaterBodyDistance = 500.0f;
    WorldBounds = FVector(10000.0f, 10000.0f, 2000.0f);

    // Initialize water mesh reference
    InitializeWaterMesh();
}

void AWorld_WaterBodyManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: BeginPlay - Ready to generate water bodies"));
}

void AWorld_WaterBodyManager::InitializeWaterMesh()
{
    // Try to load basic cube mesh for water bodies
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        WaterMesh = CubeMeshAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterBodyManager: Failed to load cube mesh"));
    }
}

void AWorld_WaterBodyManager::GenerateWaterBodies()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("WaterBodyManager: No valid world found"));
        return;
    }

    // Clear existing water bodies
    ClearWaterBodies();

    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Generating %d water bodies"), MaxWaterBodies);

    // Generate central lake
    FWorld_WaterBodyData LakeData;
    LakeData.WaterType = EWorld_WaterBodyType::Lake;
    LakeData.Location = FVector(0.0f, 0.0f, -50.0f);
    LakeData.Scale = FVector(40.0f, 30.0f, 2.0f);
    LakeData.Depth = 200.0f;
    LakeData.bIsSwimmable = true;
    WaterBodies.Add(LakeData);
    CreateWaterBody(LakeData);

    // Generate river system
    CreateRiverSystem(FVector(-1000.0f, 2000.0f, -30.0f), FVector(1000.0f, -1500.0f, -30.0f), 6);

    // Generate ponds in different biome areas
    TArray<FVector> PondLocations = {
        FVector(2000.0f, 1500.0f, -20.0f),   // Forest area
        FVector(-2500.0f, -1000.0f, -20.0f), // Swamp area
        FVector(1800.0f, -2000.0f, -20.0f),  // Savana area
        FVector(-1800.0f, 1800.0f, -20.0f),  // Mountain area
        FVector(2200.0f, -800.0f, -20.0f)    // Desert oasis
    };

    for (const FVector& Location : PondLocations)
    {
        FWorld_WaterBodyData PondData;
        PondData.WaterType = EWorld_WaterBodyType::Pond;
        PondData.Location = Location;
        PondData.Scale = FVector(15.0f, 15.0f, 1.0f);
        PondData.Depth = 100.0f;
        PondData.bIsSwimmable = true;
        WaterBodies.Add(PondData);
        CreateWaterBody(PondData);
    }

    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Generated %d water bodies"), WaterBodies.Num());
}

void AWorld_WaterBodyManager::ClearWaterBodies()
{
    // Destroy all spawned water actors
    for (AActor* WaterActor : SpawnedWaterActors)
    {
        if (IsValid(WaterActor))
        {
            WaterActor->Destroy();
        }
    }
    
    SpawnedWaterActors.Empty();
    WaterBodies.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Cleared all water bodies"));
}

AActor* AWorld_WaterBodyManager::CreateWaterBody(const FWorld_WaterBodyData& WaterData)
{
    if (!GetWorld() || !WaterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterBodyManager: Cannot create water body - missing world or mesh"));
        return nullptr;
    }

    // Spawn static mesh actor for water body
    AStaticMeshActor* WaterActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        WaterData.Location,
        FRotator::ZeroRotator
    );

    if (!WaterActor)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterBodyManager: Failed to spawn water actor"));
        return nullptr;
    }

    // Configure static mesh component
    UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetStaticMesh(WaterMesh);
        WaterActor->SetActorScale3D(WaterData.Scale);
        
        // Set water material if available
        if (WaterMaterial)
        {
            MeshComp->SetMaterial(0, WaterMaterial);
        }
    }

    // Set actor label based on water type
    FString WaterTypeName;
    switch (WaterData.WaterType)
    {
        case EWorld_WaterBodyType::Lake:
            WaterTypeName = TEXT("Lake");
            break;
        case EWorld_WaterBodyType::River:
            WaterTypeName = TEXT("River");
            break;
        case EWorld_WaterBodyType::Pond:
            WaterTypeName = TEXT("Pond");
            break;
        case EWorld_WaterBodyType::Stream:
            WaterTypeName = TEXT("Stream");
            break;
        case EWorld_WaterBodyType::Waterfall:
            WaterTypeName = TEXT("Waterfall");
            break;
    }
    
    WaterActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *WaterTypeName, SpawnedWaterActors.Num()));

    // Add to spawned actors list
    SpawnedWaterActors.Add(WaterActor);

    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Created %s at %s"), *WaterTypeName, *WaterData.Location.ToString());
    return WaterActor;
}

void AWorld_WaterBodyManager::CreateRiverSystem(const FVector& StartLocation, const FVector& EndLocation, int32 Segments)
{
    if (Segments <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterBodyManager: Invalid segment count for river system"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Creating river system with %d segments"), Segments);

    // Calculate river path with some randomness for natural curves
    for (int32 i = 0; i < Segments; ++i)
    {
        float Alpha = static_cast<float>(i) / static_cast<float>(Segments - 1);
        FVector BaseLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
        
        // Add some random offset for natural curves
        FVector RandomOffset = FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );
        
        FVector SegmentLocation = BaseLocation + RandomOffset;
        
        FWorld_WaterBodyData RiverData;
        RiverData.WaterType = EWorld_WaterBodyType::River;
        RiverData.Location = SegmentLocation;
        RiverData.Scale = FVector(50.0f, 5.0f, 1.0f);
        RiverData.Depth = 50.0f;
        RiverData.bIsSwimmable = true;
        RiverData.CurrentStrength = 1.0f;
        
        WaterBodies.Add(RiverData);
        CreateWaterBody(RiverData);
    }
}

void AWorld_WaterBodyManager::CreateLake(const FVector& Location, float Radius)
{
    FWorld_WaterBodyData LakeData;
    LakeData.WaterType = EWorld_WaterBodyType::Lake;
    LakeData.Location = Location;
    LakeData.Scale = FVector(Radius / 50.0f, Radius / 50.0f, 2.0f); // Convert radius to scale
    LakeData.Depth = 300.0f;
    LakeData.bIsSwimmable = true;
    
    WaterBodies.Add(LakeData);
    CreateWaterBody(LakeData);
    
    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Created lake at %s with radius %f"), *Location.ToString(), Radius);
}

TArray<AActor*> AWorld_WaterBodyManager::GetWaterActorsInRadius(const FVector& Location, float Radius) const
{
    TArray<AActor*> NearbyWaterActors;
    
    for (AActor* WaterActor : SpawnedWaterActors)
    {
        if (IsValid(WaterActor))
        {
            float Distance = FVector::Dist(WaterActor->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                NearbyWaterActors.Add(WaterActor);
            }
        }
    }
    
    return NearbyWaterActors;
}

bool AWorld_WaterBodyManager::IsLocationNearWater(const FVector& Location, float CheckRadius) const
{
    TArray<AActor*> NearbyWater = GetWaterActorsInRadius(Location, CheckRadius);
    return NearbyWater.Num() > 0;
}

void AWorld_WaterBodyManager::RegenerateAllWaterBodies()
{
    UE_LOG(LogTemp, Log, TEXT("WaterBodyManager: Regenerating all water bodies"));
    GenerateWaterBodies();
}

FVector AWorld_WaterBodyManager::GetRandomLocationInBounds() const
{
    return FVector(
        FMath::RandRange(-WorldBounds.X, WorldBounds.X),
        FMath::RandRange(-WorldBounds.Y, WorldBounds.Y),
        FMath::RandRange(-100.0f, 0.0f) // Water bodies slightly below ground
    );
}

bool AWorld_WaterBodyManager::IsValidWaterLocation(const FVector& Location) const
{
    // Check if location is too close to existing water bodies
    for (const FWorld_WaterBodyData& ExistingWater : WaterBodies)
    {
        float Distance = FVector::Dist(ExistingWater.Location, Location);
        if (Distance < MinWaterBodyDistance)
        {
            return false;
        }
    }
    
    return true;
}