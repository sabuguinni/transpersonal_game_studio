#include "World_WaterSystemManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AWorld_WaterSystemManager::AWorld_WaterSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    WaterLevel = 0.0f;
    bEnableWaterPhysics = true;
    WaterDensity = 1000.0f;
    WaterViscosity = 0.001f;
    WaterPlaneMesh = nullptr;
    WaterMaterial = nullptr;
}

void AWorld_WaterSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate initial water systems for biomes
    GenerateBiomeWaterSystems();
}

void AWorld_WaterSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableWaterPhysics)
    {
        UpdateWaterFlow(DeltaTime);
        ApplyWaterPhysics();
    }
}

void AWorld_WaterSystemManager::CreateWaterBody(const FWorld_WaterBodyData& WaterData)
{
    WaterBodies.Add(WaterData);
    SpawnWaterMesh(WaterData);
    
    UE_LOG(LogTemp, Log, TEXT("Created water body of type %d at location %s"), 
           (int32)WaterData.WaterType, *WaterData.Location.ToString());
}

void AWorld_WaterSystemManager::RemoveWaterBody(int32 Index)
{
    if (WaterBodies.IsValidIndex(Index))
    {
        WaterBodies.RemoveAt(Index);
        
        if (WaterMeshComponents.IsValidIndex(Index) && WaterMeshComponents[Index])
        {
            WaterMeshComponents[Index]->DestroyComponent();
            WaterMeshComponents.RemoveAt(Index);
        }
    }
}

void AWorld_WaterSystemManager::UpdateWaterLevel(float NewLevel)
{
    WaterLevel = NewLevel;
    
    // Update all water mesh components
    for (UStaticMeshComponent* WaterMesh : WaterMeshComponents)
    {
        if (WaterMesh)
        {
            FVector CurrentLocation = WaterMesh->GetComponentLocation();
            CurrentLocation.Z = WaterLevel;
            WaterMesh->SetWorldLocation(CurrentLocation);
        }
    }
}

bool AWorld_WaterSystemManager::IsLocationInWater(const FVector& Location) const
{
    for (const FWorld_WaterBodyData& WaterBody : WaterBodies)
    {
        FVector Distance = Location - WaterBody.Location;
        float ScaledDistanceX = FMath::Abs(Distance.X) / (WaterBody.Scale.X * 100.0f);
        float ScaledDistanceY = FMath::Abs(Distance.Y) / (WaterBody.Scale.Y * 100.0f);
        
        if (ScaledDistanceX <= 1.0f && ScaledDistanceY <= 1.0f && 
            Location.Z <= WaterBody.Location.Z && Location.Z >= (WaterBody.Location.Z - WaterBody.Depth))
        {
            return true;
        }
    }
    return false;
}

float AWorld_WaterSystemManager::GetWaterDepthAtLocation(const FVector& Location) const
{
    for (const FWorld_WaterBodyData& WaterBody : WaterBodies)
    {
        FVector Distance = Location - WaterBody.Location;
        float ScaledDistanceX = FMath::Abs(Distance.X) / (WaterBody.Scale.X * 100.0f);
        float ScaledDistanceY = FMath::Abs(Distance.Y) / (WaterBody.Scale.Y * 100.0f);
        
        if (ScaledDistanceX <= 1.0f && ScaledDistanceY <= 1.0f)
        {
            return FMath::Max(0.0f, WaterBody.Location.Z - Location.Z);
        }
    }
    return 0.0f;
}

FVector AWorld_WaterSystemManager::GetWaterFlowAtLocation(const FVector& Location) const
{
    for (const FWorld_WaterBodyData& WaterBody : WaterBodies)
    {
        if (WaterBody.bHasCurrentFlow)
        {
            FVector Distance = Location - WaterBody.Location;
            float ScaledDistanceX = FMath::Abs(Distance.X) / (WaterBody.Scale.X * 100.0f);
            float ScaledDistanceY = FMath::Abs(Distance.Y) / (WaterBody.Scale.Y * 100.0f);
            
            if (ScaledDistanceX <= 1.0f && ScaledDistanceY <= 1.0f)
            {
                return WaterBody.FlowDirection * 100.0f; // Scale flow velocity
            }
        }
    }
    return FVector::ZeroVector;
}

void AWorld_WaterSystemManager::GenerateBiomeWaterSystems()
{
    // Create water systems for each biome
    TArray<TPair<FString, FVector>> BiomeLocations = {
        {"Savana", FVector(0, 0, 50)},
        {"Pantano", FVector(-50000, -45000, 30)},
        {"Floresta", FVector(-45000, 40000, 80)},
        {"Deserto", FVector(55000, 0, 20)},
        {"Montanha", FVector(40000, 50000, 150)}
    };
    
    for (const auto& Biome : BiomeLocations)
    {
        FString BiomeName = Biome.Key;
        FVector BiomeCenter = Biome.Value;
        
        if (BiomeName == "Pantano")
        {
            // Swamp - multiple small water bodies
            for (int32 i = 0; i < 5; i++)
            {
                FWorld_WaterBodyData SwampWater;
                SwampWater.WaterType = EWorld_WaterType::Swamp;
                SwampWater.Location = BiomeCenter + FVector(
                    FMath::RandRange(-8000, 8000),
                    FMath::RandRange(-8000, 8000),
                    0
                );
                SwampWater.Scale = FVector(8.0f, 8.0f, 1.0f);
                SwampWater.Depth = 50.0f;
                SwampWater.bHasCurrentFlow = false;
                CreateWaterBody(SwampWater);
            }
        }
        else if (BiomeName == "Floresta")
        {
            // Forest - river system
            CreateRiverSystem(
                BiomeCenter + FVector(-10000, -10000, 0),
                BiomeCenter + FVector(10000, 10000, 0),
                800.0f
            );
            
            // Forest lake
            CreateLakeSystem(BiomeCenter + FVector(5000, -5000, 0), 1500.0f);
        }
        else if (BiomeName == "Montanha")
        {
            // Mountain - high altitude lake
            CreateLakeSystem(BiomeCenter + FVector(0, 0, 200), 2500.0f);
        }
        else if (BiomeName == "Savana")
        {
            // Savanna - watering hole
            CreateLakeSystem(BiomeCenter + FVector(3000, 3000, 0), 1000.0f);
        }
        else if (BiomeName == "Deserto")
        {
            // Desert - small oasis
            CreateLakeSystem(BiomeCenter + FVector(2000, -2000, 0), 600.0f);
        }
    }
}

void AWorld_WaterSystemManager::CreateRiverSystem(const FVector& StartLocation, const FVector& EndLocation, float Width)
{
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    float Distance = FVector::Dist(StartLocation, EndLocation);
    int32 Segments = FMath::CeilToInt(Distance / 1000.0f); // 1km segments
    
    for (int32 i = 0; i < Segments; i++)
    {
        float Alpha = (float)i / (float)(Segments - 1);
        FVector SegmentLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
        
        FWorld_WaterBodyData RiverSegment;
        RiverSegment.WaterType = EWorld_WaterType::River;
        RiverSegment.Location = SegmentLocation;
        RiverSegment.Scale = FVector(Width / 100.0f, 10.0f, 1.0f);
        RiverSegment.Depth = 200.0f;
        RiverSegment.bHasCurrentFlow = true;
        RiverSegment.FlowDirection = Direction;
        
        CreateWaterBody(RiverSegment);
    }
}

void AWorld_WaterSystemManager::CreateLakeSystem(const FVector& CenterLocation, float Radius)
{
    FWorld_WaterBodyData Lake;
    Lake.WaterType = EWorld_WaterType::Lake;
    Lake.Location = CenterLocation;
    Lake.Scale = FVector(Radius / 100.0f, Radius / 100.0f, 1.0f);
    Lake.Depth = 300.0f;
    Lake.bHasCurrentFlow = false;
    
    CreateWaterBody(Lake);
}

void AWorld_WaterSystemManager::SpawnWaterMesh(const FWorld_WaterBodyData& WaterData)
{
    if (!WaterPlaneMesh)
    {
        // Try to load default plane mesh
        WaterPlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
    }
    
    if (WaterPlaneMesh)
    {
        UStaticMeshComponent* WaterMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("WaterMesh_%d"), WaterMeshComponents.Num())
        );
        
        if (WaterMeshComp)
        {
            WaterMeshComp->SetStaticMesh(WaterPlaneMesh);
            WaterMeshComp->SetWorldLocation(WaterData.Location);
            WaterMeshComp->SetWorldScale3D(WaterData.Scale);
            WaterMeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
            
            if (WaterMaterial)
            {
                WaterMeshComp->SetMaterial(0, WaterMaterial);
            }
            
            WaterMeshComponents.Add(WaterMeshComp);
        }
    }
}

void AWorld_WaterSystemManager::UpdateWaterFlow(float DeltaTime)
{
    // Update water flow animations and effects
    for (const FWorld_WaterBodyData& WaterBody : WaterBodies)
    {
        if (WaterBody.bHasCurrentFlow)
        {
            // Implement flow simulation logic here
            // This could include particle effects, mesh deformation, etc.
        }
    }
}

void AWorld_WaterSystemManager::ApplyWaterPhysics()
{
    // Apply water physics to actors in water
    if (UWorld* World = GetWorld())
    {
        // This would integrate with the physics system to apply buoyancy,
        // drag, and other water effects to actors
    }
}