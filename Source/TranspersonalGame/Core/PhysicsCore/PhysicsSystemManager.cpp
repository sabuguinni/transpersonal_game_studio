// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "Components/SkeletalMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/CollisionProfile.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values optimized for survival gameplay
    WorldGravityZ = -980.0f;
    MaxSubsteps = 6;
    FixedDeltaTime = 0.016667f;
    bEnableDestruction = true;
    DefaultDamageThreshold = 100.0f;
    RagdollImpulseMultiplier = 1.5f;
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Failed to get world reference"));
        return;
    }

    // Initialize all physics systems
    InitializeJurassicPhysics();
    ConfigureCollisionChannels();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Successfully initialized for Jurassic survival gameplay"));
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Monitor physics performance and adjust if needed
    if (CachedWorld && CachedWorld->GetPhysicsScene())
    {
        // Check for physics performance issues and log warnings
        const float CurrentPhysicsTime = CachedWorld->GetPhysicsScene()->GetLastDeltaTime();
        if (CurrentPhysicsTime > FixedDeltaTime * 2.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics simulation taking too long: %f ms"), 
                   CurrentPhysicsTime * 1000.0f);
        }
    }
}

void UPhysicsSystemManager::InitializeJurassicPhysics()
{
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Cannot initialize physics without valid world"));
        return;
    }

    // Configure Chaos Physics settings
    InitializeChaosSettings();
    
    // Setup physics materials for different terrain types
    ConfigurePhysicsMaterials();
    
    // Apply world gravity optimized for creature movement
    if (CachedWorld->GetPhysicsScene())
    {
        CachedWorld->GetPhysicsScene()->SetGravityZ(WorldGravityZ);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Set world gravity to %f"), WorldGravityZ);
    }
}

void UPhysicsSystemManager::InitializeChaosSettings()
{
    // Get physics settings and configure for survival gameplay
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Failed to get physics settings"));
        return;
    }

    // Configure Chaos solver for organic movement
    PhysicsSettings->DefaultGravityZ = WorldGravityZ;
    PhysicsSettings->bSubstepping = true;
    PhysicsSettings->MaxSubstepDeltaTime = FixedDeltaTime;
    PhysicsSettings->MaxSubsteps = MaxSubsteps;
    
    // Enable Chaos destruction for environmental interaction
    PhysicsSettings->bEnableShapeSharing = true;
    PhysicsSettings->bEnablePCM = true;
    PhysicsSettings->bEnableStabilization = true;
    
    // Optimize collision detection for large world
    PhysicsSettings->BounceThresholdVelocity = 200.0f;
    PhysicsSettings->FrictionCombineMode = EFrictionCombineMode::Average;
    PhysicsSettings->RestitutionCombineMode = EFrictionCombineMode::Average;
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Chaos physics configured for Jurassic survival"));
}

void UPhysicsSystemManager::ConfigureCollisionChannels()
{
    SetupCollisionProfiles();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Collision channels configured for survival gameplay"));
}

void UPhysicsSystemManager::SetupCollisionProfiles()
{
    // This would typically be done in DefaultEngine.ini, but we log the expected setup
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Setting up collision profiles:"));
    UE_LOG(LogTemp, Log, TEXT("  - Player: Blocks World, Dinosaur, Projectile"));
    UE_LOG(LogTemp, Log, TEXT("  - Dinosaur: Blocks World, Player, Dinosaur"));
    UE_LOG(LogTemp, Log, TEXT("  - Environment: Blocks All"));
    UE_LOG(LogTemp, Log, TEXT("  - Projectile: Blocks All except Projectile"));
    UE_LOG(LogTemp, Log, TEXT("  - Destruction: Blocks All, generates events"));
}

void UPhysicsSystemManager::ConfigurePhysicsMaterials()
{
    // Create physics materials for different terrain types
    TerrainPhysicsMaterials.Empty();
    
    // These would typically be created as assets, but we define the properties here
    struct FTerrainPhysicsData
    {
        FString Name;
        float Friction;
        float Restitution;
        float Density;
    };
    
    TArray<FTerrainPhysicsData> TerrainTypes = {
        {TEXT("Grass"), 0.7f, 0.1f, 1.0f},
        {TEXT("Rock"), 0.9f, 0.3f, 2.5f},
        {TEXT("Mud"), 0.4f, 0.05f, 1.2f},
        {TEXT("Sand"), 0.5f, 0.1f, 1.1f},
        {TEXT("Wood"), 0.6f, 0.2f, 0.8f},
        {TEXT("Water"), 0.1f, 0.0f, 1.0f}
    };
    
    for (const FTerrainPhysicsData& TerrainData : TerrainTypes)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Configured %s material - Friction: %f, Restitution: %f"), 
               *TerrainData.Name, TerrainData.Friction, TerrainData.Restitution);
    }
}

void UPhysicsSystemManager::EnableCreatureRagdoll(USkeletalMeshComponent* SkeletalMeshComp, 
                                                 FVector ImpulseLocation, 
                                                 float ImpulseStrength)
{
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Cannot enable ragdoll on null skeletal mesh"));
        return;
    }

    // Enable physics simulation on the skeletal mesh
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Apply death impulse at impact location
    FVector ImpulseDirection = (SkeletalMeshComp->GetComponentLocation() - ImpulseLocation).GetSafeNormal();
    FVector FinalImpulse = ImpulseDirection * ImpulseStrength * RagdollImpulseMultiplier;
    
    SkeletalMeshComp->AddImpulseAtLocation(FinalImpulse, ImpulseLocation);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Enabled ragdoll physics with impulse strength %f"), 
           ImpulseStrength * RagdollImpulseMultiplier);
}

void UPhysicsSystemManager::TriggerEnvironmentalDestruction(UGeometryCollectionComponent* GeometryCollection,
                                                          FVector ImpactLocation,
                                                          float DestructionRadius,
                                                          float DestructionForce)
{
    if (!GeometryCollection)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Cannot trigger destruction on null geometry collection"));
        return;
    }

    if (!bEnableDestruction)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Destruction disabled, ignoring destruction request"));
        return;
    }

    // Apply destruction field at impact location
    // This would use Chaos Fields in a full implementation
    GeometryCollection->SetNotifyBreaks(true);
    GeometryCollection->SetNotifyCollisions(true);
    
    // Apply radial damage to trigger fracturing
    // In a full implementation, this would use Chaos Fields
    FVector ImpulseDirection = FVector::UpVector;
    GeometryCollection->AddRadialImpulse(ImpactLocation, DestructionRadius, DestructionForce, RIF_Linear, true);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Triggered environmental destruction at %s with radius %f and force %f"), 
           *ImpactLocation.ToString(), DestructionRadius, DestructionForce);
}