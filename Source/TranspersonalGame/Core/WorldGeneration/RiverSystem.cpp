#include "RiverSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogRiverSystem, Log, All);

ARiverSystem::ARiverSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default values
    RiverWidth = 500.0f;
    RiverDepth = 100.0f;
    FlowSpeed = 200.0f;
    WaterLevel = 0.0f;
    bIsFlowing = true;
    
    // Initialize river segments array
    RiverSegments.Empty();
    RiverPoints.Empty();
    
    // Set up default river material path
    WaterMaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial");
}

void ARiverSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogRiverSystem, Log, TEXT("RiverSystem: BeginPlay called"));
    
    // Auto-generate river if no points are set
    if (RiverPoints.Num() == 0)
    {
        GenerateDefaultRiverPath();
    }
    
    // Create river mesh
    CreateRiverMesh();
}

void ARiverSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsFlowing)
    {
        UpdateWaterFlow(DeltaTime);
    }
}

void ARiverSystem::GenerateDefaultRiverPath()
{
    UE_LOG(LogRiverSystem, Log, TEXT("Generating default river path for 200km2 world"));
    
    // Clear existing points
    RiverPoints.Empty();
    
    // Create major river from MONTANHA through FLORESTA to PANTANO
    // Using 200km2 biome coordinates from brain memories
    
    RiverPoints.Add(FVector(35000, 45000, 400));   // Mountain source
    RiverPoints.Add(FVector(20000, 35000, 300));   // Mountain exit
    RiverPoints.Add(FVector(0, 25000, 200));       // Floresta entry
    RiverPoints.Add(FVector(-15000, 15000, 150));  // Floresta middle
    RiverPoints.Add(FVector(-30000, 0, 100));      // Floresta exit
    RiverPoints.Add(FVector(-45000, -20000, 50));  // Pantano entry
    RiverPoints.Add(FVector(-55000, -35000, 0));   // Pantano delta
    
    UE_LOG(LogRiverSystem, Log, TEXT("Generated river with %d points"), RiverPoints.Num());
}

void ARiverSystem::CreateRiverMesh()
{
    if (RiverPoints.Num() < 2)
    {
        UE_LOG(LogRiverSystem, Warning, TEXT("Need at least 2 river points to create mesh"));
        return;
    }
    
    UE_LOG(LogRiverSystem, Log, TEXT("Creating river mesh with %d segments"), RiverPoints.Num() - 1);
    
    // Clear existing segments
    for (AStaticMeshActor* Segment : RiverSegments)
    {
        if (IsValid(Segment))
        {
            Segment->Destroy();
        }
    }
    RiverSegments.Empty();
    
    // Create river segments between points
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        CreateRiverSegment(RiverPoints[i], RiverPoints[i + 1], i);
    }
    
    UE_LOG(LogRiverSystem, Log, TEXT("Created %d river segments"), RiverSegments.Num());
}

void ARiverSystem::CreateRiverSegment(const FVector& StartPoint, const FVector& EndPoint, int32 SegmentIndex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate segment properties
    FVector MidPoint = (StartPoint + EndPoint) * 0.5f;
    FVector Direction = (EndPoint - StartPoint).GetSafeNormal();
    float Distance = FVector::Dist(StartPoint, EndPoint);
    
    // Calculate rotation to align with river flow
    FRotator Rotation = Direction.Rotation();
    
    // Spawn river segment actor
    AStaticMeshActor* RiverSegment = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        MidPoint,
        Rotation
    );
    
    if (RiverSegment)
    {
        // Set up mesh component
        UStaticMeshComponent* MeshComp = RiverSegment->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Load basic plane mesh for water surface
            UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
            if (PlaneMesh)
            {
                MeshComp->SetStaticMesh(PlaneMesh);
                
                // Scale to fit river segment
                FVector Scale = FVector(Distance / 100.0f, RiverWidth / 100.0f, 1.0f);
                MeshComp->SetWorldScale3D(Scale);
                
                UE_LOG(LogRiverSystem, Log, TEXT("River segment %d: Distance=%.1f, Scale=(%s)"), 
                       SegmentIndex, Distance, *Scale.ToString());
            }
        }
        
        // Set actor label for identification
        RiverSegment->SetActorLabel(FString::Printf(TEXT("RiverSegment_%d"), SegmentIndex));
        
        // Add to segments array
        RiverSegments.Add(RiverSegment);
    }
}

void ARiverSystem::UpdateWaterFlow(float DeltaTime)
{
    // Update water flow animation
    // This could animate water materials, spawn particles, etc.
    
    static float FlowTime = 0.0f;
    FlowTime += DeltaTime * FlowSpeed;
    
    // Update material parameters for flowing water effect
    for (AStaticMeshActor* Segment : RiverSegments)
    {
        if (IsValid(Segment))
        {
            UStaticMeshComponent* MeshComp = Segment->GetStaticMeshComponent();
            if (MeshComp)
            {
                // Here we could set material parameters for water flow animation
                // For now, just log the flow update periodically
                static float LastLogTime = 0.0f;
                if (FlowTime - LastLogTime > 5.0f)  // Log every 5 seconds
                {
                    UE_LOG(LogRiverSystem, VeryVerbose, TEXT("Water flow time: %.2f"), FlowTime);
                    LastLogTime = FlowTime;
                }
            }
        }
    }
}

void ARiverSystem::SetRiverWidth(float NewWidth)
{
    RiverWidth = FMath::Clamp(NewWidth, 50.0f, 2000.0f);
    
    // Update existing segments
    for (AStaticMeshActor* Segment : RiverSegments)
    {
        if (IsValid(Segment))
        {
            UStaticMeshComponent* MeshComp = Segment->GetStaticMeshComponent();
            if (MeshComp)
            {
                FVector CurrentScale = MeshComp->GetComponentScale();
                CurrentScale.Y = RiverWidth / 100.0f;
                MeshComp->SetWorldScale3D(CurrentScale);
            }
        }
    }
    
    UE_LOG(LogRiverSystem, Log, TEXT("River width set to %.1f"), RiverWidth);
}

void ARiverSystem::SetFlowSpeed(float NewSpeed)
{
    FlowSpeed = FMath::Clamp(NewSpeed, 0.0f, 1000.0f);
    UE_LOG(LogRiverSystem, Log, TEXT("River flow speed set to %.1f"), FlowSpeed);
}

void ARiverSystem::AddRiverPoint(const FVector& NewPoint)
{
    RiverPoints.Add(NewPoint);
    UE_LOG(LogRiverSystem, Log, TEXT("Added river point: %s (Total: %d)"), *NewPoint.ToString(), RiverPoints.Num());
    
    // Recreate mesh with new point
    CreateRiverMesh();
}

void ARiverSystem::ClearRiverPoints()
{
    RiverPoints.Empty();
    
    // Destroy existing segments
    for (AStaticMeshActor* Segment : RiverSegments)
    {
        if (IsValid(Segment))
        {
            Segment->Destroy();
        }
    }
    RiverSegments.Empty();
    
    UE_LOG(LogRiverSystem, Log, TEXT("Cleared all river points and segments"));
}

TArray<FVector> ARiverSystem::GetRiverPoints() const
{
    return RiverPoints;
}

int32 ARiverSystem::GetSegmentCount() const
{
    return RiverSegments.Num();
}

bool ARiverSystem::IsPointNearRiver(const FVector& TestPoint, float MaxDistance) const
{
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        FVector ClosestPoint = FMath::ClosestPointOnSegment(TestPoint, RiverPoints[i], RiverPoints[i + 1]);
        float Distance = FVector::Dist(TestPoint, ClosestPoint);
        
        if (Distance <= MaxDistance)
        {
            return true;
        }
    }
    
    return false;
}