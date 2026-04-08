// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "FrameBudgetManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "RenderingThread.h"
#include "RHI.h"

DEFINE_LOG_CATEGORY(LogFrameBudget);

DECLARE_CYCLE_STAT(TEXT("Frame Budget Monitoring"), STAT_FrameBudgetMonitoring, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Budget Analysis"), STAT_BudgetAnalysis, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Emergency Scaling"), STAT_EmergencyScaling, STATGROUP_Game);

void UFrameBudgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogFrameBudget, Log, TEXT("Frame Budget Manager initialized"));
    
    // Initialize platform-specific budgets
    InitializePlatformBudgets();
    
    // Set up budget monitoring
    if (bBudgetMonitoringEnabled)
    {
        GetWorld()->GetTimerManager().SetTimer(
            BudgetMonitoringTimer,
            this,
            &UFrameBudgetManager::UpdateBudgetMonitoring,
            0.1f, // Monitor every 100ms
            true
        );
    }
}

void UFrameBudgetManager::Deinitialize()
{
    if (GetWorld() && BudgetMonitoringTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(BudgetMonitoringTimer);
    }
    
    UE_LOG(LogFrameBudget, Log, TEXT("Frame Budget Manager deinitialized"));
    
    Super::Deinitialize();
}

void UFrameBudgetManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    // Auto-detect platform target
    EPerformanceTarget DetectedTarget = DetectPlatformTarget();
    SetPerformanceTarget(DetectedTarget);
    
    UE_LOG(LogFrameBudget, Log, TEXT("Frame Budget Manager started for world: %s, Target: %s"), 
           *InWorld.GetName(), 
           DetectedTarget == EPerformanceTarget::PC_60fps ? TEXT("PC 60fps") : TEXT("Console 30fps"));
}

UFrameBudgetManager* UFrameBudgetManager::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UFrameBudgetManager>();
    }
    return nullptr;
}

void UFrameBudgetManager::InitializeFrameBudget()
{
    SCOPE_CYCLE_COUNTER(STAT_FrameBudgetMonitoring);
    
    // Clear previous data
    BudgetUsageHistory.Empty();
    LastCategoryTimes.Empty();
    
    // Initialize tracking arrays
    for (int32 i = 0; i < static_cast<int32>(EBudgetCategory::Overhead) + 1; ++i)
    {
        EBudgetCategory Category = static_cast<EBudgetCategory>(i);
        BudgetUsageHistory.Add(Category, TArray<float>());
        LastCategoryTimes.Add(Category, 0.0f);
    }
    
    UE_LOG(LogFrameBudget, Log, TEXT("Frame budget initialized with target: %s"), 
           CurrentTarget == EPerformanceTarget::PC_60fps ? TEXT("PC 60fps") : TEXT("Console 30fps"));
}

void UFrameBudgetManager::SetPerformanceTarget(EPerformanceTarget Target)
{
    CurrentTarget = Target;
    
    // Update budget allocation based on target
    switch (Target)
    {
        case EPerformanceTarget::Console_30fps:
            CurrentBudget.TotalFrameTimeMS = 33.33f;
            CurrentBudget.PhysicsBudgetMS = 6.0f;
            CurrentBudget.RenderingBudgetMS = 16.0f;
            CurrentBudget.GameLogicBudgetMS = 8.0f;
            CurrentBudget.AudioBudgetMS = 2.0f;
            CurrentBudget.AIBudgetMS = 3.0f;
            CurrentBudget.AnimationBudgetMS = 2.0f;
            CurrentBudget.NetworkingBudgetMS = 1.0f;
            CurrentBudget.OverheadBudgetMS = 3.33f;
            break;
            
        case EPerformanceTarget::PC_60fps:
            CurrentBudget.TotalFrameTimeMS = 16.67f;
            CurrentBudget.PhysicsBudgetMS = 3.0f;
            CurrentBudget.RenderingBudgetMS = 8.0f;
            CurrentBudget.GameLogicBudgetMS = 4.0f;
            CurrentBudget.AudioBudgetMS = 1.0f;
            CurrentBudget.AIBudgetMS = 1.5f;
            CurrentBudget.AnimationBudgetMS = 1.0f;
            CurrentBudget.NetworkingBudgetMS = 0.5f;
            CurrentBudget.OverheadBudgetMS = 1.67f;
            break;
            
        case EPerformanceTarget::PC_120fps:
            CurrentBudget.TotalFrameTimeMS = 8.33f;
            CurrentBudget.PhysicsBudgetMS = 1.5f;
            CurrentBudget.RenderingBudgetMS = 4.0f;
            CurrentBudget.GameLogicBudgetMS = 2.0f;
            CurrentBudget.AudioBudgetMS = 0.5f;
            CurrentBudget.AIBudgetMS = 0.75f;
            CurrentBudget.AnimationBudgetMS = 0.5f;
            CurrentBudget.NetworkingBudgetMS = 0.25f;
            CurrentBudget.OverheadBudgetMS = 0.83f;
            break;
    }
    
    UE_LOG(LogFrameBudget, Log, TEXT("Performance target set to: %s (Total budget: %.2fms)"), 
           Target == EPerformanceTarget::PC_60fps ? TEXT("PC 60fps") : 
           Target == EPerformanceTarget::Console_30fps ? TEXT("Console 30fps") : TEXT("PC 120fps"),
           CurrentBudget.TotalFrameTimeMS);
}

FFrameBudget UFrameBudgetManager::GetCurrentFrameBudget() const
{
    return CurrentBudget;
}

bool UFrameBudgetManager::IsWithinBudget(EBudgetCategory Category) const
{
    float Usage = GetBudgetUsage(Category);
    return Usage <= 100.0f;
}

float UFrameBudgetManager::GetBudgetUsage(EBudgetCategory Category) const
{
    if (!LastCategoryTimes.Contains(Category))
    {
        return 0.0f;
    }
    
    float ActualTime = LastCategoryTimes[Category];
    float BudgetTime = 0.0f;
    
    switch (Category)
    {
        case EBudgetCategory::Physics:
            BudgetTime = CurrentBudget.PhysicsBudgetMS;
            break;
        case EBudgetCategory::Rendering:
            BudgetTime = CurrentBudget.RenderingBudgetMS;
            break;
        case EBudgetCategory::GameLogic:
            BudgetTime = CurrentBudget.GameLogicBudgetMS;
            break;
        case EBudgetCategory::Audio:
            BudgetTime = CurrentBudget.AudioBudgetMS;
            break;
        case EBudgetCategory::AI:
            BudgetTime = CurrentBudget.AIBudgetMS;
            break;
        case EBudgetCategory::Animation:
            BudgetTime = CurrentBudget.AnimationBudgetMS;
            break;
        case EBudgetCategory::Networking:
            BudgetTime = CurrentBudget.NetworkingBudgetMS;
            break;
        case EBudgetCategory::Overhead:
            BudgetTime = CurrentBudget.OverheadBudgetMS;
            break;
    }
    
    return BudgetTime > 0.0f ? (ActualTime / BudgetTime) * 100.0f : 0.0f;
}

void UFrameBudgetManager::EnforceBudget()
{
    SCOPE_CYCLE_COUNTER(STAT_EmergencyScaling);
    
    FBudgetAnalysis Analysis = AnalyzeBudgetUsage();
    
    if (Analysis.bEmergencyScalingRequired)
    {
        ApplyEmergencyScaling();
        
        UE_LOG(LogFrameBudget, Warning, TEXT("Emergency budget enforcement triggered! Overage: %.1f%%, Worst offender: %s"), 
               Analysis.BudgetOveragePercent,
               *UEnum::GetValueAsString(Analysis.WorstOffender));
    }
}

void UFrameBudgetManager::SetBudgetMonitoring(bool bEnabled)
{
    bBudgetMonitoringEnabled = bEnabled;
    
    if (bEnabled && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BudgetMonitoringTimer,
            this,
            &UFrameBudgetManager::UpdateBudgetMonitoring,
            0.1f,
            true
        );
    }
    else if (GetWorld() && BudgetMonitoringTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(BudgetMonitoringTimer);
    }
    
    UE_LOG(LogFrameBudget, Log, TEXT("Budget monitoring %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FBudgetAnalysis UFrameBudgetManager::AnalyzeBudgetUsage()
{
    SCOPE_CYCLE_COUNTER(STAT_BudgetAnalysis);
    
    FBudgetAnalysis Analysis;
    float WorstOverage = 0.0f;
    
    // Check each category for budget violations
    for (int32 i = 0; i < static_cast<int32>(EBudgetCategory::Overhead) + 1; ++i)
    {
        EBudgetCategory Category = static_cast<EBudgetCategory>(i);
        float Usage = GetBudgetUsage(Category);
        
        if (Usage > 100.0f)
        {
            Analysis.CategoriesOverBudget.Add(Category);
            
            float Overage = Usage - 100.0f;
            if (Overage > WorstOverage)
            {
                WorstOverage = Overage;
                Analysis.WorstOffender = Category;
            }
        }
    }
    
    Analysis.BudgetOveragePercent = WorstOverage;
    Analysis.bEmergencyScalingRequired = WorstOverage > EmergencyScalingThreshold;
    
    // Generate recommendations
    if (Analysis.CategoriesOverBudget.Num() > 0)
    {
        Analysis.RecommendedActions.Add(TEXT("Reduce LOD complexity"));
        Analysis.RecommendedActions.Add(TEXT("Decrease particle density"));
        Analysis.RecommendedActions.Add(TEXT("Optimize physics simulation"));
        Analysis.RecommendedActions.Add(TEXT("Reduce shadow quality"));
        
        if (Analysis.bEmergencyScalingRequired)
        {
            Analysis.RecommendedActions.Add(TEXT("EMERGENCY: Force quality reduction"));
        }
    }
    
    return Analysis;
}

void UFrameBudgetManager::InitializePlatformBudgets()
{
    // Platform-specific initialization
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    SetPerformanceTarget(EPerformanceTarget::PC_60fps);
#elif PLATFORM_PS5 || PLATFORM_XBOXONE || PLATFORM_SWITCH
    SetPerformanceTarget(EPerformanceTarget::Console_30fps);
#else
    SetPerformanceTarget(EPerformanceTarget::PC_60fps); // Default
#endif
}

void UFrameBudgetManager::UpdateBudgetMonitoring()
{
    if (!bBudgetMonitoringEnabled)
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_FrameBudgetMonitoring);
    
    // Update category times from stats
    LastCategoryTimes[EBudgetCategory::Physics] = CalculateCategoryUsage(EBudgetCategory::Physics);
    LastCategoryTimes[EBudgetCategory::Rendering] = CalculateCategoryUsage(EBudgetCategory::Rendering);
    LastCategoryTimes[EBudgetCategory::GameLogic] = CalculateCategoryUsage(EBudgetCategory::GameLogic);
    LastCategoryTimes[EBudgetCategory::Audio] = CalculateCategoryUsage(EBudgetCategory::Audio);
    LastCategoryTimes[EBudgetCategory::AI] = CalculateCategoryUsage(EBudgetCategory::AI);
    LastCategoryTimes[EBudgetCategory::Animation] = CalculateCategoryUsage(EBudgetCategory::Animation);
    LastCategoryTimes[EBudgetCategory::Networking] = CalculateCategoryUsage(EBudgetCategory::Networking);
    LastCategoryTimes[EBudgetCategory::Overhead] = CalculateCategoryUsage(EBudgetCategory::Overhead);
    
    // Store history for trend analysis
    for (auto& Pair : LastCategoryTimes)
    {
        TArray<float>& History = BudgetUsageHistory[Pair.Key];
        History.Add(Pair.Value);
        
        // Keep only last 100 samples
        if (History.Num() > 100)
        {
            History.RemoveAt(0);
        }
    }
    
    // Check for budget violations
    FBudgetAnalysis Analysis = AnalyzeBudgetUsage();
    if (Analysis.CategoriesOverBudget.Num() > 0)
    {
        LogBudgetViolations(Analysis);
        
        if (Analysis.bEmergencyScalingRequired)
        {
            EnforceBudget();
        }
    }
}

float UFrameBudgetManager::CalculateCategoryUsage(EBudgetCategory Category) const
{
    // Get timing data from UE5 stats system
    float CategoryTime = 0.0f;
    
    switch (Category)
    {
        case EBudgetCategory::Physics:
            // Get physics timing from stats
            CategoryTime = FPlatformTime::ToMilliseconds(GGameThreadTime) * 0.18f; // Approximate physics portion
            break;
        case EBudgetCategory::Rendering:
            CategoryTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
            break;
        case EBudgetCategory::GameLogic:
            CategoryTime = FPlatformTime::ToMilliseconds(GGameThreadTime) * 0.6f; // Approximate game logic portion
            break;
        case EBudgetCategory::Audio:
            CategoryTime = 0.5f; // Placeholder - would need audio thread timing
            break;
        case EBudgetCategory::AI:
            CategoryTime = FPlatformTime::ToMilliseconds(GGameThreadTime) * 0.1f; // Approximate AI portion
            break;
        case EBudgetCategory::Animation:
            CategoryTime = FPlatformTime::ToMilliseconds(GGameThreadTime) * 0.08f; // Approximate animation portion
            break;
        case EBudgetCategory::Networking:
            CategoryTime = 0.2f; // Placeholder - would need network thread timing
            break;
        case EBudgetCategory::Overhead:
            CategoryTime = FPlatformTime::ToMilliseconds(GGameThreadTime) * 0.04f; // Approximate overhead
            break;
    }
    
    return CategoryTime;
}

void UFrameBudgetManager::ApplyEmergencyScaling()
{
    // Emergency scaling measures
    UE_LOG(LogFrameBudget, Warning, TEXT("Applying emergency scaling measures"));
    
    // Reduce scalability settings
    if (GEngine)
    {
        // Reduce shadow quality
        GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 1"));
        
        // Reduce post-process quality
        GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 1"));
        
        // Reduce effects quality
        GEngine->Exec(GetWorld(), TEXT("sg.EffectsQuality 1"));
        
        // Reduce texture quality
        GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 1"));
        
        // Reduce view distance
        GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.7"));
    }
}

UFrameBudgetManager::EPerformanceTarget UFrameBudgetManager::DetectPlatformTarget() const
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    return EPerformanceTarget::PC_60fps;
#elif PLATFORM_PS5 || PLATFORM_XBOXONE || PLATFORM_SWITCH
    return EPerformanceTarget::Console_30fps;
#else
    return EPerformanceTarget::PC_60fps;
#endif
}

void UFrameBudgetManager::LogBudgetViolations(const FBudgetAnalysis& Analysis)
{
    UE_LOG(LogFrameBudget, Warning, TEXT("Budget violations detected:"));
    UE_LOG(LogFrameBudget, Warning, TEXT("  Categories over budget: %d"), Analysis.CategoriesOverBudget.Num());
    UE_LOG(LogFrameBudget, Warning, TEXT("  Worst offender: %s (%.1f%% over)"), 
           *UEnum::GetValueAsString(Analysis.WorstOffender), Analysis.BudgetOveragePercent);
    
    for (const FString& Action : Analysis.RecommendedActions)
    {
        UE_LOG(LogFrameBudget, Warning, TEXT("  Recommendation: %s"), *Action);
    }
}

// Budget Enforcer Component Implementation

UBudgetEnforcerComponent::UBudgetEnforcerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UBudgetEnforcerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    FrameBudgetManager = UFrameBudgetManager::Get(this);
    if (!FrameBudgetManager)
    {
        UE_LOG(LogFrameBudget, Warning, TEXT("Budget Enforcer Component could not find Frame Budget Manager"));
    }
}

void UBudgetEnforcerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoEnforceBudget && FrameBudgetManager)
    {
        float Usage = GetBudgetUsagePercent();
        if (Usage > 100.0f + ViolationThreshold)
        {
            ForceBudgetCompliance();
        }
    }
}

void UBudgetEnforcerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    FrameBudgetManager = nullptr;
    Super::EndPlay(EndPlayReason);
}

void UBudgetEnforcerComponent::SetBudgetCategory(UFrameBudgetManager::EBudgetCategory Category)
{
    MonitoredCategory = Category;
}

bool UBudgetEnforcerComponent::IsWithinBudget() const
{
    return FrameBudgetManager ? FrameBudgetManager->IsWithinBudget(MonitoredCategory) : true;
}

float UBudgetEnforcerComponent::GetBudgetUsagePercent() const
{
    return FrameBudgetManager ? FrameBudgetManager->GetBudgetUsage(MonitoredCategory) : 0.0f;
}

void UBudgetEnforcerComponent::ForceBudgetCompliance()
{
    if (CurrentQualityIndex < QualityReductionSteps.Num() - 1)
    {
        CurrentQualityIndex++;
        float QualityScale = QualityReductionSteps[CurrentQualityIndex];
        
        UE_LOG(LogFrameBudget, Warning, TEXT("Budget Enforcer reducing quality to %.2f for category %s"), 
               QualityScale, *UEnum::GetValueAsString(MonitoredCategory));
        
        // Apply quality reduction based on category
        // This would be implemented per-component type
    }
}