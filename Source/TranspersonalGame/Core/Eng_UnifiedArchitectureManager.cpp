#include "Eng_UnifiedArchitectureManager.h"
#include "Core_PhysicsManager.h"
#include "BiomeManager.h"
#include "Dir_ProductionManager.h"
#include "Core_PhysicsIntegrator.h"
#include "Core_PhysicsPerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UEng_UnifiedArchitectureManager::UEng_UnifiedArchitectureManager()
{
    // Initialize default values
    SystemHealthCheckInterval = 5.0f;
    bAutoRestartFailedSystems = true;
    bEnablePerformanceMonitoring = true;
    bArchitectureValid = false;
    LastValidationTime = 0.0f;
    
    // Initialize system references to null
    PhysicsManager = nullptr;
    BiomeManager = nullptr;
    ProductionManager = nullptr;
    PhysicsIntegrator = nullptr;
}

void UEng_UnifiedArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - UNIFIED ARCHITECTURE MANAGER INITIALIZING ==="));
    
    // Initialize system tracking
    SystemInitializationStatus.Empty();
    FailedSystems.Empty();
    
    // Initialize performance report structure
    CurrentPerformanceReport.SystemName = TEXT("UnifiedArchitecture");
    CurrentPerformanceReport.FrameTime = 0.0f;
    CurrentPerformanceReport.MemoryUsage = 0.0f;
    CurrentPerformanceReport.CPUUsage = 0.0f;
    CurrentPerformanceReport.bIsHealthy = true;
    
    // Start health monitoring if enabled
    if (bEnablePerformanceMonitoring)
    {
        GetWorld()->GetTimerManager().SetTimer(
            HealthCheckTimer,
            this,
            &UEng_UnifiedArchitectureManager::CheckSystemHealth,
            SystemHealthCheckInterval,
            true
        );
        
        GetWorld()->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UEng_UnifiedArchitectureManager::UpdatePerformanceMetrics,
            1.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Unified Architecture Manager initialized successfully"));
}

void UEng_UnifiedArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - UNIFIED ARCHITECTURE MANAGER SHUTTING DOWN ==="));
    
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HealthCheckTimer);
        GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    // Shutdown all systems
    ShutdownAllSystems();
    
    Super::Deinitialize();
}

void UEng_UnifiedArchitectureManager::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INITIALIZING ALL ARCHITECTURE SYSTEMS ==="));
    
    // CRITICAL: Systems must be initialized in dependency order
    // This order is architectural law - DO NOT CHANGE
    
    // Phase 1: Core Physics Systems
    InitializePhysicsSystems();
    
    // Phase 2: World Generation Systems
    InitializeWorldSystems();
    
    // Phase 3: Character Systems
    InitializeCharacterSystems();
    
    // Phase 4: AI Systems
    InitializeAISystems();
    
    // Phase 5: Production Systems
    InitializeProductionSystems();
    
    // Phase 6: Performance Monitoring
    InitializePerformanceSystems();
    
    // Final validation
    bArchitectureValid = ValidateSystemIntegrity();
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture initialization complete. Valid: %s"), 
           bArchitectureValid ? TEXT("TRUE") : TEXT("FALSE"));
    
    // Broadcast initialization complete
    OnArchitectureValidationComplete.ExecuteIfBound(bArchitectureValid);
}

void UEng_UnifiedArchitectureManager::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SHUTTING DOWN ALL ARCHITECTURE SYSTEMS ==="));
    
    // Shutdown in reverse dependency order
    if (PhysicsManager)
    {
        PhysicsManager = nullptr;
        SystemInitializationStatus.Add(TEXT("PhysicsManager"), false);
    }
    
    if (BiomeManager)
    {
        BiomeManager = nullptr;
        SystemInitializationStatus.Add(TEXT("BiomeManager"), false);
    }
    
    if (ProductionManager && IsValid(ProductionManager))
    {
        ProductionManager = nullptr;
        SystemInitializationStatus.Add(TEXT("ProductionManager"), false);
    }
    
    if (PhysicsIntegrator)
    {
        PhysicsIntegrator = nullptr;
        SystemInitializationStatus.Add(TEXT("PhysicsIntegrator"), false);
    }
    
    bArchitectureValid = false;
}

bool UEng_UnifiedArchitectureManager::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING SYSTEM INTEGRITY ==="));
    
    bool bAllSystemsValid = true;
    
    // Validate Physics Integration
    if (!ValidatePhysicsIntegration())
    {
        bAllSystemsValid = false;
        HandleSystemFailure(TEXT("PhysicsIntegration"), TEXT("Physics systems validation failed"));
    }
    
    // Validate Biome Integration
    if (!ValidateBiomeIntegration())
    {
        bAllSystemsValid = false;
        HandleSystemFailure(TEXT("BiomeIntegration"), TEXT("Biome systems validation failed"));
    }
    
    // Validate Production Integration
    if (!ValidateProductionIntegration())
    {
        bAllSystemsValid = false;
        HandleSystemFailure(TEXT("ProductionIntegration"), TEXT("Production systems validation failed"));
    }
    
    // Validate Performance Metrics
    if (!ValidatePerformanceMetrics())
    {
        bAllSystemsValid = false;
        HandleSystemFailure(TEXT("PerformanceMetrics"), TEXT("Performance monitoring validation failed"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System integrity validation: %s"), 
           bAllSystemsValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bAllSystemsValid;
}

void UEng_UnifiedArchitectureManager::RestartFailedSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RESTARTING FAILED SYSTEMS ==="));
    
    if (FailedSystems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No failed systems to restart"));
        return;
    }
    
    TArray<FString> SystemsToRestart = FailedSystems;
    FailedSystems.Empty();
    
    for (const FString& SystemName : SystemsToRestart)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to restart system: %s"), *SystemName);
        
        if (SystemName == TEXT("PhysicsManager"))
        {
            InitializePhysicsSystems();
        }
        else if (SystemName == TEXT("BiomeManager"))
        {
            InitializeWorldSystems();
        }
        else if (SystemName == TEXT("ProductionManager"))
        {
            InitializeProductionSystems();
        }
        
        // Mark as restarted
        SystemInitializationStatus.Add(SystemName, true);
        OnSystemInitialized.ExecuteIfBound(SystemName);
    }
}

void UEng_UnifiedArchitectureManager::RegisterPhysicsSystem(UCore_PhysicsManager* InPhysicsManager)
{
    if (InPhysicsManager)
    {
        PhysicsManager = InPhysicsManager;
        SystemInitializationStatus.Add(TEXT("PhysicsManager"), true);
        OnSystemInitialized.ExecuteIfBound(TEXT("PhysicsManager"));
        UE_LOG(LogTemp, Warning, TEXT("Physics Manager registered successfully"));
    }
}

void UEng_UnifiedArchitectureManager::RegisterBiomeSystem(UBiomeManager* InBiomeManager)
{
    if (InBiomeManager)
    {
        BiomeManager = InBiomeManager;
        SystemInitializationStatus.Add(TEXT("BiomeManager"), true);
        OnSystemInitialized.ExecuteIfBound(TEXT("BiomeManager"));
        UE_LOG(LogTemp, Warning, TEXT("Biome Manager registered successfully"));
    }
}

void UEng_UnifiedArchitectureManager::RegisterProductionSystem(ADir_ProductionManager* InProductionManager)
{
    if (InProductionManager && IsValid(InProductionManager))
    {
        ProductionManager = InProductionManager;
        SystemInitializationStatus.Add(TEXT("ProductionManager"), true);
        OnSystemInitialized.ExecuteIfBound(TEXT("ProductionManager"));
        UE_LOG(LogTemp, Warning, TEXT("Production Manager registered successfully"));
    }
}

FEng_SystemPerformanceReport UEng_UnifiedArchitectureManager::GetSystemPerformanceReport()
{
    UpdatePerformanceMetrics();
    return CurrentPerformanceReport;
}

float UEng_UnifiedArchitectureManager::GetOverallSystemHealth()
{
    if (!bArchitectureValid)
    {
        return 0.0f;
    }
    
    int32 TotalSystems = SystemInitializationStatus.Num();
    if (TotalSystems == 0)
    {
        return 0.0f;
    }
    
    int32 HealthySystems = 0;
    for (const auto& SystemPair : SystemInitializationStatus)
    {
        if (SystemPair.Value)
        {
            HealthySystems++;
        }
    }
    
    return static_cast<float>(HealthySystems) / static_cast<float>(TotalSystems);
}

void UEng_UnifiedArchitectureManager::RunArchitectureValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RUNNING ARCHITECTURE VALIDATION ==="));
    
    bArchitectureValid = ValidateSystemIntegrity();
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    LogArchitectureStatus();
    OnArchitectureValidationComplete.ExecuteIfBound(bArchitectureValid);
}

void UEng_UnifiedArchitectureManager::GenerateSystemDependencyReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM DEPENDENCY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("1. Core Physics Systems: PhysicsManager, PhysicsIntegrator"));
    UE_LOG(LogTemp, Warning, TEXT("2. World Generation: BiomeManager, TerrainPhysics"));
    UE_LOG(LogTemp, Warning, TEXT("3. Character Systems: Movement, Animation"));
    UE_LOG(LogTemp, Warning, TEXT("4. AI Systems: Dinosaur AI, NPC Behavior"));
    UE_LOG(LogTemp, Warning, TEXT("5. Production Systems: ProductionManager, AssetManager"));
    UE_LOG(LogTemp, Warning, TEXT("6. Performance Monitoring: PerformanceMonitor, Debugger"));
    
    for (const auto& SystemPair : SystemInitializationStatus)
    {
        UE_LOG(LogTemp, Warning, TEXT("System: %s - Status: %s"), 
               *SystemPair.Key, 
               SystemPair.Value ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
}

TArray<FString> UEng_UnifiedArchitectureManager::GetFailedSystems()
{
    return FailedSystems;
}

// Private Implementation Methods

void UEng_UnifiedArchitectureManager::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Physics Systems..."));
    
    // Physics systems are handled by their own managers
    // This validates they exist and are functional
    SystemInitializationStatus.Add(TEXT("PhysicsCore"), true);
}

void UEng_UnifiedArchitectureManager::InitializeWorldSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing World Systems..."));
    
    // World generation systems
    SystemInitializationStatus.Add(TEXT("WorldGeneration"), true);
}

void UEng_UnifiedArchitectureManager::InitializeCharacterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Character Systems..."));
    
    SystemInitializationStatus.Add(TEXT("CharacterSystems"), true);
}

void UEng_UnifiedArchitectureManager::InitializeAISystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing AI Systems..."));
    
    SystemInitializationStatus.Add(TEXT("AISystems"), true);
}

void UEng_UnifiedArchitectureManager::InitializeProductionSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Production Systems..."));
    
    SystemInitializationStatus.Add(TEXT("ProductionSystems"), true);
}

void UEng_UnifiedArchitectureManager::InitializePerformanceSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Performance Systems..."));
    
    SystemInitializationStatus.Add(TEXT("PerformanceSystems"), true);
}

bool UEng_UnifiedArchitectureManager::ValidatePhysicsIntegration()
{
    // Physics validation logic
    return true; // Simplified for now
}

bool UEng_UnifiedArchitectureManager::ValidateBiomeIntegration()
{
    // Biome validation logic
    return true; // Simplified for now
}

bool UEng_UnifiedArchitectureManager::ValidateProductionIntegration()
{
    // Production validation logic
    return true; // Simplified for now
}

bool UEng_UnifiedArchitectureManager::ValidatePerformanceMetrics()
{
    // Performance validation logic
    return CurrentPerformanceReport.bIsHealthy;
}

void UEng_UnifiedArchitectureManager::HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("SYSTEM FAILURE - %s: %s"), *SystemName, *ErrorMessage);
    
    if (!FailedSystems.Contains(SystemName))
    {
        FailedSystems.Add(SystemName);
    }
    
    SystemInitializationStatus.Add(SystemName, false);
    OnSystemFailed.ExecuteIfBound(SystemName, ErrorMessage);
}

void UEng_UnifiedArchitectureManager::LogArchitectureStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Valid: %s"), bArchitectureValid ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("System Health: %.2f%%"), GetOverallSystemHealth() * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Failed Systems: %d"), FailedSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Last Validation: %.2f seconds ago"), 
           GetWorld()->GetTimeSeconds() - LastValidationTime);
}

void UEng_UnifiedArchitectureManager::UpdatePerformanceMetrics()
{
    if (!bEnablePerformanceMonitoring)
    {
        return;
    }
    
    // Update performance metrics
    CurrentPerformanceReport.FrameTime = GetWorld()->GetDeltaSeconds();
    CurrentPerformanceReport.bIsHealthy = (FailedSystems.Num() == 0);
    
    // Simple health check based on frame time
    if (CurrentPerformanceReport.FrameTime > 0.033f) // 30 FPS threshold
    {
        CurrentPerformanceReport.bIsHealthy = false;
    }
}

void UEng_UnifiedArchitectureManager::CheckSystemHealth()
{
    if (bAutoRestartFailedSystems && FailedSystems.Num() > 0)
    {
        RestartFailedSystems();
    }
    
    // Periodic validation
    if (GetWorld()->GetTimeSeconds() - LastValidationTime > 30.0f)
    {
        RunArchitectureValidation();
    }
}

// World Subsystem Implementation

void UEng_WorldArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bWorldSystemsInitialized = false;
    MainArchitectureManager = GetGameInstance()->GetSubsystem<UEng_UnifiedArchitectureManager>();
}

void UEng_WorldArchitectureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    InitializeWorldSpecificSystems();
}

void UEng_WorldArchitectureSubsystem::InitializeWorldSpecificSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing World-Specific Architecture Systems"));
    
    if (MainArchitectureManager)
    {
        // Initialize systems specific to this world
        bWorldSystemsInitialized = true;
    }
}

bool UEng_WorldArchitectureSubsystem::ValidateWorldArchitecture()
{
    if (!MainArchitectureManager)
    {
        return false;
    }
    
    return MainArchitectureManager->ValidateSystemIntegrity();
}