#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize physics settings
    bPhysicsEnabled = true;
    bRagdollEnabled = true;
    bDestructionEnabled = true;
    bAdvancedCollisionEnabled = true;
    
    PhysicsUpdateRate = 60.0f;
    GravityScale = 1.0f;
    LinearDamping = 0.01f;
    AngularDamping = 0.01f;
    
    MaxPhysicsObjects = 1000;
    CurrentPhysicsObjects = 0;
    
    // Initialize biome-specific physics settings
    InitializeBiomePhysicsSettings();
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager - BeginPlay"));
    
    // Initialize physics subsystems
    InitializePhysicsSubsystems();
    
    // Set up physics world settings
    ConfigurePhysicsWorld();
    
    // Register for physics events
    RegisterPhysicsEvents();
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsEnabled)
        return;
        
    // Update physics performance monitoring
    UpdatePhysicsPerformance(DeltaTime);
    
    // Process physics object pool
    ProcessPhysicsObjectPool();
    
    // Update biome-specific physics
    UpdateBiomePhysics(DeltaTime);
}

void UCore_PhysicsSystemManager::InitializePhysicsSubsystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Physics Subsystems..."));
    
    // Initialize ragdoll manager
    if (bRagdollEnabled)
    {
        // Ragdoll system initialization
        RagdollSettings.MaxRagdolls = 20;
        RagdollSettings.RagdollLifetime = 30.0f;
        RagdollSettings.BlendInTime = 0.2f;
        RagdollSettings.BlendOutTime = 0.5f;
        
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll system initialized - Max: %d"), RagdollSettings.MaxRagdolls);
    }
    
    // Initialize destruction manager
    if (bDestructionEnabled)
    {
        // Destruction system initialization
        DestructionSettings.MaxDestructibleObjects = 50;
        DestructionSettings.DestructionForceThreshold = 1000.0f;
        DestructionSettings.ChunkLifetime = 60.0f;
        DestructionSettings.bEnableParticleEffects = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Destruction system initialized - Max objects: %d"), DestructionSettings.MaxDestructibleObjects);
    }
    
    // Initialize collision manager
    if (bAdvancedCollisionEnabled)
    {
        // Advanced collision system initialization
        CollisionSettings.MaxCollisionQueries = 500;
        CollisionSettings.CollisionComplexity = ECollisionTraceFlag::CTF_UseDefault;
        CollisionSettings.bEnableComplexCollision = true;
        CollisionSettings.bOptimizeCollisionMeshes = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Advanced collision system initialized"));
    }
}

void UCore_PhysicsSystemManager::ConfigurePhysicsWorld()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Configure world physics settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Set gravity based on biome
        FVector CurrentGravity = FVector(0.0f, 0.0f, -980.0f * GravityScale);
        World->GetPhysicsScene()->SetGravity(CurrentGravity);
        
        UE_LOG(LogTemp, Warning, TEXT("Physics world gravity set to: %s"), *CurrentGravity.ToString());
    }
}

void UCore_PhysicsSystemManager::RegisterPhysicsEvents()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Register for collision events
    if (World->GetPhysicsScene())
    {
        // Set up collision event handling
        UE_LOG(LogTemp, Warning, TEXT("Physics events registered"));
    }
}

void UCore_PhysicsSystemManager::InitializeBiomePhysicsSettings()
{
    // Swamp biome physics - muddy, slow movement
    FCore_BiomePhysicsSettings SwampSettings;
    SwampSettings.BiomeType = ECore_BiomeType::Swamp;
    SwampSettings.GravityModifier = 1.2f;  // Slightly heavier feeling
    SwampSettings.LinearDampingModifier = 3.0f;  // High resistance
    SwampSettings.AngularDampingModifier = 2.0f;
    SwampSettings.FrictionModifier = 0.3f;  // Slippery mud
    SwampSettings.RestitutionModifier = 0.1f;  // Low bounce
    BiomePhysicsSettings.Add(ECore_BiomeType::Swamp, SwampSettings);
    
    // Forest biome physics - normal with vegetation resistance
    FCore_BiomePhysicsSettings ForestSettings;
    ForestSettings.BiomeType = ECore_BiomeType::Forest;
    ForestSettings.GravityModifier = 1.0f;
    ForestSettings.LinearDampingModifier = 1.5f;  // Vegetation resistance
    ForestSettings.AngularDampingModifier = 1.2f;
    ForestSettings.FrictionModifier = 0.8f;  // Good grip on forest floor
    ForestSettings.RestitutionModifier = 0.3f;
    BiomePhysicsSettings.Add(ECore_BiomeType::Forest, ForestSettings);
    
    // Savanna biome physics - standard open plains
    FCore_BiomePhysicsSettings SavannaSettings;
    SavannaSettings.BiomeType = ECore_BiomeType::Savanna;
    SavannaSettings.GravityModifier = 1.0f;
    SavannaSettings.LinearDampingModifier = 1.0f;
    SavannaSettings.AngularDampingModifier = 1.0f;
    SavannaSettings.FrictionModifier = 0.7f;
    SavannaSettings.RestitutionModifier = 0.4f;
    BiomePhysicsSettings.Add(ECore_BiomeType::Savanna, SavannaSettings);
    
    // Desert biome physics - loose sand, low friction
    FCore_BiomePhysicsSettings DesertSettings;
    DesertSettings.BiomeType = ECore_BiomeType::Desert;
    DesertSettings.GravityModifier = 0.9f;  // Slightly lighter feeling
    DesertSettings.LinearDampingModifier = 0.5f;  // Low air resistance
    DesertSettings.AngularDampingModifier = 0.8f;
    DesertSettings.FrictionModifier = 0.2f;  // Very slippery sand
    DesertSettings.RestitutionModifier = 0.6f;  // Sand absorbs some impact
    BiomePhysicsSettings.Add(ECore_BiomeType::Desert, DesertSettings);
    
    // Mountain biome physics - rocky, high friction
    FCore_BiomePhysicsSettings MountainSettings;
    MountainSettings.BiomeType = ECore_BiomeType::Mountain;
    MountainSettings.GravityModifier = 1.1f;  // Slightly heavier at altitude
    MountainSettings.LinearDampingModifier = 0.8f;  // Thin air
    MountainSettings.AngularDampingModifier = 0.9f;
    MountainSettings.FrictionModifier = 1.2f;  // Rocky surfaces
    MountainSettings.RestitutionModifier = 0.8f;  // Hard surfaces bounce
    BiomePhysicsSettings.Add(ECore_BiomeType::Mountain, MountainSettings);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome physics settings initialized for %d biomes"), BiomePhysicsSettings.Num());
}

void UCore_PhysicsSystemManager::UpdatePhysicsPerformance(float DeltaTime)
{
    // Count active physics objects
    CurrentPhysicsObjects = 0;
    
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentPhysicsObjects++;
                    }
                }
            }
        }
    }
    
    // Performance optimization
    if (CurrentPhysicsObjects > MaxPhysicsObjects * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics object count high: %d/%d - Consider optimization"), 
               CurrentPhysicsObjects, MaxPhysicsObjects);
    }
}

void UCore_PhysicsSystemManager::ProcessPhysicsObjectPool()
{
    // Manage physics object lifecycle
    // Remove old physics objects that are no longer needed
    // This helps maintain performance
}

void UCore_PhysicsSystemManager::UpdateBiomePhysics(float DeltaTime)
{
    // Update physics based on current biome
    // This would typically check the player's location and apply biome-specific physics
    
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // For now, apply default savanna physics
    // In a full implementation, this would detect the current biome
    ECore_BiomeType CurrentBiome = ECore_BiomeType::Savanna;
    
    if (BiomePhysicsSettings.Contains(CurrentBiome))
    {
        const FCore_BiomePhysicsSettings& Settings = BiomePhysicsSettings[CurrentBiome];
        
        // Apply biome-specific gravity
        FVector BiomeGravity = FVector(0.0f, 0.0f, -980.0f * Settings.GravityModifier);
        World->GetPhysicsScene()->SetGravity(BiomeGravity);
    }
}

bool UCore_PhysicsSystemManager::EnableRagdollForActor(AActor* Actor)
{
    if (!Actor || !bRagdollEnabled)
        return false;
        
    // Implementation for enabling ragdoll physics on an actor
    UE_LOG(LogTemp, Warning, TEXT("Enabling ragdoll for actor: %s"), *Actor->GetName());
    
    // Find skeletal mesh component
    if (USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        // Enable ragdoll physics
        SkelMeshComp->SetSimulatePhysics(true);
        SkelMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll enabled for: %s"), *Actor->GetName());
        return true;
    }
    
    return false;
}

bool UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, FVector ImpactPoint, float Force)
{
    if (!Actor || !bDestructionEnabled)
        return false;
        
    if (Force < DestructionSettings.DestructionForceThreshold)
        return false;
        
    UE_LOG(LogTemp, Warning, TEXT("Triggering destruction for: %s at force: %f"), *Actor->GetName(), Force);
    
    // Implementation for destruction
    // This would typically create destruction chunks and apply forces
    
    return true;
}

void UCore_PhysicsSystemManager::SetBiomePhysics(ECore_BiomeType BiomeType)
{
    if (!BiomePhysicsSettings.Contains(BiomeType))
        return;
        
    const FCore_BiomePhysicsSettings& Settings = BiomePhysicsSettings[BiomeType];
    
    // Apply biome physics settings
    GravityScale = Settings.GravityModifier;
    LinearDamping = Settings.LinearDampingModifier;
    AngularDamping = Settings.AngularDampingModifier;
    
    UE_LOG(LogTemp, Warning, TEXT("Applied physics settings for biome: %d"), (int32)BiomeType);
    
    // Update world gravity immediately
    ConfigurePhysicsWorld();
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsObjectCount() const
{
    return CurrentPhysicsObjects;
}

bool UCore_PhysicsSystemManager::IsPhysicsSystemHealthy() const
{
    return bPhysicsEnabled && 
           CurrentPhysicsObjects < MaxPhysicsObjects &&
           GEngine != nullptr;
}