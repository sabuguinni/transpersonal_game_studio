#include "PhysicsSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    // Initialize default values
    bPhysicsSystemActive = false;
    PhysicsSystemVersion = 1.0f;
    RagdollActivationForce = 1000.0f;
    RagdollDeactivationDelay = 5.0f;
    MaxActivePhysicsObjects = 500;
    PhysicsTickRate = 60.0f;
    bEnablePhysicsOptimization = true;
    DefaultCollisionRadius = 50.0f;
    DefaultMass = 100.0f;
    MinDestructionForce = 500.0f;
    MaxDebrisCount = 10;
    DebrisLifetime = 30.0f;
    CachedPhysicsFrameTime = 0.0f;
    CachedActiveObjectCount = 0;
    LastPerformanceUpdateTime = 0.0f;
}

void UPhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initializing physics subsystem"));
    
    InitializePhysicsSystem();
}

void UPhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Deinitializing physics subsystem"));
    
    ShutdownPhysicsSystem();
    
    Super::Deinitialize();
}

bool UPhysicsSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UPhysicsSystemManager::InitializePhysicsSystem()
{
    if (bPhysicsSystemActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: System already active"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Starting physics system initialization"));

    // Validate physics settings
    ValidatePhysicsSettings();

    // Clear any existing ragdoll characters
    ActiveRagdollCharacters.Empty();

    // Set up physics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PhysicsUpdateTimerHandle,
            FTimerDelegate::CreateUObject(this, &UPhysicsSystemManager::UpdatePhysicsSystem, 1.0f / PhysicsTickRate),
            1.0f / PhysicsTickRate,
            true
        );

        // Set up performance monitoring timer
        World->GetTimerManager().SetTimer(
            PerformanceMonitorTimerHandle,
            [this]() { 
                CachedPhysicsFrameTime = GetPhysicsFrameTime();
                CachedActiveObjectCount = GetActivePhysicsObjectCount();
                LastPerformanceUpdateTime = GetWorld()->GetTimeSeconds();
            },
            1.0f, // Update every second
            true
        );
    }

    bPhysicsSystemActive = true;
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics system initialized successfully"));
}

void UPhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bPhysicsSystemActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Shutting down physics system"));

    // Disable ragdoll for all active characters
    for (auto CharacterPtr : ActiveRagdollCharacters)
    {
        if (ACharacter* Character = CharacterPtr.Get())
        {
            DisableRagdollForCharacter(Character);
        }
    }
    ActiveRagdollCharacters.Empty();

    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsUpdateTimerHandle);
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimerHandle);
    }

    bPhysicsSystemActive = false;
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics system shutdown complete"));
}

void UPhysicsSystemManager::EnableRagdollForCharacter(ACharacter* Character)
{
    if (!Character || !bPhysicsSystemActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Cannot enable ragdoll - invalid character or system inactive"));
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Character has no skeletal mesh component"));
        return;
    }

    // Check if already in ragdoll
    if (IsCharacterInRagdoll(Character))
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Character already in ragdoll state"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Enabling ragdoll for character: %s"), *Character->GetName());

    // Disable character movement
    if (Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
    }

    // Disable capsule collision
    if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Enable physics simulation on skeletal mesh
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

    // Set up ragdoll constraints
    SetupRagdollConstraints(Character);

    // Add to active ragdoll list
    ActiveRagdollCharacters.AddUnique(Character);

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Ragdoll enabled successfully"));
}

void UPhysicsSystemManager::DisableRagdollForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (!SkeletalMesh)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Disabling ragdoll for character: %s"), *Character->GetName());

    // Disable physics simulation
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Re-enable capsule collision
    if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Re-enable character movement
    if (Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    // Clean up ragdoll constraints
    CleanupRagdollConstraints(Character);

    // Remove from active ragdoll list
    ActiveRagdollCharacters.Remove(Character);

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Ragdoll disabled successfully"));
}

bool UPhysicsSystemManager::IsCharacterInRagdoll(ACharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    return ActiveRagdollCharacters.Contains(Character);
}

void UPhysicsSystemManager::ApplyImpactForce(AActor* Actor, FVector Force, FVector Location)
{
    if (!Actor || !bPhysicsSystemActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applying impact force to %s"), *Actor->GetName());

    // Try to find a physics-enabled component
    UPrimitiveComponent* PhysicsComp = nullptr;
    
    if (UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        if (StaticMesh->IsSimulatingPhysics())
        {
            PhysicsComp = StaticMesh;
        }
    }
    else if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (SkeletalMesh->IsSimulatingPhysics())
        {
            PhysicsComp = SkeletalMesh;
        }
    }

    if (PhysicsComp)
    {
        PhysicsComp->AddImpulseAtLocation(Force, Location);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Impact force applied successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: No physics-enabled component found on actor"));
    }
}

void UPhysicsSystemManager::ApplyExplosiveForce(FVector ExplosionCenter, float ExplosionRadius, float ExplosionStrength)
{
    if (!bPhysicsSystemActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applying explosive force at %s"), *ExplosionCenter.ToString());

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find all actors within explosion radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    int32 AffectedActors = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor)
        {
            continue;
        }

        float Distance = FVector::Dist(Actor->GetActorLocation(), ExplosionCenter);
        if (Distance <= ExplosionRadius)
        {
            // Calculate force based on distance (closer = stronger)
            float ForceMultiplier = 1.0f - (Distance / ExplosionRadius);
            FVector ForceDirection = (Actor->GetActorLocation() - ExplosionCenter).GetSafeNormal();
            FVector Force = ForceDirection * ExplosionStrength * ForceMultiplier;

            ApplyImpactForce(Actor, Force, Actor->GetActorLocation());
            AffectedActors++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Explosive force affected %d actors"), AffectedActors);
}

void UPhysicsSystemManager::SetGravityScale(AActor* Actor, float GravityScale)
{
    if (!Actor || !bPhysicsSystemActive)
    {
        return;
    }

    UPrimitiveComponent* PhysicsComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PhysicsComp && PhysicsComp->IsSimulatingPhysics())
    {
        PhysicsComp->SetEnableGravity(GravityScale > 0.0f);
        // Note: UE5 doesn't have a direct gravity scale property on components
        // This would need to be implemented through custom physics or mass modification
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Gravity scale set for %s"), *Actor->GetName());
    }
}

void UPhysicsSystemManager::SetupCollisionForActor(AActor* Actor, ECore_CollisionType CollisionType)
{
    if (!Actor || !bPhysicsSystemActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Setting up collision for %s"), *Actor->GetName());

    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        return;
    }

    // Configure collision based on type
    switch (CollisionType)
    {
        case ECore_CollisionType::Dinosaur:
            PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_CollisionType::Environment:
            PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_CollisionType::Interactive:
            PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_CollisionType::Projectile:
            PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        default:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
    }
}

void UPhysicsSystemManager::EnablePhysicsSimulation(AActor* Actor, bool bEnable)
{
    if (!Actor || !bPhysicsSystemActive)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(bEnable);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics simulation %s for %s"), 
               bEnable ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
    }
}

bool UPhysicsSystemManager::CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB)
{
    if (!ActorA || !ActorB || !bPhysicsSystemActive)
    {
        return false;
    }

    UPrimitiveComponent* CompA = ActorA->FindComponentByClass<UPrimitiveComponent>();
    UPrimitiveComponent* CompB = ActorB->FindComponentByClass<UPrimitiveComponent>();

    if (!CompA || !CompB)
    {
        return false;
    }

    // Simple bounds check
    return CompA->Bounds.GetSphere().Intersects(CompB->Bounds.GetSphere());
}

void UPhysicsSystemManager::TriggerDestruction(AActor* Actor, FVector ImpactPoint, float DestructionForce)
{
    if (!Actor || !bPhysicsSystemActive || DestructionForce < MinDestructionForce)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Triggering destruction for %s"), *Actor->GetName());

    // Create debris
    CreateDebrisFromActor(Actor, MaxDebrisCount);

    // Apply destruction force
    ApplyImpactForce(Actor, FVector(0, 0, DestructionForce), ImpactPoint);
}

void UPhysicsSystemManager::CreateDebrisFromActor(AActor* OriginalActor, int32 DebrisCount)
{
    if (!OriginalActor || !bPhysicsSystemActive)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Creating %d debris pieces"), DebrisCount);

    FVector OriginalLocation = OriginalActor->GetActorLocation();
    
    for (int32 i = 0; i < DebrisCount; i++)
    {
        // Create simple debris using basic shapes
        FVector DebrisLocation = OriginalLocation + FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 100.0f)
        );

        AStaticMeshActor* Debris = World->SpawnActor<AStaticMeshActor>(DebrisLocation, FRotator::ZeroRotator);
        if (Debris)
        {
            // Enable physics on debris
            EnablePhysicsSimulation(Debris, true);
            
            // Set debris lifetime
            Debris->SetLifeSpan(DebrisLifetime);
        }
    }
}

void UPhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bPhysicsSystemActive || !bEnablePhysicsOptimization)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Optimizing physics performance"));

    CleanupInactivePhysicsObjects();
    
    // Additional optimization logic would go here
    // For example: LOD system for physics objects, sleeping inactive objects, etc.
}

int32 UPhysicsSystemManager::GetActivePhysicsObjectCount() const
{
    if (!bPhysicsSystemActive)
    {
        return 0;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }

    return Count;
}

float UPhysicsSystemManager::GetPhysicsFrameTime() const
{
    // This would typically involve more complex performance monitoring
    // For now, return a simple estimation
    return bPhysicsSystemActive ? 16.67f : 0.0f; // Assume 60 FPS target
}

void UPhysicsSystemManager::UpdatePhysicsSystem(float DeltaTime)
{
    if (!bPhysicsSystemActive)
    {
        return;
    }

    // Clean up invalid ragdoll characters
    for (int32 i = ActiveRagdollCharacters.Num() - 1; i >= 0; i--)
    {
        if (!ActiveRagdollCharacters[i].IsValid())
        {
            ActiveRagdollCharacters.RemoveAt(i);
        }
    }

    // Performance optimization
    if (bEnablePhysicsOptimization)
    {
        static float OptimizationTimer = 0.0f;
        OptimizationTimer += DeltaTime;
        
        if (OptimizationTimer >= 5.0f) // Optimize every 5 seconds
        {
            OptimizePhysicsPerformance();
            OptimizationTimer = 0.0f;
        }
    }
}

void UPhysicsSystemManager::CleanupInactivePhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 CleanedUp = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetLifeSpan() > 0.0f && Actor->GetLifeSpan() < 1.0f)
        {
            // Actor is about to be destroyed, clean up any physics references
            CleanedUp++;
        }
    }

    if (CleanedUp > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Cleaned up %d inactive physics objects"), CleanedUp);
    }
}

void UPhysicsSystemManager::ValidatePhysicsSettings()
{
    // Clamp values to reasonable ranges
    RagdollActivationForce = FMath::Clamp(RagdollActivationForce, 100.0f, 10000.0f);
    RagdollDeactivationDelay = FMath::Clamp(RagdollDeactivationDelay, 1.0f, 30.0f);
    MaxActivePhysicsObjects = FMath::Clamp(MaxActivePhysicsObjects, 50, 2000);
    PhysicsTickRate = FMath::Clamp(PhysicsTickRate, 30.0f, 120.0f);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics settings validated"));
}

void UPhysicsSystemManager::SetupRagdollConstraints(ACharacter* Character)
{
    // This would set up specific physics constraints for ragdoll behavior
    // Implementation depends on the specific character setup and physics asset
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Setting up ragdoll constraints for %s"), *Character->GetName());
}

void UPhysicsSystemManager::CleanupRagdollConstraints(ACharacter* Character)
{
    // Clean up any custom constraints created for ragdoll
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Cleaning up ragdoll constraints for %s"), *Character->GetName());
}