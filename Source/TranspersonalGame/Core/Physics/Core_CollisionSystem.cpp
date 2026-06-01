#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    CollisionType = ECore_CollisionType::Environment;
    BaseDamage = 10.0f;
    ImpactForceMultiplier = 1000.0f;
    bEnableCollisionEvents = true;
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (AActor* Owner = GetOwner())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem initialized for actor: %s"), *Owner->GetName());
    }
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCore_CollisionSystem::SetCollisionType(ECore_CollisionType NewType)
{
    CollisionType = NewType;
    UE_LOG(LogTemp, Log, TEXT("Collision type changed to: %d"), (int32)NewType);
}

ECore_CollisionType UCore_CollisionSystem::GetCollisionType() const
{
    return CollisionType;
}

void UCore_CollisionSystem::ProcessCollision(const FHitResult& HitResult, AActor* OtherActor)
{
    if (!bEnableCollisionEvents || !OtherActor)
    {
        return;
    }

    // Determine collision type based on other actor
    if (UCore_CollisionSystem* OtherCollisionSystem = OtherActor->FindComponentByClass<UCore_CollisionSystem>())
    {
        ECore_CollisionType OtherType = OtherCollisionSystem->GetCollisionType();
        
        switch (OtherType)
        {
        case ECore_CollisionType::Dinosaur:
            HandleDinosaurCollision(OtherActor, HitResult);
            break;
        case ECore_CollisionType::Player:
            HandlePlayerCollision(OtherActor, HitResult);
            break;
        case ECore_CollisionType::Environment:
            HandleEnvironmentCollision(OtherActor, HitResult);
            break;
        default:
            break;
        }
    }

    // Create collision data for blueprint event
    FCore_CollisionData CollisionData = GetCollisionData();
    OnCollisionProcessed(CollisionData, OtherActor);
}

FCore_CollisionData UCore_CollisionSystem::GetCollisionData() const
{
    FCore_CollisionData Data;
    Data.CollisionType = CollisionType;
    Data.Damage = BaseDamage;
    Data.ImpactForce = FVector(ImpactForceMultiplier, 0.0f, 0.0f);
    Data.bCanBlock = true;
    return Data;
}

void UCore_CollisionSystem::ApplyImpactForce(AActor* TargetActor, const FVector& ImpactPoint, float Force)
{
    if (!TargetActor)
    {
        return;
    }

    if (UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            FVector ForceDirection = (TargetActor->GetActorLocation() - ImpactPoint).GetSafeNormal();
            FVector AppliedForce = ForceDirection * Force * ImpactForceMultiplier;
            
            PrimComp->AddImpulseAtLocation(AppliedForce, ImpactPoint);
            
            UE_LOG(LogTemp, Log, TEXT("Applied impact force: %s to actor: %s"), 
                   *AppliedForce.ToString(), *TargetActor->GetName());
        }
    }
}

void UCore_CollisionSystem::EnableCollisionEvents(bool bEnable)
{
    bEnableCollisionEvents = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Collision events %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UCore_CollisionSystem::IsCollisionEnabled() const
{
    return bEnableCollisionEvents;
}

void UCore_CollisionSystem::HandleDinosaurCollision(AActor* DinosaurActor, const FHitResult& HitResult)
{
    if (!DinosaurActor)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur collision detected with: %s"), *DinosaurActor->GetName());
    
    // Apply damage based on collision type
    float CollisionDamage = BaseDamage;
    if (CollisionType == ECore_CollisionType::Player)
    {
        CollisionDamage *= 2.0f; // Players take more damage from dinosaurs
    }
    
    // Apply impact force
    ApplyImpactForce(DinosaurActor, HitResult.ImpactPoint, BaseDamage * 10.0f);
}

void UCore_CollisionSystem::HandlePlayerCollision(AActor* PlayerActor, const FHitResult& HitResult)
{
    if (!PlayerActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Player collision detected with: %s"), *PlayerActor->GetName());
    
    // Lighter impact for player collisions
    ApplyImpactForce(PlayerActor, HitResult.ImpactPoint, BaseDamage * 5.0f);
}

void UCore_CollisionSystem::HandleEnvironmentCollision(AActor* EnvironmentActor, const FHitResult& HitResult)
{
    if (!EnvironmentActor)
    {
        return;
    }

    UE_LOG(LogTemp, Verbose, TEXT("Environment collision with: %s"), *EnvironmentActor->GetName());
    
    // Environment objects may break or move
    ApplyImpactForce(EnvironmentActor, HitResult.ImpactPoint, BaseDamage * 15.0f);
}

void UCore_CollisionSystem::CalculateImpactForce(const FHitResult& HitResult, FVector& OutForce)
{
    FVector ImpactNormal = HitResult.ImpactNormal;
    float ImpactVelocity = HitResult.Distance; // Approximation
    
    OutForce = -ImpactNormal * ImpactVelocity * ImpactForceMultiplier;
}