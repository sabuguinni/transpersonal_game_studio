#include "BuildSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildSystem, Log, All);

AEng_BuildSystemManager::AEng_BuildSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second
    
    // Initialize build system parameters
    MaxConcurrentBuilds = 10;
    BuildQueueSize = 50;
    bIsSystemActive = true;
    bEnableDebugLogging = true;
    
    // Initialize build performance metrics
    TotalBuildsCompleted = 0;
    AverageBuildTime = 0.0f;
    LastBuildTime = 0.0f;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BuildSystemRoot"));
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build System Manager initialized"));
}

void AEng_BuildSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize build system on game start
    InitializeBuildSystem();
    
    // Start build queue processing
    StartBuildQueueProcessor();
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build System Manager started"));
}

void AEng_BuildSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsSystemActive)
    {
        // Process pending builds
        ProcessBuildQueue(DeltaTime);
        
        // Update performance metrics
        UpdatePerformanceMetrics(DeltaTime);
        
        // Check for system health
        MonitorSystemHealth();
    }
}

void AEng_BuildSystemManager::InitializeBuildSystem()
{
    // Clear any existing build queue
    BuildQueue.Empty();
    ActiveBuilds.Empty();
    
    // Initialize build templates
    InitializeBuildTemplates();
    
    // Set up build validation rules
    SetupBuildValidation();
    
    // Initialize resource management
    InitializeResourceManagement();
    
    bIsSystemActive = true;
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build System initialized successfully"));
}

void AEng_BuildSystemManager::StartBuildQueueProcessor()
{
    if (!GetWorld())
    {
        UE_LOG(LogBuildSystem, Error, TEXT("Cannot start build queue processor - no valid world"));
        return;
    }
    
    // Start the build processing timer
    GetWorld()->GetTimerManager().SetTimer(
        BuildProcessorTimer,
        this,
        &AEng_BuildSystemManager::ProcessNextBuild,
        0.1f, // Process every 100ms
        true  // Loop
    );
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build queue processor started"));
}

bool AEng_BuildSystemManager::QueueBuild(const FEng_BuildRequest& BuildRequest)
{
    if (BuildQueue.Num() >= BuildQueueSize)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Build queue full - cannot queue new build"));
        return false;
    }
    
    // Validate build request
    if (!ValidateBuildRequest(BuildRequest))
    {
        UE_LOG(LogBuildSystem, Error, TEXT("Invalid build request"));
        return false;
    }
    
    // Add to queue
    BuildQueue.Add(BuildRequest);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogBuildSystem, Log, TEXT("Build queued: %s"), *BuildRequest.BuildName);
    }
    
    return true;
}

void AEng_BuildSystemManager::ProcessBuildQueue(float DeltaTime)
{
    // Remove completed builds from active list
    for (int32 i = ActiveBuilds.Num() - 1; i >= 0; i--)
    {
        if (ActiveBuilds[i].bIsCompleted)
        {
            OnBuildCompleted(ActiveBuilds[i]);
            ActiveBuilds.RemoveAt(i);
        }
    }
    
    // Start new builds if we have capacity
    while (ActiveBuilds.Num() < MaxConcurrentBuilds && BuildQueue.Num() > 0)
    {
        FEng_BuildRequest NextBuild = BuildQueue[0];
        BuildQueue.RemoveAt(0);
        
        StartBuild(NextBuild);
    }
}

void AEng_BuildSystemManager::ProcessNextBuild()
{
    if (BuildQueue.Num() == 0 || ActiveBuilds.Num() >= MaxConcurrentBuilds)
    {
        return;
    }
    
    FEng_BuildRequest NextBuild = BuildQueue[0];
    BuildQueue.RemoveAt(0);
    
    StartBuild(NextBuild);
}

void AEng_BuildSystemManager::StartBuild(const FEng_BuildRequest& BuildRequest)
{
    FEng_ActiveBuild NewBuild;
    NewBuild.BuildRequest = BuildRequest;
    NewBuild.StartTime = GetWorld()->GetTimeSeconds();
    NewBuild.bIsCompleted = false;
    NewBuild.BuildProgress = 0.0f;
    
    // Add to active builds
    ActiveBuilds.Add(NewBuild);
    
    // Start the actual build process
    ExecuteBuild(NewBuild);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogBuildSystem, Log, TEXT("Started build: %s"), *BuildRequest.BuildName);
    }
}

void AEng_BuildSystemManager::ExecuteBuild(FEng_ActiveBuild& Build)
{
    // This is where the actual build logic would go
    // For now, simulate a build process
    
    float BuildDuration = FMath::RandRange(1.0f, 5.0f); // Random build time
    
    if (GetWorld())
    {
        FTimerHandle BuildTimer;
        GetWorld()->GetTimerManager().SetTimer(
            BuildTimer,
            [this, &Build]()
            {
                CompleteBuild(Build);
            },
            BuildDuration,
            false
        );
    }
}

void AEng_BuildSystemManager::CompleteBuild(FEng_ActiveBuild& Build)
{
    Build.bIsCompleted = true;
    Build.CompletionTime = GetWorld()->GetTimeSeconds();
    Build.BuildProgress = 1.0f;
    
    // Update metrics
    TotalBuildsCompleted++;
    LastBuildTime = Build.CompletionTime - Build.StartTime;
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogBuildSystem, Log, TEXT("Completed build: %s (%.2fs)"), 
               *Build.BuildRequest.BuildName, LastBuildTime);
    }
}

void AEng_BuildSystemManager::OnBuildCompleted(const FEng_ActiveBuild& CompletedBuild)
{
    // Handle build completion
    if (OnBuildCompletedDelegate.IsBound())
    {
        OnBuildCompletedDelegate.Broadcast(CompletedBuild.BuildRequest);
    }
    
    // Update performance metrics
    UpdateBuildMetrics(CompletedBuild);
}

bool AEng_BuildSystemManager::ValidateBuildRequest(const FEng_BuildRequest& BuildRequest) const
{
    // Basic validation
    if (BuildRequest.BuildName.IsEmpty())
    {
        return false;
    }
    
    if (BuildRequest.BuildType == EEng_BuildType::None)
    {
        return false;
    }
    
    // Add more validation rules as needed
    return true;
}

void AEng_BuildSystemManager::InitializeBuildTemplates()
{
    // Initialize common build templates
    BuildTemplates.Empty();
    
    // Add default templates for different build types
    // This would be expanded based on actual build requirements
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build templates initialized"));
}

void AEng_BuildSystemManager::SetupBuildValidation()
{
    // Set up validation rules for different build types
    // This ensures builds meet quality and performance standards
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build validation rules configured"));
}

void AEng_BuildSystemManager::InitializeResourceManagement()
{
    // Initialize resource tracking for builds
    // This manages memory, CPU, and other resources during builds
    
    UE_LOG(LogBuildSystem, Log, TEXT("Resource management initialized"));
}

void AEng_BuildSystemManager::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update average build time
    if (TotalBuildsCompleted > 0)
    {
        // Simple moving average calculation
        float TotalTime = AverageBuildTime * (TotalBuildsCompleted - 1) + LastBuildTime;
        AverageBuildTime = TotalTime / TotalBuildsCompleted;
    }
}

void AEng_BuildSystemManager::UpdateBuildMetrics(const FEng_ActiveBuild& CompletedBuild)
{
    // Update detailed metrics for completed builds
    float BuildDuration = CompletedBuild.CompletionTime - CompletedBuild.StartTime;
    
    // Track performance by build type
    // This data can be used for optimization
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogBuildSystem, Log, TEXT("Build metrics updated for: %s"), 
               *CompletedBuild.BuildRequest.BuildName);
    }
}

void AEng_BuildSystemManager::MonitorSystemHealth()
{
    // Monitor system health and performance
    if (BuildQueue.Num() > BuildQueueSize * 0.8f)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Build queue approaching capacity: %d/%d"), 
               BuildQueue.Num(), BuildQueueSize);
    }
    
    if (ActiveBuilds.Num() == MaxConcurrentBuilds)
    {
        UE_LOG(LogBuildSystem, Log, TEXT("All build slots in use: %d/%d"), 
               ActiveBuilds.Num(), MaxConcurrentBuilds);
    }
}

void AEng_BuildSystemManager::StopBuildSystem()
{
    bIsSystemActive = false;
    
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BuildProcessorTimer);
    }
    
    // Clear queues
    BuildQueue.Empty();
    ActiveBuilds.Empty();
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build System stopped"));
}

int32 AEng_BuildSystemManager::GetQueueSize() const
{
    return BuildQueue.Num();
}

int32 AEng_BuildSystemManager::GetActiveBuildsCount() const
{
    return ActiveBuilds.Num();
}

float AEng_BuildSystemManager::GetAverageBuildTime() const
{
    return AverageBuildTime;
}

int32 AEng_BuildSystemManager::GetTotalBuildsCompleted() const
{
    return TotalBuildsCompleted;
}