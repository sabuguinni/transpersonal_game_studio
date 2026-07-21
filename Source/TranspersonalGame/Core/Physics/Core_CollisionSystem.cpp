#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize default collision profiles
    InitializeDefaultProfiles();
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateCollisionProfiles();
    LastCollisionUpdate = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Initialized for actor %s"), *GetOwner()->GetName());
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bCollisionEnabled)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean up old collision events
    if (CurrentTime - LastCollisionUpdate > CollisionDeduplicationTime)
    {
        TArray<TPair<AActor*, AActor*>> KeysToRemove;
        for (auto& CollisionPair : RecentCollisions)
        {
            if (CurrentTime - CollisionPair.Value > CollisionDeduplicationTime)
            {
                KeysToRemove.Add(CollisionPair.Key);
            }
        }
        
        for (const auto& Key : KeysToRemove)
        {
            RecentCollisions.Remove(Key);
        }
        
        LastCollisionUpdate = CurrentTime;
    }
}

void UCore_CollisionSystem::RegisterCollisionCallback(AActor* Actor)
{
    if (!Actor)
        return;
    
    if (!RegisteredActors.Contains(Actor))
    {
        RegisteredActors.Add(Actor);
        
        // Set up collision events on all primitive components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                PrimComp->OnComponentHit.AddDynamic(this, &UCore_CollisionSystem::OnComponentHit);
                PrimComp->SetNotifyRigidBodyCollision(true);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Registered collision callback for %s"), *Actor->GetName());
    }
}

void UCore_CollisionSystem::UnregisterCollisionCallback(AActor* Actor)
{
    if (!Actor)
        return;
    
    RegisteredActors.Remove(Actor);
    
    // Remove collision events from all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->OnComponentHit.RemoveDynamic(this, &UCore_CollisionSystem::OnComponentHit);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Unregistered collision callback for %s"), *Actor->GetName());
}

float UCore_CollisionSystem::CalculateImpactDamage(float ImpactVelocity, float ObjectMass, ECore_ObjectType ObjectType)
{
    if (ImpactVelocity < MinImpactVelocity)
        return 0.0f;
    
    // Base damage calculation: velocity^2 * mass * type multiplier
    float BaseDamage = FMath::Square(ImpactVelocity) * ObjectMass * MassScalingFactor;
    
    // Apply object type multiplier
    float TypeMultiplier = 1.0f;
    switch (ObjectType)
    {
        case ECore_ObjectType::Character:
            TypeMultiplier = 0.8f;
            break;
        case ECore_ObjectType::Dinosaur:
            TypeMultiplier = 1.2f;
            break;
        case ECore_ObjectType::Vehicle:
            TypeMultiplier = 1.5f;
            break;
        case ECore_ObjectType::Projectile:
            TypeMultiplier = 2.0f;
            break;
        case ECore_ObjectType::Environment:
            TypeMultiplier = 0.5f;
            break;
        default:
            TypeMultiplier = 1.0f;
            break;
    }
    
    float FinalDamage = BaseDamage * BaseDamageMultiplier * TypeMultiplier;
    
    // Apply critical impact multiplier
    if (ImpactVelocity >= CriticalImpactThreshold)
    {
        FinalDamage *= 2.0f;
    }
    
    return FinalDamage;
}

void UCore_CollisionSystem::ApplyCollisionImpulse(AActor* Actor, const FVector& ImpactPoint, const FVector& ImpactForce)
{
    if (!Actor)
        return;
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive && RootPrimitive->IsSimulatingPhysics())
    {
        RootPrimitive->AddImpulseAtLocation(ImpactForce, ImpactPoint);
        
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Applied impulse %s at %s to %s"), 
               *ImpactForce.ToString(), *ImpactPoint.ToString(), *Actor->GetName());
    }
}

bool UCore_CollisionSystem::ShouldProcessCollision(AActor* ActorA, AActor* ActorB)
{
    if (!ActorA || !ActorB)
        return false;
    
    // Check if actors are in ignore list
    if (IgnoredActors.Contains(ActorA) || IgnoredActors.Contains(ActorB))
        return false;
    
    // Check for recent collision to avoid duplicate processing
    TPair<AActor*, AActor*> CollisionPair = TPair<AActor*, AActor*>(ActorA, ActorB);
    TPair<AActor*, AActor*> ReversePair = TPair<AActor*, AActor*>(ActorB, ActorA);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (RecentCollisions.Contains(CollisionPair) || RecentCollisions.Contains(ReversePair))
    {
        return false;
    }
    
    // Check collision type filters
    UPrimitiveComponent* PrimA = Cast<UPrimitiveComponent>(ActorA->GetRootComponent());
    UPrimitiveComponent* PrimB = Cast<UPrimitiveComponent>(ActorB->GetRootComponent());
    
    if (PrimA && PrimB)
    {
        bool bStaticA = PrimA->Mobility == EComponentMobility::Static;
        bool bStaticB = PrimB->Mobility == EComponentMobility::Static;
        
        if ((bStaticA || bStaticB) && !bCollideWithStatic)
            return false;
        
        if (!bStaticA && !bStaticB && !bCollideWithDynamic)
            return false;
    }
    
    return true;
}

FName UCore_CollisionSystem::GetCollisionProfileForType(ECore_ObjectType ObjectType)
{
    if (CollisionProfiles.Contains(ObjectType))
    {
        return CollisionProfiles[ObjectType];
    }
    
    return FName("Default");
}

void UCore_CollisionSystem::SetActorCollisionResponse(AActor* Actor, ECore_ObjectType ObjectType)
{
    if (!Actor)
        return;
    
    FName ProfileName = GetCollisionProfileForType(ObjectType);
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionProfileName(ProfileName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Set collision profile %s for %s"), 
           *ProfileName.ToString(), *Actor->GetName());
}

void UCore_CollisionSystem::SetActorCollisionEnabled(AActor* Actor, bool bEnabled)
{
    if (!Actor)
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Set collision enabled %s for %s"), 
           bEnabled ? TEXT("true") : TEXT("false"), *Actor->GetName());
}

bool UCore_CollisionSystem::PerformCollisionTrace(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
    if (!GetWorld())
        return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.bTraceComplex = true;
    
    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldDynamic, QueryParams);
}

TArray<AActor*> UCore_CollisionSystem::GetOverlappingActors(AActor* Actor)
{
    TArray<AActor*> OverlappingActors;
    
    if (!Actor)
        return OverlappingActors;
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->GetOverlappingActors(OverlappingActors);
    }
    
    return OverlappingActors;
}

void UCore_CollisionSystem::HandleCollision(AActor* ActorA, AActor* ActorB, const FHitResult& HitResult)
{
    if (!ShouldProcessCollision(ActorA, ActorB))
        return;
    
    // Record collision to prevent duplicate processing
    TPair<AActor*, AActor*> CollisionPair = TPair<AActor*, AActor*>(ActorA, ActorB);
    RecentCollisions.Add(CollisionPair, GetWorld()->GetTimeSeconds());
    
    // Calculate impact parameters
    FVector RelativeVelocity = FVector::ZeroVector;
    float ImpactVelocity = 0.0f;
    
    UPrimitiveComponent* PrimA = Cast<UPrimitiveComponent>(ActorA->GetRootComponent());
    UPrimitiveComponent* PrimB = Cast<UPrimitiveComponent>(ActorB->GetRootComponent());
    
    if (PrimA && PrimB)
    {
        FVector VelocityA = PrimA->GetPhysicsLinearVelocity();
        FVector VelocityB = PrimB->GetPhysicsLinearVelocity();
        RelativeVelocity = VelocityA - VelocityB;
        ImpactVelocity = RelativeVelocity.Size();
    }
    
    if (ImpactVelocity < MinImpactVelocity)
        return;
    
    // Calculate masses
    float MassA = PrimA ? PrimA->GetMass() : 1.0f;
    float MassB = PrimB ? PrimB->GetMass() : 1.0f;
    
    // Calculate collision response
    FVector ResponseForce = CalculateResponseForce(HitResult, ImpactVelocity, MassA + MassB);
    
    // Apply collision response
    ApplyCollisionImpulse(ActorA, HitResult.ImpactPoint, -ResponseForce * 0.5f);
    ApplyCollisionImpulse(ActorB, HitResult.ImpactPoint, ResponseForce * 0.5f);
    
    // Calculate and apply damage
    float DamageA = CalculateImpactDamage(ImpactVelocity, MassB, ECore_ObjectType::Default);
    float DamageB = CalculateImpactDamage(ImpactVelocity, MassA, ECore_ObjectType::Default);
    
    if (DamageA > 0.0f)
        ApplyCollisionDamage(ActorA, DamageA, HitResult.ImpactPoint);
    
    if (DamageB > 0.0f)
        ApplyCollisionDamage(ActorB, DamageB, HitResult.ImpactPoint);
    
    // Trigger events
    OnCollisionDetected(ActorA, ActorB, HitResult.ImpactPoint, ImpactVelocity);
    
    if (ImpactVelocity >= CriticalImpactThreshold)
    {
        OnCriticalImpact(ActorA, DamageA, HitResult.ImpactPoint);
        OnCriticalImpact(ActorB, DamageB, HitResult.ImpactPoint);
    }
    
    OnCollisionResponse(ActorA, -ResponseForce * 0.5f);
    OnCollisionResponse(ActorB, ResponseForce * 0.5f);
}

FVector UCore_CollisionSystem::CalculateResponseForce(const FHitResult& HitResult, float ImpactVelocity, float TotalMass)
{
    FVector ImpactNormal = HitResult.ImpactNormal;
    float ForceMultiplier = ImpactVelocity * TotalMass * MassScalingFactor;
    
    return ImpactNormal * ForceMultiplier;
}

void UCore_CollisionSystem::ApplyCollisionDamage(AActor* Actor, float Damage, const FVector& ImpactPoint)
{
    if (!Actor || Damage <= 0.0f)
        return;
    
    // Try to apply damage through UE5 damage system
    UGameplayStatics::ApplyPointDamage(Actor, Damage, ImpactPoint, FHitResult(), 
                                       nullptr, GetOwner(), UDamageType::StaticClass());
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Applied %f damage to %s at %s"), 
           Damage, *Actor->GetName(), *ImpactPoint.ToString());
}

void UCore_CollisionSystem::UpdateCollisionProfiles()
{
    // Ensure all object types have collision profiles
    if (!CollisionProfiles.Contains(ECore_ObjectType::Character))
        CollisionProfiles.Add(ECore_ObjectType::Character, FName("Pawn"));
    
    if (!CollisionProfiles.Contains(ECore_ObjectType::Dinosaur))
        CollisionProfiles.Add(ECore_ObjectType::Dinosaur, FName("Pawn"));
    
    if (!CollisionProfiles.Contains(ECore_ObjectType::Vehicle))
        CollisionProfiles.Add(ECore_ObjectType::Vehicle, FName("Vehicle"));
    
    if (!CollisionProfiles.Contains(ECore_ObjectType::Projectile))
        CollisionProfiles.Add(ECore_ObjectType::Projectile, FName("Projectile"));
    
    if (!CollisionProfiles.Contains(ECore_ObjectType::Environment))
        CollisionProfiles.Add(ECore_ObjectType::Environment, FName("WorldStatic"));
}

void UCore_CollisionSystem::InitializeDefaultProfiles()
{
    CollisionProfiles.Empty();
    
    CollisionProfiles.Add(ECore_ObjectType::Default, FName("Default"));
    CollisionProfiles.Add(ECore_ObjectType::Character, FName("Pawn"));
    CollisionProfiles.Add(ECore_ObjectType::Dinosaur, FName("Pawn"));
    CollisionProfiles.Add(ECore_ObjectType::Vehicle, FName("Vehicle"));
    CollisionProfiles.Add(ECore_ObjectType::Projectile, FName("Projectile"));
    CollisionProfiles.Add(ECore_ObjectType::Environment, FName("WorldStatic"));
    CollisionProfiles.Add(ECore_ObjectType::Interactive, FName("OverlapAllDynamic"));
    CollisionProfiles.Add(ECore_ObjectType::Debris, FName("PhysicsActor"));
}

UFUNCTION()
void UCore_CollisionSystem::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                                          UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                                          const FHitResult& Hit)
{
    if (!bCollisionEnabled || !OtherActor)
        return;
    
    HandleCollision(GetOwner(), OtherActor, Hit);
}