#include "Core_PhysicsArchitecturalIntegrator.h"
#include "Eng_ArchitecturalCore.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UCore_PhysicsArchitecturalIntegrator::UCore_PhysicsArchitecturalIntegrator()
{
    ArchitecturalCore = nullptr;
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
}

void UCore_PhysicsArchitecturalIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Initializing Physics Architectural Integration"));

    // Get reference to architectural core
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        ArchitecturalCore = GameInstance->GetSubsystem<UEng_ArchitecturalCore>();
        if (!ArchitecturalCore)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: Failed to get Architectural Core reference"));
            return;
        }
    }

    // Register all core physics systems
    RegisterCorePhysicsSystems();

    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Physics Architectural Integration initialized successfully"));
}

void UCore_PhysicsArchitecturalIntegrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Shutting down Physics Architectural Integration"));

    // Shutdown all physics systems
    ShutdownPhysicsSystems();

    // Clear references
    ArchitecturalCore = nullptr;
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;

    // Clear registration data
    RegisteredSystems.Empty();
    PerformanceMetrics.Empty();

    Super::Deinitialize();
}

bool UCore_PhysicsArchitecturalIntegrator::RegisterPhysicsSystem(const FString& SystemName, int32 Priority, bool bIsCritical, 
                                                                const TArray<FString>& Dependencies, float PerformanceBudget)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: Cannot register system with empty name"));
        return false;
    }

    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: System %s already registered, updating registration"), *SystemName);
    }

    // Create registration data
    FCore_PhysicsSystemRegistration Registration;
    Registration.SystemName = SystemName;
    Registration.InitializationPriority = Priority;
    Registration.bIsCriticalSystem = bIsCritical;
    Registration.Dependencies = Dependencies;
    Registration.PerformanceBudgetMs = PerformanceBudget;
    Registration.SystemStatus = ESystemStatus::Registered;

    // Add to registered systems
    RegisteredSystems.Add(SystemName, Registration);

    // Initialize performance metrics
    FCore_PhysicsPerformanceMetrics Metrics;
    PerformanceMetrics.Add(SystemName, Metrics);

    // Register with architectural core if available
    if (ArchitecturalCore)
    {
        ArchitecturalCore->RegisterCoreSystem(SystemName, Priority, bIsCritical);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsArchitecturalIntegrator: Registered physics system %s with priority %d"), *SystemName, Priority);
    return true;
}

bool UCore_PhysicsArchitecturalIntegrator::ValidatePhysicsSystemArchitecture(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: Cannot validate unregistered system %s"), *SystemName);
        return false;
    }

    const FCore_PhysicsSystemRegistration& Registration = RegisteredSystems[SystemName];

    // Validate dependencies
    if (!ValidateSystemDependencies(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: System %s failed dependency validation"), *SystemName);
        return false;
    }

    // Validate performance compliance
    if (!CheckPerformanceCompliance(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: System %s failed performance compliance"), *SystemName);
        return false;
    }

    // Validate with architectural core
    if (ArchitecturalCore)
    {
        if (!ArchitecturalCore->ValidateSystemCompliance(SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: System %s failed architectural core validation"), *SystemName);
            return false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsArchitecturalIntegrator: System %s passed all architectural validations"), *SystemName);
    return true;
}

void UCore_PhysicsArchitecturalIntegrator::UpdatePhysicsPerformanceMetrics(const FString& SystemName, const FCore_PhysicsPerformanceMetrics& Metrics)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Updating metrics for unregistered system %s"), *SystemName);
        return;
    }

    PerformanceMetrics.Add(SystemName, Metrics);

    // Check if performance is within acceptable bounds
    const FCore_PhysicsSystemRegistration& Registration = RegisteredSystems[SystemName];
    if (Metrics.AverageFrameTimeMs > Registration.PerformanceBudgetMs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: System %s exceeding performance budget: %.2fms > %.2fms"), 
               *SystemName, Metrics.AverageFrameTimeMs, Registration.PerformanceBudgetMs);
    }

    // Report to architectural core
    if (ArchitecturalCore)
    {
        ArchitecturalCore->ReportSystemPerformance(SystemName, Metrics.AverageFrameTimeMs, Metrics.MemoryUsageMB);
    }
}

TArray<FCore_PhysicsPerformanceMetrics> UCore_PhysicsArchitecturalIntegrator::GetAllPhysicsPerformanceMetrics() const
{
    TArray<FCore_PhysicsPerformanceMetrics> AllMetrics;
    
    for (const auto& MetricsPair : PerformanceMetrics)
    {
        AllMetrics.Add(MetricsPair.Value);
    }
    
    return AllMetrics;
}

void UCore_PhysicsArchitecturalIntegrator::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Initializing physics systems in priority order"));

    TArray<FString> InitOrder = GetInitializationOrder();
    
    for (const FString& SystemName : InitOrder)
    {
        if (FCore_PhysicsSystemRegistration* Registration = RegisteredSystems.Find(SystemName))
        {
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsArchitecturalIntegrator: Initializing system %s (Priority: %d)"), 
                   *SystemName, Registration->InitializationPriority);

            // Update system status
            Registration->SystemStatus = ESystemStatus::Initializing;

            // System-specific initialization logic would go here
            // For now, mark as initialized
            Registration->SystemStatus = ESystemStatus::Active;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: All physics systems initialized"));
}

void UCore_PhysicsArchitecturalIntegrator::ShutdownPhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Shutting down physics systems"));

    TArray<FString> InitOrder = GetInitializationOrder();
    
    // Shutdown in reverse order
    for (int32 i = InitOrder.Num() - 1; i >= 0; i--)
    {
        const FString& SystemName = InitOrder[i];
        if (FCore_PhysicsSystemRegistration* Registration = RegisteredSystems.Find(SystemName))
        {
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsArchitecturalIntegrator: Shutting down system %s"), *SystemName);
            Registration->SystemStatus = ESystemStatus::Shutdown;
        }
    }
}

bool UCore_PhysicsArchitecturalIntegrator::ValidateAllPhysicsSystems()
{
    bool bAllValid = true;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        if (!ValidatePhysicsSystemArchitecture(SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: System %s failed validation"), *SystemName);
            bAllValid = false;
        }
    }
    
    return bAllValid;
}

FCore_PhysicsSystemRegistration UCore_PhysicsArchitecturalIntegrator::GetPhysicsSystemRegistration(const FString& SystemName) const
{
    if (const FCore_PhysicsSystemRegistration* Registration = RegisteredSystems.Find(SystemName))
    {
        return *Registration;
    }
    
    return FCore_PhysicsSystemRegistration();
}

bool UCore_PhysicsArchitecturalIntegrator::IsPhysicsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FString> UCore_PhysicsArchitecturalIntegrator::GetRegisteredPhysicsSystems() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

void UCore_PhysicsArchitecturalIntegrator::RegisterCorePhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Registering core physics systems"));

    // Register Physics Manager (highest priority)
    RegisterPhysicsSystem(TEXT("PhysicsManager"), 10, true, TArray<FString>(), 2.0f);

    // Register Collision System
    TArray<FString> CollisionDeps;
    CollisionDeps.Add(TEXT("PhysicsManager"));
    RegisterPhysicsSystem(TEXT("CollisionSystem"), 11, true, CollisionDeps, 1.5f);

    // Register Material Physics
    TArray<FString> MaterialDeps;
    MaterialDeps.Add(TEXT("PhysicsManager"));
    MaterialDeps.Add(TEXT("CollisionSystem"));
    RegisterPhysicsSystem(TEXT("MaterialPhysics"), 12, true, MaterialDeps, 1.0f);

    // Register Terrain Physics
    TArray<FString> TerrainDeps;
    TerrainDeps.Add(TEXT("PhysicsManager"));
    TerrainDeps.Add(TEXT("CollisionSystem"));
    TerrainDeps.Add(TEXT("MaterialPhysics"));
    RegisterPhysicsSystem(TEXT("TerrainPhysics"), 13, true, TerrainDeps, 1.5f);

    // Register Ragdoll System
    TArray<FString> RagdollDeps;
    RagdollDeps.Add(TEXT("PhysicsManager"));
    RagdollDeps.Add(TEXT("CollisionSystem"));
    RegisterPhysicsSystem(TEXT("RagdollSystem"), 14, false, RagdollDeps, 2.0f);

    // Register Destruction System
    TArray<FString> DestructionDeps;
    DestructionDeps.Add(TEXT("PhysicsManager"));
    DestructionDeps.Add(TEXT("CollisionSystem"));
    DestructionDeps.Add(TEXT("MaterialPhysics"));
    RegisterPhysicsSystem(TEXT("DestructionSystem"), 15, false, DestructionDeps, 3.0f);

    // Register Fluid Dynamics
    TArray<FString> FluidDeps;
    FluidDeps.Add(TEXT("PhysicsManager"));
    FluidDeps.Add(TEXT("CollisionSystem"));
    FluidDeps.Add(TEXT("MaterialPhysics"));
    RegisterPhysicsSystem(TEXT("FluidDynamics"), 16, false, FluidDeps, 4.0f);

    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitecturalIntegrator: Registered %d core physics systems"), RegisteredSystems.Num());
}

bool UCore_PhysicsArchitecturalIntegrator::ValidateSystemDependencies(const FString& SystemName) const
{
    const FCore_PhysicsSystemRegistration* Registration = RegisteredSystems.Find(SystemName);
    if (!Registration)
    {
        return false;
    }

    // Check that all dependencies are registered
    for (const FString& Dependency : Registration->Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: System %s missing dependency %s"), *SystemName, *Dependency);
            return false;
        }

        // Check that dependency has higher priority (lower number)
        const FCore_PhysicsSystemRegistration* DepRegistration = RegisteredSystems.Find(Dependency);
        if (DepRegistration && DepRegistration->InitializationPriority >= Registration->InitializationPriority)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsArchitecturalIntegrator: System %s dependency %s has invalid priority"), *SystemName, *Dependency);
            return false;
        }
    }

    return true;
}

TArray<FString> UCore_PhysicsArchitecturalIntegrator::GetInitializationOrder() const
{
    TArray<FString> OrderedSystems;
    TArray<TPair<int32, FString>> PrioritySystemPairs;

    // Create priority-system pairs
    for (const auto& SystemPair : RegisteredSystems)
    {
        PrioritySystemPairs.Add(TPair<int32, FString>(SystemPair.Value.InitializationPriority, SystemPair.Key));
    }

    // Sort by priority (lower numbers first)
    PrioritySystemPairs.Sort([](const TPair<int32, FString>& A, const TPair<int32, FString>& B) {
        return A.Key < B.Key;
    });

    // Extract system names in order
    for (const auto& Pair : PrioritySystemPairs)
    {
        OrderedSystems.Add(Pair.Value);
    }

    return OrderedSystems;
}

bool UCore_PhysicsArchitecturalIntegrator::CheckPerformanceCompliance(const FString& SystemName) const
{
    const FCore_PhysicsPerformanceMetrics* Metrics = PerformanceMetrics.Find(SystemName);
    const FCore_PhysicsSystemRegistration* Registration = RegisteredSystems.Find(SystemName);

    if (!Metrics || !Registration)
    {
        return true; // No metrics yet, assume compliant
    }

    // Check frame time compliance
    if (Metrics->AverageFrameTimeMs > Registration->PerformanceBudgetMs)
    {
        return false;
    }

    // Check memory usage (arbitrary limit for now)
    if (Metrics->MemoryUsageMB > 100.0f)
    {
        return false;
    }

    return true;
}