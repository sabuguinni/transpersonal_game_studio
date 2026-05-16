#include "Perf_CollisionOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UPerf_CollisionOptimizer::UPerf_CollisionOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    OptimizationUpdateInterval = 0.5f;
    MaxCollisionsPerFrame = 100;
    bEnableCollisionBatching = true;
    bEnableDistanceCulling = true;
    
    CollisionCPUTime = 0.0f;
    ActiveCollisionCount = 0;
    CulledCollisionCount = 0;
    AverageFrameTime = 16.67f; // 60fps target
    LastOptimizationTime = 0.0f;
}

void UPerf_CollisionOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize collision optimization
    UWorld* World = GetWorld();
    if (World)
    {
        // Get all actors for initial optimization
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        // Track relevant actors
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    TrackedActors.Add(Actor);
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("CollisionOptimizer: Tracking %d collision actors"), TrackedActors.Num());
    }
}

void UPerf_CollisionOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update optimization periodically
    if (CurrentTime - LastOptimizationTime >= OptimizationUpdateInterval)
    {
        LastOptimizationTime = CurrentTime;
        
        // Get player location for distance calculations
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
        
        // Optimize collision for tracked actors
        int32 OptimizedThisFrame = 0;
        for (int32 i = TrackedActors.Num() - 1; i >= 0; --i)
        {
            if (OptimizedThisFrame >= MaxCollisionsPerFrame)
                break;
                
            if (TrackedActors[i].IsValid())
            {
                AActor* Actor = TrackedActors[i].Get();
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                OptimizeCollisionForActor(Actor, Distance);
                OptimizedThisFrame++;
            }
            else
            {
                // Remove invalid actors
                TrackedActors.RemoveAt(i);
            }
        }
        
        // Update performance metrics
        ActiveCollisionCount = TrackedActors.Num() - CulledCollisionCount;
    }
}

void UPerf_CollisionOptimizer::OptimizeCollisionForActor(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor || !Actor->GetRootComponent())
        return;
        
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!PrimComp)
        return;
        
    // Calculate appropriate LOD level
    bool bIsImportant = CalculateCollisionImportance(Actor) > 0.7f;
    EPerf_LODLevel LODLevel = CalculateCollisionLOD(DistanceToPlayer, bIsImportant);
    
    // Update collision LOD
    UpdateCollisionLOD(Actor, LODLevel);
    
    // Store LOD level for tracking
    ActorLODMap.Add(Actor, LODLevel);
}

void UPerf_CollisionOptimizer::UpdateCollisionLOD(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
        return;
        
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!PrimComp)
        return;
        
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            UpdateCollisionComplexity(PrimComp, EPerf_LODLevel::High);
            break;
            
        case EPerf_LODLevel::Medium:
            UpdateCollisionComplexity(PrimComp, EPerf_LODLevel::Medium);
            break;
            
        case EPerf_LODLevel::Low:
            UpdateCollisionComplexity(PrimComp, EPerf_LODLevel::Low);
            break;
            
        case EPerf_LODLevel::Disabled:
            DisableCollisionForActor(Actor);
            CulledCollisionCount++;
            break;
    }
}

void UPerf_CollisionOptimizer::BatchOptimizeCollisions(const TArray<AActor*>& Actors)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    for (AActor* Actor : Actors)
    {
        if (Actor)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            OptimizeCollisionForActor(Actor, Distance);
        }
    }
}

FPerf_CollisionMetrics UPerf_CollisionOptimizer::AnalyzeCollisionPerformance()
{
    FPerf_CollisionMetrics Metrics;
    
    UWorld* World = GetWorld();
    if (!World)
        return Metrics;
        
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ComplexCollisions = 0;
    int32 SimpleCollisions = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                // Check collision complexity
                UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(PrimComp);
                if (StaticMeshComp)
                {
                    if (StaticMeshComp->GetCollisionTraceFlag() == ECollisionTraceFlag::CTF_UseComplexAsSimple)
                        ComplexCollisions++;
                    else
                        SimpleCollisions++;
                }
                else
                {
                    SimpleCollisions++;
                }
            }
        }
    }
    
    Metrics.TotalCollisionActors = ComplexCollisions + SimpleCollisions;
    Metrics.ComplexCollisionActors = ComplexCollisions;
    Metrics.SimpleCollisionActors = SimpleCollisions;
    Metrics.CollisionCPUTime = CollisionCPUTime;
    Metrics.AverageCollisionChecksPerFrame = static_cast<float>(Metrics.TotalCollisionActors) * 0.1f; // Estimate
    
    return Metrics;
}

void UPerf_CollisionOptimizer::ProfileCollisionSystem(float Duration)
{
    // Start profiling collision system performance
    float StartTime = FPlatformTime::Seconds();
    
    // Perform collision analysis
    FPerf_CollisionMetrics Metrics = AnalyzeCollisionPerformance();
    
    float EndTime = FPlatformTime::Seconds();
    CollisionCPUTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    UE_LOG(LogTemp, Log, TEXT("Collision Profile: %d total actors, %.2fms CPU time"), 
           Metrics.TotalCollisionActors, CollisionCPUTime);
}

EPerf_LODLevel UPerf_CollisionOptimizer::CalculateCollisionLOD(float Distance, bool bIsImportant)
{
    // Adjust distances for important objects
    float HighDist = bIsImportant ? LODSettings.HighDetailDistance * 1.5f : LODSettings.HighDetailDistance;
    float MediumDist = bIsImportant ? LODSettings.MediumDetailDistance * 1.5f : LODSettings.MediumDetailDistance;
    float LowDist = bIsImportant ? LODSettings.LowDetailDistance * 1.5f : LODSettings.LowDetailDistance;
    float CullDist = bIsImportant ? LODSettings.CullingDistance * 2.0f : LODSettings.CullingDistance;
    
    if (Distance <= HighDist)
        return EPerf_LODLevel::High;
    else if (Distance <= MediumDist)
        return EPerf_LODLevel::Medium;
    else if (Distance <= LowDist)
        return EPerf_LODLevel::Low;
    else if (Distance <= CullDist)
        return EPerf_LODLevel::Low;
    else
        return EPerf_LODLevel::Disabled;
}

void UPerf_CollisionOptimizer::SetCollisionLODSettings(const FPerf_CollisionLODSettings& NewSettings)
{
    LODSettings = NewSettings;
    
    // Re-optimize all tracked actors with new settings
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    for (TWeakObjectPtr<AActor> ActorPtr : TrackedActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            OptimizeCollisionForActor(Actor, Distance);
        }
    }
}

void UPerf_CollisionOptimizer::CullDistantCollisions(float MaxDistance)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    CulledCollisionCount = 0;
    
    for (TWeakObjectPtr<AActor> ActorPtr : TrackedActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            if (Distance > MaxDistance)
            {
                DisableCollisionForActor(Actor);
                CulledCollisionCount++;
            }
        }
    }
}

void UPerf_CollisionOptimizer::EnableCollisionBatching(bool bEnable)
{
    bEnableCollisionBatching = bEnable;
    
    if (bEnable)
    {
        // Reduce tick frequency for batched processing
        PrimaryComponentTick.TickInterval = 0.2f;
        MaxCollisionsPerFrame = 50;
    }
    else
    {
        // Increase tick frequency for immediate processing
        PrimaryComponentTick.TickInterval = 0.1f;
        MaxCollisionsPerFrame = 100;
    }
}

bool UPerf_CollisionOptimizer::IsCollisionPerformanceOptimal() const
{
    return (CollisionCPUTime <= TARGET_COLLISION_CPU_TIME) && 
           (ActiveCollisionCount <= MAX_COLLISION_ACTORS);
}

void UPerf_CollisionOptimizer::UpdateCollisionComplexity(UPrimitiveComponent* Component, EPerf_LODLevel LODLevel)
{
    if (!Component)
        return;
        
    UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Component);
    if (StaticMeshComp)
    {
        switch (LODLevel)
        {
            case EPerf_LODLevel::High:
                StaticMeshComp->SetCollisionTraceFlag(ECollisionTraceFlag::CTF_UseComplexAsSimple);
                break;
                
            case EPerf_LODLevel::Medium:
                StaticMeshComp->SetCollisionTraceFlag(ECollisionTraceFlag::CTF_UseSimpleAsComplex);
                break;
                
            case EPerf_LODLevel::Low:
                StaticMeshComp->SetCollisionTraceFlag(ECollisionTraceFlag::CTF_UseSimpleAsComplex);
                OptimizeCollisionChannels(Component);
                break;
        }
    }
}

void UPerf_CollisionOptimizer::DisableCollisionForActor(AActor* Actor)
{
    if (!Actor)
        return;
        
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp)
    {
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UPerf_CollisionOptimizer::EnableCollisionForActor(AActor* Actor)
{
    if (!Actor)
        return;
        
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp)
    {
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

float UPerf_CollisionOptimizer::CalculateCollisionImportance(AActor* Actor)
{
    if (!Actor)
        return 0.0f;
        
    // Characters and pawns are always important
    if (Cast<ACharacter>(Actor) || Cast<APawn>(Actor))
        return 1.0f;
        
    // Large static meshes are moderately important
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp && StaticMeshComp->GetStaticMesh())
    {
        FVector BoundsSize = StaticMeshComp->GetStaticMesh()->GetBounds().BoxExtent;
        float MaxExtent = FMath::Max3(BoundsSize.X, BoundsSize.Y, BoundsSize.Z);
        
        if (MaxExtent > 500.0f)
            return 0.8f;
        else if (MaxExtent > 200.0f)
            return 0.6f;
        else
            return 0.4f;
    }
    
    return 0.3f; // Default importance for other actors
}

void UPerf_CollisionOptimizer::OptimizeCollisionChannels(UPrimitiveComponent* Component)
{
    if (!Component)
        return;
        
    // Simplify collision responses for distant objects
    Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    Component->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
    Component->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
}