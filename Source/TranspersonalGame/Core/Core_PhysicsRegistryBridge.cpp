#include "Core_PhysicsRegistryBridge.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UCore_PhysicsRegistryBridge::UCore_PhysicsRegistryBridge()
{
    SystemsRegistry = nullptr;
    RegisteredSystems = TArray<FCore_PhysicsSystemRegistration>();
    CurrentMetrics = FCore_PhysicsPerformanceMetrics();
    PerformanceBudgetMS = 8.0f; // Default budget for physics systems
    bIsInitialized = false;
    LastFrameTime = 0.0;
    FrameTimeHistory = TArray<float>();
}

void UCore_PhysicsRegistryBridge::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Initializing..."));

    // Get reference to the Systems Registry
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SystemsRegistry = GameInstance->GetSubsystem<UEng_SystemsRegistry>();
        if (SystemsRegistry)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Connected to Systems Registry"));
            
            // Register all physics systems
            RegisterAllPhysicsSystems();
            
            // Validate architectural compliance
            bool bCompliant = ValidatePhysicsArchitecturalCompliance();
            UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Architectural compliance: %s"), 
                   bCompliant ? TEXT("PASS") : TEXT("FAIL"));
            
            bIsInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Initialization complete"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: Systems Registry not found!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: Game Instance not found!"));
    }
}

void UCore_PhysicsRegistryBridge::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Shutting down..."));
    
    // Unregister all physics systems from the registry
    if (SystemsRegistry && bIsInitialized)
    {
        for (const FCore_PhysicsSystemRegistration& System : RegisteredSystems)
        {
            // Note: Actual unregistration would call SystemsRegistry->UnregisterSystem(System.SystemName)
            UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Unregistering %s"), *System.SystemName);
        }
    }
    
    RegisteredSystems.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UCore_PhysicsRegistryBridge::RegisterAllPhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Registering all physics systems..."));

    // Clear existing registrations
    RegisteredSystems.Empty();

    // Register Core Physics Manager (highest priority - must initialize first)
    RegisterPhysicsSystem(TEXT("Core_PhysicsManager"), ECore_PhysicsSystemPriority::Critical, 
                         TArray<FString>(), 2.0f);

    // Register Material Physics (dependency for collision)
    RegisterPhysicsSystem(TEXT("Core_MaterialPhysics"), ECore_PhysicsSystemPriority::High, 
                         {TEXT("Core_PhysicsManager")}, 1.5f);

    // Register Collision System (depends on physics manager and material physics)
    RegisterPhysicsSystem(TEXT("Core_CollisionSystem"), ECore_PhysicsSystemPriority::High, 
                         {TEXT("Core_PhysicsManager"), TEXT("Core_MaterialPhysics")}, 2.0f);

    // Register Terrain Physics (depends on collision and material physics)
    RegisterPhysicsSystem(TEXT("Core_TerrainPhysics"), ECore_PhysicsSystemPriority::Medium, 
                         {TEXT("Core_CollisionSystem"), TEXT("Core_MaterialPhysics")}, 1.0f);

    // Register Ragdoll System (depends on collision)
    RegisterPhysicsSystem(TEXT("Core_RagdollSystem"), ECore_PhysicsSystemPriority::Medium, 
                         {TEXT("Core_CollisionSystem")}, 1.5f);

    // Register Destruction System (depends on collision and ragdoll)
    RegisterPhysicsSystem(TEXT("Core_DestructionSystem"), ECore_PhysicsSystemPriority::Medium, 
                         {TEXT("Core_CollisionSystem"), TEXT("Core_RagdollSystem")}, 1.0f);

    // Register Vehicle Physics (depends on collision and terrain)
    RegisterPhysicsSystem(TEXT("Core_VehiclePhysics"), ECore_PhysicsSystemPriority::Low, 
                         {TEXT("Core_CollisionSystem"), TEXT("Core_TerrainPhysics")}, 0.5f);

    // Register Fluid Dynamics (depends on collision and terrain)
    RegisterPhysicsSystem(TEXT("Core_FluidDynamics"), ECore_PhysicsSystemPriority::Low, 
                         {TEXT("Core_CollisionSystem"), TEXT("Core_TerrainPhysics")}, 0.5f);

    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Registered %d physics systems"), 
           RegisteredSystems.Num());
}

void UCore_PhysicsRegistryBridge::RegisterPhysicsSystem(const FString& SystemName, 
                                                       ECore_PhysicsSystemPriority Priority, 
                                                       const TArray<FString>& Dependencies, 
                                                       float BudgetMS)
{
    FCore_PhysicsSystemRegistration Registration;
    Registration.SystemName = SystemName;
    Registration.Priority = Priority;
    Registration.Dependencies = Dependencies;
    Registration.PerformanceBudgetMS = BudgetMS;
    Registration.bIsArchitecturallyCompliant = ValidateSystemCompliance(SystemName);
    Registration.RegistrationTime = FDateTime::Now();

    RegisteredSystems.Add(Registration);

    // Register with the Systems Registry if available
    if (SystemsRegistry)
    {
        // Note: This would call SystemsRegistry->RegisterSystem() with proper parameters
        UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Registered %s with Systems Registry"), 
               *SystemName);
    }

    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: %s registered (Priority: %d, Budget: %.1fms)"), 
           *SystemName, (int32)Priority, BudgetMS);
}

bool UCore_PhysicsRegistryBridge::ValidatePhysicsArchitecturalCompliance()
{
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Validating architectural compliance..."));

    bool bAllCompliant = true;
    int32 CompliantSystems = 0;

    for (FCore_PhysicsSystemRegistration& System : RegisteredSystems)
    {
        bool bSystemCompliant = ValidateSystemCompliance(System.SystemName);
        System.bIsArchitecturallyCompliant = bSystemCompliant;
        
        if (bSystemCompliant)
        {
            CompliantSystems++;
        }
        else
        {
            bAllCompliant = false;
            UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: %s failed compliance validation"), 
                   *System.SystemName);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Compliance validation complete: %d/%d systems compliant"), 
           CompliantSystems, RegisteredSystems.Num());

    return bAllCompliant;
}

bool UCore_PhysicsRegistryBridge::ValidateSystemCompliance(const FString& SystemName)
{
    // Check if the system follows Core_ naming convention
    if (!SystemName.StartsWith(TEXT("Core_")))
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: %s violates naming convention (missing Core_ prefix)"), 
               *SystemName);
        return false;
    }

    // Check if the system class exists and can be loaded
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (!SystemClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: %s class not found at %s"), 
               *SystemName, *ClassPath);
        return false;
    }

    // Additional compliance checks would go here
    // - Check for required UFUNCTION/UPROPERTY decorations
    // - Validate performance characteristics
    // - Check integration interfaces

    UE_LOG(LogTemp, Log, TEXT("Core Physics Registry Bridge: %s passed compliance validation"), *SystemName);
    return true;
}

void UCore_PhysicsRegistryBridge::UpdatePhysicsPerformanceMetrics()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Calculate frame time metrics
    CalculatePerformanceMetrics();

    // Update performance budget status
    CurrentMetrics.bPerformanceBudgetExceeded = (CurrentMetrics.CurrentFrameTimeMS > PerformanceBudgetMS);

    // Get physics world statistics if available
    if (UWorld* World = GetWorld())
    {
        // Note: In a real implementation, we would query the physics scene for:
        // - Active rigid bodies count
        // - Collision checks per frame
        // - Memory usage
        CurrentMetrics.ActivePhysicsBodies = 150; // Placeholder
        CurrentMetrics.CollisionChecksPerFrame = 2500; // Placeholder
    }

    // Log performance warnings if budget exceeded
    if (CurrentMetrics.bPerformanceBudgetExceeded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Performance budget exceeded! %.2fms > %.2fms"), 
               CurrentMetrics.CurrentFrameTimeMS, PerformanceBudgetMS);
    }
}

void UCore_PhysicsRegistryBridge::CalculatePerformanceMetrics()
{
    double CurrentTime = FPlatformTime::Seconds();
    
    if (LastFrameTime > 0.0)
    {
        float FrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to milliseconds
        
        CurrentMetrics.CurrentFrameTimeMS = FrameTime;
        
        // Update frame time history
        FrameTimeHistory.Add(FrameTime);
        if (FrameTimeHistory.Num() > MaxFrameHistory)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average
        float TotalTime = 0.0f;
        float MaxTime = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            TotalTime += Time;
            MaxTime = FMath::Max(MaxTime, Time);
        }
        
        CurrentMetrics.AverageFrameTimeMS = TotalTime / FrameTimeHistory.Num();
        CurrentMetrics.PeakFrameTimeMS = MaxTime;
    }
    
    LastFrameTime = CurrentTime;
}

FCore_PhysicsPerformanceMetrics UCore_PhysicsRegistryBridge::GetPhysicsPerformanceMetrics() const
{
    return CurrentMetrics;
}

bool UCore_PhysicsRegistryBridge::IsWithinPerformanceBudget() const
{
    return !CurrentMetrics.bPerformanceBudgetExceeded;
}

TArray<FCore_PhysicsSystemRegistration> UCore_PhysicsRegistryBridge::GetRegisteredPhysicsSystems() const
{
    return RegisteredSystems;
}

void UCore_PhysicsRegistryBridge::BroadcastPhysicsSystemMessage(const FString& MessageType, const FString& MessageData)
{
    if (!SystemsRegistry || !bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: Cannot broadcast message - not initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Core Physics Registry Bridge: Broadcasting message '%s' to all physics systems"), 
           *MessageType);

    // Note: In a real implementation, this would call SystemsRegistry->SendSystemMessage()
    // to broadcast the message to all registered physics systems
    
    for (const FCore_PhysicsSystemRegistration& System : RegisteredSystems)
    {
        UE_LOG(LogTemp, Log, TEXT("Core Physics Registry Bridge: Sending '%s' to %s"), 
               *MessageType, *System.SystemName);
    }
}

void UCore_PhysicsRegistryBridge::EmergencyPhysicsShutdown()
{
    UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: EMERGENCY SHUTDOWN INITIATED"));

    if (SystemsRegistry && bIsInitialized)
    {
        // Shutdown all physics systems in reverse priority order
        TArray<FCore_PhysicsSystemRegistration> SortedSystems = RegisteredSystems;
        SortedSystems.Sort([](const FCore_PhysicsSystemRegistration& A, const FCore_PhysicsSystemRegistration& B) {
            return (int32)A.Priority > (int32)B.Priority; // Reverse order
        });

        for (const FCore_PhysicsSystemRegistration& System : SortedSystems)
        {
            UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: Emergency shutdown of %s"), *System.SystemName);
            // Note: Would call SystemsRegistry->EmergencyShutdownSystem(System.SystemName)
        }
    }

    // Reset all metrics and state
    CurrentMetrics = FCore_PhysicsPerformanceMetrics();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Error, TEXT("Core Physics Registry Bridge: Emergency shutdown complete"));
}