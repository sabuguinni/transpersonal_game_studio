#include "ConsciousnessSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

UConsciousnessSystem::UConsciousnessSystem()
{
    CurrentConsciousnessState = EConsciousnessState::Ordinary;
    CurrentRealityLayer = ERealityLayer::Physical;
    LastStateTransitionTime = 0.0f;
}

void UConsciousnessSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ConsciousnessSystem: Initialized"));
    
    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UConsciousnessSystem::UpdateConsciousnessMetrics, 1.0f),
            1.0f,
            true
        );
    }
}

void UConsciousnessSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UConsciousnessSystem::SetConsciousnessState(EConsciousnessState NewState)
{
    if (NewState == CurrentConsciousnessState)
    {
        return;
    }

    if (!CanTransitionToState(NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsciousnessSystem: Cannot transition to state %d"), (int32)NewState);
        return;
    }

    EConsciousnessState OldState = CurrentConsciousnessState;
    CurrentConsciousnessState = NewState;
    LastStateTransitionTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("ConsciousnessSystem: State changed from %d to %d"), (int32)OldState, (int32)NewState);

    OnConsciousnessStateChanged.Broadcast(OldState, NewState);
}

bool UConsciousnessSystem::CanTransitionToState(EConsciousnessState TargetState) const
{
    // Check cooldown
    if (GetWorld() && (GetWorld()->GetTimeSeconds() - LastStateTransitionTime) < StateTransitionCooldown)
    {
        return false;
    }

    // Check consciousness requirements based on target state
    switch (TargetState)
    {
        case EConsciousnessState::Ordinary:
            return true; // Can always return to ordinary
            
        case EConsciousnessState::Meditative:
            return ConsciousnessMetrics.Awareness >= 30.0f;
            
        case EConsciousnessState::Lucid:
            return ConsciousnessMetrics.Awareness >= 50.0f && ConsciousnessMetrics.Clarity >= 40.0f;
            
        case EConsciousnessState::Transcendent:
            return ConsciousnessMetrics.Awareness >= 70.0f && 
                   ConsciousnessMetrics.Clarity >= 60.0f && 
                   ConsciousnessMetrics.Wisdom >= 50.0f;
                   
        case EConsciousnessState::Unity:
            return ConsciousnessMetrics.Awareness >= 85.0f && 
                   ConsciousnessMetrics.Clarity >= 80.0f && 
                   ConsciousnessMetrics.Compassion >= 75.0f &&
                   ConsciousnessMetrics.Integration >= 70.0f;
                   
        case EConsciousnessState::Void:
            return ConsciousnessMetrics.Awareness >= 90.0f && 
                   ConsciousnessMetrics.Clarity >= 85.0f && 
                   ConsciousnessMetrics.Wisdom >= 80.0f;
    }

    return false;
}

void UConsciousnessSystem::ShiftToRealityLayer(ERealityLayer NewLayer)
{
    if (NewLayer == CurrentRealityLayer)
    {
        return;
    }

    if (!CanAccessRealityLayer(NewLayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsciousnessSystem: Cannot access reality layer %d"), (int32)NewLayer);
        return;
    }

    ERealityLayer OldLayer = CurrentRealityLayer;
    CurrentRealityLayer = NewLayer;

    UE_LOG(LogTemp, Log, TEXT("ConsciousnessSystem: Reality layer changed from %d to %d"), (int32)OldLayer, (int32)NewLayer);

    OnRealityLayerChanged.Broadcast(OldLayer, NewLayer);
}

bool UConsciousnessSystem::CanAccessRealityLayer(ERealityLayer TargetLayer) const
{
    // Reality layer access based on consciousness state and metrics
    switch (TargetLayer)
    {
        case ERealityLayer::Physical:
            return true; // Always accessible
            
        case ERealityLayer::Emotional:
            return ConsciousnessMetrics.Compassion >= 40.0f;
            
        case ERealityLayer::Mental:
            return ConsciousnessMetrics.Clarity >= 50.0f;
            
        case ERealityLayer::Archetypal:
            return CurrentConsciousnessState >= EConsciousnessState::Lucid && 
                   ConsciousnessMetrics.Wisdom >= 60.0f;
                   
        case ERealityLayer::Causal:
            return CurrentConsciousnessState >= EConsciousnessState::Transcendent && 
                   ConsciousnessMetrics.Integration >= 70.0f;
                   
        case ERealityLayer::Absolute:
            return CurrentConsciousnessState >= EConsciousnessState::Unity && 
                   ConsciousnessMetrics.Awareness >= 85.0f;
    }

    return false;
}

void UConsciousnessSystem::UpdateConsciousnessMetric(const FString& MetricName, float Delta)
{
    FConsciousnessMetrics OldMetrics = ConsciousnessMetrics;
    
    if (MetricName == "Awareness")
    {
        ConsciousnessMetrics.Awareness = FMath::Clamp(ConsciousnessMetrics.Awareness + Delta, 0.0f, 100.0f);
    }
    else if (MetricName == "Clarity")
    {
        ConsciousnessMetrics.Clarity = FMath::Clamp(ConsciousnessMetrics.Clarity + Delta, 0.0f, 100.0f);
    }
    else if (MetricName == "Compassion")
    {
        ConsciousnessMetrics.Compassion = FMath::Clamp(ConsciousnessMetrics.Compassion + Delta, 0.0f, 100.0f);
    }
    else if (MetricName == "Wisdom")
    {
        ConsciousnessMetrics.Wisdom = FMath::Clamp(ConsciousnessMetrics.Wisdom + Delta, 0.0f, 100.0f);
    }
    else if (MetricName == "Integration")
    {
        ConsciousnessMetrics.Integration = FMath::Clamp(ConsciousnessMetrics.Integration + Delta, 0.0f, 100.0f);
    }

    OnConsciousnessMetricsUpdated.Broadcast(ConsciousnessMetrics);
}

void UConsciousnessSystem::SetConsciousnessMetrics(const FConsciousnessMetrics& NewMetrics)
{
    ConsciousnessMetrics = NewMetrics;
    
    // Clamp all values
    ConsciousnessMetrics.Awareness = FMath::Clamp(ConsciousnessMetrics.Awareness, 0.0f, 100.0f);
    ConsciousnessMetrics.Clarity = FMath::Clamp(ConsciousnessMetrics.Clarity, 0.0f, 100.0f);
    ConsciousnessMetrics.Compassion = FMath::Clamp(ConsciousnessMetrics.Compassion, 0.0f, 100.0f);
    ConsciousnessMetrics.Wisdom = FMath::Clamp(ConsciousnessMetrics.Wisdom, 0.0f, 100.0f);
    ConsciousnessMetrics.Integration = FMath::Clamp(ConsciousnessMetrics.Integration, 0.0f, 100.0f);

    OnConsciousnessMetricsUpdated.Broadcast(ConsciousnessMetrics);
}

void UConsciousnessSystem::LoadRealityShiftData(UDataTable* DataTable)
{
    RealityShiftDataTable = DataTable;
    UE_LOG(LogTemp, Log, TEXT("ConsciousnessSystem: Loaded reality shift data table"));
}

void UConsciousnessSystem::UpdateConsciousnessMetrics(float DeltaTime)
{
    // Natural decay of metrics over time (simulates need for practice)
    bool bMetricsChanged = false;
    
    if (ConsciousnessMetrics.Awareness > 50.0f)
    {
        ConsciousnessMetrics.Awareness = FMath::Max(50.0f, ConsciousnessMetrics.Awareness - MetricDecayRate * DeltaTime);
        bMetricsChanged = true;
    }
    
    if (ConsciousnessMetrics.Clarity > 50.0f)
    {
        ConsciousnessMetrics.Clarity = FMath::Max(50.0f, ConsciousnessMetrics.Clarity - MetricDecayRate * DeltaTime);
        bMetricsChanged = true;
    }
    
    if (ConsciousnessMetrics.Compassion > 50.0f)
    {
        ConsciousnessMetrics.Compassion = FMath::Max(50.0f, ConsciousnessMetrics.Compassion - MetricDecayRate * DeltaTime);
        bMetricsChanged = true;
    }
    
    if (ConsciousnessMetrics.Wisdom > 50.0f)
    {
        ConsciousnessMetrics.Wisdom = FMath::Max(50.0f, ConsciousnessMetrics.Wisdom - MetricDecayRate * DeltaTime);
        bMetricsChanged = true;
    }
    
    if (ConsciousnessMetrics.Integration > 50.0f)
    {
        ConsciousnessMetrics.Integration = FMath::Max(50.0f, ConsciousnessMetrics.Integration - MetricDecayRate * DeltaTime);
        bMetricsChanged = true;
    }

    if (bMetricsChanged)
    {
        OnConsciousnessMetricsUpdated.Broadcast(ConsciousnessMetrics);
    }

    ProcessStateTransitions();
}

void UConsciousnessSystem::ProcessStateTransitions()
{
    // Auto-downgrade consciousness state if metrics fall below thresholds
    switch (CurrentConsciousnessState)
    {
        case EConsciousnessState::Unity:
            if (ConsciousnessMetrics.Awareness < 80.0f || ConsciousnessMetrics.Integration < 65.0f)
            {
                SetConsciousnessState(EConsciousnessState::Transcendent);
            }
            break;
            
        case EConsciousnessState::Transcendent:
            if (ConsciousnessMetrics.Awareness < 65.0f || ConsciousnessMetrics.Wisdom < 45.0f)
            {
                SetConsciousnessState(EConsciousnessState::Lucid);
            }
            break;
            
        case EConsciousnessState::Lucid:
            if (ConsciousnessMetrics.Awareness < 45.0f || ConsciousnessMetrics.Clarity < 35.0f)
            {
                SetConsciousnessState(EConsciousnessState::Meditative);
            }
            break;
            
        case EConsciousnessState::Meditative:
            if (ConsciousnessMetrics.Awareness < 25.0f)
            {
                SetConsciousnessState(EConsciousnessState::Ordinary);
            }
            break;
    }
}