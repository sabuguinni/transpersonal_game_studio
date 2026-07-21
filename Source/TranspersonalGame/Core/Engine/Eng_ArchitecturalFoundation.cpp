#include "Eng_ArchitecturalFoundation.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngineArchitect, Log, All);

UEng_ArchitecturalFoundation::UEng_ArchitecturalFoundation()
{
    TargetFrameTime = 16.67f; // 60fps
    MaxActiveActors = 50000;
    MaxMemoryUsageMB = 8192.0f; // 8GB
}

void UEng_ArchitecturalFoundation::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("=== ENGINE ARCHITECT FOUNDATION INITIALIZING ==="));
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Register core architectural systems
    FEng_SystemRegistration CoreSystem;
    CoreSystem.SystemName = "ArchitecturalFoundation";
    CoreSystem.Priority = EEng_SystemPriority::Critical;
    CoreSystem.Status = EEng_SystemStatus::Initializing;
    CoreSystem.AgentNumber = 2; // Engine Architect
    
    RegisteredSystems.Add(CoreSystem.SystemName, CoreSystem);
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Architectural Foundation initialized with %d systems"), RegisteredSystems.Num());
}

void UEng_ArchitecturalFoundation::Deinitialize()
{
    UE_LOG(LogEngineArchitect, Log, TEXT("=== ENGINE ARCHITECT FOUNDATION SHUTTING DOWN ==="));
    
    // Shutdown all registered systems
    ShutdownAllSystems();
    
    // Clear registrations
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalFoundation::RegisterSystem(const FEng_SystemRegistration& SystemData)
{
    if (SystemData.SystemName.IsEmpty())
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemData.SystemName))
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("System '%s' already registered, updating registration"), *SystemData.SystemName);
    }
    
    // Validate dependencies
    for (const FString& Dependency : SystemData.Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            UE_LOG(LogEngineArchitect, Warning, TEXT("System '%s' depends on unregistered system '%s'"), 
                *SystemData.SystemName, *Dependency);
        }
    }
    
    RegisteredSystems.Add(SystemData.SystemName, SystemData);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Registered system: %s (Agent %d, Priority: %d)"), 
        *SystemData.SystemName, SystemData.AgentNumber, (int32)SystemData.Priority);
    
    return true;
}

bool UEng_ArchitecturalFoundation::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Cannot unregister unknown system: %s"), *SystemName);
        return false;
    }
    
    // Check if other systems depend on this one
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Dependencies.Contains(SystemName))
        {
            UE_LOG(LogEngineArchitect, Error, TEXT("Cannot unregister system '%s' - system '%s' depends on it"), 
                *SystemName, *SystemPair.Key);
            return false;
        }
    }
    
    RegisteredSystems.Remove(SystemName);
    UE_LOG(LogEngineArchitect, Log, TEXT("Unregistered system: %s"), *SystemName);
    
    return true;
}

EEng_SystemStatus UEng_ArchitecturalFoundation::GetSystemStatus(const FString& SystemName)
{
    if (const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))
    {
        return System->Status;
    }
    
    return EEng_SystemStatus::Uninitialized;
}

bool UEng_ArchitecturalFoundation::ValidateDependencies()
{
    bool bAllValid = true;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        
        for (const FString& Dependency : System.Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                UE_LOG(LogEngineArchitect, Error, TEXT("DEPENDENCY ERROR: System '%s' requires missing system '%s'"), 
                    *System.SystemName, *Dependency);
                bAllValid = false;
            }
        }
    }
    
    if (bAllValid)
    {
        UE_LOG(LogEngineArchitect, Log, TEXT("All system dependencies validated successfully"));
    }
    
    return bAllValid;
}

TArray<FString> UEng_ArchitecturalFoundation::GetInitializationOrder()
{
    TArray<FString> InitOrder;
    TSet<FString> Processed;
    TSet<FString> Processing;
    
    // Topological sort for dependency-aware initialization
    TFunction<bool(const FString&)> ProcessSystem = [&](const FString& SystemName) -> bool
    {
        if (Processed.Contains(SystemName))
        {
            return true;
        }
        
        if (Processing.Contains(SystemName))
        {
            UE_LOG(LogEngineArchitect, Error, TEXT("Circular dependency detected involving system: %s"), *SystemName);
            return false;
        }
        
        Processing.Add(SystemName);
        
        if (const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))
        {
            // Process dependencies first
            for (const FString& Dependency : System->Dependencies)
            {
                if (!ProcessSystem(Dependency))
                {
                    return false;
                }
            }
        }
        
        Processing.Remove(SystemName);
        Processed.Add(SystemName);
        InitOrder.Add(SystemName);
        
        return true;
    };\n    \n    // Process all systems by priority\n    TArray<FString> SystemsByPriority;\n    for (const auto& SystemPair : RegisteredSystems)\n    {\n        SystemsByPriority.Add(SystemPair.Key);\n    }\n    \n    // Sort by priority (Critical first)\n    SystemsByPriority.Sort([this](const FString& A, const FString& B) {\n        const FEng_SystemRegistration* SystemA = RegisteredSystems.Find(A);\n        const FEng_SystemRegistration* SystemB = RegisteredSystems.Find(B);\n        \n        if (SystemA && SystemB)\n        {\n            return (int32)SystemA->Priority < (int32)SystemB->Priority;\n        }\n        \n        return false;\n    });\n    \n    for (const FString& SystemName : SystemsByPriority)\n    {\n        ProcessSystem(SystemName);\n    }\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Generated initialization order for %d systems\"), InitOrder.Num());\n    \n    return InitOrder;\n}\n\nFEng_PerformanceMetrics UEng_ArchitecturalFoundation::GetCurrentPerformanceMetrics()\n{\n    UpdatePerformanceMetrics();\n    return CurrentMetrics;\n}\n\nbool UEng_ArchitecturalFoundation::IsPerformanceTargetMet()\n{\n    UpdatePerformanceMetrics();\n    \n    // Check frame time target\n    if (CurrentMetrics.FrameTime > TargetFrameTime * 1.1f) // 10% tolerance\n    {\n        return false;\n    }\n    \n    // Check actor count\n    if (CurrentMetrics.ActiveActors > MaxActiveActors)\n    {\n        return false;\n    }\n    \n    // Check memory usage\n    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)\n    {\n        return false;\n    }\n    \n    return true;\n}\n\nvoid UEng_ArchitecturalFoundation::InitializeAllSystems()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"=== INITIALIZING ALL SYSTEMS ===\"));\n    \n    TArray<FString> InitOrder = GetInitializationOrder();\n    \n    for (const FString& SystemName : InitOrder)\n    {\n        if (FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))\n        {\n            if (System->Status == EEng_SystemStatus::Uninitialized)\n            {\n                System->Status = EEng_SystemStatus::Initializing;\n                \n                double StartTime = FPlatformTime::Seconds();\n                \n                // System-specific initialization would happen here\n                // For now, just mark as ready\n                System->Status = EEng_SystemStatus::Ready;\n                \n                double EndTime = FPlatformTime::Seconds();\n                System->InitializationTime = (EndTime - StartTime) * 1000.0f; // Convert to ms\n                \n                UE_LOG(LogEngineArchitect, Log, TEXT(\"Initialized system '%s' in %.2fms\"), \n                    *SystemName, System->InitializationTime);\n            }\n        }\n    }\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"All systems initialization complete\"));\n}\n\nvoid UEng_ArchitecturalFoundation::ShutdownAllSystems()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"=== SHUTTING DOWN ALL SYSTEMS ===\"));\n    \n    // Shutdown in reverse order\n    TArray<FString> InitOrder = GetInitializationOrder();\n    Algo::Reverse(InitOrder);\n    \n    for (const FString& SystemName : InitOrder)\n    {\n        if (FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))\n        {\n            if (System->Status != EEng_SystemStatus::Shutdown)\n            {\n                System->Status = EEng_SystemStatus::Shutdown;\n                UE_LOG(LogEngineArchitect, Log, TEXT(\"Shutdown system: %s\"), *SystemName);\n            }\n        }\n    }\n}\n\nvoid UEng_ArchitecturalFoundation::ValidateArchitecture()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"=== ARCHITECTURAL VALIDATION ===\"));\n    \n    // Validate dependencies\n    bool bDependenciesValid = ValidateDependencies();\n    \n    // Validate performance\n    bool bPerformanceValid = IsPerformanceTargetMet();\n    \n    // Validate system states\n    int32 ReadySystems = 0;\n    int32 ErrorSystems = 0;\n    \n    for (const auto& SystemPair : RegisteredSystems)\n    {\n        switch (SystemPair.Value.Status)\n        {\n            case EEng_SystemStatus::Ready:\n            case EEng_SystemStatus::Running:\n                ReadySystems++;\n                break;\n            case EEng_SystemStatus::Error:\n                ErrorSystems++;\n                UE_LOG(LogEngineArchitect, Error, TEXT(\"System in error state: %s\"), *SystemPair.Key);\n                break;\n            default:\n                break;\n        }\n    }\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Validation Results:\"));\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"- Dependencies Valid: %s\"), bDependenciesValid ? TEXT(\"YES\") : TEXT(\"NO\"));\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"- Performance Valid: %s\"), bPerformanceValid ? TEXT(\"YES\") : TEXT(\"NO\"));\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"- Ready Systems: %d/%d\"), ReadySystems, RegisteredSystems.Num());\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"- Error Systems: %d\"), ErrorSystems);\n}\n\nvoid UEng_ArchitecturalFoundation::GenerateSystemReport()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"=== SYSTEM ARCHITECTURE REPORT ===\"));\n    \n    FString ReportPath = FPaths::ProjectLogDir() / TEXT(\"ArchitecturalReport.txt\");\n    FString Report;\n    \n    Report += FString::Printf(TEXT(\"Transpersonal Game - Architectural Report\\n\"));\n    Report += FString::Printf(TEXT(\"Generated: %s\\n\\n\"), *FDateTime::Now().ToString());\n    \n    Report += FString::Printf(TEXT(\"REGISTERED SYSTEMS (%d):\\n\"), RegisteredSystems.Num());\n    \n    for (const auto& SystemPair : RegisteredSystems)\n    {\n        const FEng_SystemRegistration& System = SystemPair.Value;\n        Report += FString::Printf(TEXT(\"- %s (Agent %d, Priority: %d, Status: %d)\\n\"), \n            *System.SystemName, System.AgentNumber, (int32)System.Priority, (int32)System.Status);\n        \n        if (System.Dependencies.Num() > 0)\n        {\n            Report += FString::Printf(TEXT(\"  Dependencies: %s\\n\"), *FString::Join(System.Dependencies, TEXT(\", \")));\n        }\n    }\n    \n    Report += FString::Printf(TEXT(\"\\nPERFORMANCE METRICS:\\n\"));\n    Report += FString::Printf(TEXT(\"- Frame Time: %.2fms (Target: %.2fms)\\n\"), CurrentMetrics.FrameTime, TargetFrameTime);\n    Report += FString::Printf(TEXT(\"- Active Actors: %d (Max: %d)\\n\"), CurrentMetrics.ActiveActors, MaxActiveActors);\n    Report += FString::Printf(TEXT(\"- Memory Usage: %.2fMB (Max: %.2fMB)\\n\"), CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);\n    \n    FFileHelper::SaveStringToFile(Report, *ReportPath);\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"System report saved to: %s\"), *ReportPath);\n}\n\nvoid UEng_ArchitecturalFoundation::UpdatePerformanceMetrics()\n{\n    // Get current frame time\n    if (GEngine && GEngine->GetGameViewport())\n    {\n        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms\n    }\n    \n    // Count active actors in current world\n    if (UWorld* World = GetWorld())\n    {\n        CurrentMetrics.ActiveActors = World->GetActorCount();\n    }\n    \n    // Get memory stats\n    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();\n    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);\n    \n    // Get render stats\n    CurrentMetrics.DrawCalls = 0; // Would need render thread access\n    CurrentMetrics.GameThreadTime = 0.0f;\n    CurrentMetrics.RenderThreadTime = 0.0f;\n    CurrentMetrics.GPUTime = 0.0f;\n}\n\nbool UEng_ArchitecturalFoundation::CheckSystemDependencies(const FString& SystemName)\n{\n    if (const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))\n    {\n        for (const FString& Dependency : System->Dependencies)\n        {\n            if (!RegisteredSystems.Contains(Dependency))\n            {\n                return false;\n            }\n            \n            EEng_SystemStatus DepStatus = GetSystemStatus(Dependency);\n            if (DepStatus != EEng_SystemStatus::Ready && DepStatus != EEng_SystemStatus::Running)\n            {\n                return false;\n            }\n        }\n    }\n    \n    return true;\n}\n\nvoid UEng_ArchitecturalFoundation::LogArchitecturalViolation(const FString& Violation)\n{\n    UE_LOG(LogEngineArchitect, Error, TEXT(\"ARCHITECTURAL VIOLATION: %s\"), *Violation);\n    \n    // In development builds, could trigger breakpoint or assert\n    #if UE_BUILD_DEVELOPMENT\n    ensureMsgf(false, TEXT(\"Architectural violation detected: %s\"), *Violation);\n    #endif\n}\n\n// WORLD ARCHITECTURAL COORDINATOR IMPLEMENTATION\n\nUEng_WorldArchitecturalCoordinator::UEng_WorldArchitecturalCoordinator()\n{\n    bWorldSystemsInitialized = false;\n    bStreamingConfigured = false;\n}\n\nvoid UEng_WorldArchitecturalCoordinator::Initialize(FSubsystemCollectionBase& Collection)\n{\n    Super::Initialize(Collection);\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"World Architectural Coordinator initializing for world: %s\"), \n        GetWorld() ? *GetWorld()->GetName() : TEXT(\"Unknown\"));\n    \n    // Initialize world-specific systems\n    InitializeWorldSystems();\n}\n\nvoid UEng_WorldArchitecturalCoordinator::Deinitialize()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"World Architectural Coordinator shutting down\"));\n    \n    bWorldSystemsInitialized = false;\n    bStreamingConfigured = false;\n    \n    Super::Deinitialize();\n}\n\nvoid UEng_WorldArchitecturalCoordinator::InitializeWorldSystems()\n{\n    if (bWorldSystemsInitialized)\n    {\n        return;\n    }\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Initializing world systems...\"));\n    \n    // Configure world partition\n    ConfigureWorldPartition();\n    \n    // Set up performance budgets\n    SetupPerformanceBudgets();\n    \n    // Configure actor culling\n    ConfigureActorCulling();\n    \n    // Register level systems\n    RegisterLevelSystems();\n    \n    bWorldSystemsInitialized = true;\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"World systems initialization complete\"));\n}\n\nbool UEng_WorldArchitecturalCoordinator::ValidateWorldConfiguration()\n{\n    if (!GetWorld())\n    {\n        UE_LOG(LogEngineArchitect, Error, TEXT(\"No valid world for configuration validation\"));\n        return false;\n    }\n    \n    bool bValid = true;\n    \n    // Check world partition setup\n    if (!bStreamingConfigured)\n    {\n        UE_LOG(LogEngineArchitect, Warning, TEXT(\"World partition not configured\"));\n        bValid = false;\n    }\n    \n    // Check performance settings\n    // Additional validation logic would go here\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"World configuration validation: %s\"), bValid ? TEXT(\"PASSED\") : TEXT(\"FAILED\"));\n    \n    return bValid;\n}\n\nvoid UEng_WorldArchitecturalCoordinator::ConfigureWorldPartition()\n{\n    if (!GetWorld())\n    {\n        return;\n    }\n    \n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Configuring world partition for large-scale world support\"));\n    \n    // World partition configuration would go here\n    // This would set up streaming, LOD, and culling for 50K+ actors\n    \n    bStreamingConfigured = true;\n}\n\nvoid UEng_WorldArchitecturalCoordinator::OptimizeStreamingSettings()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Optimizing streaming settings for performance\"));\n    \n    // Streaming optimization logic would go here\n}\n\nvoid UEng_WorldArchitecturalCoordinator::RegisterLevelSystems()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Registering level-specific systems\"));\n    \n    // Register world-specific systems with the main architectural foundation\n    if (UEng_ArchitecturalFoundation* Foundation = GetGameInstance()->GetSubsystem<UEng_ArchitecturalFoundation>())\n    {\n        FEng_SystemRegistration WorldSystem;\n        WorldSystem.SystemName = \"WorldArchitecturalCoordinator\";\n        WorldSystem.Priority = EEng_SystemPriority::High;\n        WorldSystem.Status = EEng_SystemStatus::Ready;\n        WorldSystem.AgentNumber = 2; // Engine Architect\n        \n        Foundation->RegisterSystem(WorldSystem);\n    }\n}\n\nvoid UEng_WorldArchitecturalCoordinator::SetupPerformanceBudgets()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Setting up performance budgets for world systems\"));\n    \n    // Performance budget setup logic would go here\n    // This would configure CPU/GPU/Memory budgets for different systems\n}\n\nvoid UEng_WorldArchitecturalCoordinator::ConfigureActorCulling()\n{\n    UE_LOG(LogEngineArchitect, Log, TEXT(\"Configuring actor culling for large-scale worlds\"));\n    \n    // Actor culling configuration would go here\n    // This would set up distance-based culling, frustum culling, etc.\n}"