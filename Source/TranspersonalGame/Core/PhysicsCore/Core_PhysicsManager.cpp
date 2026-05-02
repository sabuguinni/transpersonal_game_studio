#include "Core_PhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    ActivePhysicsActors = 0;
    ActiveRagdolls = 0;
    LastPhysicsUpdateTime = 0.0f;

    // Initialize default configurations
    DefaultPhysicsConfig = FCore_PhysicsConfig();
    DefaultRagdollConfig = FCore_RagdollConfig();
    DefaultDestructionConfig = FCore_DestructionConfig();
}

void UCore_PhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Initializing physics subsystem"));
    
    SetupCollisionChannels();
    
    // Start periodic cleanup timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(RagdollCleanupTimer, this, 
            &UCore_PhysicsManager::CleanupExpiredRagdolls, 5.0f, true);
        
        World->GetTimerManager().SetTimer(PhysicsStatsTimer, this, 
            &UCore_PhysicsManager::LogPhysicsStats, 30.0f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics subsystem initialized successfully"));
}

void UCore_PhysicsManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RagdollCleanupTimer);
        World->GetTimerManager().ClearTimer(PhysicsStatsTimer);
    }
    
    PhysicsStateMap.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics subsystem deinitialized"));
    
    Super::Deinitialize();
}

void UCore_PhysicsManager::ConfigurePhysicsForActor(AActor* Actor, const FCore_PhysicsConfig& Config)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Cannot configure physics for null actor"));
        return;
    }

    // Configure static mesh components
    if (UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        StaticMesh->SetMassOverrideInKg(NAME_None, Config.Mass);
        StaticMesh->SetLinearDamping(Config.LinearDamping);
        StaticMesh->SetAngularDamping(Config.AngularDamping);
        StaticMesh->SetEnableGravity(Config.bEnableGravity);
        StaticMesh->SetSimulatePhysics(Config.bSimulatePhysics);
        
        ApplyCollisionPreset(StaticMesh, Config.CollisionPreset);
    }

    // Configure skeletal mesh components
    if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        ConfigurePhysicsAsset(SkeletalMesh, Config);
    }

    // Update physics state tracking
    PhysicsStateMap.Add(Actor, Config.bSimulatePhysics ? ECore_PhysicsState::Active : ECore_PhysicsState::Inactive);
    
    if (Config.bSimulatePhysics)
    {
        ActivePhysicsActors++;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Configured physics for actor %s"), *Actor->GetName());
}

void UCore_PhysicsManager::SetPhysicsState(AActor* Actor, ECore_PhysicsState NewState)
{
    if (!Actor)
    {
        return;
    }

    ECore_PhysicsState* CurrentState = PhysicsStateMap.Find(Actor);
    if (CurrentState && *CurrentState == NewState)
    {
        return; // Already in desired state
    }

    switch (NewState)
    {
        case ECore_PhysicsState::Active:
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                PrimComp->SetSimulatePhysics(true);
                ActivePhysicsActors++;
            }
            break;

        case ECore_PhysicsState::Inactive:
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                PrimComp->SetSimulatePhysics(false);
                if (CurrentState && *CurrentState == ECore_PhysicsState::Active)
                {
                    ActivePhysicsActors = FMath::Max(0, ActivePhysicsActors - 1);
                }
            }
            break;

        case ECore_PhysicsState::Ragdoll:
            EnableRagdoll(Actor);
            break;

        case ECore_PhysicsState::Destroyed:
            DestroyWithFragments(Actor);
            break;

        default:
            break;
    }

    PhysicsStateMap.Add(Actor, NewState);
}

ECore_PhysicsState UCore_PhysicsManager::GetPhysicsState(AActor* Actor) const
{
    if (const ECore_PhysicsState* State = PhysicsStateMap.Find(Actor))
    {
        return *State;
    }
    return ECore_PhysicsState::Inactive;
}

void UCore_PhysicsManager::EnableRagdoll(AActor* Actor, const FCore_RagdollConfig& Config)
{
    if (!Actor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Cannot enable ragdoll - no skeletal mesh component found"));
        return;
    }

    // Disable animation and enable physics simulation
    if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(Config.BlendTime);
    }

    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Apply impulse if specified
    if (!Config.ImpulseForce.IsZero())
    {
        SkeletalMesh->AddImpulse(Config.ImpulseForce);
    }

    // Set lifespan for cleanup
    if (Config.LifeSpan > 0.0f)
    {
        Actor->SetLifeSpan(Config.LifeSpan);
    }

    PhysicsStateMap.Add(Actor, ECore_PhysicsState::Ragdoll);
    ActiveRagdolls++;

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Enabled ragdoll for actor %s"), *Actor->GetName());
}

void UCore_PhysicsManager::DisableRagdoll(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        return;
    }

    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    PhysicsStateMap.Add(Actor, ECore_PhysicsState::Inactive);
    ActiveRagdolls = FMath::Max(0, ActiveRagdolls - 1);

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Disabled ragdoll for actor %s"), *Actor->GetName());
}

bool UCore_PhysicsManager::IsRagdollActive(AActor* Actor) const
{
    return GetPhysicsState(Actor) == ECore_PhysicsState::Ragdoll;
}

void UCore_PhysicsManager::DestroyWithFragments(AActor* Actor, const FCore_DestructionConfig& Config)
{
    if (!Actor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Destroying actor %s with fragments"), *Actor->GetName());

    // Create fragments before destroying original
    CreateFragments(Actor, Config);

    // Apply explosion force at actor location
    ApplyExplosionForce(Actor->GetActorLocation(), Config.ExplosionForce, Config.ExplosionRadius);

    // Mark as destroyed and schedule for removal
    PhysicsStateMap.Add(Actor, ECore_PhysicsState::Destroyed);
    Actor->Destroy();
}

void UCore_PhysicsManager::ApplyExplosionForce(const FVector& Location, float Force, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find all physics actors in radius
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = World->OverlapMultiByChannel(
        OverlapResults,
        Location,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (UPrimitiveComponent* PrimComp = Result.GetComponent())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    FVector Direction = (PrimComp->GetComponentLocation() - Location).GetSafeNormal();
                    float Distance = FVector::Dist(Location, PrimComp->GetComponentLocation());
                    float FalloffForce = Force * (1.0f - (Distance / Radius));
                    
                    PrimComp->AddImpulseAtLocation(Direction * FalloffForce, Location);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Applied explosion force %f at location %s"), Force, *Location.ToString());
}

void UCore_PhysicsManager::ApplyCollisionPreset(UPrimitiveComponent* Component, ECore_CollisionPreset Preset)
{
    if (!Component)
    {
        return;
    }

    switch (Preset)
    {
        case ECore_CollisionPreset::Dinosaur:
            Component->SetCollisionObjectType(ECC_Pawn);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            break;

        case ECore_CollisionPreset::Player:
            Component->SetCollisionObjectType(ECC_Pawn);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            break;

        case ECore_CollisionPreset::Environment:
            Component->SetCollisionObjectType(ECC_WorldStatic);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            break;

        case ECore_CollisionPreset::Projectile:
            Component->SetCollisionObjectType(ECC_WorldDynamic);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            break;

        case ECore_CollisionPreset::Debris:
            Component->SetCollisionObjectType(ECC_WorldDynamic);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            break;

        default:
            // Keep default settings
            break;
    }
}

void UCore_PhysicsManager::ValidatePhysicsSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsManager: No world context for validation"));
        return;
    }

    int32 ValidActors = 0;
    int32 InvalidActors = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        bool bHasValidPhysics = false;
        
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                bHasValidPhysics = true;
                ValidActors++;
            }
        }

        if (!bHasValidPhysics && PhysicsStateMap.Contains(Actor))
        {
            InvalidActors++;
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Actor %s has physics state but no valid physics component"), *Actor->GetName());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics validation complete - Valid: %d, Invalid: %d"), ValidActors, InvalidActors);
}

void UCore_PhysicsManager::LogPhysicsStats()
{
    LastPhysicsUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager Stats - Active Physics Actors: %d, Active Ragdolls: %d, Tracked Actors: %d"), 
        ActivePhysicsActors, ActiveRagdolls, PhysicsStateMap.Num());
}

void UCore_PhysicsManager::SetupCollisionChannels()
{
    // This would typically set up custom collision channels
    // For now, we use the default UE5 channels
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Collision channels configured"));
}

void UCore_PhysicsManager::ConfigurePhysicsAsset(USkeletalMeshComponent* SkeletalMesh, const FCore_PhysicsConfig& Config)
{
    if (!SkeletalMesh || !SkeletalMesh->GetPhysicsAsset())
    {
        return;
    }

    UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
    
    // Configure physics asset properties
    for (int32 i = 0; i < PhysicsAsset->SkeletalBodySetups.Num(); ++i)
    {
        if (UBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[i])
        {
            BodySetup->DefaultInstance.SetMassOverride(Config.Mass / PhysicsAsset->SkeletalBodySetups.Num());
            BodySetup->DefaultInstance.LinearDamping = Config.LinearDamping;
            BodySetup->DefaultInstance.AngularDamping = Config.AngularDamping;
            BodySetup->DefaultInstance.bEnableGravity = Config.bEnableGravity;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Configured physics asset for skeletal mesh"));
}

void UCore_PhysicsManager::CreateFragments(AActor* OriginalActor, const FCore_DestructionConfig& Config)
{
    if (!OriginalActor)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get the original actor's mesh component
    UStaticMeshComponent* OriginalMesh = OriginalActor->FindComponentByClass<UStaticMeshComponent>();
    if (!OriginalMesh || !OriginalMesh->GetStaticMesh())
    {
        return;
    }

    FVector OriginalLocation = OriginalActor->GetActorLocation();
    FRotator OriginalRotation = OriginalActor->GetActorRotation();
    FVector OriginalScale = OriginalActor->GetActorScale3D();

    // Create fragments
    for (int32 i = 0; i < Config.MaxFragments; ++i)
    {
        // Spawn fragment actor
        AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>();
        if (!Fragment)
        {
            continue;
        }

        // Configure fragment mesh
        UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
        FragmentMesh->SetStaticMesh(OriginalMesh->GetStaticMesh());
        FragmentMesh->SetMaterial(0, OriginalMesh->GetMaterial(0));

        // Random position around original
        FVector RandomOffset = FVector(
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(0.0f, 50.0f)
        );
        Fragment->SetActorLocation(OriginalLocation + RandomOffset);
        Fragment->SetActorRotation(OriginalRotation + FRotator(
            FMath::RandRange(-45.0f, 45.0f),
            FMath::RandRange(-45.0f, 45.0f),
            FMath::RandRange(-45.0f, 45.0f)
        ));

        // Smaller scale for fragments
        float ScaleFactor = FMath::RandRange(0.1f, 0.3f);
        Fragment->SetActorScale3D(OriginalScale * ScaleFactor);

        // Enable physics
        FragmentMesh->SetSimulatePhysics(true);
        FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ApplyCollisionPreset(FragmentMesh, ECore_CollisionPreset::Debris);

        // Random impulse
        FVector RandomImpulse = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(200.0f, 800.0f)
        );
        FragmentMesh->AddImpulse(RandomImpulse);

        // Set lifespan
        Fragment->SetLifeSpan(Config.FragmentLifeSpan);

        // Track as physics actor
        PhysicsStateMap.Add(Fragment, ECore_PhysicsState::Active);
        ActivePhysicsActors++;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Created %d fragments for actor %s"), Config.MaxFragments, *OriginalActor->GetName());
}

void UCore_PhysicsManager::CleanupExpiredRagdolls()
{
    TArray<AActor*> ActorsToRemove;

    for (auto& Pair : PhysicsStateMap)
    {
        AActor* Actor = Pair.Key;
        ECore_PhysicsState State = Pair.Value;

        if (!IsValid(Actor) || State == ECore_PhysicsState::Destroyed)
        {
            ActorsToRemove.Add(Actor);
            
            if (State == ECore_PhysicsState::Ragdoll)
            {
                ActiveRagdolls = FMath::Max(0, ActiveRagdolls - 1);
            }
            else if (State == ECore_PhysicsState::Active)
            {
                ActivePhysicsActors = FMath::Max(0, ActivePhysicsActors - 1);
            }
        }
    }

    for (AActor* Actor : ActorsToRemove)
    {
        PhysicsStateMap.Remove(Actor);
    }

    if (ActorsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Cleaned up %d expired physics actors"), ActorsToRemove.Num());
    }
}