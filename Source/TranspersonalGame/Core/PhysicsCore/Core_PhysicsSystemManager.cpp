#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics settings
    GlobalGravityScale = 1.0f;
    DefaultLinearDamping = 0.01f;
    DefaultAngularDamping = 0.01f;
    bEnableRealisticPhysics = true;
    PhysicsSimulationRate = 60.0f;
    
    // Debug settings
    bShowPhysicsDebugInfo = false;
    bDrawCollisionShapes = false;
    bLogPhysicsEvents = false;
    
    // Internal state
    bPhysicsSystemInitialized = false;
    LastUpdateTime = 0.0f;
    PhysicsFrameCounter = 0;
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bPhysicsSystemInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core Physics System: Initializing physics system"));

    // Configure physics settings
    ConfigurePhysicsSettings();
    
    // Setup collision channels
    SetupCollisionChannels();
    
    // Initialize physics constraints
    InitializePhysicsConstraints();
    
    bPhysicsSystemInitialized = true;
    
    if (bLogPhysicsEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Core Physics System: Physics system initialized successfully"));
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    if (!bPhysicsSystemInitialized)
    {
        return;
    }

    LastUpdateTime += DeltaTime;
    PhysicsFrameCounter++;

    // Update ragdoll simulation
    UpdateRagdollSimulation(DeltaTime);
    
    // Update destruction simulation
    UpdateDestructionSimulation(DeltaTime);

    // Debug info
    if (bShowPhysicsDebugInfo && PhysicsFrameCounter % 60 == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics System: Frame %d, DeltaTime: %f"), PhysicsFrameCounter, DeltaTime);
    }
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bPhysicsSystemInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core Physics System: Shutting down physics system"));

    // Clean up ragdoll actors
    for (AActor* Actor : RagdollActors)
    {
        if (IsValid(Actor))
        {
            DisableRagdollPhysics(Actor);
        }
    }
    RagdollActors.Empty();

    // Clean up destructible actors
    DestructibleActors.Empty();
    
    // Clear physics data
    ActorPhysicsData.Empty();
    
    bPhysicsSystemInitialized = false;
}

bool UCore_PhysicsSystemManager::PerformLineTrace(FVector Start, FVector End, FHitResult& HitResult)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_WorldStatic,
        QueryParams
    );

    if (bDrawCollisionShapes && bHit)
    {
        DrawDebugLine(World, Start, HitResult.Location, FColor::Red, false, 1.0f);
        DrawDebugSphere(World, HitResult.Location, 5.0f, 12, FColor::Green, false, 1.0f);
    }

    return bHit;
}

bool UCore_PhysicsSystemManager::PerformSphereTrace(FVector Center, float Radius, FHitResult& HitResult)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = World->SweepSingleByChannel(
        HitResult,
        Center,
        Center + FVector(0, 0, -100), // Trace downward
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bDrawCollisionShapes)
    {
        DrawDebugSphere(World, Center, Radius, 12, bHit ? FColor::Red : FColor::Blue, false, 1.0f);
    }

    return bHit;
}

TArray<FHitResult> UCore_PhysicsSystemManager::PerformMultiLineTrace(FVector Start, FVector End)
{
    TArray<FHitResult> HitResults;
    UWorld* World = GetWorld();
    if (!World)
    {
        return HitResults;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;

    World->LineTraceMultiByChannel(
        HitResults,
        Start,
        End,
        ECC_WorldStatic,
        QueryParams
    );

    return HitResults;
}

void UCore_PhysicsSystemManager::EnableRagdollPhysics(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh && SkeletalMesh->GetPhysicsAsset())
    {
        SkeletalMesh->SetSimulatePhysics(true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        RagdollActors.AddUnique(Actor);
        
        if (bLogPhysicsEvents)
        {
            UE_LOG(LogTemp, Log, TEXT("Physics System: Enabled ragdoll for %s"), *Actor->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::DisableRagdollPhysics(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh)
    {
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        RagdollActors.Remove(Actor);
        
        if (bLogPhysicsEvents)
        {
            UE_LOG(LogTemp, Log, TEXT("Physics System: Disabled ragdoll for %s"), *Actor->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::ApplyImpulseToRagdoll(AActor* Actor, FVector Impulse, FVector Location)
{
    if (!IsValid(Actor))
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
    {
        SkeletalMesh->AddImpulseAtLocation(Impulse, Location);
        
        if (bLogPhysicsEvents)
        {
            UE_LOG(LogTemp, Log, TEXT("Physics System: Applied impulse %s to %s"), *Impulse.ToString(), *Actor->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::SimulateWindEffect(AActor* Actor, FVector WindDirection, float WindStrength)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
    {
        FVector WindForce = WindDirection.GetSafeNormal() * WindStrength;
        PrimitiveComp->AddForce(WindForce);
    }
}

void UCore_PhysicsSystemManager::ApplyGravityEffect(AActor* Actor, float GravityMultiplier)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimitiveComp)
    {
        PrimitiveComp->SetEnableGravity(true);
        // Note: UE5 doesn't have direct gravity multiplier per component
        // This would need to be implemented through custom physics or force application
    }
}

void UCore_PhysicsSystemManager::HandleWaterPhysics(AActor* Actor, bool bIsInWater)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimitiveComp)
    {
        if (bIsInWater)
        {
            // Apply buoyancy and water resistance
            PrimitiveComp->SetLinearDamping(DefaultLinearDamping * 5.0f);
            PrimitiveComp->SetAngularDamping(DefaultAngularDamping * 3.0f);
        }
        else
        {
            // Restore normal damping
            PrimitiveComp->SetLinearDamping(DefaultLinearDamping);
            PrimitiveComp->SetAngularDamping(DefaultAngularDamping);
        }
    }
}

void UCore_PhysicsSystemManager::CreateDestructibleMesh(UStaticMeshComponent* MeshComponent)
{
    if (!IsValid(MeshComponent))
    {
        return;
    }

    // Enable physics simulation for destruction
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    
    AActor* Owner = MeshComponent->GetOwner();
    if (Owner)
    {
        DestructibleActors.AddUnique(Owner);
    }
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, FVector ImpactPoint, float Force)
{
    if (!IsValid(Actor))
    {
        return;
    }

    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Apply destruction force
        FVector ForceDirection = (Actor->GetActorLocation() - ImpactPoint).GetSafeNormal();
        FVector DestructionForce = ForceDirection * Force;
        
        MeshComp->AddImpulseAtLocation(DestructionForce, ImpactPoint);
        
        if (bLogPhysicsEvents)
        {
            UE_LOG(LogTemp, Log, TEXT("Physics System: Triggered destruction on %s with force %f"), *Actor->GetName(), Force);
        }
    }
}

void UCore_PhysicsSystemManager::ConfigurePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Configure world physics settings
    if (World->GetPhysicsScene())
    {
        // Set realistic physics parameters for prehistoric survival
        // These settings provide more realistic physics simulation
    }
}

void UCore_PhysicsSystemManager::SetupCollisionChannels()
{
    // Setup custom collision channels for prehistoric game
    // This would typically be done in project settings
    // but we can configure responses here
}

void UCore_PhysicsSystemManager::InitializePhysicsConstraints()
{
    // Initialize any physics constraints needed for the game
    // Such as joint limits, spring constraints, etc.
}

void UCore_PhysicsSystemManager::UpdateRagdollSimulation(float DeltaTime)
{
    // Update ragdoll physics for all active ragdoll actors
    for (AActor* Actor : RagdollActors)
    {
        if (IsValid(Actor))
        {
            // Apply any per-frame ragdoll updates
            USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
            if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
            {
                // Apply damping or other per-frame effects
                SkeletalMesh->SetLinearDamping(DefaultLinearDamping);
                SkeletalMesh->SetAngularDamping(DefaultAngularDamping);
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdateDestructionSimulation(float DeltaTime)
{
    // Update destruction physics for all destructible actors
    for (int32 i = DestructibleActors.Num() - 1; i >= 0; i--)
    {
        AActor* Actor = DestructibleActors[i];
        if (!IsValid(Actor))
        {
            DestructibleActors.RemoveAt(i);
            continue;
        }
        
        // Check if destruction is complete and clean up if needed
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && !MeshComp->IsSimulatingPhysics())
        {
            // Destruction simulation ended
            DestructibleActors.RemoveAt(i);
        }
    }
}