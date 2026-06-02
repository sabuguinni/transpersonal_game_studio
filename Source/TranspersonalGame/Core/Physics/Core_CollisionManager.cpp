#include "Core_CollisionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UCore_CollisionManager::UCore_CollisionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    MyCollisionType = ECore_CollisionType::Character;
    bEnableCollisionLogging = true;
    MinImpactForceThreshold = 100.0f;
    CollisionCooldown = 0.1f;
    MaxCollisionHistory = 50;
    TotalCollisions = 0;
    LastCollisionTime = 0.0f;
}

void UCore_CollisionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize collision history
    CollisionHistory.Reserve(MaxCollisionHistory);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Initialized for collision type %d"), (int32)MyCollisionType);
}

void UCore_CollisionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up old collision data if needed
    if (CollisionHistory.Num() > MaxCollisionHistory)
    {
        int32 ExcessCount = CollisionHistory.Num() - MaxCollisionHistory;
        CollisionHistory.RemoveAt(0, ExcessCount);
    }
}

void UCore_CollisionManager::RegisterCollision(const FHitResult& HitResult, float ImpactForce)
{
    if (IsWithinCooldown() || ImpactForce < MinImpactForceThreshold)
    {
        return;
    }
    
    if (ShouldIgnoreCollision(HitResult.GetActor()))
    {
        return;
    }
    
    // Create collision data
    FCore_CollisionData CollisionData;
    CollisionData.CollisionType = MyCollisionType;
    CollisionData.ImpactForce = ImpactForce;
    CollisionData.ImpactLocation = HitResult.ImpactPoint;
    CollisionData.ImpactNormal = HitResult.ImpactNormal;
    CollisionData.HitActor = HitResult.GetActor();
    CollisionData.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    ProcessCollisionData(CollisionData);
    
    // Update cooldown
    LastCollisionTime = CollisionData.Timestamp;
    TotalCollisions++;
    
    // Broadcast event
    OnCollisionDetected.Broadcast(CollisionData, HitResult.GetActor());
    
    if (bEnableCollisionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Collision registered - Force: %f, Actor: %s"), 
               ImpactForce, 
               HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None"));
    }
}

void UCore_CollisionManager::SetCollisionType(ECore_CollisionType NewType)
{
    MyCollisionType = NewType;
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Collision type changed to %d"), (int32)NewType);
}

bool UCore_CollisionManager::IsCollisionTypeCompatible(ECore_CollisionType OtherType) const
{
    // Define collision compatibility rules
    switch (MyCollisionType)
    {
        case ECore_CollisionType::Character:
            return OtherType != ECore_CollisionType::Character; // Characters don't collide with each other
            
        case ECore_CollisionType::Dinosaur:
            return OtherType == ECore_CollisionType::Character || 
                   OtherType == ECore_CollisionType::Environment ||
                   OtherType == ECore_CollisionType::Projectile;
            
        case ECore_CollisionType::Projectile:
            return OtherType != ECore_CollisionType::Projectile; // Projectiles don't collide with each other
            
        case ECore_CollisionType::Trigger:
            return true; // Triggers can collide with everything
            
        default:
            return true;
    }
}

void UCore_CollisionManager::EnableCollisionTracking(bool bEnable)
{
    bEnableCollisionLogging = bEnable;
    
    if (!bEnable)
    {
        ClearCollisionHistory();
    }
}

void UCore_CollisionManager::GetRecentCollisions(TArray<FCore_CollisionData>& OutCollisions, int32 Count)
{
    OutCollisions.Empty();
    
    int32 StartIndex = FMath::Max(0, CollisionHistory.Num() - Count);
    for (int32 i = StartIndex; i < CollisionHistory.Num(); i++)
    {
        OutCollisions.Add(CollisionHistory[i]);
    }
}

void UCore_CollisionManager::ClearCollisionHistory()
{
    CollisionHistory.Empty();
    TotalCollisions = 0;
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Collision history cleared"));
}

float UCore_CollisionManager::CalculateImpactForce(const FVector& Velocity, float Mass) const
{
    // F = ma, where a is derived from velocity change
    float Speed = Velocity.Size();
    return Speed * Mass * 0.1f; // Simplified calculation
}

bool UCore_CollisionManager::ShouldIgnoreCollision(AActor* OtherActor) const
{
    if (!OtherActor)
    {
        return true;
    }
    
    // Check if actor is in ignore list
    if (IgnoredActors.Contains(OtherActor))
    {
        return true;
    }
    
    // Ignore self-collision
    if (OtherActor == GetOwner())
    {
        return true;
    }
    
    return false;
}

void UCore_CollisionManager::ProcessCollisionData(const FCore_CollisionData& CollisionData)
{
    UpdateCollisionHistory(CollisionData);
    
    // Apply collision-specific logic based on type
    switch (CollisionData.CollisionType)
    {
        case ECore_CollisionType::Character:
            // Handle character collision effects
            break;
            
        case ECore_CollisionType::Dinosaur:
            // Handle dinosaur collision effects
            break;
            
        case ECore_CollisionType::Projectile:
            // Handle projectile collision effects
            break;
            
        default:
            break;
    }
}

void UCore_CollisionManager::UpdateCollisionHistory(const FCore_CollisionData& NewCollision)
{
    CollisionHistory.Add(NewCollision);
    
    // Maintain size limit
    while (CollisionHistory.Num() > MaxCollisionHistory)
    {
        CollisionHistory.RemoveAt(0);
    }
}

bool UCore_CollisionManager::IsWithinCooldown() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastCollisionTime) < CollisionCooldown;
}