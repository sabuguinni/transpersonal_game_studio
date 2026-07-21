#include "Core_VelocitySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_VelocitySystem::UCore_VelocitySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / UpdateFrequency;
    
    // Initialize tracking containers
    PreviousPositions.Empty();
    CurrentVelocities.Empty();
    VelocityLimits.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Initialized velocity tracking system"));
}

void UCore_VelocitySystem::BeginPlay()
{
    Super::BeginPlay();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    TrackedActorCount = 0;
    TotalVelocityMagnitude = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Velocity system started"));
}

void UCore_VelocitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateVelocityTracking(DeltaTime);
    OptimizeVelocityTracking();
}

FVector UCore_VelocitySystem::GetActorVelocity(AActor* Actor) const
{
    if (!Actor)
    {
        return FVector::ZeroVector;
    }
    
    // Check if we have cached velocity
    if (const FVector* CachedVelocity = CurrentVelocities.Find(Actor))
    {
        return *CachedVelocity;
    }
    
    // Try to get velocity from physics component
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            return PrimComp->GetPhysicsLinearVelocity();
        }
    }
    
    // Try to get velocity from character movement
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            return MovementComp->Velocity;
        }
    }
    
    return FVector::ZeroVector;
}

float UCore_VelocitySystem::GetActorSpeed(AActor* Actor) const
{
    return GetActorVelocity(Actor).Size();
}

void UCore_VelocitySystem::SetVelocityLimit(AActor* Actor, float MaxSpeed)
{
    if (!Actor || MaxSpeed < 0.0f)
    {
        return;
    }
    
    VelocityLimits.Add(Actor, MaxSpeed);
    UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Set velocity limit %f for actor %s"), MaxSpeed, *Actor->GetName());
}

void UCore_VelocitySystem::ApplyVelocityDamping(AActor* Actor, float DampingFactor)
{
    if (!Actor || DampingFactor <= 0.0f)
    {
        return;
    }
    
    FVector CurrentVelocity = GetActorVelocity(Actor);
    FVector DampedVelocity = CurrentVelocity * (1.0f - DampingFactor);
    
    // Apply damped velocity to physics component
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetPhysicsLinearVelocity(DampedVelocity);
        }
    }
    
    // Update cached velocity
    CurrentVelocities.Add(Actor, DampedVelocity);
}

void UCore_VelocitySystem::ApplyMomentumTransfer(AActor* FromActor, AActor* ToActor, float TransferRatio)
{
    if (!FromActor || !ToActor || TransferRatio <= 0.0f || TransferRatio > 1.0f)
    {
        return;
    }
    
    float FromMass = 1.0f;
    float ToMass = 1.0f;
    
    // Get masses from physics components
    if (UPrimitiveComponent* FromPrimComp = FromActor->FindComponentByClass<UPrimitiveComponent>())
    {
        FromMass = FromPrimComp->GetMass();
    }
    
    if (UPrimitiveComponent* ToPrimComp = ToActor->FindComponentByClass<UPrimitiveComponent>())
    {
        ToMass = ToPrimComp->GetMass();
    }
    
    FVector FromVelocity = GetActorVelocity(FromActor);
    FVector ToVelocity = GetActorVelocity(ToActor);
    
    // Calculate momentum transfer
    FVector MomentumTransfer = FromVelocity * FromMass * TransferRatio;
    FVector NewToVelocity = ToVelocity + (MomentumTransfer / ToMass);
    FVector NewFromVelocity = FromVelocity - (MomentumTransfer / FromMass);
    
    // Apply new velocities
    if (UPrimitiveComponent* FromPrimComp = FromActor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (FromPrimComp->IsSimulatingPhysics())
        {
            FromPrimComp->SetPhysicsLinearVelocity(NewFromVelocity);
        }
    }
    
    if (UPrimitiveComponent* ToPrimComp = ToActor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (ToPrimComp->IsSimulatingPhysics())
        {
            ToPrimComp->SetPhysicsLinearVelocity(NewToVelocity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Applied momentum transfer between %s and %s"), 
           *FromActor->GetName(), *ToActor->GetName());
}

float UCore_VelocitySystem::CalculateMomentum(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    float Mass = 1.0f;
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        Mass = PrimComp->GetMass();
    }
    
    FVector Velocity = GetActorVelocity(Actor);
    return Mass * Velocity.Size();
}

void UCore_VelocitySystem::ConserveMomentum(const TArray<AActor*>& Actors)
{
    if (Actors.Num() < 2)
    {
        return;
    }
    
    FVector TotalMomentum = FVector::ZeroVector;
    float TotalMass = 0.0f;
    
    // Calculate total momentum and mass
    for (AActor* Actor : Actors)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Mass = 1.0f;
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            Mass = PrimComp->GetMass();
        }
        
        FVector Velocity = GetActorVelocity(Actor);
        TotalMomentum += Velocity * Mass;
        TotalMass += Mass;
    }
    
    // Apply conservation (simplified - equal distribution)
    if (TotalMass > 0.0f)
    {
        FVector AverageVelocity = TotalMomentum / TotalMass;
        
        for (AActor* Actor : Actors)
        {
            if (!Actor)
            {
                continue;
            }
            
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetPhysicsLinearVelocity(AverageVelocity);
                }
            }
        }
    }
}

void UCore_VelocitySystem::ProcessVelocityEffects(AActor* Actor, float DeltaTime)
{
    if (!Actor)
    {
        return;
    }
    
    // Apply air resistance if needed
    if (ShouldApplyAirResistance(Actor))
    {
        ApplyAirResistance(Actor, DeltaTime);
    }
    
    // Check velocity limits
    if (const float* MaxSpeed = VelocityLimits.Find(Actor))
    {
        float CurrentSpeed = GetActorSpeed(Actor);
        if (CurrentSpeed > *MaxSpeed)
        {
            FVector CurrentVelocity = GetActorVelocity(Actor);
            FVector LimitedVelocity = CurrentVelocity.GetSafeNormal() * (*MaxSpeed);
            
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetPhysicsLinearVelocity(LimitedVelocity);
                }
            }
        }
    }
}

bool UCore_VelocitySystem::ShouldApplyAirResistance(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Apply air resistance to fast-moving objects
    float Speed = GetActorSpeed(Actor);
    return Speed > 5.0f; // 5 m/s threshold
}

void UCore_VelocitySystem::ApplyAirResistance(AActor* Actor, float DeltaTime)
{
    if (!Actor)
    {
        return;
    }
    
    FVector Velocity = GetActorVelocity(Actor);
    float Speed = Velocity.Size();
    
    if (Speed < MinimumVelocityThreshold)
    {
        return;
    }
    
    float DragForce = CalculateDragForce(Actor, Velocity);
    FVector DragAcceleration = -Velocity.GetSafeNormal() * DragForce;
    
    // Apply drag as velocity change
    FVector VelocityChange = DragAcceleration * DeltaTime;
    FVector NewVelocity = Velocity + VelocityChange;
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetPhysicsLinearVelocity(NewVelocity);
        }
    }
}

void UCore_VelocitySystem::RegisterVelocityTracker(AActor* Actor)
{
    if (!IsActorValidForTracking(Actor))
    {
        return;
    }
    
    if (TrackedActorCount >= MaxTrackedActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VelocitySystem: Maximum tracked actors reached"));
        return;
    }
    
    PreviousPositions.Add(Actor, Actor->GetActorLocation());
    CurrentVelocities.Add(Actor, FVector::ZeroVector);
    TrackedActorCount++;
    
    UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Registered velocity tracker for %s"), *Actor->GetName());
}

void UCore_VelocitySystem::UnregisterVelocityTracker(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (PreviousPositions.Remove(Actor) > 0)
    {
        CurrentVelocities.Remove(Actor);
        VelocityLimits.Remove(Actor);
        TrackedActorCount--;
        
        UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Unregistered velocity tracker for %s"), *Actor->GetName());
    }
}

void UCore_VelocitySystem::UpdateVelocityTracking(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }
    
    TotalVelocityMagnitude = 0.0f;
    
    for (auto& Pair : PreviousPositions)
    {
        AActor* Actor = Pair.Key;
        if (!IsActorValidForTracking(Actor))
        {
            continue;
        }
        
        UpdateActorVelocity(Actor, DeltaTime);
        ProcessVelocityEffects(Actor, DeltaTime);
        
        // Update total velocity for performance monitoring
        TotalVelocityMagnitude += GetActorSpeed(Actor);
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

int32 UCore_VelocitySystem::GetTrackedActorCount() const
{
    return TrackedActorCount;
}

float UCore_VelocitySystem::GetAverageVelocity() const
{
    if (TrackedActorCount <= 0)
    {
        return 0.0f;
    }
    
    return TotalVelocityMagnitude / TrackedActorCount;
}

void UCore_VelocitySystem::OptimizeVelocityTracking()
{
    // Clean up invalid actors every few seconds
    static float LastCleanupTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastCleanupTime > 5.0f)
    {
        CleanupInvalidActors();
        LastCleanupTime = CurrentTime;
    }
}

bool UCore_VelocitySystem::IsActorValidForTracking(AActor* Actor) const
{
    if (!Actor || Actor->IsPendingKill())
    {
        return false;
    }
    
    // Check distance from owner
    if (AActor* Owner = GetOwner())
    {
        float Distance = FVector::Dist(Actor->GetActorLocation(), Owner->GetActorLocation());
        if (Distance > MaxTrackingDistance)
        {
            return false;
        }
    }
    
    return true;
}

float UCore_VelocitySystem::CalculateDragForce(AActor* Actor, const FVector& Velocity) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    float Speed = Velocity.Size();
    float CrossSectionalArea = 1.0f; // Simplified - could be calculated from bounds
    
    // F_drag = 0.5 * ρ * v² * C_d * A
    float DragForce = 0.5f * AirDensity * Speed * Speed * DefaultDragCoefficient * CrossSectionalArea;
    
    return DragForce;
}

void UCore_VelocitySystem::UpdateActorVelocity(AActor* Actor, float DeltaTime)
{
    if (!Actor)
    {
        return;
    }
    
    FVector CurrentPosition = Actor->GetActorLocation();
    FVector* PreviousPosition = PreviousPositions.Find(Actor);
    
    if (PreviousPosition)
    {
        FVector Velocity = (CurrentPosition - *PreviousPosition) / DeltaTime;
        CurrentVelocities.Add(Actor, Velocity);
        *PreviousPosition = CurrentPosition;
    }
    else
    {
        PreviousPositions.Add(Actor, CurrentPosition);
        CurrentVelocities.Add(Actor, FVector::ZeroVector);
    }
}

void UCore_VelocitySystem::CleanupInvalidActors()
{
    TArray<AActor*> ActorsToRemove;
    
    for (const auto& Pair : PreviousPositions)
    {
        if (!IsActorValidForTracking(Pair.Key))
        {
            ActorsToRemove.Add(Pair.Key);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        UnregisterVelocityTracker(Actor);
    }
    
    if (ActorsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_VelocitySystem: Cleaned up %d invalid actors"), ActorsToRemove.Num());
    }
}