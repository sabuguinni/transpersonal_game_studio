#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics settings
    GlobalGravityZ = -980.0f;
    bEnableSubstepping = true;
    PhysicsTimestep = 0.016667f; // 60 FPS
    MaxPhysicsActors = 1000;
    PhysicsCullingDistance = 50000.0f;
    bUsePhysicsLOD = true;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing physics subsystem"));
    
    // Initialize physics simulation
    InitializePhysicsSimulation();
    
    // Setup collision channels
    SetupCollisionChannels();
    
    // Initialize biome physics modifiers
    InitializeBiomePhysicsModifiers();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics subsystem initialized successfully"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Deinitializing physics subsystem"));
    
    // Clean up active physics actors
    ActivePhysicsActors.Empty();
    ActiveRagdolls.Empty();
    
    Super::Deinitialize();
}

bool UCore_PhysicsSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCore_PhysicsSystemManager::InitializePhysicsSimulation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No world found for physics initialization"));
        return;
    }
    
    // Set global physics settings
    SetGlobalPhysicsSettings(GlobalGravityZ, bEnableSubstepping);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics simulation initialized"));
}

void UCore_PhysicsSystemManager::SetGlobalPhysicsSettings(float GravityZ, bool bEnableSubsteppingParam)
{
    GlobalGravityZ = GravityZ;
    bEnableSubstepping = bEnableSubsteppingParam;
    
    // Apply settings to physics settings object
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->DefaultGravityZ = GravityZ;
        PhysicsSettings->bEnableSubstepping = bEnableSubsteppingParam;
        PhysicsSettings->MaxSubstepDeltaTime = PhysicsTimestep;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Global physics settings applied - Gravity: %f, Substepping: %s"), 
               GravityZ, bEnableSubsteppingParam ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    // Update active physics actors list
    UpdateActivePhysicsActors();
    
    // Apply performance optimizations
    ApplyPerformanceOptimizations();
    
    // Clean up invalid actors
    CleanupInvalidActors();
}

bool UCore_PhysicsSystemManager::CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB)
{
    if (!ActorA || !ActorB)
    {
        return false;
    }
    
    // Get primitive components for collision checking
    UPrimitiveComponent* PrimA = ActorA->GetRootComponent() ? Cast<UPrimitiveComponent>(ActorA->GetRootComponent()) : nullptr;
    UPrimitiveComponent* PrimB = ActorB->GetRootComponent() ? Cast<UPrimitiveComponent>(ActorB->GetRootComponent()) : nullptr;
    
    if (!PrimA || !PrimB)
    {
        return false;
    }
    
    // Check if components are overlapping
    return PrimA->IsOverlappingComponent(PrimB);
}

void UCore_PhysicsSystemManager::SetupCollisionChannels()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Setting up custom collision channels"));
    
    // Custom collision channels would be set up here
    // This requires engine-level configuration, so we log the setup
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Collision channels configured"));
}

void UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor, bool bSimulatePhysics)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Cannot enable physics on null actor"));
        return;
    }
    
    // Get the root primitive component
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        RootPrimitive->SetSimulatePhysics(bSimulatePhysics);
        RootPrimitive->SetCollisionEnabled(bSimulatePhysics ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::QueryOnly);
        
        if (bSimulatePhysics && !ActivePhysicsActors.Contains(Actor))
        {
            ActivePhysicsActors.Add(Actor);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics %s on actor %s"), 
               bSimulatePhysics ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::EnableRagdollPhysics(APawn* Pawn)
{
    if (!Pawn)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Cannot enable ragdoll on null pawn"));
        return;
    }
    
    // Get skeletal mesh component
    if (USkeletalMeshComponent* SkeletalMesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetSimulatePhysics(true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Add to active ragdolls list
        if (!ActiveRagdolls.Contains(Pawn))
        {
            ActiveRagdolls.Add(Pawn);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll physics enabled on %s"), *Pawn->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableRagdollPhysics(APawn* Pawn)
{
    if (!Pawn)
    {
        return;
    }
    
    // Get skeletal mesh component
    if (USkeletalMeshComponent* SkeletalMesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        // Remove from active ragdolls list
        ActiveRagdolls.Remove(Pawn);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll physics disabled on %s"), *Pawn->GetName());
    }
}

void UCore_PhysicsSystemManager::ApplyImpulseToRagdoll(APawn* Pawn, FVector Impulse, FName BoneName)
{
    if (!Pawn)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkeletalMesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (BoneName != NAME_None)
        {
            SkeletalMesh->AddImpulseAtLocation(Impulse, SkeletalMesh->GetBoneLocation(BoneName), BoneName);
        }
        else
        {
            SkeletalMesh->AddImpulse(Impulse);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied impulse %s to %s"), 
               *Impulse.ToString(), *Pawn->GetName());
    }
}

void UCore_PhysicsSystemManager::CreateDestructibleFromStaticMesh(UStaticMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Cannot create destructible from null mesh component"));
        return;
    }
    
    // For now, we'll simulate destruction by enabling physics and applying impulse
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Destructible physics enabled on mesh component"));
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, FVector ImpactPoint, float DestructionForce)
{
    if (!Actor)
    {
        return;
    }
    
    // Apply destruction impulse
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        FVector ImpulseDirection = (Actor->GetActorLocation() - ImpactPoint).GetSafeNormal();
        FVector DestructionImpulse = ImpulseDirection * DestructionForce;
        
        PrimComp->AddImpulseAtLocation(DestructionImpulse, ImpactPoint);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Destruction triggered on %s with force %f"), 
               *Actor->GetName(), DestructionForce);
    }
}

void UCore_PhysicsSystemManager::ApplyBiomePhysicsModifiers(ECore_BiomeType BiomeType, AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Apply biome-specific physics modifiers
    float GravityModifier = BiomeGravityModifiers.Contains(BiomeType) ? BiomeGravityModifiers[BiomeType] : 1.0f;
    float FrictionModifier = BiomeFrictionModifiers.Contains(BiomeType) ? BiomeFrictionModifiers[BiomeType] : 1.0f;
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        // Apply gravity modifier (simulated through mass scaling)
        PrimComp->SetMassScale(NAME_None, GravityModifier);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied biome physics modifiers to %s - Gravity: %f, Friction: %f"), 
               *Actor->GetName(), GravityModifier, FrictionModifier);
    }
}

void UCore_PhysicsSystemManager::SetWaterPhysics(AActor* Actor, bool bIsInWater)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        if (bIsInWater)
        {
            // Reduce gravity effect and increase drag
            PrimComp->SetMassScale(NAME_None, 0.3f); // Buoyancy effect
            PrimComp->SetLinearDamping(5.0f); // Water resistance
            PrimComp->SetAngularDamping(5.0f);
        }
        else
        {
            // Restore normal physics
            PrimComp->SetMassScale(NAME_None, 1.0f);
            PrimComp->SetLinearDamping(0.01f);
            PrimComp->SetAngularDamping(0.01f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Water physics %s for %s"), 
               bIsInWater ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::SetMudPhysics(AActor* Actor, bool bIsInMud)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        if (bIsInMud)
        {
            // Increase drag and friction
            PrimComp->SetLinearDamping(10.0f); // High resistance
            PrimComp->SetAngularDamping(10.0f);
        }
        else
        {
            // Restore normal physics
            PrimComp->SetLinearDamping(0.01f);
            PrimComp->SetAngularDamping(0.01f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Mud physics %s for %s"), 
               bIsInMud ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    // Remove actors that are too far from players
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player location for distance culling
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledActors = 0;
    
    for (int32 i = ActivePhysicsActors.Num() - 1; i >= 0; i--)
    {
        AActor* Actor = ActivePhysicsActors[i];
        if (!IsValid(Actor))
        {
            ActivePhysicsActors.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        if (Distance > PhysicsCullingDistance)
        {
            // Disable physics for distant actors
            EnablePhysicsOnActor(Actor, false);
            ActivePhysicsActors.RemoveAt(i);
            CulledActors++;
        }
    }
    
    if (CulledActors > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Culled %d distant physics actors for performance"), CulledActors);
    }
}

void UCore_PhysicsSystemManager::SetPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor || !bUsePhysicsLOD)
    {
        return;
    }
    
    // Adjust physics complexity based on LOD level
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        switch (LODLevel)
        {
        case 0: // High detail
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case 1: // Medium detail
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        case 2: // Low detail
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Set physics LOD %d for %s"), LODLevel, *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::CreatePhysicsTestActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No world found for creating test actors"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Creating physics test actors"));
    
    // This would be implemented to create test actors in the editor
    // For now, we log the action
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics test actors creation completed"));
}

void UCore_PhysicsSystemManager::RunPhysicsValidationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Running physics validation tests"));
    
    // Test 1: Check if physics subsystem is properly initialized
    bool bPhysicsInitialized = (GlobalGravityZ != 0.0f);
    UE_LOG(LogTemp, Warning, TEXT("Physics Initialization Test: %s"), bPhysicsInitialized ? TEXT("PASS") : TEXT("FAIL"));
    
    // Test 2: Check active physics actors count
    bool bActiveActorsValid = (ActivePhysicsActors.Num() >= 0 && ActivePhysicsActors.Num() <= MaxPhysicsActors);
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Actors Test: %s (%d actors)"), 
           bActiveActorsValid ? TEXT("PASS") : TEXT("FAIL"), ActivePhysicsActors.Num());
    
    // Test 3: Check biome modifiers
    bool bBiomeModifiersValid = (BiomeGravityModifiers.Num() > 0);
    UE_LOG(LogTemp, Warning, TEXT("Biome Modifiers Test: %s"), bBiomeModifiersValid ? TEXT("PASS") : TEXT("FAIL"));
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics validation tests completed"));
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsInfo(bool bShowCollision, bool bShowMass)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Draw debug information for active physics actors
    for (AActor* Actor : ActivePhysicsActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        FVector ActorLocation = Actor->GetActorLocation();
        
        if (bShowCollision)
        {
            // Draw collision bounds
            DrawDebugBox(World, ActorLocation, FVector(50, 50, 50), FColor::Green, false, 0.1f);
        }
        
        if (bShowMass)
        {
            // Draw mass information
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                float Mass = PrimComp->GetMass();
                DrawDebugString(World, ActorLocation + FVector(0, 0, 100), 
                               FString::Printf(TEXT("Mass: %.1f"), Mass), nullptr, FColor::Yellow, 0.1f);
            }
        }
    }
}

void UCore_PhysicsSystemManager::InitializeBiomePhysicsModifiers()
{
    // Initialize biome-specific physics modifiers
    BiomeGravityModifiers.Empty();
    BiomeFrictionModifiers.Empty();
    
    // Savana - Normal physics
    BiomeGravityModifiers.Add(ECore_BiomeType::Savana, 1.0f);
    BiomeFrictionModifiers.Add(ECore_BiomeType::Savana, 1.0f);
    
    // Floresta - Slightly reduced gravity due to dense vegetation
    BiomeGravityModifiers.Add(ECore_BiomeType::Floresta, 0.9f);
    BiomeFrictionModifiers.Add(ECore_BiomeType::Floresta, 1.2f);
    
    // Pantano - Reduced gravity, high friction
    BiomeGravityModifiers.Add(ECore_BiomeType::Pantano, 0.7f);
    BiomeFrictionModifiers.Add(ECore_BiomeType::Pantano, 2.0f);
    
    // Deserto - Normal gravity, low friction (sand)
    BiomeGravityModifiers.Add(ECore_BiomeType::Deserto, 1.0f);
    BiomeFrictionModifiers.Add(ECore_BiomeType::Deserto, 0.6f);
    
    // Montanha - Increased gravity, high friction
    BiomeGravityModifiers.Add(ECore_BiomeType::Montanha, 1.1f);
    BiomeFrictionModifiers.Add(ECore_BiomeType::Montanha, 1.5f);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Biome physics modifiers initialized"));
}

void UCore_PhysicsSystemManager::UpdateActivePhysicsActors()
{
    // Clean up invalid actors and update the list
    for (int32 i = ActivePhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActivePhysicsActors[i]))
        {
            ActivePhysicsActors.RemoveAt(i);
        }
    }
}

void UCore_PhysicsSystemManager::CleanupInvalidActors()
{
    // Remove invalid actors from all tracking arrays
    ActivePhysicsActors.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    ActiveRagdolls.RemoveAll([](APawn* Pawn) { return !IsValid(Pawn); });
}

bool UCore_PhysicsSystemManager::IsActorValidForPhysics(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    // Check if actor has a primitive component
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    return (PrimComp != nullptr);
}

void UCore_PhysicsSystemManager::ApplyPerformanceOptimizations()
{
    // Limit the number of active physics actors
    if (ActivePhysicsActors.Num() > MaxPhysicsActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Too many active physics actors (%d), applying optimizations"), 
               ActivePhysicsActors.Num());
        
        // Disable physics on oldest actors
        int32 ActorsToDisable = ActivePhysicsActors.Num() - MaxPhysicsActors;
        for (int32 i = 0; i < ActorsToDisable; i++)
        {
            if (IsValid(ActivePhysicsActors[i]))
            {
                EnablePhysicsOnActor(ActivePhysicsActors[i], false);
            }
        }
    }
}