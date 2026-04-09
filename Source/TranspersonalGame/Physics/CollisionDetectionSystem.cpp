#include "CollisionDetectionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogCollisionDetection, Log, All);

UCollisionDetectionSystem::UCollisionDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Set default values
    TraceSettings.TraceType = ECollisionTraceType::Line;
    TraceSettings.TraceDistance = 1000.0f;
    TraceSettings.TraceExtent = FVector(5.0f, 5.0f, 5.0f);
    TraceSettings.bTraceComplex = false;
    TraceSettings.bIgnoreSelf = true;
    TraceSettings.Priority = ECollisionPriority::Medium;
    
    FilterSettings.MinimumImpactVelocity = 10.0f;
    FilterSettings.bFilterByMass = false;
    FilterSettings.MinimumMass = 1.0f;
    
    bEnableContinuousDetection = true;
    DetectionFrequency = 60.0f;
    bUseLODBasedDetection = true;
    
    LODDistance1 = 500.0f;
    LODDistance2 = 1000.0f;
    LODDistance3 = 2000.0f;
    
    LastDetectionTime = 0.0f;
    CurrentLODLevel = 0;
}

void UCollisionDetectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCollisionDetection, Log, TEXT("CollisionDetectionSystem: Initializing for actor %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    InitializeCollisionSystem();
}

void UCollisionDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableContinuousDetection)
        return;
    
    // Update LOD level based on distance to camera
    if (bUseLODBasedDetection)
    {
        UpdateLODLevel();
    }
    
    // Perform collision detection based on frequency and LOD
    if (ShouldPerformDetection(DeltaTime))
    {
        UpdateCollisionDetection(DeltaTime);
        LastDetectionTime = GetWorld()->GetTimeSeconds();
    }
    
    // Update overlap tracking
    UpdateOverlapTracking();
}

void UCollisionDetectionSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up any bound delegates
    if (OwnerPrimitiveComponent)
    {
        OwnerPrimitiveComponent->OnComponentHit.RemoveDynamic(this, &UCollisionDetectionSystem::OnOwnerHit);
        OwnerPrimitiveComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UCollisionDetectionSystem::OnOwnerBeginOverlap);
        OwnerPrimitiveComponent->OnComponentEndOverlap.RemoveDynamic(this, &UCollisionDetectionSystem::OnOwnerEndOverlap);
    }
    
    CurrentlyOverlappingActors.Empty();
    PreviousCollisions.Empty();
    
    UE_LOG(LogCollisionDetection, Log, TEXT("CollisionDetectionSystem: Cleanup completed"));
    
    Super::EndPlay(EndPlayReason);
}

void UCollisionDetectionSystem::InitializeCollisionSystem()
{
    if (!GetOwner())
    {
        UE_LOG(LogCollisionDetection, Error, TEXT("CollisionDetectionSystem: No owner actor found"));
        return;
    }
    
    // Find the primary primitive component
    OwnerPrimitiveComponent = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
    
    if (!OwnerPrimitiveComponent)
    {
        // Try to find a static mesh component
        OwnerPrimitiveComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
        
        if (!OwnerPrimitiveComponent)
        {
            // Try to find a skeletal mesh component
            OwnerPrimitiveComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (OwnerPrimitiveComponent)
    {
        // Bind collision events
        OwnerPrimitiveComponent->OnComponentHit.AddDynamic(this, &UCollisionDetectionSystem::OnOwnerHit);
        OwnerPrimitiveComponent->OnComponentBeginOverlap.AddDynamic(this, &UCollisionDetectionSystem::OnOwnerBeginOverlap);
        OwnerPrimitiveComponent->OnComponentEndOverlap.AddDynamic(this, &UCollisionDetectionSystem::OnOwnerEndOverlap);
        
        UE_LOG(LogCollisionDetection, Log, TEXT("CollisionDetectionSystem: Bound to primitive component %s"), 
               *OwnerPrimitiveComponent->GetName());
    }
    else
    {
        UE_LOG(LogCollisionDetection, Warning, TEXT("CollisionDetectionSystem: No primitive component found on owner"));
    }
}

void UCollisionDetectionSystem::UpdateCollisionDetection(float DeltaTime)
{
    if (!GetOwner() || !GetWorld())
        return;
    
    // Clear previous collision results
    PreviousCollisions.Empty();
    
    // Perform forward-looking collision detection
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector OwnerVelocity = FVector::ZeroVector;
    
    // Get velocity from physics component if available
    if (OwnerPrimitiveComponent && OwnerPrimitiveComponent->IsSimulatingPhysics())
    {
        OwnerVelocity = OwnerPrimitiveComponent->GetPhysicsLinearVelocity();
    }
    
    // Predict future position based on velocity
    FVector PredictedLocation = OwnerLocation + (OwnerVelocity * DeltaTime * 2.0f);
    
    // Perform trace from current to predicted location
    FCollisionResult Result = PerformLineTrace(OwnerLocation, PredictedLocation);
    
    if (Result.bHit)
    {
        ProcessCollisionResult(Result, DeltaTime);
    }
    
    // Additional traces based on LOD level
    if (CurrentLODLevel <= 1)
    {
        // High detail - perform additional directional traces
        TArray<FVector> TraceDirections = {
            GetOwner()->GetActorForwardVector(),
            GetOwner()->GetActorRightVector(),
            -GetOwner()->GetActorRightVector(),
            FVector::UpVector,
            -FVector::UpVector
        };
        
        for (const FVector& Direction : TraceDirections)
        {
            FVector TraceEnd = OwnerLocation + (Direction * TraceSettings.TraceDistance * 0.5f);
            FCollisionResult DirectionalResult = PerformLineTrace(OwnerLocation, TraceEnd);
            
            if (DirectionalResult.bHit)
            {
                ProcessCollisionResult(DirectionalResult, DeltaTime);
            }
        }
    }
}

void UCollisionDetectionSystem::UpdateLODLevel()
{
    if (!GetWorld() || !GetOwner())
        return;
    
    // Get camera location (simplified - using player pawn location)
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
        return;
    
    float DistanceToCamera = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    
    int32 NewLODLevel = 0;
    if (DistanceToCamera > LODDistance3)
    {
        NewLODLevel = 3;
    }
    else if (DistanceToCamera > LODDistance2)
    {
        NewLODLevel = 2;
    }
    else if (DistanceToCamera > LODDistance1)
    {
        NewLODLevel = 1;
    }
    
    if (NewLODLevel != CurrentLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        UE_LOG(LogCollisionDetection, VeryVerbose, TEXT("CollisionDetectionSystem: LOD level changed to %d for %s"), 
               CurrentLODLevel, *GetOwner()->GetName());
    }
}

FCollisionResult UCollisionDetectionSystem::PerformLineTrace(const FVector& Start, const FVector& End)
{
    return ExecuteTrace(Start, End, ECollisionTraceType::Line);
}

FCollisionResult UCollisionDetectionSystem::PerformSphereTrace(const FVector& Start, const FVector& End, float Radius)
{
    TraceSettings.TraceExtent = FVector(Radius);
    return ExecuteTrace(Start, End, ECollisionTraceType::Sphere);
}

FCollisionResult UCollisionDetectionSystem::PerformBoxTrace(const FVector& Start, const FVector& End, const FVector& HalfExtent)
{
    TraceSettings.TraceExtent = HalfExtent;
    return ExecuteTrace(Start, End, ECollisionTraceType::Box);
}

FCollisionResult UCollisionDetectionSystem::PerformCapsuleTrace(const FVector& Start, const FVector& End, float Radius, float HalfHeight)
{
    TraceSettings.TraceExtent = FVector(Radius, Radius, HalfHeight);
    return ExecuteTrace(Start, End, ECollisionTraceType::Capsule);
}

TArray<FCollisionResult> UCollisionDetectionSystem::PerformMultiTrace(const FVector& Start, const FVector& End)
{
    TArray<FCollisionResult> Results;
    
    if (!GetWorld())
        return Results;
    
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams = BuildQueryParams();
    
    bool bHit = GetWorld()->LineTraceMultiByChannel(
        HitResults,
        Start,
        End,
        ECC_WorldDynamic,
        QueryParams
    );
    
    if (bHit)
    {
        for (const FHitResult& HitResult : HitResults)
        {
            if (PassesFilter(HitResult))
            {
                Results.Add(ConvertHitResult(HitResult));
            }
        }
    }
    
    return Results;
}

FCollisionResult UCollisionDetectionSystem::ExecuteTrace(const FVector& Start, const FVector& End, ECollisionTraceType TraceType)
{
    FCollisionResult Result;
    
    if (!GetWorld())
        return Result;
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams = BuildQueryParams();
    bool bHit = false;
    
    switch (TraceType)
    {
        case ECollisionTraceType::Line:
            bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldDynamic, QueryParams);
            break;
            
        case ECollisionTraceType::Sphere:
            bHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, 
                                                   FCollisionShape::MakeSphere(TraceSettings.TraceExtent.X), QueryParams);
            break;
            
        case ECollisionTraceType::Box:
            bHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, 
                                                   FCollisionShape::MakeBox(TraceSettings.TraceExtent), QueryParams);
            break;
            
        case ECollisionTraceType::Capsule:
            bHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, 
                                                   FCollisionShape::MakeCapsule(TraceSettings.TraceExtent.X, TraceSettings.TraceExtent.Z), QueryParams);
            break;
    }
    
    if (bHit && PassesFilter(HitResult))
    {
        Result = ConvertHitResult(HitResult);
    }
    
    return Result;
}

FCollisionQueryParams UCollisionDetectionSystem::BuildQueryParams() const
{
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = TraceSettings.bTraceComplex;
    QueryParams.bReturnPhysicalMaterial = true;
    
    if (TraceSettings.bIgnoreSelf && GetOwner())
    {
        QueryParams.AddIgnoredActor(GetOwner());
    }
    
    // Add filtered actors
    for (AActor* IgnoreActor : FilterSettings.IgnoreActors)
    {
        if (IgnoreActor)
        {
            QueryParams.AddIgnoredActor(IgnoreActor);
        }
    }
    
    return QueryParams;
}

FCollisionObjectQueryParams UCollisionDetectionSystem::BuildObjectQueryParams() const
{
    FCollisionObjectQueryParams ObjectParams;
    
    // Add all channels except ignored ones
    TArray<EObjectTypeQuery> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
    
    ObjectParams = FCollisionObjectQueryParams(ObjectTypes);
    
    return ObjectParams;
}

void UCollisionDetectionSystem::ProcessCollisionResult(const FCollisionResult& Result, float DeltaTime)
{
    if (!Result.bHit)
        return;
    
    // Store collision result
    PreviousCollisions.Add(Result);
    
    // Broadcast collision detected event
    OnCollisionDetected.Broadcast(Result, DeltaTime);
    
    // Check for high-speed collisions
    CheckForHighSpeedCollisions(Result);
    
    UE_LOG(LogCollisionDetection, VeryVerbose, TEXT("CollisionDetectionSystem: Collision detected with %s at distance %.2f"), 
           Result.HitActor ? *Result.HitActor->GetName() : TEXT("Unknown"), Result.Distance);
}

void UCollisionDetectionSystem::CheckForHighSpeedCollisions(const FCollisionResult& Result)
{
    float ImpactSpeed = Result.ImpactVelocity.Size();
    
    if (ImpactSpeed > FilterSettings.MinimumImpactVelocity * 3.0f) // High-speed threshold
    {
        if (Result.HitActor && GetOwner())
        {
            OnHighSpeedCollision.Broadcast(GetOwner(), Result.HitActor, Result.ImpactForce);
            
            UE_LOG(LogCollisionDetection, Warning, TEXT("CollisionDetectionSystem: High-speed collision detected! Speed: %.2f, Force: %.2f"), 
                   ImpactSpeed, Result.ImpactForce);
        }
    }
}

void UCollisionDetectionSystem::UpdateOverlapTracking()
{
    // This method tracks overlap changes for enter/exit events
    // Implementation would track overlapping actors and fire events when they change
    // For now, we rely on the component's built-in overlap events
}

bool UCollisionDetectionSystem::ShouldPerformDetection(float DeltaTime) const
{
    if (DetectionFrequency <= 0.0f)
        return false;
    
    float TimeSinceLastDetection = GetWorld()->GetTimeSeconds() - LastDetectionTime;
    float RequiredInterval = 1.0f / GetLODBasedFrequency();
    
    return TimeSinceLastDetection >= RequiredInterval;
}

float UCollisionDetectionSystem::GetLODBasedFrequency() const
{
    if (!bUseLODBasedDetection)
        return DetectionFrequency;
    
    // Reduce frequency based on LOD level
    switch (CurrentLODLevel)
    {
        case 0: return DetectionFrequency;
        case 1: return DetectionFrequency * 0.75f;
        case 2: return DetectionFrequency * 0.5f;
        case 3: return DetectionFrequency * 0.25f;
        default: return DetectionFrequency;
    }
}

FCollisionResult UCollisionDetectionSystem::ConvertHitResult(const FHitResult& HitResult) const
{
    FCollisionResult Result;
    
    Result.bHit = HitResult.bBlockingHit;
    Result.HitLocation = HitResult.Location;
    Result.HitNormal = HitResult.Normal;
    Result.HitActor = HitResult.GetActor();
    Result.HitComponent = HitResult.GetComponent();
    Result.Distance = HitResult.Distance;
    
    // Calculate impact velocity and force
    if (OwnerPrimitiveComponent && OwnerPrimitiveComponent->IsSimulatingPhysics())
    {
        Result.ImpactVelocity = OwnerPrimitiveComponent->GetPhysicsLinearVelocity();
        float Mass = OwnerPrimitiveComponent->GetMass();
        Result.ImpactForce = CalculateImpactForce(Result.ImpactVelocity, Mass);
    }
    
    return Result;
}

float UCollisionDetectionSystem::CalculateImpactForce(const FVector& Velocity, float Mass) const
{
    // F = ma, where a is deceleration (velocity change over time)
    // Simplified calculation assuming instant deceleration
    return Velocity.Size() * Mass;
}

bool UCollisionDetectionSystem::PassesFilter(const FHitResult& HitResult) const
{
    if (!HitResult.GetActor())
        return false;
    
    // Check ignore classes
    for (UClass* IgnoreClass : FilterSettings.IgnoreClasses)
    {
        if (IgnoreClass && HitResult.GetActor()->IsA(IgnoreClass))
        {
            return false;
        }
    }
    
    // Check mass filter
    if (FilterSettings.bFilterByMass)
    {
        if (UPrimitiveComponent* HitPrimitive = HitResult.GetComponent())
        {
            if (HitPrimitive->IsSimulatingPhysics())
            {
                float HitMass = HitPrimitive->GetMass();
                if (HitMass < FilterSettings.MinimumMass)
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

// Event handlers
void UCollisionDetectionSystem::OnOwnerHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || !PassesFilter(Hit))
        return;
    
    FCollisionResult Result = ConvertHitResult(Hit);
    ProcessCollisionResult(Result, GetWorld()->GetDeltaSeconds());
}

void UCollisionDetectionSystem::OnOwnerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
        return;
    
    bool bWasAlreadyOverlapping = CurrentlyOverlappingActors.Contains(OtherActor);
    CurrentlyOverlappingActors.Add(OtherActor);
    
    if (!bWasAlreadyOverlapping)
    {
        OnCollisionEnter.Broadcast(OverlappedComponent, OtherComponent);
        
        UE_LOG(LogCollisionDetection, VeryVerbose, TEXT("CollisionDetectionSystem: Begin overlap with %s"), 
               *OtherActor->GetName());
    }
}

void UCollisionDetectionSystem::OnOwnerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (!OtherActor)
        return;
    
    bool bWasOverlapping = CurrentlyOverlappingActors.Remove(OtherActor) > 0;
    
    if (bWasOverlapping)
    {
        OnCollisionExit.Broadcast(OverlappedComponent, OtherComponent);
        
        UE_LOG(LogCollisionDetection, VeryVerbose, TEXT("CollisionDetectionSystem: End overlap with %s"), 
               *OtherActor->GetName());
    }
}

// Public interface methods
void UCollisionDetectionSystem::SetTraceSettings(const FCollisionTraceSettings& NewSettings)
{
    TraceSettings = NewSettings;
    UE_LOG(LogCollisionDetection, Log, TEXT("CollisionDetectionSystem: Trace settings updated"));
}

void UCollisionDetectionSystem::SetFilterSettings(const FCollisionFilterSettings& NewSettings)
{
    FilterSettings = NewSettings;
    UE_LOG(LogCollisionDetection, Log, TEXT("CollisionDetectionSystem: Filter settings updated"));
}

void UCollisionDetectionSystem::AddIgnoreActor(AActor* ActorToIgnore)
{
    if (ActorToIgnore && !FilterSettings.IgnoreActors.Contains(ActorToIgnore))
    {
        FilterSettings.IgnoreActors.Add(ActorToIgnore);
    }
}

void UCollisionDetectionSystem::RemoveIgnoreActor(AActor* ActorToRemove)
{
    FilterSettings.IgnoreActors.Remove(ActorToRemove);
}

void UCollisionDetectionSystem::ClearIgnoreActors()
{
    FilterSettings.IgnoreActors.Empty();
}

bool UCollisionDetectionSystem::IsActorInRange(AActor* TargetActor, float Range) const
{
    if (!TargetActor || !GetOwner())
        return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
    return Distance <= Range;
}

TArray<AActor*> UCollisionDetectionSystem::GetActorsInRadius(const FVector& Center, float Radius) const
{
    TArray<AActor*> FoundActors;
    
    if (!GetWorld())
        return FoundActors;
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams = BuildQueryParams();
    
    bool bHit = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Center,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                FoundActors.AddUnique(Result.GetActor());
            }
        }
    }
    
    return FoundActors;
}

AActor* UCollisionDetectionSystem::GetNearestActor(const FVector& Location, float MaxRange) const
{
    TArray<AActor*> ActorsInRange = GetActorsInRadius(Location, MaxRange);
    
    AActor* NearestActor = nullptr;
    float NearestDistance = MaxRange;
    
    for (AActor* Actor : ActorsInRange)
    {
        if (!Actor)
            continue;
        
        float Distance = FVector::Dist(Location, Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestActor = Actor;
        }
    }
    
    return NearestActor;
}

void UCollisionDetectionSystem::SetLODDistances(float LOD1, float LOD2, float LOD3)
{
    LODDistance1 = LOD1;
    LODDistance2 = LOD2;
    LODDistance3 = LOD3;
    
    UE_LOG(LogCollisionDetection, Log, TEXT("CollisionDetectionSystem: LOD distances updated - LOD1: %.2f, LOD2: %.2f, LOD3: %.2f"), 
           LOD1, LOD2, LOD3);
}

int32 UCollisionDetectionSystem::GetCurrentLODLevel() const
{
    return CurrentLODLevel;
}