#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60fps
    
    // Default settings
    CollisionUpdateRate = 60.0f;
    MaxTraceDistance = 10000.0f;
    bEnableAdvancedCollision = true;
    bDebugCollision = false;
    
    LastCollisionCheckTime = 0.0f;
    LastUpdateTime = 0.0f;
    CollisionCheckCounter = 0;
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    SetupCollisionChannels();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Initialized with advanced collision detection"));
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableAdvancedCollision)
        return;
        
    LastUpdateTime += DeltaTime;
    
    // Update at specified rate
    if (LastUpdateTime >= (1.0f / CollisionUpdateRate))
    {
        UpdateCollisionTracking();
        ProcessCollisionEvents();
        CleanupInvalidComponents();
        
        LastUpdateTime = 0.0f;
        CollisionCheckCounter++;
    }
}

void UCore_CollisionSystem::SetupCollisionChannels()
{
    // Initialize collision ignore map with default settings
    CollisionIgnoreMap.Empty();
    
    // Player ignores other players but collides with dinosaurs and environment
    TArray<ECore_CollisionType> PlayerIgnores;
    PlayerIgnores.Add(ECore_CollisionType::Player);
    CollisionIgnoreMap.Add(ECore_CollisionType::Player, PlayerIgnores);
    
    // Dinosaurs collide with everything except other dinosaurs of same species
    TArray<ECore_CollisionType> DinosaurIgnores;
    DinosaurIgnores.Add(ECore_CollisionType::Dinosaur);
    CollisionIgnoreMap.Add(ECore_CollisionType::Dinosaur, DinosaurIgnores);
    
    // Environment collides with everything
    TArray<ECore_CollisionType> EnvironmentIgnores;
    CollisionIgnoreMap.Add(ECore_CollisionType::Environment, EnvironmentIgnores);
    
    // Projectiles ignore other projectiles
    TArray<ECore_CollisionType> ProjectileIgnores;
    ProjectileIgnores.Add(ECore_CollisionType::Projectile);
    CollisionIgnoreMap.Add(ECore_CollisionType::Projectile, ProjectileIgnores);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision channels configured"));
}

void UCore_CollisionSystem::RegisterCollisionObject(UPrimitiveComponent* Component, ECore_CollisionType CollisionType)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Attempted to register null component"));
        return;
    }
    
    RegisteredComponents.AddUnique(Component);
    
    // Update collision count
    int32* Count = CollisionCounts.Find(CollisionType);
    if (Count)
    {
        (*Count)++;
    }
    else
    {
        CollisionCounts.Add(CollisionType, 1);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Registered component %s as type %d"), 
           *Component->GetName(), (int32)CollisionType);
}

bool UCore_CollisionSystem::CheckCollisionBetween(AActor* ActorA, AActor* ActorB, float& Distance)
{
    if (!ActorA || !ActorB)
        return false;
        
    FVector LocationA = ActorA->GetActorLocation();
    FVector LocationB = ActorB->GetActorLocation();
    
    Distance = FVector::Dist(LocationA, LocationB);
    
    // Perform detailed collision check
    FHitResult HitResult;
    bool bHit = PerformLineTrace(LocationA, LocationB, HitResult, ECore_CollisionType::Environment);
    
    if (bDebugCollision)
    {
        DrawDebugLine(GetWorld(), LocationA, LocationB, 
                     bHit ? FColor::Red : FColor::Green, false, 1.0f);
    }
    
    return bHit;
}

TArray<FHitResult> UCore_CollisionSystem::PerformSphereTrace(FVector Start, FVector End, float Radius, ECore_CollisionType TraceType)
{
    TArray<FHitResult> HitResults;
    
    if (!GetWorld())
        return HitResults;
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    // Convert collision type to collision channel
    ECollisionChannel TraceChannel = ECC_WorldStatic;
    switch (TraceType)
    {
        case ECore_CollisionType::Player:
            TraceChannel = ECC_Pawn;
            break;
        case ECore_CollisionType::Dinosaur:
            TraceChannel = ECC_Pawn;
            break;
        case ECore_CollisionType::Environment:
            TraceChannel = ECC_WorldStatic;
            break;
        case ECore_CollisionType::Projectile:
            TraceChannel = ECC_WorldDynamic;
            break;
    }
    
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Start,
        End,
        FQuat::Identity,
        TraceChannel,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bDebugCollision && bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            DrawDebugSphere(GetWorld(), Hit.Location, Radius, 12, FColor::Orange, false, 1.0f);
        }
    }
    
    return HitResults;
}

bool UCore_CollisionSystem::PerformLineTrace(FVector Start, FVector End, FHitResult& OutHit, ECore_CollisionType TraceType)
{
    if (!GetWorld())
        return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    ECollisionChannel TraceChannel = ECC_WorldStatic;
    switch (TraceType)
    {
        case ECore_CollisionType::Player:
            TraceChannel = ECC_Pawn;
            break;
        case ECore_CollisionType::Dinosaur:
            TraceChannel = ECC_Pawn;
            break;
        case ECore_CollisionType::Environment:
            TraceChannel = ECC_WorldStatic;
            break;
        case ECore_CollisionType::Projectile:
            TraceChannel = ECC_WorldDynamic;
            break;
    }
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        TraceChannel,
        QueryParams
    );
    
    if (bDebugCollision)
    {
        DrawDebugLine(GetWorld(), Start, End, 
                     bHit ? FColor::Red : FColor::Green, false, 0.5f);
        if (bHit)
        {
            DrawDebugPoint(GetWorld(), OutHit.Location, 10.0f, FColor::Red, false, 0.5f);
        }
    }
    
    return bHit;
}

void UCore_CollisionSystem::HandleCollisionResponse(const FHitResult& Hit, ECore_CollisionResponse ResponseType)
{
    if (!Hit.GetActor())
        return;
    
    switch (ResponseType)
    {
        case ECore_CollisionResponse::Ignore:
            // Do nothing
            break;
            
        case ECore_CollisionResponse::Block:
            // Apply physics response
            ApplyPhysicsResponse(Hit, 1.0f);
            break;
            
        case ECore_CollisionResponse::Overlap:
            // Trigger overlap events
            TriggerCollisionEvents(Hit);
            break;
            
        case ECore_CollisionResponse::Destroy:
            // Destroy the hit actor
            if (Hit.GetActor())
            {
                Hit.GetActor()->Destroy();
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Handled collision response %d for actor %s"), 
           (int32)ResponseType, *Hit.GetActor()->GetName());
}

void UCore_CollisionSystem::ApplyImpactForce(AActor* Actor, FVector ImpactPoint, FVector Force)
{
    if (!Actor)
        return;
    
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->AddImpulseAtLocation(Force, ImpactPoint);
        
        if (bDebugCollision)
        {
            DrawDebugDirectionalArrow(GetWorld(), ImpactPoint, ImpactPoint + Force * 0.01f, 
                                    50.0f, FColor::Yellow, false, 2.0f);
        }
    }
}

void UCore_CollisionSystem::SetCollisionFilter(UPrimitiveComponent* Component, const TArray<ECore_CollisionType>& IgnoreTypes)
{
    if (!Component)
        return;
    
    // This would typically set up collision response matrix
    // For now, we'll store the ignore types for reference
    for (ECore_CollisionType IgnoreType : IgnoreTypes)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Component %s will ignore collision type %d"), 
               *Component->GetName(), (int32)IgnoreType);
    }
}

bool UCore_CollisionSystem::ShouldIgnoreCollision(ECore_CollisionType TypeA, ECore_CollisionType TypeB)
{
    const TArray<ECore_CollisionType>* IgnoreList = CollisionIgnoreMap.Find(TypeA);
    if (IgnoreList)
    {
        return IgnoreList->Contains(TypeB);
    }
    
    return false;
}

void UCore_CollisionSystem::UpdateCollisionTracking()
{
    LastCollisionCheckTime = GetWorld()->GetTimeSeconds();
    
    // Clean up invalid components
    RegisteredComponents.RemoveAll([](const UPrimitiveComponent* Comp) {
        return !IsValid(Comp);
    });
}

void UCore_CollisionSystem::ProcessCollisionEvents()
{
    // Process any pending collision events
    // This would typically handle queued collision responses
    
    if (CollisionCheckCounter % 60 == 0) // Log every second at 60fps
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_CollisionSystem: Processed %d collision checks, %d registered components"), 
               CollisionCheckCounter, RegisteredComponents.Num());
    }
}

void UCore_CollisionSystem::CleanupInvalidComponents()
{
    int32 InitialCount = RegisteredComponents.Num();
    
    RegisteredComponents.RemoveAll([](const UPrimitiveComponent* Comp) {
        return !IsValid(Comp) || !Comp->GetOwner();
    });
    
    int32 RemovedCount = InitialCount - RegisteredComponents.Num();
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Cleaned up %d invalid components"), RemovedCount);
    }
}

void UCore_CollisionSystem::ApplyPhysicsResponse(const FHitResult& Hit, float ImpactStrength)
{
    if (!Hit.GetActor())
        return;
    
    UPrimitiveComponent* HitComponent = Hit.GetComponent();
    if (HitComponent && HitComponent->IsSimulatingPhysics())
    {
        FVector ImpactDirection = (Hit.Location - Hit.TraceStart).GetSafeNormal();
        FVector Force = ImpactDirection * ImpactStrength * 1000.0f; // Scale force
        
        HitComponent->AddImpulseAtLocation(Force, Hit.Location);
    }
}

void UCore_CollisionSystem::TriggerCollisionEvents(const FHitResult& Hit)
{
    // This would typically trigger Blueprint events or delegate broadcasts
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Overlap event triggered with %s at %s"), 
           Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("Unknown"),
           *Hit.Location.ToString());
}