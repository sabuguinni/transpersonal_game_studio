#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize collision data
    CollisionData.CollisionType = ECore_CollisionType::Static;
    CollisionData.Mass = 1.0f;
    CollisionData.Friction = 0.7f;
    CollisionData.Restitution = 0.3f;
    CollisionData.bGenerateHitEvents = true;
    CollisionData.bCanEverAffectNavigation = true;
    
    bEnableCollisionSystem = true;
    CollisionCheckRadius = 100.0f;
    MaxCollisionForce = 1000.0f;
    LastCollisionTime = 0.0f;
    CollisionEventCount = 0;
    
    // Setup default collision object types
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCollisionSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem initialized for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableCollisionSystem || !GetOwner())
    {
        return;
    }
    
    // Process collision events
    ProcessCollisionQueue();
    
    // Cleanup old collision events
    CleanupOldCollisions();
    
    // Perform collision checks if dynamic
    if (CollisionData.CollisionType == ECore_CollisionType::Dynamic)
    {
        FVector ActorLocation = GetOwner()->GetActorLocation();
        TArray<AActor*> OverlappingActors = GetOverlappingActors(CollisionCheckRadius);
        
        for (AActor* Actor : OverlappingActors)
        {
            if (IsValidCollisionTarget(Actor))
            {
                FVector ImpactPoint = (ActorLocation + Actor->GetActorLocation()) * 0.5f;
                FVector ImpactNormal = (ActorLocation - Actor->GetActorLocation()).GetSafeNormal();
                float Distance = FVector::Dist(ActorLocation, Actor->GetActorLocation());
                
                if (Distance < CollisionCheckRadius)
                {
                    float Force = FMath::Clamp((CollisionCheckRadius - Distance) / CollisionCheckRadius * MaxCollisionForce, 0.0f, MaxCollisionForce);
                    RegisterCollisionEvent(Actor, ImpactPoint, ImpactNormal, Force);
                }
            }
        }
    }
}

void UCore_CollisionSystem::InitializeCollisionSystem()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CollisionSystem: No owner actor found"));
        return;
    }
    
    // Configure collision on owner's mesh component
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        UpdateCollisionProperties();
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision properties updated for %s"), *GetOwner()->GetName());
    }
    
    RecentCollisions.Empty();
    CollisionEventCount = 0;
    LastCollisionTime = GetWorld()->GetTimeSeconds();
}

void UCore_CollisionSystem::SetCollisionType(ECore_CollisionType NewType)
{
    CollisionData.CollisionType = NewType;
    UpdateCollisionProperties();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision type changed to %d for %s"), 
           (int32)NewType, GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

bool UCore_CollisionSystem::CheckCollisionAtLocation(const FVector& Location, float Radius)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;
    
    TArray<FOverlapResult> OverlapResults;
    bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        Location,
        FQuat::Identity,
        FCollisionObjectQueryParams(CollisionObjectTypes),
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    return bHasOverlap && OverlapResults.Num() > 0;
}

TArray<AActor*> UCore_CollisionSystem::GetOverlappingActors(float Radius)
{
    TArray<AActor*> OverlappingActors;
    
    if (!GetOwner() || !GetWorld())
    {
        return OverlappingActors;
    }
    
    FVector ActorLocation = GetOwner()->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    
    TArray<FOverlapResult> OverlapResults;
    bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        ActorLocation,
        FQuat::Identity,
        FCollisionObjectQueryParams(CollisionObjectTypes),
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHasOverlap)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor() && IsValidCollisionTarget(Result.GetActor()))
            {
                OverlappingActors.AddUnique(Result.GetActor());
            }
        }
    }
    
    return OverlappingActors;
}

void UCore_CollisionSystem::ApplyImpactForce(const FVector& ImpactPoint, const FVector& Force)
{
    if (!GetOwner())
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp && MeshComp->IsSimulatingPhysics())
    {
        MeshComp->AddImpulseAtLocation(Force, ImpactPoint);
        
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Applied impact force %s at %s"), 
               *Force.ToString(), *ImpactPoint.ToString());
    }
}

void UCore_CollisionSystem::RegisterCollisionEvent(AActor* HitActor, const FVector& ImpactPoint, const FVector& ImpactNormal, float Force)
{
    if (!HitActor)
    {
        return;
    }
    
    FCore_CollisionEvent NewEvent;
    NewEvent.HitActor = HitActor;
    NewEvent.ImpactPoint = ImpactPoint;
    NewEvent.ImpactNormal = ImpactNormal;
    NewEvent.ImpactForce = Force;
    NewEvent.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    RecentCollisions.Add(NewEvent);
    CollisionEventCount++;
    LastCollisionTime = NewEvent.Timestamp;
    
    // Broadcast collision event
    OnCollisionDetected.Broadcast(NewEvent, HitActor);
    
    // Handle collision response based on type
    FHitResult HitResult;
    HitResult.ImpactPoint = ImpactPoint;
    HitResult.ImpactNormal = ImpactNormal;
    HitResult.GetActor() = HitActor;
    
    switch (CollisionData.CollisionType)
    {
        case ECore_CollisionType::Static:
            HandleStaticCollision(HitResult);
            break;
        case ECore_CollisionType::Dynamic:
            HandleDynamicCollision(HitResult, Force);
            break;
        case ECore_CollisionType::Trigger:
            HandleTriggerCollision(HitActor);
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision registered with %s, Force: %f"), 
           *HitActor->GetName(), Force);
}

void UCore_CollisionSystem::EnableCollisionSystem(bool bEnable)
{
    bEnableCollisionSystem = bEnable;
    SetComponentTickEnabled(bEnable);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: System %s for %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_CollisionSystem::UpdateCollisionProperties()
{
    if (!GetOwner())
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    // Set collision based on collision type
    switch (CollisionData.CollisionType)
    {
        case ECore_CollisionType::Static:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldStatic);
            MeshComp->SetSimulatePhysics(false);
            break;
            
        case ECore_CollisionType::Dynamic:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetMassOverrideInKg(NAME_None, CollisionData.Mass, true);
            break;
            
        case ECore_CollisionType::Kinematic:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
            MeshComp->SetSimulatePhysics(false);
            break;
            
        case ECore_CollisionType::Trigger:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Overlap);
            break;
            
        case ECore_CollisionType::None:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            MeshComp->SetSimulatePhysics(false);
            break;
    }
    
    // Set physics material properties
    if (MeshComp->GetBodyInstance())
    {
        MeshComp->GetBodyInstance()->SetPhysMaterialOverride(nullptr);
        // Note: In a full implementation, you'd create and assign a physics material here
    }
}

void UCore_CollisionSystem::HandleStaticCollision(const FHitResult& HitResult)
{
    // Static objects don't move, but can generate sound/particles
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Static collision handled"));
}

void UCore_CollisionSystem::HandleDynamicCollision(const FHitResult& HitResult, float ImpactVelocity)
{
    if (!GetOwner())
    {
        return;
    }
    
    // Calculate and apply response force
    FVector ResponseForce = HitResult.ImpactNormal * ImpactVelocity * CollisionData.Mass * CollisionData.Restitution;
    ApplyImpactForce(HitResult.ImpactPoint, ResponseForce);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Dynamic collision handled, Response Force: %s"), 
           *ResponseForce.ToString());
}

void UCore_CollisionSystem::HandleTriggerCollision(AActor* TriggerActor)
{
    // Trigger events for gameplay systems
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Trigger collision with %s"), 
           TriggerActor ? *TriggerActor->GetName() : TEXT("Unknown"));
}

void UCore_CollisionSystem::ProcessCollisionQueue()
{
    // Process any queued collision events
    // This could be expanded to handle delayed collision responses
}

void UCore_CollisionSystem::CleanupOldCollisions()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MaxAge = 5.0f; // Keep collision events for 5 seconds
    
    RecentCollisions.RemoveAll([CurrentTime, MaxAge](const FCore_CollisionEvent& Event)
    {
        return (CurrentTime - Event.Timestamp) > MaxAge;
    });
}

float UCore_CollisionSystem::CalculateImpactForce(const FVector& Velocity, float Mass)
{
    return Velocity.Size() * Mass;
}

bool UCore_CollisionSystem::IsValidCollisionTarget(AActor* Actor)
{
    if (!Actor || Actor == GetOwner())
    {
        return false;
    }
    
    // Check if actor has collision components
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp || PrimComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        return false;
    }
    
    return true;
}