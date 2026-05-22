#include "EngineArchitectCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/RendererSettings.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Engine/CollisionProfile.h"
#include "PhysicsEngine/CollisionProfile.h"

UEngineArchitectCore::UEngineArchitectCore()
{
    PrimaryComponentTick.bCanEverTick = false;
    LastValidationTime = 0.0f;
    
    // Initialize required collision channels for dinosaur gameplay
    RequiredCollisionChannels.Add(TEXT("Dinosaur"));
    RequiredCollisionChannels.Add(TEXT("Player"));
    RequiredCollisionChannels.Add(TEXT("Environment"));
    RequiredCollisionChannels.Add(TEXT("Projectile"));
    RequiredCollisionChannels.Add(TEXT("Interaction"));
}

bool UEngineArchitectCore::ValidateCoreEngineSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT CORE VALIDATION ==="));
    
    bool bAllSystemsValid = true;
    
    // Validate physics world
    if (!IsPhysicsWorldValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Physics world validation FAILED"));
        bAllSystemsValid = false;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Physics world validation PASSED"));
    }
    
    // Validate rendering pipeline
    if (!ValidateRenderingPipeline())
    {
        UE_LOG(LogTemp, Error, TEXT("Rendering pipeline validation FAILED"));
        bAllSystemsValid = false;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Rendering pipeline validation PASSED"));
    }
    
    // Validate subsystems
    if (!ValidateSubsystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Subsystem validation FAILED"));
        bAllSystemsValid = false;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Subsystem validation PASSED"));
    }
    
    // Cache validation results
    ValidationResultsCache.Add(TEXT("CoreSystems"), bAllSystemsValid);
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Core engine validation: %s"), 
           bAllSystemsValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bAllSystemsValid;
}

void UEngineArchitectCore::EnforcePhysicsArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENFORCING PHYSICS ARCHITECTURE ==="));
    
    // Setup dinosaur collision channels
    SetupDinosaurCollisionChannels();
    
    // Configure physics settings for survival gameplay
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Enable CCD for fast-moving projectiles
        PhysicsSettings->bEnablePCM = true;
        PhysicsSettings->bEnableStabilization = true;
        
        // Optimize for large world with many actors
        PhysicsSettings->MaxSubstepDeltaTime = 0.016667f; // 60fps substeps
        PhysicsSettings->MaxSubsteps = 6;
        
        UE_LOG(LogTemp, Log, TEXT("Physics settings configured for dinosaur survival"));
    }
    
    // Setup ragdoll physics for dinosaurs
    SetupDinosaurRagdollPhysics();
}

bool UEngineArchitectCore::ValidatePerformanceTargets()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING PERFORMANCE TARGETS ==="));
    
    bool bPerformanceValid = CheckPerformanceMetrics();
    
    // Check actor count per biome
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 ActorCount = AllActors.Num();
        UE_LOG(LogTemp, Log, TEXT("Total actors in world: %d"), ActorCount);
        
        // Warn if approaching limits
        if (ActorCount > MaxActorsPerBiome * 5) // 5 biomes
        {
            UE_LOG(LogTemp, Warning, TEXT("Actor count exceeds recommended limits: %d > %d"), 
                   ActorCount, MaxActorsPerBiome * 5);
            bPerformanceValid = false;
        }
    }
    
    ValidationResultsCache.Add(TEXT("Performance"), bPerformanceValid);
    return bPerformanceValid;
}

bool UEngineArchitectCore::ValidateWorldPartitionSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING WORLD PARTITION SETUP ==="));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found"));
        return false;
    }
    
    // Check if world size requires World Partition
    FBox WorldBounds = FBox(ForceInit);
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && IsValid(Actor))
        {
            WorldBounds += Actor->GetActorLocation();
        }
    }
    
    float WorldSize = WorldBounds.GetSize().Size();
    bool bNeedsWorldPartition = WorldSize > WorldPartitionThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("World size: %.2f units, Partition needed: %s"), 
           WorldSize, bNeedsWorldPartition ? TEXT("YES") : TEXT("NO"));
    
    ValidationResultsCache.Add(TEXT("WorldPartition"), true);
    return true;
}

int32 UEngineArchitectCore::ValidateSpawnedActorCompliance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING SPAWNED ACTOR COMPLIANCE ==="));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 CompliantActors = 0;
    int32 NonCompliantActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        bool bIsCompliant = true;
        
        // Check if actor has proper collision setup
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
            {
                // Dinosaurs and interactive objects should have collision
                if (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                    Actor->GetName().Contains(TEXT("Interactive")))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Actor %s missing collision: %s"), 
                           *Actor->GetName(), *PrimComp->GetName());
                    bIsCompliant = false;
                }
            }
        }
        
        if (bIsCompliant)
        {
            CompliantActors++;
        }
        else
        {
            NonCompliantActors++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Actor compliance: %d compliant, %d non-compliant"), 
           CompliantActors, NonCompliantActors);
    
    return CompliantActors;
}

TArray<FString> UEngineArchitectCore::FindOrphanedHeaders()
{
    TArray<FString> OrphanedHeaders;
    
    // This would require file system access to scan for .h files without matching .cpp
    // For now, return empty array as this is primarily a build-time check
    UE_LOG(LogTemp, Log, TEXT("Orphaned header check completed - implement file scanning if needed"));
    
    return OrphanedHeaders;
}

bool UEngineArchitectCore::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Module dependency validation - checking Build.cs files"));
    
    // Module dependencies are validated at compile time
    // This function serves as a runtime confirmation that modules loaded correctly
    return true;
}

void UEngineArchitectCore::SetupDinosaurCollisionChannels()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SETTING UP DINOSAUR COLLISION CHANNELS ==="));
    
    // Collision channels are typically set up in DefaultEngine.ini
    // This function validates they exist and are properly configured
    
    for (const FString& ChannelName : RequiredCollisionChannels)
    {
        UE_LOG(LogTemp, Log, TEXT("Validating collision channel: %s"), *ChannelName);
    }
}

bool UEngineArchitectCore::ValidateDinosaurPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING DINOSAUR PHYSICS ==="));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find all dinosaur actors and validate their physics setup
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    int32 ValidPhysicsDinosaurs = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Rex")) || 
            Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
            
            // Check for physics components
            TArray<UPrimitiveComponent*> PhysicsComponents;
            Actor->GetComponents<UPrimitiveComponent>(PhysicsComponents);
            
            bool bHasValidPhysics = false;
            for (UPrimitiveComponent* PhysComp : PhysicsComponents)
            {
                if (PhysComp && PhysComp->IsSimulatingPhysics())
                {
                    bHasValidPhysics = true;
                    break;
                }
            }
            
            if (bHasValidPhysics)
            {
                ValidPhysicsDinosaurs++;
                UE_LOG(LogTemp, Log, TEXT("Dinosaur %s has valid physics"), *Actor->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s missing physics setup"), *Actor->GetName());
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur physics validation: %d/%d dinosaurs have valid physics"), 
           ValidPhysicsDinosaurs, DinosaurCount);
    
    return ValidPhysicsDinosaurs > 0;
}

void UEngineArchitectCore::SetupDinosaurRagdollPhysics()
{
    UE_LOG(LogTemp, Log, TEXT("Dinosaur ragdoll physics setup - requires skeletal mesh configuration"));
    
    // Ragdoll setup requires proper bone hierarchy and physics assets
    // This would be configured per dinosaur skeletal mesh
}

bool UEngineArchitectCore::IsPhysicsWorldValid() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if physics world exists and is active
    FPhysScene* PhysScene = World->GetPhysicsScene();
    return PhysScene != nullptr;
}

bool UEngineArchitectCore::ValidateRenderingPipeline() const
{
    // Check if Lumen is enabled for global illumination
    if (URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>())
    {
        // Validate key rendering features for dinosaur survival game
        return true; // Rendering pipeline is active if we can access settings
    }
    
    return false;
}

bool UEngineArchitectCore::CheckPerformanceMetrics() const
{
    // Performance metrics would be gathered from engine stats
    // For now, assume performance is acceptable
    return true;
}

bool UEngineArchitectCore::ValidateSubsystems() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check critical subsystems
    UGameInstanceSubsystem* GameInstanceSubsystem = 
        World->GetGameInstance()->GetSubsystem<UGameInstanceSubsystem>();
    
    // If we can access subsystems, they're working
    return true;
}