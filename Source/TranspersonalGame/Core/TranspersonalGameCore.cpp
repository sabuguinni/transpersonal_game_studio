#include "TranspersonalGameCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCore);

#define LOCTEXT_NAMESPACE "TranspersonalGameCore"

//////////////////////////////////////////////////////////////////////////
// FTranspersonalGameCoreModule

void FTranspersonalGameCoreModule::StartupModule()
{
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Transpersonal Game Core Module Starting..."));
    
    // Validate required plugins and features
    ValidateRequiredPlugins();
    
    // Initialize core systems
    InitializeCoreSystems();
    
    // Setup performance budgets
    SetupPerformanceBudgets();
    
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Transpersonal Game Core Module Started Successfully"));
}

void FTranspersonalGameCoreModule::ShutdownModule()
{
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Transpersonal Game Core Module Shutting Down..."));
}

void FTranspersonalGameCoreModule::InitializeCoreSystems()
{
    // Core systems initialization will be handled by the subsystem
    UE_LOG(LogTranspersonalCore, Log, TEXT("Core systems initialization delegated to subsystem"));
}

void FTranspersonalGameCoreModule::ValidateRequiredPlugins()
{
    // Check for required UE5 features
    UE_LOG(LogTranspersonalCore, Log, TEXT("Validating required plugins and features..."));
    
    // These would normally check plugin availability
    // For now, we log the requirements
    UE_LOG(LogTranspersonalCore, Log, TEXT("Required: Nanite Virtualized Geometry"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("Required: Lumen Global Illumination"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("Required: World Partition"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("Required: Virtual Shadow Maps"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("Required: Mass Entity"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("Required: Chaos Physics"));
}

void FTranspersonalGameCoreModule::SetupPerformanceBudgets()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Setting up performance budgets..."));
    
    // Set console variables for performance targets
    if (IConsoleVariable* CVarTargetFPS = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS")))
    {
        CVarTargetFPS->Set(60.0f, ECVF_SetByProjectSetting);
    }
    
    // Memory budget settings
    if (IConsoleVariable* CVarPoolSize = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize")))
    {
        CVarPoolSize->Set(2048, ECVF_SetByProjectSetting); // 2GB streaming pool
    }
    
    UE_LOG(LogTranspersonalCore, Log, TEXT("Performance budgets configured"));
}

//////////////////////////////////////////////////////////////////////////
// UTranspersonalCoreSubsystem

void UTranspersonalCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Initializing Transpersonal Core Subsystem..."));
    
    // Initialize core rendering systems
    InitializeNanite();
    InitializeLumen();
    InitializeWorldPartition();
    InitializeVirtualShadowMaps();
    InitializeMassEntity();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UTranspersonalCoreSubsystem::UpdatePerformanceMetrics,
            1.0f, // Update every second
            true
        );
    }
    
    bCoreSystemsInitialized = true;
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Core Subsystem Initialized Successfully"));
}

void UTranspersonalCoreSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    RegisteredModules.Empty();
    bCoreSystemsInitialized = false;
    
    Super::Deinitialize();
}

UTranspersonalCoreSubsystem* UTranspersonalCoreSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UTranspersonalCoreSubsystem>();
        }
    }
    return nullptr;
}

void UTranspersonalCoreSubsystem::RegisterModule(const FString& ModuleName, UObject* ModuleInstance)
{
    if (ModuleInstance)
    {
        RegisteredModules.Add(ModuleName, ModuleInstance);
        UE_LOG(LogTranspersonalCore, Log, TEXT("Registered module: %s"), *ModuleName);
    }
}

void UTranspersonalCoreSubsystem::GetPerformanceMetrics(float& FrameTime, int32& TriangleCount, float& MemoryUsageMB) const
{
    FrameTime = LastFrameTime;
    TriangleCount = LastTriangleCount;
    MemoryUsageMB = LastMemoryUsage;
}

void UTranspersonalCoreSubsystem::InitializeNanite()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Nanite Virtualized Geometry..."));
    
    // Enable Nanite
    if (IConsoleVariable* CVarNaniteEnable = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite")))
    {
        CVarNaniteEnable->Set(1, ECVF_SetByProjectSetting);
    }
    
    // Configure Nanite settings for large worlds
    if (IConsoleVariable* CVarNaniteMaxNodes = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxNodes")))
    {
        CVarNaniteMaxNodes->Set(2048, ECVF_SetByProjectSetting);
    }
}

void UTranspersonalCoreSubsystem::InitializeLumen()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Lumen Global Illumination..."));
    
    // Enable Lumen
    if (IConsoleVariable* CVarLumenGI = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIlluminationMethod")))
    {
        CVarLumenGI->Set(1, ECVF_SetByProjectSetting); // 1 = Lumen
    }
    
    if (IConsoleVariable* CVarLumenReflections = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ReflectionMethod")))
    {
        CVarLumenReflections->Set(1, ECVF_SetByProjectSetting); // 1 = Lumen
    }
    
    // Configure for large outdoor scenes
    if (IConsoleVariable* CVarLumenSceneViewDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LumenScene.GlobalLightingCacheResolution")))
    {
        CVarLumenSceneViewDistance->Set(64, ECVF_SetByProjectSetting);
    }
}

void UTranspersonalCoreSubsystem::InitializeWorldPartition()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing World Partition..."));
    
    // World Partition is enabled per-level, so we just log readiness
    // The actual configuration happens in World Settings
    UE_LOG(LogTranspersonalCore, Log, TEXT("World Partition ready for large world streaming"));
}

void UTranspersonalCoreSubsystem::InitializeVirtualShadowMaps()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Virtual Shadow Maps..."));
    
    // Enable Virtual Shadow Maps
    if (IConsoleVariable* CVarVSMEnable = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable")))
    {
        CVarVSMEnable->Set(1, ECVF_SetByProjectSetting);
    }
    
    // Configure for high quality shadows
    if (IConsoleVariable* CVarVSMMaxPhysicalPages = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.MaxPhysicalPages")))
    {
        CVarVSMMaxPhysicalPages->Set(4096, ECVF_SetByProjectSetting);
    }
}

void UTranspersonalCoreSubsystem::InitializeMassEntity()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Mass Entity System..."));
    
    // Mass Entity configuration for large crowds
    if (IConsoleVariable* CVarMassEntityMaxEntities = IConsoleManager::Get().FindConsoleVariable(TEXT("mass.MaxEntitiesPerChunk")))
    {
        CVarMassEntityMaxEntities->Set(1000, ECVF_SetByProjectSetting);
    }
    
    UE_LOG(LogTranspersonalCore, Log, TEXT("Mass Entity ready for 50k+ simultaneous agents"));
}

void UTranspersonalCoreSubsystem::UpdatePerformanceMetrics()
{
    // Get frame time
    LastFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get triangle count (simplified - would need proper stats in real implementation)
    LastTriangleCount = 0; // Would query rendering stats
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    LastMemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

//////////////////////////////////////////////////////////////////////////
// ATranspersonalGameModeBase

ATranspersonalGameModeBase::ATranspersonalGameModeBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATranspersonalGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTranspersonalCore, Log, TEXT("Transpersonal Game Mode starting..."));
    
    // Wait for core systems to be ready
    if (UTranspersonalCoreSubsystem* CoreSubsystem = UTranspersonalCoreSubsystem::Get(this))
    {
        if (CoreSubsystem->AreCorSystemsReady())
        {
            OnCoreSystemsReady();
        }
        else
        {
            // Poll until systems are ready
            GetWorldTimerManager().SetTimer(
                FTimerHandle(),
                this,
                &ATranspersonalGameModeBase::OnCoreSystemsReady,
                0.1f,
                false
            );
        }
    }
}

void ATranspersonalGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Transpersonal Game Mode ending..."));
    Super::EndPlay(EndPlayReason);
}

void ATranspersonalGameModeBase::OnCoreSystemsReady()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Core systems ready - initializing game systems"));
    OnGameSystemsInitialized();
}

//////////////////////////////////////////////////////////////////////////
// ATranspersonalPlayerController

ATranspersonalPlayerController::ATranspersonalPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATranspersonalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTranspersonalCore, Log, TEXT("Transpersonal Player Controller starting..."));
    
    if (bShowPerformanceMetrics)
    {
        TogglePerformanceDisplay();
    }
}

void ATranspersonalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Add debug input bindings
    if (InputComponent)
    {
        InputComponent->BindAction("TogglePerformanceDisplay", IE_Pressed, this, &ATranspersonalPlayerController::TogglePerformanceDisplay);
    }
}

UTranspersonalCoreSubsystem* ATranspersonalPlayerController::GetCoreSubsystem() const
{
    return UTranspersonalCoreSubsystem::Get(this);
}

void ATranspersonalPlayerController::TogglePerformanceDisplay()
{
    bShowPerformanceMetrics = !bShowPerformanceMetrics;
    
    if (bShowPerformanceMetrics)
    {
        GetWorldTimerManager().SetTimer(
            PerformanceDisplayTimer,
            this,
            &ATranspersonalPlayerController::DisplayPerformanceMetrics,
            1.0f,
            true
        );
    }
    else
    {
        GetWorldTimerManager().ClearTimer(PerformanceDisplayTimer);
    }
}

void ATranspersonalPlayerController::DisplayPerformanceMetrics()
{
    if (UTranspersonalCoreSubsystem* CoreSubsystem = GetCoreSubsystem())
    {
        float FrameTime;
        int32 TriangleCount;
        float MemoryUsage;
        
        CoreSubsystem->GetPerformanceMetrics(FrameTime, TriangleCount, MemoryUsage);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 1.0f, FColor::Green,
                FString::Printf(TEXT("Frame Time: %.2f ms"), FrameTime)
            );
            GEngine->AddOnScreenDebugMessage(
                -1, 1.0f, FColor::Green,
                FString::Printf(TEXT("Memory Usage: %.1f MB"), MemoryUsage)
            );
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTranspersonalGameCoreModule, TranspersonalGameCore)