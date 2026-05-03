#include "Combat_CombatStateManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UCombat_CombatStateManager::UCombat_CombatStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize combat settings
    CombatTimeoutDuration = 30.0f;
    IntensityDecayRate = 0.5f;
    MaxCombatRange = 2000.0f;
    
    // Initialize state
    CombatState = FCombat_CombatState();
    PrimaryTarget = nullptr;
    LastCombatActivity = 0.0f;
}

void UCombat_CombatStateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat state
    CombatState.CurrentPhase = ECombat_CombatPhase::PreCombat;
    CombatState.ThreatLevel = ECombat_ThreatLevel::None;
    LastCombatActivity = GetWorld()->GetTimeSeconds();
}

void UCombat_CombatStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IsInCombat())
    {
        CombatState.TimeInCombat += DeltaTime;
        UpdateCombatIntensity(DeltaTime);
        CalculateThreatLevel();
        CheckCombatTimeout(DeltaTime);
    }
}

void UCombat_CombatStateManager::InitiateCombat(AActor* Target, ECombat_ThreatLevel InitialThreatLevel)
{
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCombat_CombatStateManager::InitiateCombat - Invalid target"));
        return;
    }
    
    // Store previous phase for event handling
    ECombat_CombatPhase OldPhase = CombatState.CurrentPhase;
    
    // Set combat state
    PrimaryTarget = Target;
    CombatState.CurrentPhase = ECombat_CombatPhase::Engagement;
    CombatState.ThreatLevel = InitialThreatLevel;
    CombatState.TimeInCombat = 0.0f;
    CombatState.CombatIntensity = 0.3f; // Start with moderate intensity
    CombatState.bIsPlayerInvolved = Target->IsA<APawn>();
    LastCombatActivity = GetWorld()->GetTimeSeconds();
    
    // Add target to enemies list if not already present
    if (!EnemiesInCombat.Contains(Target))
    {
        EnemiesInCombat.Add(Target);
        CombatState.EnemiesInRange = EnemiesInCombat.Num();
    }
    
    // Check if this is pack combat
    CombatState.bIsPackCombat = EnemiesInCombat.Num() > 1;
    
    // Fire phase change event
    OnCombatPhaseChanged(OldPhase, CombatState.CurrentPhase);
    
    LogCombatEvent(FString::Printf(TEXT("Combat initiated against %s with threat level %d"), 
        *Target->GetName(), (int32)InitialThreatLevel));
}

void UCombat_CombatStateManager::EndCombat(bool bVictory)
{
    ECombat_CombatPhase OldPhase = CombatState.CurrentPhase;
    
    // Set final phase based on outcome
    CombatState.CurrentPhase = bVictory ? ECombat_CombatPhase::Victory : ECombat_CombatPhase::Defeat;
    
    // Record combat metrics before clearing
    RecordCombatMetrics();
    
    // Clear combat data
    EnemiesInCombat.Empty();
    CombatState.EnemiesInRange = 0;
    PrimaryTarget = nullptr;
    CombatState.CombatIntensity = 0.0f;
    
    // Fire phase change event
    OnCombatPhaseChanged(OldPhase, CombatState.CurrentPhase);
    
    LogCombatEvent(FString::Printf(TEXT("Combat ended - %s after %.2f seconds"), 
        bVictory ? TEXT("Victory") : TEXT("Defeat"), CombatState.TimeInCombat));
    
    // Transition to post-combat after a delay
    FTimerHandle PostCombatTimer;
    GetWorld()->GetTimerManager().SetTimer(PostCombatTimer, [this]()
    {
        UpdateCombatPhase(ECombat_CombatPhase::PostCombat);
        
        // Return to pre-combat after post-combat phase
        FTimerHandle PreCombatTimer;
        GetWorld()->GetTimerManager().SetTimer(PreCombatTimer, [this]()
        {
            UpdateCombatPhase(ECombat_CombatPhase::PreCombat);
            CombatState.ThreatLevel = ECombat_ThreatLevel::None;
        }, 5.0f, false);
        
    }, 2.0f, false);
}

void UCombat_CombatStateManager::UpdateCombatPhase(ECombat_CombatPhase NewPhase)
{
    if (CombatState.CurrentPhase != NewPhase)
    {
        ECombat_CombatPhase OldPhase = CombatState.CurrentPhase;
        CombatState.CurrentPhase = NewPhase;
        OnCombatPhaseChanged(OldPhase, NewPhase);
        
        LogCombatEvent(FString::Printf(TEXT("Combat phase changed from %d to %d"), 
            (int32)OldPhase, (int32)NewPhase));
    }
}

void UCombat_CombatStateManager::UpdateThreatLevel(ECombat_ThreatLevel NewThreatLevel)
{
    if (CombatState.ThreatLevel != NewThreatLevel)
    {
        ECombat_ThreatLevel OldLevel = CombatState.ThreatLevel;
        CombatState.ThreatLevel = NewThreatLevel;
        OnThreatLevelChanged(OldLevel, NewThreatLevel);
        
        LogCombatEvent(FString::Printf(TEXT("Threat level changed from %d to %d"), 
            (int32)OldLevel, (int32)NewThreatLevel));
    }
}

void UCombat_CombatStateManager::AddEnemyToRange(AActor* Enemy)
{
    if (Enemy && !EnemiesInCombat.Contains(Enemy))
    {
        EnemiesInCombat.Add(Enemy);
        CombatState.EnemiesInRange = EnemiesInCombat.Num();
        CombatState.bIsPackCombat = EnemiesInCombat.Num() > 1;
        LastCombatActivity = GetWorld()->GetTimeSeconds();
        
        // Escalate to active combat if we have multiple enemies
        if (CombatState.CurrentPhase == ECombat_CombatPhase::Engagement && EnemiesInCombat.Num() > 1)
        {
            UpdateCombatPhase(ECombat_CombatPhase::ActiveCombat);
        }
        
        LogCombatEvent(FString::Printf(TEXT("Enemy %s added to combat range. Total enemies: %d"), 
            *Enemy->GetName(), EnemiesInCombat.Num()));
    }
}

void UCombat_CombatStateManager::RemoveEnemyFromRange(AActor* Enemy)
{
    if (Enemy && EnemiesInCombat.Contains(Enemy))
    {
        EnemiesInCombat.Remove(Enemy);
        CombatState.EnemiesInRange = EnemiesInCombat.Num();
        CombatState.bIsPackCombat = EnemiesInCombat.Num() > 1;
        
        LogCombatEvent(FString::Printf(TEXT("Enemy %s removed from combat range. Remaining enemies: %d"), 
            *Enemy->GetName(), EnemiesInCombat.Num()));
        
        // End combat if no enemies remain
        if (EnemiesInCombat.Num() == 0)
        {
            EndCombat(true); // Victory by elimination
        }
    }
}

bool UCombat_CombatStateManager::IsInCombat() const
{
    return CombatState.CurrentPhase != ECombat_CombatPhase::PreCombat && 
           CombatState.CurrentPhase != ECombat_CombatPhase::PostCombat;
}

void UCombat_CombatStateManager::LogCombatEvent(const FString& EventDescription)
{
    UE_LOG(LogTemp, Log, TEXT("[CombatStateManager] %s - %s"), 
        *GetOwner()->GetName(), *EventDescription);
}

void UCombat_CombatStateManager::RecordCombatMetrics()
{
    // Log comprehensive combat metrics
    UE_LOG(LogTemp, Log, TEXT("[CombatMetrics] Combat Duration: %.2f seconds"), CombatState.TimeInCombat);
    UE_LOG(LogTemp, Log, TEXT("[CombatMetrics] Max Intensity: %.2f"), CombatState.CombatIntensity);
    UE_LOG(LogTemp, Log, TEXT("[CombatMetrics] Max Enemies: %d"), CombatState.EnemiesInRange);
    UE_LOG(LogTemp, Log, TEXT("[CombatMetrics] Pack Combat: %s"), CombatState.bIsPackCombat ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("[CombatMetrics] Player Involved: %s"), CombatState.bIsPlayerInvolved ? TEXT("Yes") : TEXT("No"));
}

void UCombat_CombatStateManager::UpdateCombatIntensity(float DeltaTime)
{
    // Base intensity calculation
    float TargetIntensity = 0.0f;
    
    // Factor in number of enemies
    TargetIntensity += FMath::Min(EnemiesInCombat.Num() * 0.2f, 0.8f);
    
    // Factor in threat level
    switch (CombatState.ThreatLevel)
    {
        case ECombat_ThreatLevel::Low:
            TargetIntensity += 0.1f;
            break;
        case ECombat_ThreatLevel::Medium:
            TargetIntensity += 0.3f;
            break;
        case ECombat_ThreatLevel::High:
            TargetIntensity += 0.5f;
            break;
        case ECombat_ThreatLevel::Critical:
            TargetIntensity += 0.7f;
            break;
        case ECombat_ThreatLevel::Overwhelming:
            TargetIntensity += 1.0f;
            break;
    }
    
    // Factor in combat duration (intensity increases over time)
    TargetIntensity += FMath::Min(CombatState.TimeInCombat / 60.0f, 0.3f);
    
    // Clamp target intensity
    TargetIntensity = FMath::Clamp(TargetIntensity, 0.0f, 1.0f);
    
    // Smooth interpolation to target intensity
    CombatState.CombatIntensity = FMath::FInterpTo(CombatState.CombatIntensity, TargetIntensity, DeltaTime, 2.0f);
}

void UCombat_CombatStateManager::CalculateThreatLevel()
{
    ECombat_ThreatLevel NewThreatLevel = ECombat_ThreatLevel::None;
    
    if (EnemiesInCombat.Num() == 0)
    {
        NewThreatLevel = ECombat_ThreatLevel::None;
    }
    else if (EnemiesInCombat.Num() == 1)
    {
        NewThreatLevel = ECombat_ThreatLevel::Low;
    }
    else if (EnemiesInCombat.Num() <= 3)
    {
        NewThreatLevel = ECombat_ThreatLevel::Medium;
    }
    else if (EnemiesInCombat.Num() <= 5)
    {
        NewThreatLevel = ECombat_ThreatLevel::High;
    }
    else if (EnemiesInCombat.Num() <= 8)
    {
        NewThreatLevel = ECombat_ThreatLevel::Critical;
    }
    else
    {
        NewThreatLevel = ECombat_ThreatLevel::Overwhelming;
    }
    
    // Update if changed
    if (NewThreatLevel != CombatState.ThreatLevel)
    {
        UpdateThreatLevel(NewThreatLevel);
    }
}

void UCombat_CombatStateManager::CheckCombatTimeout(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceActivity = CurrentTime - LastCombatActivity;
    
    if (TimeSinceActivity > CombatTimeoutDuration)
    {
        // Combat has timed out - force end
        LogCombatEvent(TEXT("Combat timed out - forcing end"));
        EndCombat(false); // Timeout counts as defeat
    }
}

void UCombat_CombatStateManager::OnCombatPhaseChanged(ECombat_CombatPhase OldPhase, ECombat_CombatPhase NewPhase)
{
    // Override in subclasses for custom phase change handling
    // This could trigger animations, sound effects, UI updates, etc.
}

void UCombat_CombatStateManager::OnThreatLevelChanged(ECombat_ThreatLevel OldLevel, ECombat_ThreatLevel NewLevel)
{
    // Override in subclasses for custom threat level change handling
    // This could trigger different AI behaviors, alert systems, etc.
}