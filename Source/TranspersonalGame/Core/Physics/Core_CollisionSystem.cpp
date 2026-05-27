#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentCollisionType = ECore_CollisionType::Environment;
    bCollisionEnabled = true;
    MinImpactForceThreshold = 100.0f;
    MaxTraceDistance = 10000.0f;
    bDebugTraces = false;
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem initialized for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Continuous collision monitoring if needed
    if (!bCollisionEnabled)
        return;
}

bool UCore_CollisionSystem::PerformLineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, ECore_CollisionType TraceType)
{
    if (!bCollisionEnabled || !GetWorld())
        return false;

    FCollisionQueryParams QueryParams = CreateQueryParams(TraceType);
    ECollisionChannel TraceChannel = GetCollisionChannelFromType(TraceType);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        TraceChannel,
        QueryParams
    );

    if (bDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugLine(GetWorld(), Start, End, TraceColor, false, 1.0f, 0, 2.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 25.0f, 12, FColor::Red, false, 1.0f);
        }
    }

    if (bHit)
    {
        float ImpactForce = FVector::Dist(Start, End) * 10.0f; // Simple force calculation
        ProcessCollisionEvent(OutHit, ImpactForce);
    }

    return bHit;
}

bool UCore_CollisionSystem::PerformSphereTrace(const FVector& Center, float Radius, FHitResult& OutHit, ECore_CollisionType TraceType)
{
    if (!bCollisionEnabled || !GetWorld())
        return false;

    FCollisionQueryParams QueryParams = CreateQueryParams(TraceType);
    ECollisionChannel TraceChannel = GetCollisionChannelFromType(TraceType);
    
    bool bHit = GetWorld()->SweepSingleByChannel(
        OutHit,
        Center,
        Center + FVector(0, 0, 1), // Small sweep distance
        FQuat::Identity,
        TraceChannel,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugSphere(GetWorld(), Center, Radius, 12, TraceColor, false, 1.0f);
    }

    if (bHit)
    {
        float ImpactForce = Radius * 50.0f; // Force based on sphere size
        ProcessCollisionEvent(OutHit, ImpactForce);
    }

    return bHit;
}

bool UCore_CollisionSystem::PerformBoxTrace(const FVector& Center, const FVector& HalfExtents, const FRotator& Rotation, FHitResult& OutHit, ECore_CollisionType TraceType)
{
    if (!bCollisionEnabled || !GetWorld())
        return false;

    FCollisionQueryParams QueryParams = CreateQueryParams(TraceType);
    ECollisionChannel TraceChannel = GetCollisionChannelFromType(TraceType);
    
    bool bHit = GetWorld()->SweepSingleByChannel(
        OutHit,
        Center,
        Center + FVector(0, 0, 1), // Small sweep distance
        Rotation.Quaternion(),
        TraceChannel,
        FCollisionShape::MakeBox(HalfExtents),
        QueryParams
    );

    if (bDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugBox(GetWorld(), Center, HalfExtents, Rotation.Quaternion(), TraceColor, false, 1.0f);
    }

    if (bHit)
    {
        float ImpactForce = HalfExtents.Size() * 25.0f; // Force based on box size
        ProcessCollisionEvent(OutHit, ImpactForce);
    }

    return bHit;
}

void UCore_CollisionSystem::ProcessCollisionEvent(const FHitResult& HitResult, float ImpactForce)
{
    if (ImpactForce < MinImpactForceThreshold)
        return;

    FCore_CollisionData CollisionData;
    CollisionData.CollisionType = CurrentCollisionType;
    CollisionData.ImpactForce = ImpactForce;
    CollisionData.ImpactLocation = HitResult.ImpactPoint;
    CollisionData.ImpactNormal = HitResult.ImpactNormal;
    CollisionData.HitActor = HitResult.GetActor();
    CollisionData.HitComponent = HitResult.GetComponent();

    // Broadcast collision event
    OnCollisionDetected.Broadcast(CollisionData, HitResult);

    // Handle collision response
    HandleCollisionResponse(CollisionData);

    UE_LOG(LogTemp, Log, TEXT("Collision detected: Force=%.2f, Actor=%s"), 
           ImpactForce, 
           CollisionData.HitActor ? *CollisionData.HitActor->GetName() : TEXT("None"));
}

void UCore_CollisionSystem::SetCollisionType(ECore_CollisionType NewType)
{
    CurrentCollisionType = NewType;
    UE_LOG(LogTemp, Log, TEXT("Collision type changed to: %d"), (int32)NewType);
}

void UCore_CollisionSystem::EnableCollisionDetection(bool bEnable)
{
    bCollisionEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Collision detection %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_CollisionSystem::ApplyImpactForce(UPrimitiveComponent* Component, const FVector& Force, const FVector& Location)
{
    if (!Component || !Component->IsSimulatingPhysics())
        return;

    Component->AddImpulseAtLocation(Force, Location);
    
    UE_LOG(LogTemp, Log, TEXT("Applied impact force: %.2f at location: %s"), 
           Force.Size(), *Location.ToString());
}

void UCore_CollisionSystem::SetPhysicsProperties(UPrimitiveComponent* Component, float Mass, float LinearDamping, float AngularDamping)
{
    if (!Component)
        return;

    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (BodyInstance)
    {
        BodyInstance->SetMassOverride(Mass);
        BodyInstance->LinearDamping = LinearDamping;
        BodyInstance->AngularDamping = AngularDamping;
        BodyInstance->UpdateMassProperties();
        
        UE_LOG(LogTemp, Log, TEXT("Physics properties updated: Mass=%.2f, LinearDamp=%.2f, AngularDamp=%.2f"), 
               Mass, LinearDamping, AngularDamping);
    }
}

void UCore_CollisionSystem::HandleCollisionResponse(const FCore_CollisionData& CollisionData)
{
    // Handle different collision types
    switch (CollisionData.CollisionType)
    {
        case ECore_CollisionType::Player:
            // Player collision response
            if (CollisionData.ImpactForce > 500.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("High impact player collision: %.2f"), CollisionData.ImpactForce);
            }
            break;

        case ECore_CollisionType::Dinosaur:
            // Dinosaur collision response
            if (CollisionData.HitComponent && CollisionData.ImpactForce > 300.0f)
            {
                ApplyImpactForce(CollisionData.HitComponent, 
                               CollisionData.ImpactNormal * CollisionData.ImpactForce * 0.5f, 
                               CollisionData.ImpactLocation);
            }
            break;

        case ECore_CollisionType::Environment:
            // Environmental collision response
            break;

        case ECore_CollisionType::Projectile:
            // Projectile collision response
            if (CollisionData.HitActor)
            {
                UE_LOG(LogTemp, Log, TEXT("Projectile hit: %s"), *CollisionData.HitActor->GetName());
            }
            break;

        default:
            break;
    }
}

ECollisionChannel UCore_CollisionSystem::GetCollisionChannelFromType(ECore_CollisionType Type)
{
    switch (Type)
    {
        case ECore_CollisionType::Player:
            return ECC_Pawn;
        case ECore_CollisionType::Dinosaur:
            return ECC_Pawn;
        case ECore_CollisionType::Environment:
            return ECC_WorldStatic;
        case ECore_CollisionType::Projectile:
            return ECC_WorldDynamic;
        case ECore_CollisionType::Debris:
            return ECC_WorldDynamic;
        case ECore_CollisionType::Water:
            return ECC_WorldStatic;
        case ECore_CollisionType::Vegetation:
            return ECC_WorldStatic;
        default:
            return ECC_WorldStatic;
    }
}

FCollisionQueryParams UCore_CollisionSystem::CreateQueryParams(ECore_CollisionType TraceType)
{
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;
    
    return QueryParams;
}