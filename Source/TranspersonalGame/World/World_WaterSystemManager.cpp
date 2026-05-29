#include "World_WaterSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AWorld_WaterSystemManager::AWorld_WaterSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    GlobalWaterLevel = 50.0f;
    WaterFlowSpeed = 1.0f;
    bEnableWaterPhysics = true;
}

void AWorld_WaterSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (WaterBodies.Num() == 0)
    {
        GenerateWaterBodies();
    }
    
    if (RiverSegments.Num() == 0)
    {
        CreateRiverSystem();
    }
}

void AWorld_WaterSystemManager::GenerateWaterBodies()
{
    // Create forest lake
    FWorld_WaterBody ForestLake;
    ForestLake.Location = FVector(-45000.0f, 40000.0f, GlobalWaterLevel);
    ForestLake.Scale = FVector(50.0f, 50.0f, 1.0f);
    ForestLake.BiomeType = EBiomeType::Forest;
    ForestLake.WaterDepth = 200.0f;
    ForestLake.bIsRiver = false;
    WaterBodies.Add(ForestLake);

    // Create desert oasis
    FWorld_WaterBody DesertOasis;
    DesertOasis.Location = FVector(50000.0f, -40000.0f, GlobalWaterLevel);
    DesertOasis.Scale = FVector(20.0f, 20.0f, 1.0f);
    DesertOasis.BiomeType = EBiomeType::Desert;
    DesertOasis.WaterDepth = 150.0f;
    DesertOasis.bIsRiver = false;
    WaterBodies.Add(DesertOasis);

    // Create grassland pond
    FWorld_WaterBody GrasslandPond;
    GrasslandPond.Location = FVector(0.0f, 0.0f, GlobalWaterLevel);
    GrasslandPond.Scale = FVector(30.0f, 30.0f, 1.0f);
    GrasslandPond.BiomeType = EBiomeType::Grassland;
    GrasslandPond.WaterDepth = 100.0f;
    GrasslandPond.bIsRiver = false;
    WaterBodies.Add(GrasslandPond);

    // Spawn water meshes for each body
    for (const FWorld_WaterBody& WaterBody : WaterBodies)
    {
        SpawnWaterMeshAtLocation(WaterBody);
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated %d water bodies"), WaterBodies.Num());
}

void AWorld_WaterSystemManager::CreateRiverSystem()
{
    // Create river connecting forest to grassland
    FWorld_RiverSegment ForestToGrassland;
    ForestToGrassland.StartPoint = FVector(-30000.0f, 30000.0f, 45.0f);
    ForestToGrassland.EndPoint = FVector(-15000.0f, 15000.0f, 40.0f);
    ForestToGrassland.Width = 1000.0f;
    ForestToGrassland.FlowRate = 1.5f;
    RiverSegments.Add(ForestToGrassland);

    // Create river connecting grassland to desert
    FWorld_RiverSegment GrasslandToDesert;
    GrasslandToDesert.StartPoint = FVector(15000.0f, -15000.0f, 35.0f);
    GrasslandToDesert.EndPoint = FVector(30000.0f, -30000.0f, 30.0f);
    GrasslandToDesert.Width = 800.0f;
    GrasslandToDesert.FlowRate = 1.0f;
    RiverSegments.Add(GrasslandToDesert);

    // Create river meshes
    for (const FWorld_RiverSegment& RiverSegment : RiverSegments)
    {
        CreateRiverMesh(RiverSegment);
    }

    UE_LOG(LogTemp, Warning, TEXT("Created river system with %d segments"), RiverSegments.Num());
}

void AWorld_WaterSystemManager::UpdateWaterLevels()
{
    for (FWorld_WaterBody& WaterBody : WaterBodies)
    {
        WaterBody.Location.Z = GlobalWaterLevel;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Updated water levels to %f"), GlobalWaterLevel);
}

FWorld_WaterBody AWorld_WaterSystemManager::CreateLakeAtLocation(FVector Location, EBiomeType BiomeType, float Size)
{
    FWorld_WaterBody NewLake;
    NewLake.Location = Location;
    NewLake.Scale = FVector(Size, Size, 1.0f);
    NewLake.BiomeType = BiomeType;
    NewLake.WaterDepth = Size * 2.0f;
    NewLake.bIsRiver = false;
    
    WaterBodies.Add(NewLake);
    SpawnWaterMeshAtLocation(NewLake);
    
    return NewLake;
}

void AWorld_WaterSystemManager::ConnectBiomesWithRiver(FVector StartBiome, FVector EndBiome)
{
    FWorld_RiverSegment NewRiver;
    NewRiver.StartPoint = StartBiome;
    NewRiver.EndPoint = EndBiome;
    NewRiver.Width = 500.0f;
    NewRiver.FlowRate = 1.0f;
    
    RiverSegments.Add(NewRiver);
    CreateRiverMesh(NewRiver);
}

bool AWorld_WaterSystemManager::IsLocationNearWater(FVector Location, float Radius) const
{
    for (const FWorld_WaterBody& WaterBody : WaterBodies)
    {
        float Distance = FVector::Dist(Location, WaterBody.Location);
        float WaterRadius = FMath::Max(WaterBody.Scale.X, WaterBody.Scale.Y) * 100.0f;
        if (Distance <= (WaterRadius + Radius))
        {
            return true;
        }
    }
    return false;
}

float AWorld_WaterSystemManager::GetWaterDepthAtLocation(FVector Location) const
{
    for (const FWorld_WaterBody& WaterBody : WaterBodies)
    {
        float Distance = FVector::Dist(Location, WaterBody.Location);
        float WaterRadius = FMath::Max(WaterBody.Scale.X, WaterBody.Scale.Y) * 100.0f;
        if (Distance <= WaterRadius)
        {
            return WaterBody.WaterDepth;
        }
    }
    return 0.0f;
}

void AWorld_WaterSystemManager::SpawnWaterMeshAtLocation(const FWorld_WaterBody& WaterBody)
{
    if (UWorld* World = GetWorld())
    {
        AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            WaterBody.Location,
            FRotator::ZeroRotator
        );
        
        if (WaterActor)
        {
            WaterActor->SetActorScale3D(WaterBody.Scale);
            
            FString BiomeName = UEnum::GetValueAsString(WaterBody.BiomeType);
            FString WaterLabel = FString::Printf(TEXT("Water_%s_%s"), 
                WaterBody.bIsRiver ? TEXT("River") : TEXT("Lake"), 
                *BiomeName);
            WaterActor->SetActorLabel(WaterLabel);
            
            if (UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent())
            {
                SetupWaterMaterial(MeshComp);
            }
        }
    }
}

void AWorld_WaterSystemManager::CreateRiverMesh(const FWorld_RiverSegment& RiverSegment)
{
    if (UWorld* World = GetWorld())
    {
        FVector MidPoint = (RiverSegment.StartPoint + RiverSegment.EndPoint) * 0.5f;
        FVector Direction = (RiverSegment.EndPoint - RiverSegment.StartPoint).GetSafeNormal();
        FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        
        AStaticMeshActor* RiverActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            MidPoint,
            Rotation
        );
        
        if (RiverActor)
        {
            float Length = FVector::Dist(RiverSegment.StartPoint, RiverSegment.EndPoint);
            FVector RiverScale = FVector(Length / 100.0f, RiverSegment.Width / 100.0f, 0.5f);
            RiverActor->SetActorScale3D(RiverScale);
            RiverActor->SetActorLabel(TEXT("River_Segment"));
            
            if (UStaticMeshComponent* MeshComp = RiverActor->GetStaticMeshComponent())
            {
                SetupWaterMaterial(MeshComp);
            }
        }
    }
}

void AWorld_WaterSystemManager::SetupWaterMaterial(UStaticMeshComponent* WaterMesh)
{
    if (WaterMesh)
    {
        // Material setup would be done here in a full implementation
        // For now, just ensure the mesh component is valid
        UE_LOG(LogTemp, Log, TEXT("Water material setup for mesh component"));
    }
}