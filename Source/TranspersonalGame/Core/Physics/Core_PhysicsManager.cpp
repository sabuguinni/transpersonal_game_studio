#include "Core_PhysicsManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

ACore_PhysicsManager::ACore_PhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    // Initialize default settings
    DefaultSettings.Mass = 100.0f;
    DefaultSettings.LinearDamping = 0.01f;
    DefaultSettings.AngularDamping = 0.01f;
    DefaultSettings.bEnableGravity = true;
    DefaultSettings.PhysicsMode = ECore_PhysicsMode::Simulated;
    
    GlobalGravityScale = 1.0f;
    bEnablePhysicsSimulation = true;
    MaxSimulatedBodies = 1000;
    CurrentSimulatedBodies = 0;
}

void ACore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Initialized with max bodies: %d"), MaxSimulatedBodies);
    
    // Set initial gravity scale
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            WorldSettings->GlobalGravityZ = WorldSettings->GlobalGravityZ * GlobalGravityScale;
        }
    }
    
    // Initial count of simulated bodies
    UpdateSimulatedBodiesCount();
}

void ACore_PhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnablePhysicsSimulation)
    {
        UpdateSimulatedBodiesCount();
        EnforceBodyLimits();
    }
}

void ACore_PhysicsManager::ApplyPhysicsSettings(UPrimitiveComponent* Component, const FCore_PhysicsSettings& Settings)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Null component passed to ApplyPhysicsSettings"));
        return;
    }
    
    // Set physics mode
    switch (Settings.PhysicsMode)
    {
        case ECore_PhysicsMode::Static:
            Component->SetMobility(EComponentMobility::Static);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_PhysicsMode::Kinematic:
            Component->SetMobility(EComponentMobility::Movable);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetSimulatePhysics(false);
            break;
            
        case ECore_PhysicsMode::Simulated:
            Component->SetMobility(EComponentMobility::Movable);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetSimulatePhysics(true);
            break;
            
        case ECore_PhysicsMode::Ragdoll:
            if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(Component))
            {
                SkelComp->SetSimulatePhysics(true);
                SkelComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            break;
    }
    
    // Apply physics properties
    if (Component->GetBodyInstance())
    {
        Component->SetMassOverrideInKg(NAME_None, Settings.Mass, true);
        Component->SetLinearDamping(Settings.LinearDamping);
        Component->SetAngularDamping(Settings.AngularDamping);
        Component->SetEnableGravity(Settings.bEnableGravity);
    }
    
    // Track this component
    TrackedComponents.AddUnique(Component);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Applied physics settings to %s"), 
           *Component->GetName());
}

void ACore_PhysicsManager::EnableRagdoll(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkelComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkelComp->SetSimulatePhysics(true);
        SkelComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkelComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Enabled ragdoll for %s"), *Actor->GetName());
    }
}

void ACore_PhysicsManager::DisableRagdoll(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkelComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkelComp->SetSimulatePhysics(false);
        SkelComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Disabled ragdoll for %s"), *Actor->GetName());
    }
}

void ACore_PhysicsManager::SetGlobalGravityScale(float NewScale)
{
    GlobalGravityScale = NewScale;
    
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            WorldSettings->GlobalGravityZ = -980.0f * GlobalGravityScale; // Standard gravity
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Set global gravity scale to %f"), NewScale);
}

void ACore_PhysicsManager::AddForceToActor(AActor* Actor, FVector Force, bool bAccelChange)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddForce(Force, NAME_None, bAccelChange);
        }
    }
}

void ACore_PhysicsManager::AddImpulseToActor(AActor* Actor, FVector Impulse, bool bVelChange)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddImpulse(Impulse, NAME_None, bVelChange);
        }
    }
}

bool ACore_PhysicsManager::IsActorSimulatingPhysics(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        return PrimComp->IsSimulatingPhysics();
    }
    
    return false;
}

void ACore_PhysicsManager::OptimizePhysicsPerformance()
{
    // Clean up invalid weak pointers
    TrackedComponents.RemoveAll([](const TWeakObjectPtr<UPrimitiveComponent>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
    
    // Disable physics on distant objects if over limit
    if (CurrentSimulatedBodies > MaxSimulatedBodies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Over body limit (%d/%d), optimizing..."), 
               CurrentSimulatedBodies, MaxSimulatedBodies);
        
        // Find player location for distance calculations
        FVector PlayerLocation = FVector::ZeroVector;
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
        
        // Sort components by distance and disable furthest ones
        TrackedComponents.Sort([PlayerLocation](const TWeakObjectPtr<UPrimitiveComponent>& A, const TWeakObjectPtr<UPrimitiveComponent>& B)
        {
            if (!A.IsValid() || !B.IsValid()) return false;
            
            float DistA = FVector::Dist(A->GetComponentLocation(), PlayerLocation);
            float DistB = FVector::Dist(B->GetComponentLocation(), PlayerLocation);
            return DistA > DistB; // Furthest first
        });
        
        int32 ToDisable = CurrentSimulatedBodies - MaxSimulatedBodies;
        for (int32 i = 0; i < FMath::Min(ToDisable, TrackedComponents.Num()); i++)
        {
            if (TrackedComponents[i].IsValid())
            {
                TrackedComponents[i]->SetSimulatePhysics(false);
            }
        }
    }
}

void ACore_PhysicsManager::UpdateSimulatedBodiesCount()
{
    CurrentSimulatedBodies = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && IsActorSimulatingPhysics(Actor))
            {
                CurrentSimulatedBodies++;
            }
        }
    }
}

void ACore_PhysicsManager::EnforceBodyLimits()
{
    if (CurrentSimulatedBodies > MaxSimulatedBodies)
    {
        OptimizePhysicsPerformance();
    }
}