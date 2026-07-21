#include "Crowd_MassEntityTrafficController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

ACrowd_MassEntityTrafficController::ACrowd_MassEntityTrafficController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for traffic control

    // Create root component
    TrafficRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TrafficRoot"));
    RootComponent = TrafficRoot;

    // Create controller mesh
    ControllerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ControllerMesh"));
    ControllerMesh->SetupAttachment(RootComponent);
    ControllerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    TrafficDensity = 1.0f;
    MaxConcurrentEntities = 50000;
    bEnableTrafficControl = true;
    UpdateFrequency = 10.0f;
    CullingDistance = 5000.0f;
    bUseDistanceCulling = true;
    
    // Internal state
    LastUpdateTime = 0.0f;
    bTrafficPaused = false;
    TotalActiveEntities = 0;

    // Initialize default traffic flow
    CurrentTrafficFlow.FlowDirection = FVector::ForwardVector;
    CurrentTrafficFlow.FlowSpeed = 200.0f;
    CurrentTrafficFlow.EntityCount = 0;
    CurrentTrafficFlow.DensityFactor = 1.0f;

    // Create default traffic lanes
    FCrowd_TrafficLane DefaultLane;
    DefaultLane.StartPoint = FVector(-2000.0f, 0.0f, 100.0f);
    DefaultLane.EndPoint = FVector(2000.0f, 0.0f, 100.0f);
    DefaultLane.LaneWidth = 300.0f;
    DefaultLane.MaxEntities = 1000;
    DefaultLane.bIsBidirectional = false;
    TrafficLanes.Add(DefaultLane);

    // Initialize lane entity counts
    LaneEntityCounts.Add(0);
}

void ACrowd_MassEntityTrafficController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTrafficSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Traffic Controller initialized with %d lanes"), TrafficLanes.Num());
}

void ACrowd_MassEntityTrafficController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableTrafficControl || bTrafficPaused)
    {
        return;
    }

    LastUpdateTime += DeltaTime;
    
    // Update at specified frequency
    if (LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateTrafficFlow(LastUpdateTime);
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityTrafficController::InitializeTrafficSystem()
{
    // Validate traffic lanes
    ValidateTrafficLanes();
    
    // Initialize lane entity counts
    LaneEntityCounts.Empty();
    for (int32 i = 0; i < TrafficLanes.Num(); i++)
    {
        LaneEntityCounts.Add(0);
    }
    
    // Reset traffic state
    TotalActiveEntities = 0;
    bTrafficPaused = false;
    
    UE_LOG(LogTemp, Log, TEXT("Traffic system initialized with %d lanes, max entities: %d"), 
           TrafficLanes.Num(), MaxConcurrentEntities);
}

void ACrowd_MassEntityTrafficController::UpdateTrafficFlow(float DeltaTime)
{
    if (TrafficLanes.Num() == 0)
    {
        return;
    }

    // Update lane occupancy
    UpdateLaneOccupancy();
    
    // Balance traffic load across lanes
    BalanceTrafficLoad();
    
    // Cull distant entities if enabled
    if (bUseDistanceCulling)
    {
        CullDistantEntities();
    }
    
    // Update current traffic flow data
    CurrentTrafficFlow.EntityCount = TotalActiveEntities;
    CurrentTrafficFlow.DensityFactor = FMath::Clamp(
        static_cast<float>(TotalActiveEntities) / static_cast<float>(MaxConcurrentEntities),
        0.0f, 2.0f
    );
    
    // Adjust flow speed based on density
    float BaseFl owSpeed = 200.0f;
    CurrentTrafficFlow.FlowSpeed = BaseFl owSpeed * FMath::Lerp(1.0f, 0.3f, CurrentTrafficFlow.DensityFactor);
}

void ACrowd_MassEntityTrafficController::AddTrafficLane(const FCrowd_TrafficLane& NewLane)
{
    TrafficLanes.Add(NewLane);
    LaneEntityCounts.Add(0);
    
    UE_LOG(LogTemp, Log, TEXT("Added new traffic lane. Total lanes: %d"), TrafficLanes.Num());
}

void ACrowd_MassEntityTrafficController::RemoveTrafficLane(int32 LaneIndex)
{
    if (LaneIndex >= 0 && LaneIndex < TrafficLanes.Num())
    {
        TrafficLanes.RemoveAt(LaneIndex);
        if (LaneIndex < LaneEntityCounts.Num())
        {
            LaneEntityCounts.RemoveAt(LaneIndex);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Removed traffic lane %d. Remaining lanes: %d"), 
               LaneIndex, TrafficLanes.Num());
    }
}

void ACrowd_MassEntityTrafficController::SetTrafficDensity(float NewDensity)
{
    TrafficDensity = FMath::Clamp(NewDensity, 0.1f, 3.0f);
    
    // Adjust max entities based on density
    int32 BaseMaxEntities = 50000;
    MaxConcurrentEntities = FMath::RoundToInt(BaseMaxEntities * TrafficDensity);
    
    UE_LOG(LogTemp, Log, TEXT("Traffic density set to %.2f, max entities: %d"), 
           TrafficDensity, MaxConcurrentEntities);
}

int32 ACrowd_MassEntityTrafficController::GetActiveEntityCount() const
{
    return TotalActiveEntities;
}

void ACrowd_MassEntityTrafficController::PauseTrafficFlow()
{
    bTrafficPaused = true;
    UE_LOG(LogTemp, Warning, TEXT("Traffic flow paused"));
}

void ACrowd_MassEntityTrafficController::ResumeTrafficFlow()
{
    bTrafficPaused = false;
    UE_LOG(LogTemp, Warning, TEXT("Traffic flow resumed"));
}

void ACrowd_MassEntityTrafficController::ClearAllTraffic()
{
    // Reset all lane entity counts
    for (int32& Count : LaneEntityCounts)
    {
        Count = 0;
    }
    
    TotalActiveEntities = 0;
    CurrentTrafficFlow.EntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("All traffic cleared"));
}

bool ACrowd_MassEntityTrafficController::IsLaneAvailable(int32 LaneIndex) const
{
    if (LaneIndex < 0 || LaneIndex >= TrafficLanes.Num() || LaneIndex >= LaneEntityCounts.Num())
    {
        return false;
    }
    
    return LaneEntityCounts[LaneIndex] < TrafficLanes[LaneIndex].MaxEntities;
}

FVector ACrowd_MassEntityTrafficController::GetLaneDirection(int32 LaneIndex) const
{
    if (LaneIndex < 0 || LaneIndex >= TrafficLanes.Num())
    {
        return FVector::ForwardVector;
    }
    
    const FCrowd_TrafficLane& Lane = TrafficLanes[LaneIndex];
    return (Lane.EndPoint - Lane.StartPoint).GetSafeNormal();
}

float ACrowd_MassEntityTrafficController::GetLaneOccupancy(int32 LaneIndex) const
{
    if (LaneIndex < 0 || LaneIndex >= TrafficLanes.Num() || LaneIndex >= LaneEntityCounts.Num())
    {
        return 0.0f;
    }
    
    const FCrowd_TrafficLane& Lane = TrafficLanes[LaneIndex];
    if (Lane.MaxEntities <= 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(LaneEntityCounts[LaneIndex]) / static_cast<float>(Lane.MaxEntities);
}

void ACrowd_MassEntityTrafficController::OptimizeLaneDistribution()
{
    if (TrafficLanes.Num() <= 1)
    {
        return;
    }
    
    // Calculate average occupancy
    float TotalOccupancy = 0.0f;
    for (int32 i = 0; i < TrafficLanes.Num(); i++)
    {
        TotalOccupancy += GetLaneOccupancy(i);
    }
    float AverageOccupancy = TotalOccupancy / TrafficLanes.Num();
    
    // Redistribute entities from overloaded lanes to underloaded ones
    for (int32 i = 0; i < TrafficLanes.Num(); i++)
    {
        float CurrentOccupancy = GetLaneOccupancy(i);
        if (CurrentOccupancy > AverageOccupancy + 0.2f) // Overloaded
        {
            // Find underloaded lane
            for (int32 j = 0; j < TrafficLanes.Num(); j++)
            {
                if (j != i && GetLaneOccupancy(j) < AverageOccupancy - 0.2f)
                {
                    // Transfer some entities (simulation)
                    int32 TransferCount = FMath::Min(
                        LaneEntityCounts[i] / 4,
                        TrafficLanes[j].MaxEntities - LaneEntityCounts[j]
                    );
                    
                    if (TransferCount > 0)
                    {
                        LaneEntityCounts[i] -= TransferCount;
                        LaneEntityCounts[j] += TransferCount;
                        break;
                    }
                }
            }
        }
    }
}

void ACrowd_MassEntityTrafficController::UpdateLaneOccupancy()
{
    TotalActiveEntities = 0;
    
    for (int32 Count : LaneEntityCounts)
    {
        TotalActiveEntities += Count;
    }
    
    // Simulate entity movement and lane changes
    for (int32 i = 0; i < LaneEntityCounts.Num(); i++)
    {
        // Simulate some entities leaving the lane
        if (LaneEntityCounts[i] > 0 && FMath::RandRange(0.0f, 1.0f) < 0.05f)
        {
            LaneEntityCounts[i] = FMath::Max(0, LaneEntityCounts[i] - FMath::RandRange(1, 5));
        }
        
        // Simulate new entities entering if lane has capacity
        if (IsLaneAvailable(i) && FMath::RandRange(0.0f, 1.0f) < (TrafficDensity * 0.1f))
        {
            LaneEntityCounts[i] += FMath::RandRange(1, 3);
            LaneEntityCounts[i] = FMath::Min(LaneEntityCounts[i], TrafficLanes[i].MaxEntities);
        }
    }
}

void ACrowd_MassEntityTrafficController::BalanceTrafficLoad()
{
    if (TotalActiveEntities > MaxConcurrentEntities * 0.9f) // Near capacity
    {
        OptimizeLaneDistribution();
    }
}

void ACrowd_MassEntityTrafficController::CullDistantEntities()
{
    // Get player location for distance culling
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            FVector ControllerLocation = GetActorLocation();
            
            float DistanceToPlayer = FVector::Dist(PlayerLocation, ControllerLocation);
            
            if (DistanceToPlayer > CullingDistance)
            {
                // Reduce entity count for distant traffic controller
                for (int32& Count : LaneEntityCounts)
                {
                    Count = FMath::RoundToInt(Count * 0.5f);
                }
            }
        }
    }
}

void ACrowd_MassEntityTrafficController::ValidateTrafficLanes()
{
    for (int32 i = TrafficLanes.Num() - 1; i >= 0; i--)
    {
        FCrowd_TrafficLane& Lane = TrafficLanes[i];
        
        // Ensure lane has valid endpoints
        if (Lane.StartPoint.Equals(Lane.EndPoint, 1.0f))
        {
            Lane.EndPoint = Lane.StartPoint + FVector(1000.0f, 0.0f, 0.0f);
        }
        
        // Ensure positive lane width
        if (Lane.LaneWidth <= 0.0f)
        {
            Lane.LaneWidth = 200.0f;
        }
        
        // Ensure positive max entities
        if (Lane.MaxEntities <= 0)
        {
            Lane.MaxEntities = 500;
        }
    }
}