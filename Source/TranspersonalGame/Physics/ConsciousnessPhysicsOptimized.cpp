/**
 * @file ConsciousnessPhysicsOptimized.cpp
 * @brief Implementation of optimized consciousness physics system
 * @author Performance Optimizer
 * @version 1.0
 */

#include "ConsciousnessPhysicsOptimized.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Async/Async.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

// Static instance for profiler
UConsciousnessPhysicsProfiler* UConsciousnessPhysicsProfiler::Instance = nullptr;

UConsciousnessPhysicsOptimized::UConsciousnessPhysicsOptimized()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize frame time history
    FrameTimeHistory.SetNum(FrameHistorySize);
    for (int32 i = 0; i < FrameHistorySize; i++)
    {
        FrameTimeHistory[i] = TargetFrameTime;
    }
}

void UConsciousnessPhysicsOptimized::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeThreading();
    
    // Initialize profiler
    UConsciousnessPhysicsProfiler::GetInstance();
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Optimizer initialized with %d worker threads"), 
           bUseMultithreading ? MaxWorkerThreads : 0);
}

void UConsciousnessPhysicsOptimized::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownThreading();
    Super::EndPlay(EndPlayReason);
}

void UConsciousnessPhysicsOptimized::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    SCOPE_CYCLE_COUNTER(STAT_ConsciousnessPhysicsOptimized);
    
    UConsciousnessPhysicsProfiler::GetInstance()->BeginFrame();
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Update LOD system periodically
    UpdateLODSystem();
    
    // Adjust quality based on performance if adaptive quality is enabled
    if (bAdaptiveQualityEnabled)
    {
        AdjustQualityBasedOnPerformance();
    }
    
    // Optimize field updates based on current settings
    OptimizeFieldUpdates();
    
    // Call parent implementation with optimizations
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UConsciousnessPhysicsProfiler::GetInstance()->EndFrame();
}

void UConsciousnessPhysicsOptimized::SetOptimizationLevel(EConsciousnessOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    
    switch (OptimizationLevel)
    {
        case EConsciousnessOptimizationLevel::High:
            MaxFieldUpdatesPerFrame = 200;
            LODDistance = 5000.0f;
            bUseMultithreading = true;
            break;
            
        case EConsciousnessOptimizationLevel::Medium:
            MaxFieldUpdatesPerFrame = 100;
            LODDistance = 2000.0f;
            bUseMultithreading = true;
            break;
            
        case EConsciousnessOptimizationLevel::Low:
            MaxFieldUpdatesPerFrame = 50;
            LODDistance = 1000.0f;
            bUseMultithreading = false;
            break;
            
        case EConsciousnessOptimizationLevel::Adaptive:
            bAdaptiveQualityEnabled = true;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics optimization level set to: %d"), (int32)OptimizationLevel);
}

void UConsciousnessPhysicsOptimized::OptimizeForCurrentFramerate()
{
    float CurrentFPS = 1.0f / AverageUpdateTime;
    
    if (CurrentFPS < 30.0f)
    {
        SetOptimizationLevel(EConsciousnessOptimizationLevel::Low);
    }
    else if (CurrentFPS < 45.0f)
    {
        SetOptimizationLevel(EConsciousnessOptimizationLevel::Medium);
    }
    else
    {
        SetOptimizationLevel(EConsciousnessOptimizationLevel::High);
    }
}

void UConsciousnessPhysicsOptimized::EnableAdaptiveQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    
    if (bEnable)
    {
        OptimizationLevel = EConsciousnessOptimizationLevel::Adaptive;
    }
}

float UConsciousnessPhysicsOptimized::GetPerformanceScore() const
{
    // Calculate performance score based on frame time vs target
    float FrameTimeRatio = AverageUpdateTime / TargetFrameTime;
    float Score = FMath::Clamp(2.0f - FrameTimeRatio, 0.0f, 1.0f);
    return Score;
}

void UConsciousnessPhysicsOptimized::UpdatePerformanceMetrics(float DeltaTime)
{
    CurrentFrameTime = DeltaTime;
    
    // Update frame time history
    FrameTimeHistory[FrameHistoryIndex] = DeltaTime;
    FrameHistoryIndex = (FrameHistoryIndex + 1) % FrameHistorySize;
    
    // Calculate average frame time
    float TotalTime = 0.0f;
    for (float Time : FrameTimeHistory)
    {
        TotalTime += Time;
    }
    AverageUpdateTime = TotalTime / FrameHistorySize;
}

void UConsciousnessPhysicsOptimized::UpdateLODSystem()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLODUpdateTime < LODUpdateInterval)
    {
        return;
    }
    
    LastLODUpdateTime = CurrentTime;
    
    // Get player location for distance calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update distances for all consciousness fields
    FieldDistances.Empty();
    
    // Find all consciousness field components in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (UConsciousnessFieldComponent* FieldComponent = Actor->FindComponentByClass<UConsciousnessFieldComponent>())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            FieldDistances.Add(FieldComponent, Distance);
        }
    }
    
    ActiveFieldCount = FieldDistances.Num();
}

void UConsciousnessPhysicsOptimized::AdjustQualityBasedOnPerformance()
{
    if (QualityAdjustmentCooldown > 0.0f)
    {
        QualityAdjustmentCooldown -= GetWorld()->GetDeltaSeconds();
        return;
    }
    
    float PerformanceScore = GetPerformanceScore();
    
    // Adjust settings based on performance
    if (PerformanceScore < 0.3f) // Poor performance
    {
        MaxFieldUpdatesPerFrame = FMath::Max(25, MaxFieldUpdatesPerFrame - 10);
        LODDistance = FMath::Max(500.0f, LODDistance - 200.0f);
        QualityAdjustmentCooldown = QualityAdjustmentInterval;
        
        UE_LOG(LogTemp, Warning, TEXT("Consciousness Physics: Reducing quality due to poor performance (Score: %.2f)"), PerformanceScore);
    }
    else if (PerformanceScore > 0.8f) // Good performance
    {
        MaxFieldUpdatesPerFrame = FMath::Min(200, MaxFieldUpdatesPerFrame + 5);
        LODDistance = FMath::Min(5000.0f, LODDistance + 100.0f);
        QualityAdjustmentCooldown = QualityAdjustmentInterval;
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness Physics: Increasing quality due to good performance (Score: %.2f)"), PerformanceScore);
    }
}

void UConsciousnessPhysicsOptimized::OptimizeFieldUpdates()
{
    int32 UpdatesThisFrame = 0;
    
    // Sort fields by distance and importance
    TArray<TPair<UConsciousnessFieldComponent*, float>> SortedFields;
    for (const auto& FieldPair : FieldDistances)
    {
        SortedFields.Add(TPair<UConsciousnessFieldComponent*, float>(FieldPair.Key, FieldPair.Value));
    }
    
    // Sort by distance (closest first)
    SortedFields.Sort([](const TPair<UConsciousnessFieldComponent*, float>& A, const TPair<UConsciousnessFieldComponent*, float>& B)
    {
        return A.Value < B.Value;
    });
    
    // Update fields within budget
    for (const auto& FieldPair : SortedFields)
    {
        if (UpdatesThisFrame >= MaxFieldUpdatesPerFrame)
        {
            break;
        }
        
        UConsciousnessFieldComponent* Field = FieldPair.Key;
        float Distance = FieldPair.Value;
        
        if (ShouldUpdateField(Field, Distance))
        {
            float StartTime = FPlatformTime::Seconds();
            UpdateFieldWithLOD(Field, Distance, GetWorld()->GetDeltaSeconds());
            float UpdateTime = FPlatformTime::Seconds() - StartTime;
            
            UConsciousnessPhysicsProfiler::GetInstance()->RecordFieldUpdate(UpdateTime);
            UpdatesThisFrame++;
        }
    }
}

void UConsciousnessPhysicsOptimized::InitializeThreading()
{
    if (!bUseMultithreading)
    {
        return;
    }
    
    bThreadingEnabled = true;
    WorkerTasks.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics: Threading initialized with %d worker threads"), MaxWorkerThreads);
}

void UConsciousnessPhysicsOptimized::ShutdownThreading()
{
    bThreadingEnabled = false;
    
    // Wait for all tasks to complete
    FScopeLock Lock(&TaskMutex);
    for (auto& Task : WorkerTasks)
    {
        if (Task.IsValid())
        {
            // Tasks will complete naturally
        }
    }
    WorkerTasks.Empty();
}

float UConsciousnessPhysicsOptimized::CalculateDistanceToPlayer(const FVector& Position) const
{
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return MAX_FLT;
    }
    
    return FVector::Dist(Position, PlayerPawn->GetActorLocation());
}

bool UConsciousnessPhysicsOptimized::ShouldUpdateField(UConsciousnessFieldComponent* Field, float Distance) const
{
    if (!Field || !IsValid(Field))
    {
        return false;
    }
    
    // Skip fields beyond LOD distance
    if (Distance > LODDistance)
    {
        return false;
    }
    
    // Reduce update frequency for distant fields
    if (Distance > LODDistance * 0.5f)
    {
        // Update every other frame for medium distance
        return (GFrameCounter % 2) == 0;
    }
    else if (Distance > LODDistance * 0.25f)
    {
        // Update every 3rd frame for far distance
        return (GFrameCounter % 3) == 0;
    }
    
    return true; // Update every frame for close fields
}

void UConsciousnessPhysicsOptimized::UpdateFieldWithLOD(UConsciousnessFieldComponent* Field, float Distance, float DeltaTime)
{
    if (!Field || !IsValid(Field))
    {
        return;
    }
    
    // Adjust update quality based on distance
    float QualityMultiplier = 1.0f;
    if (Distance > LODDistance * 0.5f)
    {
        QualityMultiplier = 0.5f; // Reduce quality for distant fields
    }
    else if (Distance > LODDistance * 0.25f)
    {
        QualityMultiplier = 0.75f; // Slightly reduce quality for medium distance
    }
    
    // Apply LOD-adjusted update (this would call into the field's update method with quality settings)
    // Field->UpdateWithQuality(DeltaTime, QualityMultiplier);
}

// Async Task Implementation
FConsciousnessPhysicsTask::FConsciousnessPhysicsTask(TWeakObjectPtr<UConsciousnessPhysicsOptimized> InComponent,
                                                   TArray<TWeakObjectPtr<UConsciousnessFieldComponent>> InFields,
                                                   float InDeltaTime)
    : Component(InComponent)
    , Fields(InFields)
    , DeltaTime(InDeltaTime)
{
}

void FConsciousnessPhysicsTask::DoWork()
{
    if (!Component.IsValid())
    {
        return;
    }
    
    // Process consciousness field calculations in parallel
    for (const auto& WeakField : Fields)
    {
        if (UConsciousnessFieldComponent* Field = WeakField.Get())
        {
            // Perform consciousness field calculations
            // This would contain the actual physics calculations
            float StartTime = FPlatformTime::Seconds();
            
            // Simulate consciousness field update work
            // Field->CalculateResonance(DeltaTime);
            // Field->UpdateFieldStrength(DeltaTime);
            
            float CalculationTime = FPlatformTime::Seconds() - StartTime;
            UConsciousnessPhysicsProfiler::GetInstance()->RecordResonanceCalculation(CalculationTime);
        }
    }
}

// Profiler Implementation
UConsciousnessPhysicsProfiler* UConsciousnessPhysicsProfiler::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<UConsciousnessPhysicsProfiler>();
        Instance->AddToRoot(); // Prevent garbage collection
        Instance->FrameTimeHistory.SetNum(60); // 1 second of history at 60 FPS
        Instance->FieldUpdateTimeHistory.SetNum(100);
    }
    return Instance;
}

void UConsciousnessPhysicsProfiler::BeginFrame()
{
    FrameStartTime = FPlatformTime::Seconds();
    FieldUpdatesThisFrame = 0;
}

void UConsciousnessPhysicsProfiler::EndFrame()
{
    float FrameTime = FPlatformTime::Seconds() - FrameStartTime;
    
    // Add to history
    FrameTimeHistory.Add(FrameTime);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    UpdateAverages();
}

void UConsciousnessPhysicsProfiler::RecordFieldUpdate(float UpdateTime)
{
    FieldUpdatesThisFrame++;
    
    FieldUpdateTimeHistory.Add(UpdateTime);
    if (FieldUpdateTimeHistory.Num() > 100)
    {
        FieldUpdateTimeHistory.RemoveAt(0);
    }
}

void UConsciousnessPhysicsProfiler::RecordResonanceCalculation(float CalculationTime)
{
    // Record resonance calculation performance
    // This could be used for more detailed profiling
}

void UConsciousnessPhysicsProfiler::UpdateAverages()
{
    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float Total = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            Total += Time;
        }
        AverageFrameTime = Total / FrameTimeHistory.Num();
    }
    
    // Calculate average field update time
    if (FieldUpdateTimeHistory.Num() > 0)
    {
        float Total = 0.0f;
        for (float Time : FieldUpdateTimeHistory)
        {
            Total += Time;
        }
        AverageFieldUpdateTime = Total / FieldUpdateTimeHistory.Num();
    }
}