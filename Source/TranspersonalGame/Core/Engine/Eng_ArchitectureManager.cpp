#include "Eng_ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AEng_ArchitectureManager::AEng_ArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create architecture visualization mesh
    ArchitectureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArchitectureMesh"));
    ArchitectureMesh->SetupAttachment(RootComponent);

    // Initialize system state
    bIsSystemInitialized = false;
    SystemLoadTime = 0.0f;
    ActiveModuleCount = 0;
    CurrentFrameRate = 0.0f;
    MemoryUsageMB = 0.0f;
    SystemState = EEng_SystemState::Initializing;

    // Performance monitoring settings
    LastPerformanceUpdate = 0.0f;
    bPerformanceMonitoringEnabled = true;

    // Setup default mesh
    SetupArchitectureVisualization();
}

void AEng_ArchitectureManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize the architecture system
    InitializeArchitecture();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager: System started"));
}

void AEng_ArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update performance metrics every second
    LastPerformanceUpdate += DeltaTime;
    if (LastPerformanceUpdate >= 1.0f)
    {
        UpdatePerformanceMetrics();
        LastPerformanceUpdate = 0.0f;
    }
}

void AEng_ArchitectureManager::InitializeArchitecture()
{
    if (bIsSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture already initialized"));
        return;
    }

    float StartTime = GetWorld()->GetTimeSeconds();

    // Initialize core systems
    RegisteredModules.Empty();
    
    // Register core modules
    RegisterModule(TEXT("Core"));
    RegisterModule(TEXT("Engine"));
    RegisterModule(TEXT("Physics"));
    RegisterModule(TEXT("Rendering"));

    // Initialize performance monitoring
    InitializePerformanceMonitoring();

    // Validate module dependencies
    ValidateModuleDependencies();

    // Calculate initialization time
    SystemLoadTime = GetWorld()->GetTimeSeconds() - StartTime;
    bIsSystemInitialized = true;
    SystemState = EEng_SystemState::Running;

    UE_LOG(LogTemp, Warning, TEXT("Architecture initialized in %.3f seconds"), SystemLoadTime);
}

void AEng_ArchitectureManager::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating system integrity..."));

    // Check registered modules
    bool bAllModulesValid = true;
    for (const FString& ModuleName : RegisteredModules)
    {
        // Basic module validation
        if (ModuleName.IsEmpty())
        {
            bAllModulesValid = false;
            UE_LOG(LogTemp, Error, TEXT("Invalid module found: empty name"));
        }
    }

    // Check system state
    if (SystemState == EEng_SystemState::Error)
    {
        bAllModulesValid = false;
        UE_LOG(LogTemp, Error, TEXT("System is in error state"));
    }

    // Update system state based on validation
    if (bAllModulesValid)
    {
        SetSystemState(EEng_SystemState::Running);
        UE_LOG(LogTemp, Warning, TEXT("System integrity validation: PASSED"));
    }
    else
    {
        SetSystemState(EEng_SystemState::Error);
        UE_LOG(LogTemp, Error, TEXT("System integrity validation: FAILED"));
    }
}

void AEng_ArchitectureManager::RegisterModule(const FString& ModuleName)
{
    if (ModuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register module with empty name"));
        return;
    }

    if (RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Module %s is already registered"), *ModuleName);
        return;
    }

    RegisteredModules.Add(ModuleName);
    ActiveModuleCount = RegisteredModules.Num();

    UE_LOG(LogTemp, Warning, TEXT("Registered module: %s (Total: %d)"), *ModuleName, ActiveModuleCount);
}

void AEng_ArchitectureManager::UnregisterModule(const FString& ModuleName)
{
    if (RegisteredModules.Remove(ModuleName) > 0)
    {
        ActiveModuleCount = RegisteredModules.Num();
        UE_LOG(LogTemp, Warning, TEXT("Unregistered module: %s (Remaining: %d)"), *ModuleName, ActiveModuleCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Module %s was not registered"), *ModuleName);
    }
}

void AEng_ArchitectureManager::UpdatePerformanceMetrics()
{
    if (!bPerformanceMonitoringEnabled)
        return;

    // Get current frame rate
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    }

    // Estimate memory usage (simplified)
    MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);

    // Log performance data periodically
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLogTime > 10.0f) // Log every 10 seconds
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance: FPS=%.1f, Memory=%.1fMB, Modules=%d"), 
               CurrentFrameRate, MemoryUsageMB, ActiveModuleCount);
        LastLogTime = CurrentTime;
    }
}

float AEng_ArchitectureManager::GetSystemLoadPercentage() const
{
    // Simple load calculation based on frame rate and module count
    float TargetFPS = 60.0f;
    float FPSLoad = FMath::Clamp(1.0f - (CurrentFrameRate / TargetFPS), 0.0f, 1.0f);
    float ModuleLoad = FMath::Clamp(ActiveModuleCount / 20.0f, 0.0f, 1.0f); // Assume 20 is max modules
    
    return (FPSLoad + ModuleLoad) * 50.0f; // Convert to percentage
}

void AEng_ArchitectureManager::SetSystemState(EEng_SystemState NewState)
{
    if (SystemState != NewState)
    {
        EEng_SystemState OldState = SystemState;
        SystemState = NewState;
        
        UE_LOG(LogTemp, Warning, TEXT("System state changed: %d -> %d"), 
               (int32)OldState, (int32)NewState);
    }
}

EEng_SystemState AEng_ArchitectureManager::GetSystemState() const
{
    return SystemState;
}

void AEng_ArchitectureManager::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bIsSystemInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Load Time: %.3f seconds"), SystemLoadTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Modules: %d"), ActiveModuleCount);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f FPS"), CurrentFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("System State: %d"), (int32)SystemState);
    UE_LOG(LogTemp, Warning, TEXT("System Load: %.1f%%"), GetSystemLoadPercentage());
    
    UE_LOG(LogTemp, Warning, TEXT("Registered Modules:"));
    for (const FString& ModuleName : RegisteredModules)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s"), *ModuleName);
    }
}

void AEng_ArchitectureManager::RunArchitectureValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Running architecture validation..."));
    
    ValidateSystemIntegrity();
    LogSystemStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete"));
}

void AEng_ArchitectureManager::SetupArchitectureVisualization()
{
    // Try to set a default cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded() && ArchitectureMesh)
    {
        ArchitectureMesh->SetStaticMesh(CubeMeshAsset.Object);
        ArchitectureMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
        
        // Set a distinctive color for identification
        ArchitectureMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(0.0f, 1.0f, 1.0f));
    }
}

void AEng_ArchitectureManager::InitializePerformanceMonitoring()
{
    bPerformanceMonitoringEnabled = true;
    LastPerformanceUpdate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring initialized"));
}

void AEng_ArchitectureManager::ValidateModuleDependencies()
{
    // Basic dependency validation
    TArray<FString> RequiredModules = {TEXT("Core"), TEXT("Engine")};
    
    for (const FString& RequiredModule : RequiredModules)
    {
        if (!RegisteredModules.Contains(RequiredModule))
        {
            UE_LOG(LogTemp, Error, TEXT("Missing required module: %s"), *RequiredModule);
            SetSystemState(EEng_SystemState::Error);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Module dependencies validated successfully"));
}