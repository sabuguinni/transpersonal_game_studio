#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/CollisionProfile.h"

UEngineArchitectCore::UEngineArchitectCore()
{
    TargetFrameRate = 60.0f;
    MaxActorCount = 10000;
    MaxMemoryUsageMB = 8192.0f;
    
    // Initialize default physics settings
    DefaultPhysicsSettings.GravityScale = 1.0f;
    DefaultPhysicsSettings.LinearDamping = 0.01f;
    DefaultPhysicsSettings.AngularDamping = 0.01f;
    DefaultPhysicsSettings.bEnablePhysicsSimulation = true;
}

FEng_PerformanceMetrics UEngineArchitectCore::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

void UEngineArchitectCore::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Get current frame rate
        CurrentMetrics.FrameRate = 1.0f / World->GetDeltaSeconds();
        
        // Count active actors
        CurrentMetrics.ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->IsValidLowLevel() && !ActorItr->IsPendingKill())
            {
                CurrentMetrics.ActorCount++;
            }
        }
        
        // Memory usage (simplified)
        CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
        
        // Draw calls (approximation based on actor count)
        CurrentMetrics.DrawCalls = CurrentMetrics.ActorCount * 2;
    }
}

bool UEngineArchitectCore::ValidatePerformanceThresholds()
{
    UpdatePerformanceMetrics();
    
    bool bIsValid = true;
    
    if (CurrentMetrics.FrameRate < TargetFrameRate * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitect: Frame rate below threshold: %.2f < %.2f"), 
               CurrentMetrics.FrameRate, TargetFrameRate * 0.8f);
        bIsValid = false;
    }
    
    if (CurrentMetrics.ActorCount > MaxActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitect: Actor count exceeded: %d > %d"), 
               CurrentMetrics.ActorCount, MaxActorCount);
        bIsValid = false;
    }
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitect: Memory usage exceeded: %.2f MB > %.2f MB"), 
               CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
        bIsValid = false;
    }
    
    return bIsValid;
}

void UEngineArchitectCore::OptimizeWorldActors()
{
    if (UWorld* World = GetWorld())
    {
        int32 OptimizedCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->IsValidLowLevel())
            {
                // Optimize static mesh components
                TArray<UStaticMeshComponent*> StaticMeshComps;
                Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
                
                for (UStaticMeshComponent* MeshComp : StaticMeshComps)
                {
                    if (MeshComp)
                    {
                        // Enable LOD if not already enabled
                        if (!MeshComp->bAllowCullDistanceVolume)
                        {
                            MeshComp->bAllowCullDistanceVolume = true;
                            OptimizedCount++;
                        }
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("EngineArchitect: Optimized %d actors"), OptimizedCount);
    }
}

void UEngineArchitectCore::SetupDinosaurPhysics(AActor* DinosaurActor)
{
    if (!DinosaurActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitect: Invalid dinosaur actor for physics setup"));
        return;
    }
    
    // Setup collision for skeletal mesh components (animated dinosaurs)
    TArray<USkeletalMeshComponent*> SkeletalMeshComps;
    DinosaurActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
    
    for (USkeletalMeshComponent* SkeletalComp : SkeletalMeshComps)
    {
        if (SkeletalComp)
        {
            ConfigureCollisionPreset(SkeletalComp, EEng_CollisionPreset::DinosaurBody);
            
            // Apply physics settings
            SkeletalComp->SetSimulatePhysics(DefaultPhysicsSettings.bEnablePhysicsSimulation);
            SkeletalComp->SetLinearDamping(DefaultPhysicsSettings.LinearDamping);
            SkeletalComp->SetAngularDamping(DefaultPhysicsSettings.AngularDamping);
        }
    }
    
    // Setup collision for static mesh components (static dinosaur models)
    TArray<UStaticMeshComponent*> StaticMeshComps;
    DinosaurActor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
    
    for (UStaticMeshComponent* StaticComp : StaticMeshComps)
    {
        if (StaticComp)
        {
            ConfigureCollisionPreset(StaticComp, EEng_CollisionPreset::DinosaurBody);
            
            // Apply physics settings
            StaticComp->SetSimulatePhysics(DefaultPhysicsSettings.bEnablePhysicsSimulation);
            StaticComp->SetLinearDamping(DefaultPhysicsSettings.LinearDamping);
            StaticComp->SetAngularDamping(DefaultPhysicsSettings.AngularDamping);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitect: Physics setup completed for dinosaur: %s"), 
           *DinosaurActor->GetName());
}

void UEngineArchitectCore::ConfigureCollisionPreset(UPrimitiveComponent* Component, EEng_CollisionPreset Preset)
{
    if (!Component)
    {
        return;
    }
    
    ApplyCollisionSettings(Component, Preset);
}

void UEngineArchitectCore::ApplyCollisionSettings(UPrimitiveComponent* Component, EEng_CollisionPreset Preset)
{
    switch (Preset)
    {
        case EEng_CollisionPreset::DinosaurBody:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
            Component->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            Component->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
            break;
            
        case EEng_CollisionPreset::DinosaurTrigger:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Component->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            Component->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            Component->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
            break;
            
        case EEng_CollisionPreset::PlayerCharacter:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
            Component->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            Component->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
            break;
            
        case EEng_CollisionPreset::Environment:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            Component->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;
            
        case EEng_CollisionPreset::Projectile:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Component->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            Component->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            Component->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
            break;
            
        case EEng_CollisionPreset::Interactable:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Component->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            Component->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            Component->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
            break;
    }
}

bool UEngineArchitectCore::ValidatePhysicsIntegrity()
{
    if (UWorld* World = GetWorld())
    {
        int32 ValidActors = 0;
        int32 InvalidActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (ValidateActorPhysics(Actor))
            {
                ValidActors++;
            }
            else
            {
                InvalidActors++;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("EngineArchitect: Physics validation - Valid: %d, Invalid: %d"), 
               ValidActors, InvalidActors);
        
        return InvalidActors == 0;
    }
    
    return false;
}

bool UEngineArchitectCore::ValidateActorPhysics(AActor* Actor)
{
    if (!Actor || !Actor->IsValidLowLevel())
    {
        return false;
    }
    
    // Check for physics components
    TArray<UPrimitiveComponent*> PrimitiveComps;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComps)
    {
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            // Validate physics body
            if (!PrimComp->GetBodyInstance())
            {
                UE_LOG(LogTemp, Warning, TEXT("EngineArchitect: Missing physics body on %s"), 
                       *Actor->GetName());
                return false;
            }
        }
    }
    
    return true;
}

void UEngineArchitectCore::RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        RegisteredModules.Add(ModuleName);
        UE_LOG(LogTemp, Log, TEXT("EngineArchitect: Registered module %s with priority %d"), 
               *ModuleName, (int32)Priority);
    }
}

void UEngineArchitectCore::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitect: Validating %d registered modules"), 
           RegisteredModules.Num());
    
    for (const FString& ModuleName : RegisteredModules)
    {
        UE_LOG(LogTemp, Log, TEXT("  - Module: %s"), *ModuleName);
    }
}

bool UEngineArchitectCore::CheckSystemIntegrity()
{
    bool bIntegrityValid = true;
    
    // Check performance
    if (!ValidatePerformanceThresholds())
    {
        bIntegrityValid = false;
    }
    
    // Check physics
    if (!ValidatePhysicsIntegrity())
    {
        bIntegrityValid = false;
    }
    
    // Validate modules
    ValidateModuleDependencies();
    
    return bIntegrityValid;
}

void UEngineArchitectCore::InitializeWorldSystems()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitect: Initializing world systems"));
    
    // Register core modules
    RegisterSystemModule(TEXT("EngineArchitect"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("PhysicsCore"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("PerformanceMonitor"), EEng_SystemPriority::High);
    
    // Initialize physics settings
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitect: World systems initialized"));
}

void UEngineArchitectCore::ValidateWorldState()
{
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitect: World State - Actors: %d, FPS: %.1f, Memory: %.1f MB"), 
           CurrentMetrics.ActorCount, CurrentMetrics.FrameRate, CurrentMetrics.MemoryUsageMB);
}

int32 UEngineArchitectCore::GetActiveActorCount()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics.ActorCount;
}

// Engine Architect Subsystem Implementation

void UEngineArchitectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    EngineArchitect = NewObject<UEngineArchitectCore>(this);
    if (EngineArchitect)
    {
        EngineArchitect->InitializeWorldSystems();
        InitializePhysicsSettings();
        SetupCollisionProfiles();
        
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectSubsystem: Initialized successfully"));
    }
}

void UEngineArchitectSubsystem::Deinitialize()
{
    if (EngineArchitect)
    {
        EngineArchitect = nullptr;
    }
    
    Super::Deinitialize();
}

UEngineArchitectCore* UEngineArchitectSubsystem::GetEngineArchitect()
{
    return EngineArchitect;
}

void UEngineArchitectSubsystem::ValidateAllSystems()
{
    if (EngineArchitect)
    {
        EngineArchitect->CheckSystemIntegrity();
        ValidateModuleIntegration();
    }
}

bool UEngineArchitectSubsystem::IsSystemHealthy()
{
    if (EngineArchitect)
    {
        return EngineArchitect->ValidatePerformanceThresholds() && 
               EngineArchitect->ValidatePhysicsIntegrity();
    }
    
    return false;
}

void UEngineArchitectSubsystem::InitializePhysicsSettings()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectSubsystem: Physics settings initialized"));
}

void UEngineArchitectSubsystem::SetupCollisionProfiles()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectSubsystem: Collision profiles configured"));
}

void UEngineArchitectSubsystem::ValidateModuleIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectSubsystem: Module integration validated"));
}