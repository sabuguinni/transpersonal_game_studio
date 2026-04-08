#include "TranspersonalGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ATranspersonalGameState::ATranspersonalGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second
    
    // Initialize default values
    CurrentGamePhase = EGamePhase::Initialization;
    CurrentTimeOfDay = ETimeOfDay::Dawn;
    GameTimeHours = 6.0f; // Start at dawn
    SurvivalTimeMinutes = 0.0f;
    CreatureCount = 0;
    PlayerDeathCount = 0;
    TimeProgressionSpeed = 1.0f;
    HoursPerMinute = 0.5f; // 48 minutes for full day cycle
}

void ATranspersonalGameState::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Game state initialized"));
    
    // Set initial game phase
    SetGamePhase(EGamePhase::EarlyGame);
    
    // Update initial time of day
    UpdateTimeOfDay();
}

void ATranspersonalGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Only update time on server
    if (HasAuthority())
    {
        // Update survival time
        SurvivalTimeMinutes += DeltaTime / 60.0f;
        
        // Update game time
        float TimeIncrement = (HoursPerMinute * TimeProgressionSpeed * DeltaTime) / 60.0f;
        GameTimeHours += TimeIncrement;
        
        // Wrap around 24 hours
        if (GameTimeHours >= 24.0f)
        {
            GameTimeHours -= 24.0f;
        }
        
        // Update time of day and game phase
        UpdateTimeOfDay();
        UpdateGamePhase();
    }
}

void ATranspersonalGameState::SetGamePhase(EGamePhase NewPhase)
{
    if (CurrentGamePhase != NewPhase)
    {
        EGamePhase OldPhase = CurrentGamePhase;
        CurrentGamePhase = NewPhase;
        
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Game phase changed from %d to %d"), 
               (int32)OldPhase, (int32)NewPhase);
        
        OnGamePhaseChanged();
    }
}

void ATranspersonalGameState::RegisterCreature()
{
    CreatureCount++;
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Creature registered. Total count: %d"), CreatureCount);
    
    OnCreatureCountChangedDelegate.Broadcast(CreatureCount);
}

void ATranspersonalGameState::UnregisterCreature()
{
    CreatureCount = FMath::Max(0, CreatureCount - 1);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Creature unregistered. Total count: %d"), CreatureCount);
    
    OnCreatureCountChangedDelegate.Broadcast(CreatureCount);
}

void ATranspersonalGameState::RecordPlayerDeath()
{
    PlayerDeathCount++;
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Player death recorded. Total deaths: %d"), PlayerDeathCount);
    
    // Player deaths might affect game phase
    UpdateGamePhase();
}

bool ATranspersonalGameState::IsNightTime() const
{
    return CurrentTimeOfDay == ETimeOfDay::Dusk || 
           CurrentTimeOfDay == ETimeOfDay::Night || 
           CurrentTimeOfDay == ETimeOfDay::Midnight;
}

bool ATranspersonalGameState::IsDayTime() const
{
    return CurrentTimeOfDay == ETimeOfDay::Dawn || 
           CurrentTimeOfDay == ETimeOfDay::Morning || 
           CurrentTimeOfDay == ETimeOfDay::Noon || 
           CurrentTimeOfDay == ETimeOfDay::Afternoon;
}

void ATranspersonalGameState::UpdateTimeOfDay()
{
    ETimeOfDay NewTimeOfDay = CurrentTimeOfDay;
    
    // Determine time of day based on hours
    if (GameTimeHours >= 5.0f && GameTimeHours < 7.0f)
    {
        NewTimeOfDay = ETimeOfDay::Dawn;
    }
    else if (GameTimeHours >= 7.0f && GameTimeHours < 11.0f)
    {
        NewTimeOfDay = ETimeOfDay::Morning;
    }
    else if (GameTimeHours >= 11.0f && GameTimeHours < 14.0f)
    {
        NewTimeOfDay = ETimeOfDay::Noon;
    }
    else if (GameTimeHours >= 14.0f && GameTimeHours < 18.0f)
    {
        NewTimeOfDay = ETimeOfDay::Afternoon;
    }
    else if (GameTimeHours >= 18.0f && GameTimeHours < 20.0f)
    {
        NewTimeOfDay = ETimeOfDay::Dusk;
    }
    else if (GameTimeHours >= 20.0f && GameTimeHours < 23.0f)
    {
        NewTimeOfDay = ETimeOfDay::Night;
    }
    else
    {
        NewTimeOfDay = ETimeOfDay::Midnight;
    }
    
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        OnTimeOfDayChanged();
    }
}

void ATranspersonalGameState::UpdateGamePhase()
{
    EGamePhase NewPhase = CurrentGamePhase;
    
    // Determine game phase based on survival time and other factors
    if (SurvivalTimeMinutes < 10.0f)
    {
        NewPhase = EGamePhase::EarlyGame;
    }
    else if (SurvivalTimeMinutes < 60.0f)
    {
        NewPhase = EGamePhase::MidGame;
    }
    else if (SurvivalTimeMinutes < 180.0f)
    {
        NewPhase = EGamePhase::LateGame;
    }
    else
    {
        NewPhase = EGamePhase::EndGame;
    }
    
    // Player deaths can accelerate phase progression
    if (PlayerDeathCount > 0)
    {
        if (NewPhase == EGamePhase::EarlyGame && SurvivalTimeMinutes > 5.0f)
        {
            NewPhase = EGamePhase::MidGame;
        }
        else if (NewPhase == EGamePhase::MidGame && SurvivalTimeMinutes > 30.0f)
        {
            NewPhase = EGamePhase::LateGame;
        }
    }
    
    SetGamePhase(NewPhase);
}

void ATranspersonalGameState::OnTimeOfDayChanged()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Time of day changed to %d at game time %.1f"), 
           (int32)CurrentTimeOfDay, GameTimeHours);
    
    OnTimeOfDayChangedDelegate.Broadcast(CurrentTimeOfDay);
}

void ATranspersonalGameState::OnGamePhaseChanged()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameState: Game phase changed to %d after %.1f minutes"), 
           (int32)CurrentGamePhase, SurvivalTimeMinutes);
    
    OnGamePhaseChangedDelegate.Broadcast(CurrentGamePhase);
}

void ATranspersonalGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ATranspersonalGameState, CurrentGamePhase);
    DOREPLIFETIME(ATranspersonalGameState, CurrentTimeOfDay);
    DOREPLIFETIME(ATranspersonalGameState, GameTimeHours);
    DOREPLIFETIME(ATranspersonalGameState, SurvivalTimeMinutes);
    DOREPLIFETIME(ATranspersonalGameState, CreatureCount);
    DOREPLIFETIME(ATranspersonalGameState, PlayerDeathCount);
}