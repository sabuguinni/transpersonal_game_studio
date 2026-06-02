#include "World_WaterSystemManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_WaterSystemManager::AWorld_WaterSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxRiverSegments = 20;
    MaxLakes = 8;
    MinDistanceBetweenLakes = 2000.0f;
    bUseInstancedMeshes = true;
    WaterLODDistance = 5000.0f;
    WaterSystemBounds = 10000.0f;
    bWaterSystemInitialized = false;

    // Set up default river segments
    RiverSegments.Empty();
    FWorld_RiverSegment MainRiver;
    MainRiver.StartPoint = FVector(-5000, -3000, 50);
    MainRiver.EndPoint = FVector(5000, 3000, 30);
    MainRiver.Width = 400.0f;
    MainRiver.FlowSpeed = 2.0f;
    MainRiver.BiomeType = TEXT("Mixed");
    RiverSegments.Add(MainRiver);

    // Set up default lakes
    Lakes.Empty();
    FWorld_LakeData ForestLake;
    ForestLake.Position = FVector(-3000, 2000, 40);
    ForestLake.Radius = 800.0f;
    ForestLake.LakeType = TEXT("Forest");
    ForestLake.bHasFish = true;
    Lakes.Add(ForestLake);

    FWorld_LakeData PlainsLake;
    PlainsLake.Position = FVector(3000, -2000, 45);
    PlainsLake.Radius = 600.0f;
    PlainsLake.LakeType = TEXT("Plains");
    PlainsLake.bHasFish = false;
    Lakes.Add(PlainsLake);
}

void AWorld_WaterSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bWaterSystemInitialized)
    {
        GenerateWaterSystemInEditor();
        bWaterSystemInitialized = true;
    }
}

void AWorld_WaterSystemManager::GenerateRiverSystem()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: No valid world"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Generating river system with %d segments"), RiverSegments.Num());

    for (int32 i = 0; i < RiverSegments.Num(); i++)
    {
        CreateRiverSegment(RiverSegments[i]);
    }

    OptimizeWaterRendering();
}

void AWorld_WaterSystemManager::GenerateLakes()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: No valid world"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Generating %d lakes"), Lakes.Num());

    for (int32 i = 0; i < Lakes.Num(); i++)
    {
        CreateLake(Lakes[i]);
    }
}

void AWorld_WaterSystemManager::CreateRiverSegment(const FWorld_RiverSegment& SegmentData)
{
    if (!GetWorld())
    {
        return;
    }

    // Calculate midpoint for river segment placement
    FVector MidPoint = (SegmentData.StartPoint + SegmentData.EndPoint) * 0.5f;
    
    // Calculate rotation to align with flow direction
    FVector Direction = (SegmentData.EndPoint - SegmentData.StartPoint).GetSafeNormal();
    FRotator Rotation = Direction.Rotation();

    // Spawn river segment actor
    AActor* RiverActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), MidPoint, Rotation);
    if (!RiverActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Failed to spawn river actor"));
        return;
    }

    // Add static mesh component
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(RiverActor);
    if (MeshComponent)
    {
        RiverActor->SetRootComponent(MeshComponent);
        
        // Set scale based on river width and length
        float Length = FVector::Dist(SegmentData.StartPoint, SegmentData.EndPoint);
        FVector Scale = FVector(Length / 100.0f, SegmentData.Width / 100.0f, 0.1f);
        RiverActor->SetActorScale3D(Scale);

        SetupWaterMaterial(MeshComponent, true);
        
        FString RiverName = FString::Printf(TEXT("RiverSegment_%s_%d"), 
            *SegmentData.BiomeType, SpawnedWaterActors.Num());
        RiverActor->SetActorLabel(RiverName);
        
        SpawnedWaterActors.Add(RiverActor);
        OnRiverSegmentCreated(RiverActor);
        
        UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Created river segment %s"), *RiverName);
    }
}

void AWorld_WaterSystemManager::CreateLake(const FWorld_LakeData& LakeData)
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn lake actor
    AActor* LakeActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), LakeData.Position, FRotator::ZeroRotator);
    if (!LakeActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaterSystemManager: Failed to spawn lake actor"));
        return;
    }

    // Add static mesh component
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(LakeActor);
    if (MeshComponent)
    {
        LakeActor->SetRootComponent(MeshComponent);
        
        // Set scale based on lake radius
        FVector Scale = FVector(LakeData.Radius / 50.0f, LakeData.Radius / 50.0f, 0.1f);
        LakeActor->SetActorScale3D(Scale);

        SetupWaterMaterial(MeshComponent, false);
        
        FString LakeName = FString::Printf(TEXT("Lake_%s_%d"), 
            *LakeData.LakeType, SpawnedWaterActors.Num());
        LakeActor->SetActorLabel(LakeName);
        
        SpawnedWaterActors.Add(LakeActor);
        OnLakeCreated(LakeActor);
        
        UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Created lake %s"), *LakeName);
    }
}

void AWorld_WaterSystemManager::ClearWaterBodies()
{
    for (AActor* WaterActor : SpawnedWaterActors)
    {
        if (IsValid(WaterActor))
        {
            WaterActor->Destroy();
        }
    }
    SpawnedWaterActors.Empty();
    UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Cleared all water bodies"));
}

bool AWorld_WaterSystemManager::IsPositionNearWater(const FVector& Position, float Threshold) const
{
    // Check distance to rivers
    for (const FWorld_RiverSegment& River : RiverSegments)
    {
        FVector MidPoint = (River.StartPoint + River.EndPoint) * 0.5f;
        if (FVector::Dist(Position, MidPoint) < Threshold)
        {
            return true;
        }
    }

    // Check distance to lakes
    for (const FWorld_LakeData& Lake : Lakes)
    {
        if (FVector::Dist(Position, Lake.Position) < (Lake.Radius + Threshold))
        {
            return true;
        }
    }

    return false;
}

FVector AWorld_WaterSystemManager::GetNearestWaterPosition(const FVector& FromPosition) const
{
    FVector NearestPosition = FVector::ZeroVector;
    float MinDistance = FLT_MAX;

    // Check rivers
    for (const FWorld_RiverSegment& River : RiverSegments)
    {
        FVector MidPoint = (River.StartPoint + River.EndPoint) * 0.5f;
        float Distance = FVector::Dist(FromPosition, MidPoint);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestPosition = MidPoint;
        }
    }

    // Check lakes
    for (const FWorld_LakeData& Lake : Lakes)
    {
        float Distance = FVector::Dist(FromPosition, Lake.Position);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestPosition = Lake.Position;
        }
    }

    return NearestPosition;
}

void AWorld_WaterSystemManager::UpdateWaterFlow(float DeltaTime)
{
    // Update water material parameters for flow animation
    if (WaterMaterial)
    {
        // This would update material parameters for flow animation
        // Implementation depends on material setup
    }
}

void AWorld_WaterSystemManager::GenerateWaterSystemInEditor()
{
    ClearWaterBodies();
    GenerateRiverSystem();
    GenerateLakes();
    ValidateWaterSystem();
}

void AWorld_WaterSystemManager::ValidateWaterSystem()
{
    int32 ValidRivers = 0;
    int32 ValidLakes = 0;

    for (const AActor* WaterActor : SpawnedWaterActors)
    {
        if (IsValid(WaterActor))
        {
            if (WaterActor->GetActorLabel().Contains(TEXT("River")))
            {
                ValidRivers++;
            }
            else if (WaterActor->GetActorLabel().Contains(TEXT("Lake")))
            {
                ValidLakes++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Validation complete - %d rivers, %d lakes"), 
        ValidRivers, ValidLakes);
}

void AWorld_WaterSystemManager::OnRiverSegmentCreated(AActor* NewActor)
{
    if (IsValid(NewActor))
    {
        // Additional setup for river segments
        UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: River segment created: %s"), *NewActor->GetActorLabel());
    }
}

void AWorld_WaterSystemManager::OnLakeCreated(AActor* NewActor)
{
    if (IsValid(NewActor))
    {
        // Additional setup for lakes
        UE_LOG(LogTemp, Log, TEXT("WaterSystemManager: Lake created: %s"), *NewActor->GetActorLabel());
    }
}

void AWorld_WaterSystemManager::SetupWaterMaterial(UStaticMeshComponent* MeshComponent, bool bIsRiver)
{
    if (!MeshComponent)
    {
        return;
    }

    UMaterialInterface* MaterialToUse = bIsRiver ? RiverMaterial : WaterMaterial;
    if (MaterialToUse)
    {
        MeshComponent->SetMaterial(0, MaterialToUse);
    }
}

void AWorld_WaterSystemManager::OptimizeWaterRendering()
{
    // Implement LOD and culling optimizations
    for (AActor* WaterActor : SpawnedWaterActors)
    {
        if (IsValid(WaterActor))
        {
            UStaticMeshComponent* MeshComp = WaterActor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                // Set up LOD distances
                MeshComp->SetCullDistance(WaterLODDistance);
            }
        }
    }
}

FVector AWorld_WaterSystemManager::CalculateRiverFlowDirection(int32 SegmentIndex) const
{
    if (RiverSegments.IsValidIndex(SegmentIndex))
    {
        const FWorld_RiverSegment& Segment = RiverSegments[SegmentIndex];
        return (Segment.EndPoint - Segment.StartPoint).GetSafeNormal();
    }
    return FVector::ForwardVector;
}