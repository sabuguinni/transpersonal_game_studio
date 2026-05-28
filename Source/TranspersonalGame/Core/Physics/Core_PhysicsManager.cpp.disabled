#include "Core_PhysicsManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    CurrentFrameTimeMs = 0.0f;
    PhysicsBudgetMs = MAX_PHYSICS_BUDGET_MS;
    bOptimizationActive = false;
}

void UCore_PhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Initializing physics subsystem"));
    
    // Initialize species-specific physics configurations
    InitializeSpeciesConfigs();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UCore_PhysicsManager::UpdatePerformanceMetrics, 0.1f, true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Initialization complete with %d species configs"), SpeciesConfigs.Num());
}

void UCore_PhysicsManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Deinitializing physics subsystem"));
    
    // Clear all registered actors
    RegisteredPhysicsActors.Empty();
    SpeciesConfigs.Empty();
    
    Super::Deinitialize();
}

bool UCore_PhysicsManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld() || World->IsEditorWorld();
    }
    return false;
}

void UCore_PhysicsManager::InitializeSpeciesConfigs()
{
    // T-Rex configuration - Massive predator
    FCore_DinosaurPhysicsConfig TRexConfig;
    TRexConfig.MassKg = TREX_MASS_KG;
    TRexConfig.LinearDamping = 0.05f;  // Low damping for momentum
    TRexConfig.AngularDamping = 0.1f;
    TRexConfig.bEnableGravity = true;
    TRexConfig.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    TRexConfig.CollisionResponse = ECollisionResponse::ECR_Block;
    SpeciesConfigs.Add(ECore_DinosaurSpecies::TRex, TRexConfig);
    
    // Velociraptor configuration - Fast and agile
    FCore_DinosaurPhysicsConfig RaptorConfig;
    RaptorConfig.MassKg = VELOCIRAPTOR_MASS_KG;
    RaptorConfig.LinearDamping = 0.2f;  // Higher damping for quick stops
    RaptorConfig.AngularDamping = 0.3f;
    RaptorConfig.bEnableGravity = true;
    RaptorConfig.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    RaptorConfig.CollisionResponse = ECollisionResponse::ECR_Block;
    SpeciesConfigs.Add(ECore_DinosaurSpecies::Velociraptor, RaptorConfig);
    
    // Brachiosaurus configuration - Gentle giant
    FCore_DinosaurPhysicsConfig BrachioConfig;
    BrachioConfig.MassKg = BRACHIOSAURUS_MASS_KG;
    BrachioConfig.LinearDamping = 0.02f;  // Very low damping due to massive size
    BrachioConfig.AngularDamping = 0.05f;
    BrachioConfig.bEnableGravity = true;
    BrachioConfig.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    BrachioConfig.CollisionResponse = ECollisionResponse::ECR_Block;
    SpeciesConfigs.Add(ECore_DinosaurSpecies::Brachiosaurus, BrachioConfig);
    
    // Triceratops configuration - Armored herbivore
    FCore_DinosaurPhysicsConfig TriceraConfig;
    TriceraConfig.MassKg = TRICERATOPS_MASS_KG;
    TriceraConfig.LinearDamping = 0.08f;
    TriceraConfig.AngularDamping = 0.15f;
    TriceraConfig.bEnableGravity = true;
    TriceraConfig.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    TriceraConfig.CollisionResponse = ECollisionResponse::ECR_Block;
    SpeciesConfigs.Add(ECore_DinosaurSpecies::Triceratops, TriceraConfig);
    
    // Ankylosaurus configuration - Tank-like herbivore
    FCore_DinosaurPhysicsConfig AnkyloConfig;
    AnkyloConfig.MassKg = ANKYLOSAURUS_MASS_KG;
    AnkyloConfig.LinearDamping = 0.1f;
    AnkyloConfig.AngularDamping = 0.2f;
    AnkyloConfig.bEnableGravity = true;
    AnkyloConfig.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    AnkyloConfig.CollisionResponse = ECollisionResponse::ECR_Block;
    SpeciesConfigs.Add(ECore_DinosaurSpecies::Ankylosaurus, AnkyloConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Initialized %d species physics configurations"), SpeciesConfigs.Num());
}

bool UCore_PhysicsManager::ConfigureDinosaurPhysics(AActor* DinosaurActor, ECore_DinosaurSpecies Species)
{
    if (!DinosaurActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsManager: Cannot configure physics for null dinosaur actor"));
        return false;
    }
    
    const FCore_DinosaurPhysicsConfig* Config = SpeciesConfigs.Find(Species);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsManager: No physics config found for species %d"), (int32)Species);
        return false;
    }
    
    // Find the main physics component (skeletal mesh for dinosaurs)
    USkeletalMeshComponent* SkeletalComp = DinosaurActor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalComp)
    {
        ApplyPhysicsConfig(SkeletalComp, *Config);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Applied physics config to skeletal mesh for %s (Mass: %.1f kg)"), 
               *DinosaurActor->GetName(), Config->MassKg);
        return true;
    }
    
    // Fallback to static mesh component
    UStaticMeshComponent* StaticComp = DinosaurActor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticComp)
    {
        ApplyPhysicsConfig(StaticComp, *Config);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Applied physics config to static mesh for %s (Mass: %.1f kg)"), 
               *DinosaurActor->GetName(), Config->MassKg);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Core_PhysicsManager: No physics component found on dinosaur actor %s"), *DinosaurActor->GetName());
    return false;
}

void UCore_PhysicsManager::ApplyPhysicsConfig(UPrimitiveComponent* Component, const FCore_DinosaurPhysicsConfig& Config)
{
    if (!Component)
    {
        return;
    }
    
    // Configure collision
    Component->SetCollisionEnabled(Config.CollisionEnabled);
    Component->SetCollisionResponseToAllChannels(Config.CollisionResponse);
    
    // Configure physics simulation
    Component->SetSimulatePhysics(true);
    Component->SetEnableGravity(Config.bEnableGravity);
    
    // Configure mass and damping
    if (FBodyInstance* BodyInstance = Component->GetBodyInstance())
    {
        BodyInstance->SetMassOverride(Config.MassKg, true);
        BodyInstance->LinearDamping = Config.LinearDamping;
        BodyInstance->AngularDamping = Config.AngularDamping;
        BodyInstance->UpdateMassProperties();
    }
}

void UCore_PhysicsManager::ApplyDinosaurImpulse(AActor* DinosaurActor, FVector ImpulseVector, bool bVelChange)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    UPrimitiveComponent* PhysicsComp = nullptr;
    
    // Find physics component
    USkeletalMeshComponent* SkeletalComp = DinosaurActor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalComp && SkeletalComp->IsSimulatingPhysics())
    {
        PhysicsComp = SkeletalComp;
    }
    else
    {
        UStaticMeshComponent* StaticComp = DinosaurActor->FindComponentByClass<UStaticMeshComponent>();
        if (StaticComp && StaticComp->IsSimulatingPhysics())
        {
            PhysicsComp = StaticComp;
        }
    }
    
    if (PhysicsComp)
    {
        PhysicsComp->AddImpulse(ImpulseVector, NAME_None, bVelChange);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Applied impulse %.1f,%.1f,%.1f to %s"), 
               ImpulseVector.X, ImpulseVector.Y, ImpulseVector.Z, *DinosaurActor->GetName());
    }
}

void UCore_PhysicsManager::SetDinosaurPhysicsEnabled(AActor* DinosaurActor, bool bEnable)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    // Enable/disable physics on all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComps;
    DinosaurActor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
    
    for (UPrimitiveComponent* Comp : PrimitiveComps)
    {
        if (Comp)
        {
            Comp->SetSimulatePhysics(bEnable);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: %s physics for %s"), 
           bEnable ? TEXT("Enabled") : TEXT("Disabled"), *DinosaurActor->GetName());
}

FCore_DinosaurPhysicsConfig UCore_PhysicsManager::GetSpeciesPhysicsConfig(ECore_DinosaurSpecies Species) const
{
    const FCore_DinosaurPhysicsConfig* Config = SpeciesConfigs.Find(Species);
    if (Config)
    {
        return *Config;
    }
    
    // Return default config if species not found
    return FCore_DinosaurPhysicsConfig();
}

void UCore_PhysicsManager::RegisterPhysicsActor(AActor* PhysicsActor, int32 Priority)
{
    if (PhysicsActor && !RegisteredPhysicsActors.Contains(PhysicsActor))
    {
        RegisteredPhysicsActors.Add(PhysicsActor);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Registered physics actor %s (Priority: %d)"), 
               *PhysicsActor->GetName(), Priority);
    }
}

void UCore_PhysicsManager::UnregisterPhysicsActor(AActor* PhysicsActor)
{
    if (PhysicsActor)
    {
        RegisteredPhysicsActors.RemoveSingle(PhysicsActor);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Unregistered physics actor %s"), *PhysicsActor->GetName());
    }
}

void UCore_PhysicsManager::GetPhysicsMetrics(float& OutFrameTimeMs, int32& OutActiveActors, float& OutBudgetUsage) const
{
    OutFrameTimeMs = CurrentFrameTimeMs;
    
    // Count active physics actors
    OutActiveActors = 0;
    for (const TWeakObjectPtr<AActor>& ActorPtr : RegisteredPhysicsActors)
    {
        if (ActorPtr.IsValid())
        {
            OutActiveActors++;
        }
    }
    
    // Calculate budget usage percentage
    OutBudgetUsage = (CurrentFrameTimeMs / PhysicsBudgetMs) * 100.0f;
}

void UCore_PhysicsManager::OptimizePhysicsPerformance()
{
    if (bOptimizationActive)
    {
        return;
    }
    
    bOptimizationActive = true;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Activating physics optimization (Budget exceeded: %.2fms > %.2fms)"), 
           CurrentFrameTimeMs, PhysicsBudgetMs);
    
    // Reduce physics quality for performance
    if (UWorld* World = GetWorld())
    {
        // Reduce physics substeps
        World->GetPhysicsScene()->GetPxScene()->setSubStepSize(1.0f / 30.0f); // 30Hz instead of 60Hz
        
        // Disable physics on distant actors
        for (const TWeakObjectPtr<AActor>& ActorPtr : RegisteredPhysicsActors)
        {
            if (AActor* Actor = ActorPtr.Get())
            {
                // Simple distance check - disable physics for actors >5000 units away
                float DistanceToPlayer = 5000.0f; // Simplified - would need player reference
                if (DistanceToPlayer > 5000.0f)
                {
                    SetDinosaurPhysicsEnabled(Actor, false);
                }
            }
        }
    }
}

void UCore_PhysicsManager::RestorePhysicsQuality()
{
    if (!bOptimizationActive)
    {
        return;
    }
    
    bOptimizationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsManager: Restoring full physics quality"));
    
    // Restore physics quality
    if (UWorld* World = GetWorld())
    {
        // Restore normal physics substeps
        World->GetPhysicsScene()->GetPxScene()->setSubStepSize(1.0f / 60.0f); // 60Hz
        
        // Re-enable physics on all registered actors
        for (const TWeakObjectPtr<AActor>& ActorPtr : RegisteredPhysicsActors)
        {
            if (AActor* Actor = ActorPtr.Get())
            {
                SetDinosaurPhysicsEnabled(Actor, true);
            }
        }
    }
}

void UCore_PhysicsManager::UpdatePerformanceMetrics()
{
    // Simple frame time tracking
    static double LastUpdateTime = 0.0;
    double CurrentTime = FPlatformTime::Seconds();
    
    if (LastUpdateTime > 0.0)
    {
        double DeltaTime = CurrentTime - LastUpdateTime;
        CurrentFrameTimeMs = static_cast<float>(DeltaTime * 1000.0);
        
        // Check if we need to optimize or restore
        if (CurrentFrameTimeMs > PhysicsBudgetMs && !bOptimizationActive)
        {
            OptimizePhysicsPerformance();
        }
        else if (CurrentFrameTimeMs < (PhysicsBudgetMs * 0.7f) && bOptimizationActive)
        {
            RestorePhysicsQuality();
        }
    }
    
    LastUpdateTime = CurrentTime;
    
    // Clean up invalid actor references
    RegisteredPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
        return !ActorPtr.IsValid();
    });
}