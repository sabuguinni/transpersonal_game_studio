#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

DEFINE_LOG_CATEGORY(LogCorePhysics);

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    bPhysicsSystemActive = false;
    ActivePhysicsObjects = 0;
    PhysicsPerformanceMetric = 0.0f;
    
    // Performance Settings
    PhysicsLODDistance1 = 1000.0f;
    PhysicsLODDistance2 = 2500.0f;
    PhysicsLODDistance3 = 5000.0f;
    MaxPhysicsObjects = 500;
    
    // Ragdoll Settings
    RagdollBlendTime = 0.5f;
    RagdollLinearDamping = 0.1f;
    RagdollAngularDamping = 0.1f;
    
    // Destruction Settings
    DestructionImpactThreshold = 1000.0f;
    MaxDebrisCount = 20;
    DebrisLifetime = 30.0f;
    
    // Performance monitoring
    LastPerformanceUpdate = 0.0f;
    PerformanceUpdateInterval = 1.0f;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics System Manager initializing..."));
    
    InitializePhysicsSystems();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics System Manager initialized successfully"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics System Manager shutting down..."));
    
    ShutdownPhysicsSystems();
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystems()
{
    if (bPhysicsSystemActive)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("Physics systems already active"));
        return;
    }
    
    // Setup collision channels and profiles
    SetupCollisionChannels();
    ConfigureCollisionProfiles();
    
    // Setup physical materials
    SetupPhysicalMaterials();
    
    // Initialize tracking arrays
    TrackedPhysicsActors.Empty();
    RagdollCharacters.Empty();
    
    bPhysicsSystemActive = true;
    ActivePhysicsObjects = 0;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics systems initialized"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystems()
{
    if (!bPhysicsSystemActive)
    {
        return;
    }
    
    // Disable all active ragdolls
    for (auto& CharacterPtr : RagdollCharacters)
    {
        if (CharacterPtr.IsValid())
        {
            DisableRagdollForCharacter(CharacterPtr.Get());
        }
    }
    
    // Clear tracking arrays
    TrackedPhysicsActors.Empty();
    RagdollCharacters.Empty();
    
    bPhysicsSystemActive = false;
    ActivePhysicsObjects = 0;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics systems shutdown"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSystems(float DeltaTime)
{
    if (!bPhysicsSystemActive)
    {
        return;
    }
    
    // Update performance metrics
    LastPerformanceUpdate += DeltaTime;
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        CleanupInvalidActors();
        LastPerformanceUpdate = 0.0f;
    }
    
    // Update physics LODs based on player position
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            UpdatePhysicsLODs(PlayerPawn->GetActorLocation());
        }
    }
}

void UCore_PhysicsSystemManager::SetupCollisionChannels()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Setting up collision channels"));
    
    // Note: Collision channels are typically set up in Project Settings
    // This function can be used to validate or modify collision settings at runtime
}

void UCore_PhysicsSystemManager::ConfigureCollisionProfiles()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Configuring collision profiles"));
    
    // Note: Collision profiles are typically set up in Project Settings
    // This function can be used to validate or modify collision profiles at runtime
}

bool UCore_PhysicsSystemManager::CheckCollisionBetween(AActor* ActorA, AActor* ActorB, FHitResult& HitResult)
{
    if (!ActorA || !ActorB)
    {
        return false;
    }
    
    FVector StartLocation = ActorA->GetActorLocation();
    FVector EndLocation = ActorB->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ActorA);
    QueryParams.bTraceComplex = false;
    
    if (UWorld* World = ActorA->GetWorld())
    {
        return World->LineTraceSingleByChannel(
            HitResult,
            StartLocation,
            EndLocation,
            ECollisionChannel::ECC_WorldStatic,
            QueryParams
        );
    }
    
    return false;
}

void UCore_PhysicsSystemManager::EnableRagdollForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("Cannot enable ragdoll: Character is null"));
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("Cannot enable ragdoll: Character has no mesh component"));
        return;
    }
    
    // Enable physics simulation
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Set physics properties
    MeshComp->SetLinearDamping(RagdollLinearDamping);
    MeshComp->SetAngularDamping(RagdollAngularDamping);
    
    // Add to tracking
    RagdollCharacters.AddUnique(Character);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Ragdoll enabled for character: %s"), *Character->GetName());
}

void UCore_PhysicsSystemManager::DisableRagdollForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Disable physics simulation
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Remove from tracking
    RagdollCharacters.RemoveAll([Character](const TWeakObjectPtr<ACharacter>& Ptr)
    {
        return Ptr.Get() == Character;
    });
    
    UE_LOG(LogCorePhysics, Log, TEXT("Ragdoll disabled for character: %s"), *Character->GetName());
}

void UCore_PhysicsSystemManager::BlendToRagdoll(ACharacter* Character, float BlendTime)
{
    if (!Character)
    {
        return;
    }
    
    // For now, immediately enable ragdoll
    // In a full implementation, this would blend from animation to physics over time
    EnableRagdollForCharacter(Character);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Blending to ragdoll for character: %s (BlendTime: %f)"), 
           *Character->GetName(), BlendTime);
}

void UCore_PhysicsSystemManager::DestroyObject(AActor* Actor, FVector ImpactLocation, float ImpactForce)
{
    if (!Actor)
    {
        return;
    }
    
    if (ImpactForce < DestructionImpactThreshold)
    {
        UE_LOG(LogCorePhysics, Log, TEXT("Impact force too low for destruction: %f < %f"), 
               ImpactForce, DestructionImpactThreshold);
        return;
    }
    
    // Create debris before destroying the original
    CreateDebris(Actor, ImpactLocation, MaxDebrisCount);
    
    // Destroy the original actor
    Actor->Destroy();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Object destroyed: %s (Impact: %f)"), 
           *Actor->GetName(), ImpactForce);
}

void UCore_PhysicsSystemManager::CreateDebris(AActor* OriginalActor, FVector ImpactLocation, int32 DebrisCount)
{
    if (!OriginalActor || DebrisCount <= 0)
    {
        return;
    }
    
    UWorld* World = OriginalActor->GetWorld();
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
    
    // Create debris pieces
    for (int32 i = 0; i < DebrisCount; ++i)
    {
        FVector DebrisLocation = ImpactLocation + FMath::VRand() * 100.0f;
        FRotator DebrisRotation = FRotator(FMath::RandRange(-180, 180), 
                                          FMath::RandRange(-180, 180), 
                                          FMath::RandRange(-180, 180));
        
        AStaticMeshActor* DebrisActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), 
            DebrisLocation, 
            DebrisRotation
        );
        
        if (DebrisActor)
        {
            // Set up the debris mesh
            UStaticMeshComponent* DebrisMesh = DebrisActor->GetStaticMeshComponent();
            if (DebrisMesh)
            {
                DebrisMesh->SetStaticMesh(OriginalMesh->GetStaticMesh());
                DebrisMesh->SetMaterial(0, OriginalMesh->GetMaterial(0));
                
                // Enable physics
                DebrisMesh->SetSimulatePhysics(true);
                DebrisMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                
                // Apply random impulse
                FVector RandomImpulse = FMath::VRand() * FMath::RandRange(500.0f, 1500.0f);
                DebrisMesh->AddImpulse(RandomImpulse);
                
                // Scale down the debris
                float DebrisScale = FMath::RandRange(0.1f, 0.5f);
                DebrisActor->SetActorScale3D(FVector(DebrisScale));
            }
            
            // Set up destruction timer
            DebrisActor->SetLifeSpan(DebrisLifetime);
            
            // Track the debris
            TrackedPhysicsActors.Add(DebrisActor);
        }
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Created %d debris pieces from %s"), 
           DebrisCount, *OriginalActor->GetName());
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bPhysicsSystemActive)
    {
        return;
    }
    
    // Remove invalid actors
    CleanupInvalidActors();
    
    // Check if we're over the physics object limit
    if (ActivePhysicsObjects > MaxPhysicsObjects)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("Physics object count exceeds limit: %d > %d"), 
               ActivePhysicsObjects, MaxPhysicsObjects);
        
        // Could implement object culling here
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics performance optimized. Active objects: %d"), 
           ActivePhysicsObjects);
}

void UCore_PhysicsSystemManager::SetPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case 0: // High detail
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case 1: // Medium detail
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case 2: // Low detail
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case 3: // No physics
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
            
        default:
            break;
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsLODs(const FVector& PlayerLocation)
{
    for (auto& ActorPtr : TrackedPhysicsActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            ProcessPhysicsLOD(Actor, Distance);
        }
    }
}

void UCore_PhysicsSystemManager::SetupPhysicalMaterials()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Setting up physical materials"));
    
    // Physical materials are typically set up as assets
    // This function can be used to validate or modify material properties at runtime
}

void UCore_PhysicsSystemManager::ApplyPhysicalMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* Material)
{
    if (!Component || !Material)
    {
        return;
    }
    
    Component->SetPhysMaterialOverride(Material);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Applied physical material to component: %s"), 
           *Component->GetName());
}

void UCore_PhysicsSystemManager::UpdatePerformanceMetrics()
{
    ActivePhysicsObjects = TrackedPhysicsActors.Num() + RagdollCharacters.Num();
    
    // Calculate performance metric (simplified)
    PhysicsPerformanceMetric = (float)ActivePhysicsObjects / (float)MaxPhysicsObjects;
    
    UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Performance update - Active objects: %d, Metric: %f"), 
           ActivePhysicsObjects, PhysicsPerformanceMetric);
}

void UCore_PhysicsSystemManager::CleanupInvalidActors()
{
    // Remove invalid physics actors
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& Ptr)
    {
        return !Ptr.IsValid();
    });
    
    // Remove invalid ragdoll characters
    RagdollCharacters.RemoveAll([](const TWeakObjectPtr<ACharacter>& Ptr)
    {
        return !Ptr.IsValid();
    });
}

void UCore_PhysicsSystemManager::ProcessPhysicsLOD(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor)
    {
        return;
    }
    
    int32 LODLevel = 0;
    
    if (DistanceToPlayer > PhysicsLODDistance3)
    {
        LODLevel = 3; // No physics
    }
    else if (DistanceToPlayer > PhysicsLODDistance2)
    {
        LODLevel = 2; // Low detail
    }
    else if (DistanceToPlayer > PhysicsLODDistance1)
    {
        LODLevel = 1; // Medium detail
    }
    else
    {
        LODLevel = 0; // High detail
    }
    
    SetPhysicsLOD(Actor, LODLevel);
}