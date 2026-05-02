#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogProductionManager, Log, All);

UDir_ProductionManager::UDir_ProductionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    // Initialize production parameters
    MaxConcurrentProductions = 10;
    ProductionEfficiencyMultiplier = 1.0f;
    QualityControlThreshold = 0.85f;
    
    // Initialize production queues
    ActiveProductions.Empty();
    ProductionQueue.Empty();
    CompletedProductions.Empty();
    
    // Initialize production types and their base times
    ProductionBaseTimes.Add(EDir_ProductionType::AssetCreation, 300.0f); // 5 minutes
    ProductionBaseTimes.Add(EDir_ProductionType::CodeGeneration, 180.0f); // 3 minutes
    ProductionBaseTimes.Add(EDir_ProductionType::Testing, 120.0f); // 2 minutes
    ProductionBaseTimes.Add(EDir_ProductionType::Integration, 240.0f); // 4 minutes
    ProductionBaseTimes.Add(EDir_ProductionType::Optimization, 360.0f); // 6 minutes
    ProductionBaseTimes.Add(EDir_ProductionType::Documentation, 90.0f); // 1.5 minutes
    
    // Initialize priority weights
    PriorityWeights.Add(EDir_ProductionPriority::Critical, 4.0f);
    PriorityWeights.Add(EDir_ProductionPriority::High, 2.0f);
    PriorityWeights.Add(EDir_ProductionPriority::Normal, 1.0f);
    PriorityWeights.Add(EDir_ProductionPriority::Low, 0.5f);
}

void UDir_ProductionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogProductionManager, Log, TEXT("Production Manager initialized"));
    
    // Initialize production tracking
    TotalProductionsStarted = 0;
    TotalProductionsCompleted = 0;
    TotalProductionTime = 0.0f;
    AverageProductionTime = 0.0f;
    
    // Start production monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProductionMonitoringTimer,
            this,
            &UDir_ProductionManager::MonitorProductionHealth,
            30.0f, // Every 30 seconds
            true
        );
    }
}

void UDir_ProductionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active productions
    UpdateActiveProductions(DeltaTime);
    
    // Process production queue
    ProcessProductionQueue();
    
    // Update production metrics
    UpdateProductionMetrics();
}

bool UDir_ProductionManager::StartProduction(const FDir_ProductionTask& Task)
{
    // Validate task
    if (Task.TaskName.IsEmpty() || Task.ProductionType == EDir_ProductionType::None)
    {
        UE_LOG(LogProductionManager, Warning, TEXT("Invalid production task: %s"), *Task.TaskName);
        return false;
    }
    
    // Check if we can start immediately or need to queue
    if (ActiveProductions.Num() < MaxConcurrentProductions)
    {
        return StartProductionImmediate(Task);
    }
    else
    {
        // Add to queue with priority sorting
        QueueProduction(Task);
        UE_LOG(LogProductionManager, Log, TEXT("Production queued: %s (Priority: %d)"), 
            *Task.TaskName, (int32)Task.Priority);
        return true;
    }
}

bool UDir_ProductionManager::StartProductionImmediate(const FDir_ProductionTask& Task)
{
    // Create production info
    FDir_ProductionInfo ProductionInfo;
    ProductionInfo.Task = Task;
    ProductionInfo.StartTime = GetWorld()->GetTimeSeconds();
    ProductionInfo.EstimatedDuration = CalculateProductionTime(Task);
    ProductionInfo.Progress = 0.0f;
    ProductionInfo.Status = EDir_ProductionStatus::InProgress;
    ProductionInfo.QualityScore = 0.0f;
    
    // Generate unique ID
    ProductionInfo.ProductionID = FGuid::NewGuid();
    
    // Add to active productions
    ActiveProductions.Add(ProductionInfo);
    TotalProductionsStarted++;
    
    UE_LOG(LogProductionManager, Log, TEXT("Started production: %s (ID: %s, Duration: %.1fs)"), 
        *Task.TaskName, *ProductionInfo.ProductionID.ToString(), ProductionInfo.EstimatedDuration);
    
    // Notify delegates
    OnProductionStarted.Broadcast(ProductionInfo);
    
    return true;
}

void UDir_ProductionManager::QueueProduction(const FDir_ProductionTask& Task)
{
    // Insert into queue based on priority
    int32 InsertIndex = ProductionQueue.Num();
    
    for (int32 i = 0; i < ProductionQueue.Num(); i++)
    {
        if (GetPriorityWeight(Task.Priority) > GetPriorityWeight(ProductionQueue[i].Priority))
        {
            InsertIndex = i;
            break;
        }
    }
    
    ProductionQueue.Insert(Task, InsertIndex);
}

void UDir_ProductionManager::UpdateActiveProductions(float DeltaTime)
{
    for (int32 i = ActiveProductions.Num() - 1; i >= 0; i--)
    {
        FDir_ProductionInfo& Production = ActiveProductions[i];
        
        if (Production.Status != EDir_ProductionStatus::InProgress)
        {
            continue;
        }
        
        // Update progress
        float ElapsedTime = GetWorld()->GetTimeSeconds() - Production.StartTime;
        Production.Progress = FMath::Clamp(ElapsedTime / Production.EstimatedDuration, 0.0f, 1.0f);
        
        // Check if production is complete
        if (Production.Progress >= 1.0f)
        {
            CompleteProduction(i);
        }
        else
        {
            // Simulate production quality development
            UpdateProductionQuality(Production, DeltaTime);
        }
    }
}

void UDir_ProductionManager::CompleteProduction(int32 ProductionIndex)
{
    if (!ActiveProductions.IsValidIndex(ProductionIndex))
    {
        return;
    }
    
    FDir_ProductionInfo& Production = ActiveProductions[ProductionIndex];
    
    // Finalize production
    Production.Status = EDir_ProductionStatus::Completed;
    Production.Progress = 1.0f;
    Production.CompletionTime = GetWorld()->GetTimeSeconds();
    Production.ActualDuration = Production.CompletionTime - Production.StartTime;
    
    // Final quality assessment
    FinalizeProductionQuality(Production);
    
    // Update metrics
    TotalProductionsCompleted++;
    TotalProductionTime += Production.ActualDuration;
    AverageProductionTime = TotalProductionTime / TotalProductionsCompleted;
    
    UE_LOG(LogProductionManager, Log, TEXT("Completed production: %s (Quality: %.2f, Duration: %.1fs)"), 
        *Production.Task.TaskName, Production.QualityScore, Production.ActualDuration);
    
    // Move to completed list
    CompletedProductions.Add(Production);
    
    // Notify delegates
    OnProductionCompleted.Broadcast(Production);
    
    // Remove from active productions
    ActiveProductions.RemoveAt(ProductionIndex);
}

void UDir_ProductionManager::ProcessProductionQueue()
{
    // Start queued productions if we have capacity
    while (ProductionQueue.Num() > 0 && ActiveProductions.Num() < MaxConcurrentProductions)
    {
        FDir_ProductionTask NextTask = ProductionQueue[0];
        ProductionQueue.RemoveAt(0);
        
        StartProductionImmediate(NextTask);
    }
}

float UDir_ProductionManager::CalculateProductionTime(const FDir_ProductionTask& Task)
{
    float BaseTime = ProductionBaseTimes.FindRef(Task.ProductionType);
    
    // Apply complexity modifier
    float ComplexityMultiplier = 1.0f;
    switch (Task.Complexity)
    {
        case EDir_TaskComplexity::Simple: ComplexityMultiplier = 0.7f; break;
        case EDir_TaskComplexity::Medium: ComplexityMultiplier = 1.0f; break;
        case EDir_TaskComplexity::Complex: ComplexityMultiplier = 1.5f; break;
        case EDir_TaskComplexity::VeryComplex: ComplexityMultiplier = 2.0f; break;
    }
    
    // Apply efficiency multiplier
    return BaseTime * ComplexityMultiplier * ProductionEfficiencyMultiplier;
}

void UDir_ProductionManager::UpdateProductionQuality(FDir_ProductionInfo& Production, float DeltaTime)
{
    // Simulate quality development over time
    float QualityGrowthRate = 0.1f; // Quality improves by 10% per second on average
    
    // Add some randomness to simulate real production variance
    float RandomFactor = FMath::RandRange(0.8f, 1.2f);
    float QualityIncrease = QualityGrowthRate * DeltaTime * RandomFactor;
    
    Production.QualityScore = FMath::Clamp(Production.QualityScore + QualityIncrease, 0.0f, 1.0f);
}

void UDir_ProductionManager::FinalizeProductionQuality(FDir_ProductionInfo& Production)
{
    // Apply final quality modifiers based on task characteristics
    float FinalQuality = Production.QualityScore;
    
    // Priority bonus/penalty
    switch (Production.Task.Priority)
    {
        case EDir_ProductionPriority::Critical: FinalQuality *= 1.1f; break;
        case EDir_ProductionPriority::High: FinalQuality *= 1.05f; break;
        case EDir_ProductionPriority::Normal: break;
        case EDir_ProductionPriority::Low: FinalQuality *= 0.95f; break;
    }
    
    // Complexity penalty for rushed work
    if (Production.ActualDuration < Production.EstimatedDuration * 0.8f)
    {
        FinalQuality *= 0.9f; // Quality penalty for rushing
    }
    
    Production.QualityScore = FMath::Clamp(FinalQuality, 0.0f, 1.0f);
}

void UDir_ProductionManager::UpdateProductionMetrics()
{
    // Calculate current efficiency
    if (TotalProductionsCompleted > 0)
    {
        float CompletionRate = (float)TotalProductionsCompleted / (float)TotalProductionsStarted;
        ProductionEfficiencyMultiplier = FMath::Lerp(ProductionEfficiencyMultiplier, CompletionRate, 0.1f);
    }
}

void UDir_ProductionManager::MonitorProductionHealth()
{
    UE_LOG(LogProductionManager, Log, TEXT("=== PRODUCTION HEALTH MONITOR ==="));
    UE_LOG(LogProductionManager, Log, TEXT("Active Productions: %d/%d"), ActiveProductions.Num(), MaxConcurrentProductions);
    UE_LOG(LogProductionManager, Log, TEXT("Queued Productions: %d"), ProductionQueue.Num());
    UE_LOG(LogProductionManager, Log, TEXT("Completed Productions: %d"), TotalProductionsCompleted);
    UE_LOG(LogProductionManager, Log, TEXT("Average Production Time: %.1fs"), AverageProductionTime);
    UE_LOG(LogProductionManager, Log, TEXT("Production Efficiency: %.2f"), ProductionEfficiencyMultiplier);
    
    // Check for stalled productions
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (const FDir_ProductionInfo& Production : ActiveProductions)
    {
        float ElapsedTime = CurrentTime - Production.StartTime;
        if (ElapsedTime > Production.EstimatedDuration * 1.5f)
        {
            UE_LOG(LogProductionManager, Warning, TEXT("Stalled production detected: %s (%.1fs elapsed)"), 
                *Production.Task.TaskName, ElapsedTime);
        }
    }
}

float UDir_ProductionManager::GetPriorityWeight(EDir_ProductionPriority Priority) const
{
    return PriorityWeights.FindRef(Priority);
}

TArray<FDir_ProductionInfo> UDir_ProductionManager::GetActiveProductions() const
{
    return ActiveProductions;
}

TArray<FDir_ProductionTask> UDir_ProductionManager::GetProductionQueue() const
{
    return ProductionQueue;
}

FDir_ProductionStats UDir_ProductionManager::GetProductionStats() const
{
    FDir_ProductionStats Stats;
    Stats.TotalStarted = TotalProductionsStarted;
    Stats.TotalCompleted = TotalProductionsCompleted;
    Stats.AverageTime = AverageProductionTime;
    Stats.EfficiencyMultiplier = ProductionEfficiencyMultiplier;
    Stats.ActiveCount = ActiveProductions.Num();
    Stats.QueuedCount = ProductionQueue.Num();
    
    // Calculate average quality
    if (CompletedProductions.Num() > 0)
    {
        float TotalQuality = 0.0f;
        for (const FDir_ProductionInfo& Production : CompletedProductions)
        {
            TotalQuality += Production.QualityScore;
        }
        Stats.AverageQuality = TotalQuality / CompletedProductions.Num();
    }
    else
    {
        Stats.AverageQuality = 0.0f;
    }
    
    return Stats;
}

bool UDir_ProductionManager::CancelProduction(const FGuid& ProductionID)
{
    // Find and cancel active production
    for (int32 i = 0; i < ActiveProductions.Num(); i++)
    {
        if (ActiveProductions[i].ProductionID == ProductionID)
        {
            FDir_ProductionInfo CancelledProduction = ActiveProductions[i];
            CancelledProduction.Status = EDir_ProductionStatus::Cancelled;
            
            ActiveProductions.RemoveAt(i);
            
            UE_LOG(LogProductionManager, Log, TEXT("Cancelled production: %s"), *CancelledProduction.Task.TaskName);
            
            OnProductionCancelled.Broadcast(CancelledProduction);
            return true;
        }
    }
    
    return false;
}