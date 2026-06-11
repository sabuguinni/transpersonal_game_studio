#include "Core_PhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for physics management
    
    // Initialize default physics profile for realistic prehistoric survival
    CurrentProfile.Gravity = -980.0f; // Realistic Earth gravity
    CurrentProfile.LinearDamping = 0.01f; // Minimal air resistance
    CurrentProfile.AngularDamping = 0.05f; // Natural rotational damping
    CurrentProfile.MaxAngularVelocity = 3600.0f; // 10 rotations per second max
    CurrentProfile.bEnableAsyncScene = true; // Performance optimization
    CurrentProfile.BounceCombineMode = 0.5f; // Average bounce
    CurrentProfile.FrictionCombineMode = 0.5f; // Average friction
    
    // Default settings
    PhysicsMode = ECore_PhysicsMode::Realistic;
    bEnablePhysicsValidation = true;
    ValidationInterval = 1.0f;
    bEnableComplexCollision = true;
    CollisionTolerance = 0.1f;
    MaxPhysicsObjects = 1000;
    PhysicsLODDistance = 2000.0f;
    bEnablePhysicsLOD = true;
}

void UCore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Initializing physics system"));
    
    // Apply initial physics profile
    ApplyPhysicsProfile(CurrentProfile);
    
    // Initialize physics materials
    InitializePhysicsMaterials();
    
    // Validate initial physics state
    if (bEnablePhysicsValidation)
    {
        ValidatePhysicsSettings();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics system initialized successfully"));
}

void UCore_PhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastValidationTime += DeltaTime;
    
    // Periodic validation
    if (bEnablePhysicsValidation && LastValidationTime >= ValidationInterval)
    {
        UpdatePhysicsObjectCount();
        ValidatePhysicsObjects();
        
        if (bEnablePhysicsLOD)
        {
            ApplyPerformanceOptimizations();
        }
        
        LastValidationTime = 0.0f;
    }
}

void UCore_PhysicsManager::ApplyPhysicsProfile(const FCore_PhysicsProfile& Profile)
{
    CurrentProfile = Profile;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Cannot apply physics profile - no world"));
        return;
    }
    
    // Apply gravity
    World->GetWorldSettings()->GlobalGravityZ = Profile.Gravity;
    
    // Apply physics settings via engine
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->DefaultGravityZ = Profile.Gravity;
        PhysicsSettings->bEnableAsyncScene = Profile.bEnableAsyncScene;
        PhysicsSettings->MaxAngularVelocity = Profile.MaxAngularVelocity;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics profile applied - Gravity: %f"), Profile.Gravity);
}

void UCore_PhysicsManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    PhysicsMode = NewMode;
    
    // Apply mode-specific settings
    switch (NewMode)
    {
        case ECore_PhysicsMode::Realistic:
            CurrentProfile.Gravity = -980.0f;
            CurrentProfile.LinearDamping = 0.01f;
            CurrentProfile.AngularDamping = 0.05f;
            break;
            
        case ECore_PhysicsMode::Arcade:
            CurrentProfile.Gravity = -600.0f;
            CurrentProfile.LinearDamping = 0.1f;
            CurrentProfile.AngularDamping = 0.2f;
            break;
            
        case ECore_PhysicsMode::Cinematic:
            CurrentProfile.Gravity = -400.0f;
            CurrentProfile.LinearDamping = 0.2f;
            CurrentProfile.AngularDamping = 0.3f;
            break;
    }
    
    ApplyPhysicsProfile(CurrentProfile);
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics mode set to %d"), (int32)NewMode);
}

bool UCore_PhysicsManager::ValidatePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsManager: Validation failed - no world"));
        return false;
    }
    
    bool bValidationPassed = true;
    
    // Validate gravity
    float CurrentGravity = World->GetWorldSettings()->GlobalGravityZ;
    if (FMath::Abs(CurrentGravity - CurrentProfile.Gravity) > 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Gravity mismatch - Expected: %f, Current: %f"), 
               CurrentProfile.Gravity, CurrentGravity);
        bValidationPassed = false;
    }
    
    // Validate physics object count
    UpdatePhysicsObjectCount();
    if (CurrentPhysicsObjectCount > MaxPhysicsObjects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Too many physics objects - Count: %d, Max: %d"), 
               CurrentPhysicsObjectCount, MaxPhysicsObjects);
        bValidationPassed = false;
    }
    
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics validation passed"));
    }
    
    return bValidationPassed;
}

void UCore_PhysicsManager::RegisterPhysicsMaterial(const FString& MaterialName, UPhysicalMaterial* Material)
{
    if (Material)
    {
        PhysicsMaterials.Add(MaterialName, Material);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Registered physics material: %s"), *MaterialName);
    }
}

UPhysicalMaterial* UCore_PhysicsManager::GetPhysicsMaterial(const FString& MaterialName)
{
    if (UPhysicalMaterial** FoundMaterial = PhysicsMaterials.Find(MaterialName))
    {
        return *FoundMaterial;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Physics material not found: %s"), *MaterialName);
    return nullptr;
}

void UCore_PhysicsManager::OptimizePhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get all physics objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        // Find physics components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                float DistanceToPlayer = GetDistanceToPlayer(Actor);
                
                // Apply LOD based on distance
                if (DistanceToPlayer > PhysicsLODDistance)
                {
                    // Disable physics for distant objects
                    PrimComp->SetSimulatePhysics(false);
                }
                else if (DistanceToPlayer < PhysicsLODDistance * 0.5f)
                {
                    // Ensure physics is enabled for close objects
                    if (!PrimComp->IsSimulatingPhysics())
                    {
                        PrimComp->SetSimulatePhysics(true);
                    }
                }
            }
        }
    }
}

void UCore_PhysicsManager::DebugPhysicsState()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS DEBUG STATE ==="));
    UE_LOG(LogTemp, Log, TEXT("Physics Mode: %d"), (int32)PhysicsMode);
    UE_LOG(LogTemp, Log, TEXT("Gravity: %f"), CurrentProfile.Gravity);
    UE_LOG(LogTemp, Log, TEXT("Physics Objects: %d / %d"), CurrentPhysicsObjectCount, MaxPhysicsObjects);
    UE_LOG(LogTemp, Log, TEXT("Physics Materials: %d"), PhysicsMaterials.Num());
    UE_LOG(LogTemp, Log, TEXT("Validation Enabled: %s"), bEnablePhysicsValidation ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("LOD Enabled: %s"), bEnablePhysicsLOD ? TEXT("Yes") : TEXT("No"));
    
    LogPhysicsStatistics();
}

void UCore_PhysicsManager::UpdatePhysicsObjectCount()
{
    CurrentPhysicsObjectCount = 0;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentPhysicsObjectCount++;
            }
        }
    }
}

void UCore_PhysicsManager::ApplyPerformanceOptimizations()
{
    if (CurrentPhysicsObjectCount > MaxPhysicsObjects * 0.8f)
    {
        OptimizePhysicsPerformance();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Applied performance optimizations"));
    }
}

void UCore_PhysicsManager::ValidatePhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 InvalidObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Check for invalid physics state
                FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
                if (Velocity.Size() > 10000.0f) // Unrealistic velocity
                {
                    PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
                    InvalidObjects++;
                }
            }
        }
    }
    
    if (InvalidObjects > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Fixed %d invalid physics objects"), InvalidObjects);
    }
}

void UCore_PhysicsManager::LogPhysicsStatistics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 SimulatingObjects = 0;
    int32 StaticObjects = 0;
    int32 KinematicObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    SimulatingObjects++;
                }
                else if (PrimComp->Mobility == EComponentMobility::Static)
                {
                    StaticObjects++;
                }
                else
                {
                    KinematicObjects++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Statistics - Simulating: %d, Static: %d, Kinematic: %d"), 
           SimulatingObjects, StaticObjects, KinematicObjects);
}

float UCore_PhysicsManager::GetDistanceToPlayer(AActor* Actor)
{
    if (!Actor) return 0.0f;
    
    UWorld* World = GetWorld();
    if (!World) return 0.0f;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return 0.0f;
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UCore_PhysicsManager::InitializePhysicsMaterials()
{
    // Initialize default physics materials for prehistoric survival
    // These will be loaded from assets or created procedurally
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Initializing physics materials"));
    
    // Note: In a full implementation, these would load actual UPhysicalMaterial assets
    // For now, we register the material names for future asset loading
    
    TArray<FString> DefaultMaterials = {
        TEXT("Stone"),
        TEXT("Wood"),
        TEXT("Flesh"),
        TEXT("Bone"),
        TEXT("Water"),
        TEXT("Mud"),
        TEXT("Grass"),
        TEXT("Sand")
    };
    
    for (const FString& MaterialName : DefaultMaterials)
    {
        // Register placeholder - actual materials would be loaded from assets
        PhysicsMaterials.Add(MaterialName, nullptr);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Registered material slot: %s"), *MaterialName);
    }
}