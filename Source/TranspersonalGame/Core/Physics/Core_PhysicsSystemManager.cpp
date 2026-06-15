#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    GlobalGravityScale = 1.0f;
    MaxPhysicsSimulationDistance = 5000.0f;
    bEnablePhysicsOptimization = true;
    MaxActivePhysicsActors = 500;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager Initialized"));
    
    // Set up physics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PhysicsUpdateTimer, 
            FTimerDelegate::CreateUObject(this, &UCore_PhysicsSystemManager::UpdatePhysicsActors, 0.016f), 
            0.016f, true); // 60 FPS update
            
        World->GetTimerManager().SetTimer(PhysicsCleanupTimer,
            FTimerDelegate::CreateUObject(this, &UCore_PhysicsSystemManager::CleanupInactivePhysicsActors),
            5.0f, true); // Cleanup every 5 seconds
    }
    
    // Initialize default physics settings
    SetGlobalPhysicsSettings(-980.0f, 0.01f, 0.0f);
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsUpdateTimer);
        World->GetTimerManager().ClearTimer(PhysicsCleanupTimer);
    }
    
    // Clear registered actors
    RegisteredPhysicsActors.Empty();
    ActivePhysicsActors.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager Deinitialized"));
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::RegisterPhysicsActor(AActor* Actor, ECore_PhysicsType PhysicsType)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register null physics actor"));
        return;
    }
    
    RegisteredPhysicsActors.Add(Actor, PhysicsType);
    
    if (!ActivePhysicsActors.Contains(Actor))
    {
        ActivePhysicsActors.Add(Actor);
    }
    
    // Configure physics based on type
    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent()->Cast<UPrimitiveComponent>())
    {
        switch (PhysicsType)
        {
            case ECore_PhysicsType::Static:
                PrimComp->SetMobility(EComponentMobility::Static);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;
                
            case ECore_PhysicsType::Dynamic:
                PrimComp->SetMobility(EComponentMobility::Movable);
                PrimComp->SetSimulatePhysics(true);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;
                
            case ECore_PhysicsType::Kinematic:
                PrimComp->SetMobility(EComponentMobility::Movable);
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered physics actor: %s with type: %d"), 
           *Actor->GetName(), (int32)PhysicsType);
}

void UCore_PhysicsSystemManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    RegisteredPhysicsActors.Remove(Actor);
    ActivePhysicsActors.Remove(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered physics actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetGlobalPhysicsSettings(float Gravity, float LinearDamping, float AngularDamping)
{
    GlobalGravityScale = Gravity / -980.0f; // Normalize to UE5 gravity scale
    
    if (UWorld* World = GetWorld())
    {
        // Apply global physics settings
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            WorldSettings->GlobalGravityZ = Gravity;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set global physics: Gravity=%.2f, LinearDamp=%.3f, AngularDamp=%.3f"), 
           Gravity, LinearDamping, AngularDamping);
}

void UCore_PhysicsSystemManager::ApplyImpactForce(AActor* TargetActor, FVector ImpactLocation, FVector ForceDirection, float ForceMagnitude)
{
    if (!TargetActor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = TargetActor->GetRootComponent()->Cast<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            FVector NormalizedForce = ForceDirection.GetSafeNormal() * ForceMagnitude;
            PrimComp->AddImpulseAtLocation(NormalizedForce, ImpactLocation);
            
            // Calculate impact damage
            float ImpactVelocity = ForceMagnitude / FMath::Max(PrimComp->GetMass(), 1.0f);
            float Damage = CalculateImpactDamage(ImpactVelocity, PrimComp->GetMass(), ECore_PhysicsType::Dynamic);
            
            // Broadcast impact event
            OnPhysicsImpact.Broadcast(TargetActor, ImpactLocation, ForceMagnitude);
            
            UE_LOG(LogTemp, Log, TEXT("Applied impact force %.2f to %s at location %s"), 
                   ForceMagnitude, *TargetActor->GetName(), *ImpactLocation.ToString());
        }
    }
}

void UCore_PhysicsSystemManager::ApplyRadialForce(FVector Origin, float Radius, float Strength, bool bVelChange)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> ActorsInRadius;
        UKismetSystemLibrary::SphereOverlapActors(World, Origin, Radius, TArray<TEnumAsByte<EObjectTypeQuery>>(), 
                                                  AActor::StaticClass(), TArray<AActor*>(), ActorsInRadius);
        
        for (AActor* Actor : ActorsInRadius)
        {
            if (RegisteredPhysicsActors.Contains(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent()->Cast<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        FVector ForceDirection = (Actor->GetActorLocation() - Origin).GetSafeNormal();
                        float Distance = FVector::Dist(Actor->GetActorLocation(), Origin);
                        float ForceMagnitude = Strength * (1.0f - (Distance / Radius));
                        
                        if (bVelChange)
                        {
                            PrimComp->AddImpulse(ForceDirection * ForceMagnitude, NAME_None, true);
                        }
                        else
                        {
                            PrimComp->AddForce(ForceDirection * ForceMagnitude);
                        }
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Applied radial force: Origin=%s, Radius=%.2f, Strength=%.2f, Affected=%d"), 
               *Origin.ToString(), Radius, Strength, ActorsInRadius.Num());
    }
}

void UCore_PhysicsSystemManager::ApplyExplosionForce(FVector ExplosionOrigin, float InnerRadius, float OuterRadius, float Strength, bool bLinearFalloff)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> ActorsInRange;
        UKismetSystemLibrary::SphereOverlapActors(World, ExplosionOrigin, OuterRadius, TArray<TEnumAsByte<EObjectTypeQuery>>(), 
                                                  AActor::StaticClass(), TArray<AActor*>(), ActorsInRange);
        
        for (AActor* Actor : ActorsInRange)
        {
            if (RegisteredPhysicsActors.Contains(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent()->Cast<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        FVector ForceDirection = (Actor->GetActorLocation() - ExplosionOrigin).GetSafeNormal();
                        float Distance = FVector::Dist(Actor->GetActorLocation(), ExplosionOrigin);
                        
                        float ForceMagnitude = 0.0f;
                        if (Distance <= InnerRadius)
                        {
                            ForceMagnitude = Strength;
                        }
                        else if (Distance <= OuterRadius)
                        {
                            float Alpha = (Distance - InnerRadius) / (OuterRadius - InnerRadius);
                            ForceMagnitude = bLinearFalloff ? 
                                Strength * (1.0f - Alpha) : 
                                Strength * FMath::Pow(1.0f - Alpha, 2.0f);
                        }
                        
                        PrimComp->AddImpulse(ForceDirection * ForceMagnitude, NAME_None, true);
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Applied explosion force: Origin=%s, InnerR=%.2f, OuterR=%.2f, Strength=%.2f"), 
               *ExplosionOrigin.ToString(), InnerRadius, OuterRadius, Strength);
    }
}

bool UCore_PhysicsSystemManager::LineTracePhysics(FVector Start, FVector End, FHitResult& HitResult, bool bTraceComplex)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = bTraceComplex;
        QueryParams.bReturnPhysicalMaterial = true;
        
        bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams);
        
        if (bHit && HitResult.GetActor())
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Physics line trace hit: %s at %s"), 
                   *HitResult.GetActor()->GetName(), *HitResult.Location.ToString());
        }
        
        return bHit;
    }
    
    return false;
}

TArray<FHitResult> UCore_PhysicsSystemManager::SphereTracePhysics(FVector Center, float Radius, TArray<AActor*> IgnoreActors)
{
    TArray<FHitResult> HitResults;
    
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActors(IgnoreActors);
        QueryParams.bReturnPhysicalMaterial = true;
        
        World->SweepMultiByChannel(HitResults, Center, Center + FVector(0, 0, 1), FQuat::Identity, 
                                   ECC_WorldStatic, FCollisionShape::MakeSphere(Radius), QueryParams);
    }
    
    return HitResults;
}

float UCore_PhysicsSystemManager::CalculateImpactDamage(float ImpactVelocity, float ActorMass, ECore_PhysicsType PhysicsType)
{
    // Base damage calculation: velocity * mass * type modifier
    float BaseDamage = ImpactVelocity * FMath::Sqrt(ActorMass) * 0.1f;
    
    // Apply physics type modifiers
    float TypeModifier = 1.0f;
    switch (PhysicsType)
    {
        case ECore_PhysicsType::Static:
            TypeModifier = 0.5f; // Static objects deal less impact damage
            break;
        case ECore_PhysicsType::Dynamic:
            TypeModifier = 1.0f; // Full damage
            break;
        case ECore_PhysicsType::Kinematic:
            TypeModifier = 0.8f; // Slightly reduced damage
            break;
    }
    
    float FinalDamage = BaseDamage * TypeModifier;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Calculated impact damage: Velocity=%.2f, Mass=%.2f, Type=%d, Damage=%.2f"), 
           ImpactVelocity, ActorMass, (int32)PhysicsType, FinalDamage);
    
    return FinalDamage;
}

void UCore_PhysicsSystemManager::SimulateRockfall(FVector Origin, int32 RockCount, float SpreadRadius)
{
    if (UWorld* World = GetWorld())
    {
        for (int32 i = 0; i < RockCount; ++i)
        {
            // Generate random position within spread radius
            FVector RandomOffset = FVector(
                FMath::RandRange(-SpreadRadius, SpreadRadius),
                FMath::RandRange(-SpreadRadius, SpreadRadius),
                FMath::RandRange(100.0f, 500.0f)
            );
            
            FVector SpawnLocation = Origin + RandomOffset;
            
            // Spawn a basic rock actor (this would be replaced with actual rock assets)
            if (AActor* RockActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator))
            {
                // Add static mesh component for the rock
                UStaticMeshComponent* MeshComp = RockActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
                RockActor->SetRootComponent(MeshComp);
                
                // Register as dynamic physics actor
                RegisterPhysicsActor(RockActor, ECore_PhysicsType::Dynamic);
                
                // Apply initial downward velocity
                if (UPrimitiveComponent* PrimComp = RockActor->GetRootComponent()->Cast<UPrimitiveComponent>())
                {
                    PrimComp->SetSimulatePhysics(true);
                    PrimComp->AddImpulse(FVector(0, 0, -1000.0f) + FVector(FMath::RandRange(-200, 200), FMath::RandRange(-200, 200), 0));
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Simulated rockfall: Origin=%s, Count=%d, Spread=%.2f"), 
               *Origin.ToString(), RockCount, SpreadRadius);
    }
}

void UCore_PhysicsSystemManager::SimulateTreeFall(AActor* TreeActor, FVector ImpactDirection, float ImpactForce)
{
    if (!TreeActor)
    {
        return;
    }
    
    // Apply force to make tree fall in impact direction
    if (UPrimitiveComponent* PrimComp = TreeActor->GetRootComponent()->Cast<UPrimitiveComponent>())
    {
        // Switch to dynamic physics
        RegisterPhysicsActor(TreeActor, ECore_PhysicsType::Dynamic);
        
        // Apply force at the top of the tree for realistic falling motion
        FVector TreeTop = TreeActor->GetActorLocation() + FVector(0, 0, 1000.0f);
        PrimComp->AddImpulseAtLocation(ImpactDirection.GetSafeNormal() * ImpactForce, TreeTop);
        
        // Broadcast break event
        OnPhysicsBreak.Broadcast(TreeActor, TreeActor->GetActorLocation());
        
        UE_LOG(LogTemp, Log, TEXT("Simulated tree fall: %s, Direction=%s, Force=%.2f"), 
               *TreeActor->GetName(), *ImpactDirection.ToString(), ImpactForce);
    }
}

void UCore_PhysicsSystemManager::CreateDebrisField(FVector Origin, float Radius, int32 DebrisCount)
{
    if (UWorld* World = GetWorld())
    {
        for (int32 i = 0; i < DebrisCount; ++i)
        {
            // Random position within radius
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            float Distance = FMath::RandRange(0.0f, Radius);
            FVector DebrisLocation = Origin + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                FMath::RandRange(10.0f, 100.0f)
            );
            
            // Spawn debris actor
            if (AActor* DebrisActor = World->SpawnActor<AActor>(AActor::StaticClass(), DebrisLocation, FRotator::ZeroRotator))
            {
                UStaticMeshComponent* MeshComp = DebrisActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebrisMesh"));
                DebrisActor->SetRootComponent(MeshComp);
                
                RegisterPhysicsActor(DebrisActor, ECore_PhysicsType::Dynamic);
                
                // Apply random impulse
                if (UPrimitiveComponent* PrimComp = DebrisActor->GetRootComponent()->Cast<UPrimitiveComponent>())
                {
                    PrimComp->SetSimulatePhysics(true);
                    FVector RandomImpulse = FVector(
                        FMath::RandRange(-500, 500),
                        FMath::RandRange(-500, 500),
                        FMath::RandRange(200, 800)
                    );
                    PrimComp->AddImpulse(RandomImpulse);
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Created debris field: Origin=%s, Radius=%.2f, Count=%d"), 
               *Origin.ToString(), Radius, DebrisCount);
    }
}

void UCore_PhysicsSystemManager::SetActorPhysicsMaterial(AActor* Actor, ECore_SurfaceType SurfaceType)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent()->Cast<UPrimitiveComponent>())
    {
        // This would set physics material based on surface type
        // In a real implementation, you'd load the appropriate physics material asset
        UE_LOG(LogTemp, Log, TEXT("Set physics material for %s to surface type %d"), 
               *Actor->GetName(), (int32)SurfaceType);
    }
}

ECore_SurfaceType UCore_PhysicsSystemManager::GetSurfaceTypeFromHit(const FHitResult& HitResult)
{
    // Default to stone if no specific surface detected
    ECore_SurfaceType SurfaceType = ECore_SurfaceType::Stone;
    
    if (HitResult.PhysMaterial.IsValid())
    {
        // In a real implementation, you'd check the physics material properties
        // and map them to your custom surface types
        UE_LOG(LogTemp, VeryVerbose, TEXT("Detected surface type from hit result"));
    }
    
    return SurfaceType;
}

void UCore_PhysicsSystemManager::OptimizePhysicsForPerformance(float MaxPhysicsDistance)
{
    MaxPhysicsSimulationDistance = MaxPhysicsDistance;
    
    if (UWorld* World = GetWorld())
    {
        // Get player location for distance calculations
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Disable physics for distant actors
            for (auto& ActorPair : RegisteredPhysicsActors)
            {
                AActor* Actor = ActorPair.Key;
                if (Actor && IsValid(Actor))
                {
                    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                    
                    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent()->Cast<UPrimitiveComponent>())
                    {
                        if (Distance > MaxPhysicsDistance)
                        {
                            // Disable physics simulation for distant objects
                            if (PrimComp->IsSimulatingPhysics())
                            {
                                PrimComp->SetSimulatePhysics(false);
                            }
                        }
                        else
                        {
                            // Re-enable physics for close objects
                            if (ActorPair.Value == ECore_PhysicsType::Dynamic && !PrimComp->IsSimulatingPhysics())
                            {
                                PrimComp->SetSimulatePhysics(true);
                            }
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized physics for performance: MaxDistance=%.2f"), MaxPhysicsDistance);
}

void UCore_PhysicsSystemManager::SetPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent()->Cast<UPrimitiveComponent>())
    {
        // Adjust collision complexity based on LOD level
        switch (LODLevel)
        {
            case 0: // High detail
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
                break;
            case 1: // Medium detail
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
                PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
                break;
            case 2: // Low detail
                PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
                PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set physics LOD for %s to level %d"), *Actor->GetName(), LODLevel);
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsInfo(bool bEnabled, float Duration)
{
    if (!bEnabled || !GetWorld())
    {
        return;
    }
    
    // Draw debug info for all registered physics actors
    for (auto& ActorPair : RegisteredPhysicsActors)
    {
        AActor* Actor = ActorPair.Key;
        ECore_PhysicsType PhysicsType = ActorPair.Value;
        
        if (Actor && IsValid(Actor))
        {
            FVector ActorLocation = Actor->GetActorLocation();
            FColor DebugColor = FColor::White;
            
            switch (PhysicsType)
            {
                case ECore_PhysicsType::Static:
                    DebugColor = FColor::Red;
                    break;
                case ECore_PhysicsType::Dynamic:
                    DebugColor = FColor::Green;
                    break;
                case ECore_PhysicsType::Kinematic:
                    DebugColor = FColor::Blue;
                    break;
            }
            
            DrawDebugSphere(GetWorld(), ActorLocation, 50.0f, 8, DebugColor, false, Duration);
            DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 100), 
                           FString::Printf(TEXT("%s (%d)"), *Actor->GetName(), (int32)PhysicsType), 
                           nullptr, DebugColor, Duration);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Debug draw physics info: Enabled=%s, Duration=%.2f, Actors=%d"), 
           bEnabled ? TEXT("true") : TEXT("false"), Duration, RegisteredPhysicsActors.Num());
}

void UCore_PhysicsSystemManager::UpdatePhysicsActors(float DeltaTime)
{
    // Performance optimization - limit active physics actors
    if (bEnablePhysicsOptimization && ActivePhysicsActors.Num() > MaxActivePhysicsActors)
    {
        OptimizePhysicsForPerformance(MaxPhysicsSimulationDistance);
    }
    
    // Process physics events
    ProcessPhysicsEvents();
}

void UCore_PhysicsSystemManager::ProcessPhysicsEvents()
{
    // This would process collision events, impact events, etc.
    // In a real implementation, you'd handle physics event callbacks here
}

void UCore_PhysicsSystemManager::CleanupInactivePhysicsActors()
{
    // Remove invalid actors from tracking
    TArray<AActor*> ActorsToRemove;
    
    for (auto& ActorPair : RegisteredPhysicsActors)
    {
        if (!IsValid(ActorPair.Key))
        {
            ActorsToRemove.Add(ActorPair.Key);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        UnregisterPhysicsActor(Actor);
    }
    
    // Clean up active actors list
    ActivePhysicsActors.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    if (ActorsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d inactive physics actors"), ActorsToRemove.Num());
    }
}