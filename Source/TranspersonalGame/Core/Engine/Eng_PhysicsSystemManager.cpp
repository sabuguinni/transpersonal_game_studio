#include "Eng_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UEng_PhysicsSystemManager::UEng_PhysicsSystemManager()
{
    // Initialize default physics configuration
    PhysicsConfig = FEng_PhysicsConfiguration();
    bIsInitialized = false;
}

void UEng_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing..."));
    
    InitializePhysicsSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initialization complete"));
}

void UEng_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Deinitializing..."));
    
    // Clean up ragdoll actors
    for (auto& WeakActor : RagdollActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            DisableRagdollForActor(Actor);
        }
    }
    RagdollActors.Empty();
    
    // Clean up destructible actors
    DestructibleActors.Empty();
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UEng_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    // Validate configuration
    ValidatePhysicsConfiguration();
    
    // Apply physics settings
    ApplyPhysicsSettings();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics system initialized with gravity scale %f"), PhysicsConfig.GravityScale);
}

void UEng_PhysicsSystemManager::UpdatePhysicsSettings()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics settings updated"));
}

void UEng_PhysicsSystemManager::SetGravityScale(float NewGravityScale)
{
    PhysicsConfig.GravityScale = FMath::Clamp(NewGravityScale, 0.1f, 5.0f);
    
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetGravityZ(PhysicsConfig.WorldGravity.Z * PhysicsConfig.GravityScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Gravity scale set to %f"), PhysicsConfig.GravityScale);
}

void UEng_PhysicsSystemManager::SetWorldGravity(const FVector& NewGravity)
{
    PhysicsConfig.WorldGravity = NewGravity;
    
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetGravityZ(PhysicsConfig.WorldGravity.Z * PhysicsConfig.GravityScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: World gravity set to %s"), *NewGravity.ToString());
}

void UEng_PhysicsSystemManager::SetPhysicsConfiguration(const FEng_PhysicsConfiguration& NewConfig)
{
    PhysicsConfig = NewConfig;
    ValidatePhysicsConfiguration();
    
    if (bIsInitialized)
    {
        UpdatePhysicsSettings();
    }
}

void UEng_PhysicsSystemManager::EnableRagdollForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
        {
            SkeletalMesh->SetSimulatePhysics(true);
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
            
            // Add to ragdoll tracking
            RagdollActors.AddUnique(Actor);
            
            UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Enabled ragdoll for actor %s"), *Actor->GetName());
        }
    }
    else if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetSimulatePhysics(true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        RagdollActors.AddUnique(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Enabled ragdoll for actor %s"), *Actor->GetName());
    }
}

void UEng_PhysicsSystemManager::DisableRagdollForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
        {
            SkeletalMesh->SetSimulatePhysics(false);
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
    else if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    
    // Remove from ragdoll tracking
    RagdollActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return WeakActor.Get() == Actor;
    });
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Disabled ragdoll for actor %s"), *Actor->GetName());
}

bool UEng_PhysicsSystemManager::IsActorInRagdoll(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    return RagdollActors.ContainsByPredicate([Actor](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return WeakActor.Get() == Actor;
    });
}

void UEng_PhysicsSystemManager::TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float Damage)
{
    if (!Actor || !PhysicsConfig.bEnableDestruction)
    {
        return;
    }
    
    if (Damage < PhysicsConfig.DestructionThreshold)
    {
        return;
    }
    
    // Add impulse at impact point
    if (UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        FVector ImpulseDirection = (Actor->GetActorLocation() - ImpactPoint).GetSafeNormal();
        float ImpulseMagnitude = Damage * 100.0f;
        
        StaticMesh->AddImpulseAtLocation(ImpulseDirection * ImpulseMagnitude, ImpactPoint);
        
        // Track destructible actor
        DestructibleActors.AddUnique(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Triggered destruction for %s with damage %f"), *Actor->GetName(), Damage);
    }
}

void UEng_PhysicsSystemManager::SetDestructionThreshold(float NewThreshold)
{
    PhysicsConfig.DestructionThreshold = FMath::Max(NewThreshold, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Destruction threshold set to %f"), PhysicsConfig.DestructionThreshold);
}

void UEng_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    // Clean up weak references
    CleanupWeakReferences();
    
    // Cull distant physics bodies
    CullDistantPhysicsBodies();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Performance optimization complete"));
}

int32 UEng_PhysicsSystemManager::GetActivePhysicsBodies() const
{
    int32 ActiveBodies = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActiveBodies++;
                    }
                }
            }
        }
    }
    
    return ActiveBodies;
}

void UEng_PhysicsSystemManager::CullDistantPhysicsBodies()
{
    if (UWorld* World = GetWorld())
    {
        FVector PlayerLocation = FVector::ZeroVector;
        
        // Find player location
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                
                if (Distance > PhysicsConfig.CullDistance)
                {
                    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                    {
                        if (PrimComp->IsSimulatingPhysics())
                        {
                            PrimComp->SetSimulatePhysics(false);
                        }
                    }
                }
            }
        }
    }
}

void UEng_PhysicsSystemManager::DebugPhysicsSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bIsInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Gravity Scale: %f"), PhysicsConfig.GravityScale);
    UE_LOG(LogTemp, Warning, TEXT("World Gravity: %s"), *PhysicsConfig.WorldGravity.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Physics Tick Rate: %f"), PhysicsConfig.PhysicsTickRate);
    UE_LOG(LogTemp, Warning, TEXT("Max Physics Bodies: %d"), PhysicsConfig.MaxPhysicsBodies);
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Bodies: %d"), GetActivePhysicsBodies());
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll Actors: %d"), RagdollActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Destructible Actors: %d"), DestructibleActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Destruction Enabled: %s"), PhysicsConfig.bEnableDestruction ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Destruction Threshold: %f"), PhysicsConfig.DestructionThreshold);
    UE_LOG(LogTemp, Warning, TEXT("=== END DEBUG ==="));
}

void UEng_PhysicsSystemManager::LogPhysicsStats()
{
    int32 ActiveBodies = GetActivePhysicsBodies();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager Stats:"));
    UE_LOG(LogTemp, Log, TEXT("  Active Physics Bodies: %d/%d"), ActiveBodies, PhysicsConfig.MaxPhysicsBodies);
    UE_LOG(LogTemp, Log, TEXT("  Ragdoll Actors: %d"), RagdollActors.Num());
    UE_LOG(LogTemp, Log, TEXT("  Destructible Actors: %d"), DestructibleActors.Num());
}

void UEng_PhysicsSystemManager::DrawPhysicsDebugInfo(bool bEnabled)
{
    if (!bEnabled || !GetWorld())
    {
        return;
    }
    
    // Draw debug info for ragdoll actors
    for (const auto& WeakActor : RagdollActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 100.0f, 12, FColor::Red, false, 1.0f);
            DrawDebugString(GetWorld(), Actor->GetActorLocation() + FVector(0, 0, 150), TEXT("RAGDOLL"), nullptr, FColor::Red, 1.0f);
        }
    }
    
    // Draw debug info for destructible actors
    for (const auto& WeakActor : DestructibleActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 80.0f, 12, FColor::Orange, false, 1.0f);
            DrawDebugString(GetWorld(), Actor->GetActorLocation() + FVector(0, 0, 120), TEXT("DESTRUCTIBLE"), nullptr, FColor::Orange, 1.0f);
        }
    }
}

void UEng_PhysicsSystemManager::ApplyPhysicsSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Apply gravity settings
        World->GetPhysicsScene()->SetGravityZ(PhysicsConfig.WorldGravity.Z * PhysicsConfig.GravityScale);
        
        // Apply physics tick rate if possible
        if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
        {
            PhysicsSettings->MaxSubstepDeltaTime = 1.0f / PhysicsConfig.PhysicsTickRate;
            PhysicsSettings->MaxSubsteps = PhysicsConfig.MaxSubsteps;
        }
    }
}

void UEng_PhysicsSystemManager::CleanupWeakReferences()
{
    // Clean up ragdoll actors
    RagdollActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
    
    // Clean up destructible actors
    DestructibleActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
}

void UEng_PhysicsSystemManager::ValidatePhysicsConfiguration()
{
    // Clamp values to safe ranges
    PhysicsConfig.GravityScale = FMath::Clamp(PhysicsConfig.GravityScale, 0.1f, 5.0f);
    PhysicsConfig.PhysicsTickRate = FMath::Clamp(PhysicsConfig.PhysicsTickRate, 30.0f, 120.0f);
    PhysicsConfig.MaxSubsteps = FMath::Clamp(PhysicsConfig.MaxSubsteps, 1, 10);
    PhysicsConfig.CollisionTolerance = FMath::Clamp(PhysicsConfig.CollisionTolerance, 0.01f, 1.0f);
    PhysicsConfig.MaxPhysicsBodies = FMath::Clamp(PhysicsConfig.MaxPhysicsBodies, 100, 10000);
    PhysicsConfig.CullDistance = FMath::Max(PhysicsConfig.CullDistance, 1000.0f);
    PhysicsConfig.RagdollBlendTime = FMath::Clamp(PhysicsConfig.RagdollBlendTime, 0.1f, 2.0f);
    PhysicsConfig.DestructionThreshold = FMath::Max(PhysicsConfig.DestructionThreshold, 1.0f);
}