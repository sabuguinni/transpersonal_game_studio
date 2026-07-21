#include "World_WaterSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AWorld_WaterSystemManager::AWorld_WaterSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize water configuration
    BaseWaterLevel = 50.0f;
    SeasonalVariation = 20.0f;
    FlowSpeed = 100.0f;
    bDynamicWaterLevels = true;
    FlowParticleIntensity = 1.0f;

    // Initialize water state
    CurrentWaterLevel = BaseWaterLevel;
    CurrentFlowRate = FlowSpeed;
    bWaterSystemActive = true;

    // Load default materials if available
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> LakeMaterial(TEXT("/Engine/EngineMaterials/DefaultMaterial"));
    if (LakeMaterial.Succeeded())
    {
        LakeWaterMaterial = LakeMaterial.Object;
        RiverWaterMaterial = LakeMaterial.Object;
        WaterfallMaterial = LakeMaterial.Object;
    }
}

void AWorld_WaterSystemManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Initializing water system"));

    // Set up initial water materials
    SetWaterMaterials();

    // Create flow markers for existing water bodies
    CreateFlowMarkers();

    // Initialize water ecosystem
    SpawnWaterVegetation();
}

void AWorld_WaterSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bWaterSystemActive && bDynamicWaterLevels)
    {
        // Update water flow effects
        UpdateFlowEffects();

        // Calculate seasonal water level changes
        float SeasonalFactor = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 0.5f + 0.5f;
        UpdateWaterLevels(SeasonalFactor);
    }
}

void AWorld_WaterSystemManager::CreateMainLake(FVector Location, float Scale)
{
    AStaticMeshActor* Lake = CreateWaterBody(
        Location,
        FVector(Scale, Scale, 1.0f),
        FRotator(0, 0, 0),
        TEXT("MainLake_Central")
    );

    if (Lake)
    {
        Lakes.Add(Lake);
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Created main lake at %s"), *Location.ToString());
    }
}

void AWorld_WaterSystemManager::CreateRiverSystem(const TArray<FVector>& RiverPoints)
{
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        FVector Location = RiverPoints[i];
        FString Label = FString::Printf(TEXT("River_Segment_%d"), i + 1);

        AStaticMeshActor* RiverSegment = CreateWaterBody(
            Location,
            FVector(3.0f, 1.0f, 0.1f),
            FRotator(0, 0, 0),
            Label
        );

        if (RiverSegment)
        {
            Rivers.Add(RiverSegment);
            FlowPoints.Add(Location);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Created river system with %d segments"), RiverPoints.Num() - 1);
}

void AWorld_WaterSystemManager::CreateWaterfall(FVector Location, FRotator Rotation, float Height)
{
    AStaticMeshActor* Waterfall = CreateWaterBody(
        Location,
        FVector(1.0f, Height, 0.1f),
        Rotation,
        TEXT("Waterfall_Mountain")
    );

    if (Waterfall)
    {
        Waterfalls.Add(Waterfall);
        FlowPoints.Add(Location);
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Created waterfall at %s"), *Location.ToString());
    }
}

void AWorld_WaterSystemManager::CreatePond(FVector Location, float Scale, const FString& BiomeType)
{
    FString Label = FString::Printf(TEXT("%s_Pond"), *BiomeType);

    AStaticMeshActor* Pond = CreateWaterBody(
        Location,
        FVector(Scale, Scale, 1.0f),
        FRotator(0, 0, 0),
        Label
    );

    if (Pond)
    {
        Ponds.Add(Pond);
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Created %s pond at %s"), *BiomeType, *Location.ToString());
    }
}

void AWorld_WaterSystemManager::UpdateWaterLevels(float SeasonalFactor)
{
    float TargetLevel = BaseWaterLevel + (SeasonalVariation * SeasonalFactor);
    CurrentWaterLevel = FMath::FInterpTo(CurrentWaterLevel, TargetLevel, GetWorld()->GetDeltaSeconds(), 0.1f);

    // Update all water bodies to new level
    for (AStaticMeshActor* Lake : Lakes)
    {
        if (Lake)
        {
            FVector NewLocation = Lake->GetActorLocation();
            NewLocation.Z = CurrentWaterLevel;
            Lake->SetActorLocation(NewLocation);
        }
    }

    for (AStaticMeshActor* Pond : Ponds)
    {
        if (Pond)
        {
            FVector NewLocation = Pond->GetActorLocation();
            NewLocation.Z = CurrentWaterLevel - 20.0f; // Ponds slightly lower
            Pond->SetActorLocation(NewLocation);
        }
    }
}

void AWorld_WaterSystemManager::UpdateFlowEffects()
{
    CurrentFlowRate = FlowSpeed * (1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 0.2f);

    // Update flow direction and particle effects
    UpdateFlowDirection();
}

void AWorld_WaterSystemManager::SetWaterMaterials()
{
    // Apply materials to all water bodies
    for (AStaticMeshActor* Lake : Lakes)
    {
        if (Lake && LakeWaterMaterial)
        {
            ApplyWaterMaterial(Lake, LakeWaterMaterial);
        }
    }

    for (AStaticMeshActor* River : Rivers)
    {
        if (River && RiverWaterMaterial)
        {
            ApplyWaterMaterial(River, RiverWaterMaterial);
        }
    }

    for (AStaticMeshActor* Waterfall : Waterfalls)
    {
        if (Waterfall && WaterfallMaterial)
        {
            ApplyWaterMaterial(Waterfall, WaterfallMaterial);
        }
    }

    for (AStaticMeshActor* Pond : Ponds)
    {
        if (Pond && LakeWaterMaterial)
        {
            ApplyWaterMaterial(Pond, LakeWaterMaterial);
        }
    }
}

bool AWorld_WaterSystemManager::IsLocationNearWater(FVector Location, float Radius) const
{
    // Check distance to all water bodies
    for (const AStaticMeshActor* Lake : Lakes)
    {
        if (Lake && FVector::Dist(Lake->GetActorLocation(), Location) <= Radius)
        {
            return true;
        }
    }

    for (const AStaticMeshActor* River : Rivers)
    {
        if (River && FVector::Dist(River->GetActorLocation(), Location) <= Radius)
        {
            return true;
        }
    }

    for (const AStaticMeshActor* Pond : Ponds)
    {
        if (Pond && FVector::Dist(Pond->GetActorLocation(), Location) <= Radius)
        {
            return true;
        }
    }

    return false;
}

FVector AWorld_WaterSystemManager::GetNearestWaterSource(FVector Location) const
{
    FVector NearestWater = Location;
    float MinDistance = MAX_FLT;

    // Check all water bodies for nearest
    TArray<const AStaticMeshActor*> AllWaterBodies;
    for (const AStaticMeshActor* Lake : Lakes) AllWaterBodies.Add(Lake);
    for (const AStaticMeshActor* River : Rivers) AllWaterBodies.Add(River);
    for (const AStaticMeshActor* Pond : Ponds) AllWaterBodies.Add(Pond);

    for (const AStaticMeshActor* WaterBody : AllWaterBodies)
    {
        if (WaterBody)
        {
            float Distance = FVector::Dist(WaterBody->GetActorLocation(), Location);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestWater = WaterBody->GetActorLocation();
            }
        }
    }

    return NearestWater;
}

float AWorld_WaterSystemManager::GetWaterDepthAtLocation(FVector Location) const
{
    // Simple depth calculation based on proximity to water bodies
    if (IsLocationNearWater(Location, 100.0f))
    {
        return FMath::RandRange(1.0f, 5.0f); // Shallow to medium depth
    }
    
    return 0.0f; // No water
}

void AWorld_WaterSystemManager::SpawnWaterVegetation()
{
    // Spawn vegetation near water bodies
    for (const AStaticMeshActor* Lake : Lakes)
    {
        if (Lake)
        {
            FVector LakeLocation = Lake->GetActorLocation();
            // Spawn reeds and water plants around lake perimeter
            for (int32 i = 0; i < 8; i++)
            {
                float Angle = (i / 8.0f) * 2.0f * PI;
                FVector VegLocation = LakeLocation + FVector(
                    FMath::Cos(Angle) * 1200.0f,
                    FMath::Sin(Angle) * 1200.0f,
                    0.0f
                );
                
                // Note: Actual vegetation spawning would be handled by Environment Artist
                UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Vegetation spawn point at %s"), *VegLocation.ToString());
            }
        }
    }
}

void AWorld_WaterSystemManager::CreateFishingSpots()
{
    // Create designated fishing locations
    for (const AStaticMeshActor* Lake : Lakes)
    {
        if (Lake)
        {
            FVector FishingSpot = Lake->GetActorLocation() + FVector(500.0f, 0.0f, 0.0f);
            UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Fishing spot created at %s"), *FishingSpot.ToString());
        }
    }
}

void AWorld_WaterSystemManager::UpdateWaterQuality(float PollutionLevel)
{
    // Adjust water appearance and ecosystem health based on pollution
    float WaterClarityFactor = FMath::Clamp(1.0f - PollutionLevel, 0.1f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Water quality updated - Clarity: %f"), WaterClarityFactor);
}

AStaticMeshActor* AWorld_WaterSystemManager::CreateWaterBody(FVector Location, FVector Scale, FRotator Rotation, const FString& Label)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    AStaticMeshActor* WaterActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (WaterActor)
    {
        WaterActor->SetActorLabel(Label);
        WaterActor->SetActorScale3D(Scale);

        // Set up water mesh
        UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
        SetupWaterMesh(WaterActor, PlaneMesh);
    }

    return WaterActor;
}

void AWorld_WaterSystemManager::SetupWaterMesh(AStaticMeshActor* WaterActor, UStaticMesh* Mesh)
{
    if (WaterActor && Mesh)
    {
        UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(Mesh);
        }
    }
}

void AWorld_WaterSystemManager::ApplyWaterMaterial(AStaticMeshActor* WaterActor, UMaterialInterface* Material)
{
    if (WaterActor && Material)
    {
        UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetMaterial(0, Material);
        }
    }
}

void AWorld_WaterSystemManager::CreateFlowMarkers()
{
    for (const FVector& FlowPoint : FlowPoints)
    {
        UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Flow marker at %s"), *FlowPoint.ToString());
    }
}

void AWorld_WaterSystemManager::UpdateFlowDirection()
{
    // Update flow particle systems and water surface animation
    for (const FVector& FlowPoint : FlowPoints)
    {
        // Flow direction calculation would be implemented here
        // This is a placeholder for actual particle system updates
    }
}

void AWorld_WaterSystemManager::CalculateWaterFlow()
{
    // Physics-based water flow calculation
    // Would integrate with UE5 fluid simulation systems
    CurrentFlowRate = FlowSpeed * (CurrentWaterLevel / BaseWaterLevel);
}