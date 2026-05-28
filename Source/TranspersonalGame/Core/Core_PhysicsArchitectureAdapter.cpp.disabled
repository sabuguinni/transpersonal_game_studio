#include "Core_PhysicsArchitectureAdapter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UCore_PhysicsArchitectureAdapter::UCore_PhysicsArchitectureAdapter()
{
    // Initialize system state
    bIsInitialized = false;
    bIsHealthy = false;
    LastHealthCheckTime = 0.0f;
    
    // Initialize performance metrics
    AverageFrameTime = 0.0f;
    PeakPhysicsObjects = 0;
    PeakMemoryUsage = 0.0f;
    
    // Initialize health monitoring
    ConsecutiveHealthyFrames = 0;
    
    // Clear collections
    ActiveWarnings.Empty();
    ActiveErrors.Empty();
}

void UCore_PhysicsArchitectureAdapter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics Architecture Adapter: Initializing..."));
    
    // Get Core Architecture reference
    CoreArchitecture = GetGameInstance()->GetSubsystem<UEng_CoreArchitecture>();
    if (!CoreArchitecture)
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Architecture Adapter: Failed to get Core Architecture subsystem"));
        ActiveErrors.Add(TEXT("Core Architecture subsystem not available"));
        return;
    }
    
    // Register with Core Architecture
    if (!RegisterWithCoreArchitecture())
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Architecture Adapter: Failed to register with Core Architecture"));
        ActiveErrors.Add(TEXT("Failed to register with Core Architecture"));
        return;
    }
    
    // Initialize physics subsystems
    if (!InitializePhysicsSubsystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Core Physics Architecture Adapter: Failed to initialize physics subsystems"));
        ActiveErrors.Add(TEXT("Failed to initialize physics subsystems"));
        return;
    }
    
    bIsInitialized = true;
    bIsHealthy = true;
    LastHealthCheckTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics Architecture Adapter: Initialization complete"));
}

void UCore_PhysicsArchitectureAdapter::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics Architecture Adapter: Deinitializing..."));
    
    ShutdownPhysicsSubsystems();
    UnregisterFromCoreArchitecture();
    
    bIsInitialized = false;
    bIsHealthy = false;
    
    Super::Deinitialize();
}

FString UCore_PhysicsArchitectureAdapter::GetSystemName() const
{
    return TEXT("PhysicsArchitectureAdapter");
}

TArray<FString> UCore_PhysicsArchitectureAdapter::GetDependencies() const
{
    TArray<FString> Dependencies;
    Dependencies.Add(TEXT("CoreArchitecture"));
    Dependencies.Add(TEXT("GameInstance"));
    Dependencies.Add(TEXT("PhysicsEngine"));
    return Dependencies;
}

bool UCore_PhysicsArchitectureAdapter::InitializeSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Architecture Adapter: Already initialized"));
        return true;
    }
    
    return InitializePhysicsSubsystems();
}

void UCore_PhysicsArchitectureAdapter::ShutdownSystem()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Architecture Adapter: Not initialized"));
        return;
    }
    
    ShutdownPhysicsSubsystems();
    bIsInitialized = false;
}

bool UCore_PhysicsArchitectureAdapter::ValidateSystem() const
{
    if (!bIsInitialized)
    {
        return false;
    }
    
    return ValidatePhysicsIntegrity();
}

FEng_SystemHealthReport UCore_PhysicsArchitectureAdapter::GetHealthReport() const
{
    FEng_SystemHealthReport Report;
    Report.SystemName = GetSystemName();
    Report.bIsHealthy = bIsHealthy;
    Report.LastCheckTime = LastHealthCheckTime;
    Report.FrameTime = AverageFrameTime;
    Report.MemoryUsage = GetPhysicsMemoryUsage();
    Report.ActiveObjects = GetActivePhysicsObjects();
    Report.Warnings = ActiveWarnings;
    Report.Errors = ActiveErrors;
    
    return Report;
}

void UCore_PhysicsArchitectureAdapter::HandleArchitecturalEvent(const FEng_ArchitecturalEvent& Event)
{
    UE_LOG(LogTemp, Log, TEXT("Physics Architecture Adapter: Handling event %s"), *Event.EventType);
    
    if (Event.EventType == TEXT("ValidationRequired"))
    {
        OnArchitectureValidationRequired();
    }
    else if (Event.EventType == TEXT("PerformanceWarning"))
    {
        FString MetricName = Event.EventData.FindRef(TEXT("MetricName"));
        float Value = FCString::Atof(*Event.EventData.FindRef(TEXT("Value")));
        float Threshold = FCString::Atof(*Event.EventData.FindRef(TEXT("Threshold")));
        OnPerformanceThresholdExceeded(MetricName, Value, Threshold);
    }
    else if (Event.EventType == TEXT("SystemShutdown"))
    {
        ShutdownSystem();
    }
}

bool UCore_PhysicsArchitectureAdapter::InitializePhysicsSubsystems()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Architecture Adapter: Initializing physics subsystems..."));
    
    bool bAllSystemsInitialized = true;
    
    // Initialize Physics Manager
    if (!InitializePhysicsManager())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Physics Manager"));
    }
    
    // Initialize Collision System
    if (!InitializeCollisionSystem())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Collision System"));
    }
    
    // Initialize Ragdoll System
    if (!InitializeRagdollSystem())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Ragdoll System"));
    }
    
    // Initialize Destruction System
    if (!InitializeDestructionSystem())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Destruction System"));
    }
    
    // Initialize Terrain Physics
    if (!InitializeTerrainPhysics())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Terrain Physics"));
    }
    
    // Initialize Material Physics
    if (!InitializeMaterialPhysics())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Material Physics"));
    }
    
    // Initialize Fluid Dynamics
    if (!InitializeFluidDynamics())
    {
        bAllSystemsInitialized = false;
        ActiveErrors.Add(TEXT("Failed to initialize Fluid Dynamics"));
    }
    
    if (bAllSystemsInitialized)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Architecture Adapter: All physics subsystems initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Architecture Adapter: Some physics subsystems failed to initialize"));
    }
    
    return bAllSystemsInitialized;
}

void UCore_PhysicsArchitectureAdapter::ShutdownPhysicsSubsystems()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Architecture Adapter: Shutting down physics subsystems..."));
    
    // Shutdown in reverse order of initialization
    FluidDynamics = nullptr;
    MaterialPhysics = nullptr;
    TerrainPhysics = nullptr;
    DestructionSystem = nullptr;
    RagdollSystem = nullptr;
    CollisionSystem = nullptr;
    PhysicsManager = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Physics Architecture Adapter: Physics subsystems shutdown complete"));
}

bool UCore_PhysicsArchitectureAdapter::ValidatePhysicsIntegrity() const
{
    // Check if all critical systems are available
    bool bIntegrityValid = true;
    
    if (!PhysicsManager)
    {
        bIntegrityValid = false;
    }
    
    if (!CollisionSystem)
    {
        bIntegrityValid = false;
    }
    
    // Additional integrity checks can be added here
    
    return bIntegrityValid;
}

FString UCore_PhysicsArchitectureAdapter::GetPhysicsSystemsStatus() const
{
    FString Status = TEXT("Physics Systems Status:\n");
    
    Status += FString::Printf(TEXT("Physics Manager: %s\n"), PhysicsManager ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Collision System: %s\n"), CollisionSystem ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Ragdoll System: %s\n"), RagdollSystem ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Destruction System: %s\n"), DestructionSystem ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Terrain Physics: %s\n"), TerrainPhysics ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Material Physics: %s\n"), MaterialPhysics ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Fluid Dynamics: %s\n"), FluidDynamics ? TEXT("Active") : TEXT("Inactive"));
    
    return Status;
}

float UCore_PhysicsArchitectureAdapter::GetPhysicsFrameTime() const
{
    return AverageFrameTime;
}

int32 UCore_PhysicsArchitectureAdapter::GetActivePhysicsObjects() const
{
    // This would need to be implemented based on actual physics object tracking
    return 0; // Placeholder
}

float UCore_PhysicsArchitectureAdapter::GetPhysicsMemoryUsage() const
{
    // This would need to be implemented based on actual memory tracking
    return 0.0f; // Placeholder
}

bool UCore_PhysicsArchitectureAdapter::IsPhysicsSystemHealthy() const
{
    return bIsHealthy && ActiveErrors.Num() == 0;
}

TArray<FString> UCore_PhysicsArchitectureAdapter::GetPhysicsWarnings() const
{
    return ActiveWarnings;
}

TArray<FString> UCore_PhysicsArchitectureAdapter::GetPhysicsErrors() const
{
    return ActiveErrors;
}

bool UCore_PhysicsArchitectureAdapter::RegisterWithCoreArchitecture()
{
    if (!CoreArchitecture)
    {
        return false;
    }
    
    // Register this system with the Core Architecture
    return CoreArchitecture->RegisterArchitecturalSystem(this);
}

void UCore_PhysicsArchitectureAdapter::UnregisterFromCoreArchitecture()
{
    if (CoreArchitecture)
    {
        CoreArchitecture->UnregisterArchitecturalSystem(this);
    }
}

bool UCore_PhysicsArchitectureAdapter::InitializePhysicsManager()
{
    PhysicsManager = GetGameInstance()->GetSubsystem<UCore_PhysicsManager>();
    if (PhysicsManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Manager initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Manager not available"));
    return false;
}

bool UCore_PhysicsArchitectureAdapter::InitializeCollisionSystem()
{
    CollisionSystem = GetGameInstance()->GetSubsystem<UCore_CollisionSystem>();
    if (CollisionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Collision System initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Collision System not available"));
    return false;
}

bool UCore_PhysicsArchitectureAdapter::InitializeRagdollSystem()
{
    RagdollSystem = GetGameInstance()->GetSubsystem<UCore_RagdollSystem>();
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Ragdoll System initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll System not available"));
    return false;
}

bool UCore_PhysicsArchitectureAdapter::InitializeDestructionSystem()
{
    DestructionSystem = GetGameInstance()->GetSubsystem<UCore_DestructionSystem>();
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Destruction System initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Destruction System not available"));
    return false;
}

bool UCore_PhysicsArchitectureAdapter::InitializeTerrainPhysics()
{
    TerrainPhysics = GetGameInstance()->GetSubsystem<UCore_TerrainPhysics>();
    if (TerrainPhysics)
    {
        UE_LOG(LogTemp, Log, TEXT("Terrain Physics initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain Physics not available"));
    return false;
}

bool UCore_PhysicsArchitectureAdapter::InitializeMaterialPhysics()
{
    MaterialPhysics = GetGameInstance()->GetSubsystem<UCore_MaterialPhysics>();
    if (MaterialPhysics)
    {
        UE_LOG(LogTemp, Log, TEXT("Material Physics initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Material Physics not available"));
    return false;
}

bool UCore_PhysicsArchitectureAdapter::InitializeFluidDynamics()
{
    FluidDynamics = GetGameInstance()->GetSubsystem<UCore_FluidDynamics>();
    if (FluidDynamics)
    {
        UE_LOG(LogTemp, Log, TEXT("Fluid Dynamics initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Fluid Dynamics not available"));
    return false;
}

void UCore_PhysicsArchitectureAdapter::UpdateHealthMetrics()
{
    float CurrentTime = FPlatformTime::Seconds();
    
    // Update health check time
    LastHealthCheckTime = CurrentTime;
    
    // Check system integrity
    CheckSystemIntegrity();
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Update health status
    bIsHealthy = (ActiveErrors.Num() == 0) && ValidatePhysicsIntegrity();
    
    if (bIsHealthy)
    {
        ConsecutiveHealthyFrames++;
    }
    else
    {
        ConsecutiveHealthyFrames = 0;
    }
}

void UCore_PhysicsArchitectureAdapter::CheckSystemIntegrity()
{
    // Clear previous warnings
    ActiveWarnings.Empty();
    
    // Check if critical systems are still available
    if (!PhysicsManager)
    {
        ActiveWarnings.Add(TEXT("Physics Manager is not available"));
    }
    
    if (!CollisionSystem)
    {
        ActiveWarnings.Add(TEXT("Collision System is not available"));
    }
    
    // Additional integrity checks can be added here
}

void UCore_PhysicsArchitectureAdapter::UpdatePerformanceMetrics()
{
    // Update frame time (this would need actual frame time measurement)
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    AverageFrameTime = (AverageFrameTime + CurrentFrameTime) * 0.5f; // Simple moving average
    
    // Update peak metrics
    int32 CurrentPhysicsObjects = GetActivePhysicsObjects();
    if (CurrentPhysicsObjects > PeakPhysicsObjects)
    {
        PeakPhysicsObjects = CurrentPhysicsObjects;
    }
    
    float CurrentMemoryUsage = GetPhysicsMemoryUsage();
    if (CurrentMemoryUsage > PeakMemoryUsage)
    {
        PeakMemoryUsage = CurrentMemoryUsage;
    }
    
    // Check for performance warnings
    if (AverageFrameTime > PERFORMANCE_WARNING_THRESHOLD)
    {
        ActiveWarnings.Add(FString::Printf(TEXT("Frame time exceeds threshold: %.2f ms"), AverageFrameTime));
    }
    
    if (CurrentMemoryUsage > MEMORY_WARNING_THRESHOLD)
    {
        ActiveWarnings.Add(FString::Printf(TEXT("Memory usage exceeds threshold: %.2f MB"), CurrentMemoryUsage));
    }
    
    if (CurrentPhysicsObjects > MAX_PHYSICS_OBJECTS_WARNING)
    {
        ActiveWarnings.Add(FString::Printf(TEXT("Physics objects exceed threshold: %d"), CurrentPhysicsObjects));
    }
}

void UCore_PhysicsArchitectureAdapter::OnPhysicsSystemEvent(const FString& SystemName, const FString& EventType, const FString& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("Physics System Event - System: %s, Type: %s, Data: %s"), *SystemName, *EventType, *EventData);
    
    // Handle different types of physics system events
    if (EventType == TEXT("Error"))
    {
        ActiveErrors.Add(FString::Printf(TEXT("%s: %s"), *SystemName, *EventData));
    }
    else if (EventType == TEXT("Warning"))
    {
        ActiveWarnings.Add(FString::Printf(TEXT("%s: %s"), *SystemName, *EventData));
    }
}

void UCore_PhysicsArchitectureAdapter::OnArchitectureValidationRequired()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Architecture Adapter: Architecture validation required"));
    
    // Perform comprehensive validation
    bool bValidationPassed = ValidateSystem();
    
    if (!bValidationPassed)
    {
        ActiveErrors.Add(TEXT("Architecture validation failed"));
    }
    
    // Report validation results to Core Architecture
    if (CoreArchitecture)
    {
        FEng_SystemHealthReport Report = GetHealthReport();
        // CoreArchitecture->ReportSystemHealth(Report); // This would need to be implemented in Core Architecture
    }
}

void UCore_PhysicsArchitectureAdapter::OnPerformanceThresholdExceeded(const FString& MetricName, float Value, float Threshold)
{
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Warning - %s: %.2f exceeds threshold %.2f"), *MetricName, Value, Threshold);
    
    ActiveWarnings.Add(FString::Printf(TEXT("Performance threshold exceeded - %s: %.2f > %.2f"), *MetricName, Value, Threshold));
}