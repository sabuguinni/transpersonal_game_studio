#include "NPC_DinosaurEmotionalStates.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNPC_DinosaurEmotionalStates::UNPC_DinosaurEmotionalStates()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    EmotionalProfile = FNPC_EmotionalProfile();
    StateTransitionSpeed = 2.0f;
    StressDecayRate = 1.0f;
    EmotionalVolatility = 0.5f;
    LastStateChangeTime = 0.0f;
    PreviousState = ENPC_EmotionalState::Calm;
}

void UNPC_DinosaurEmotionalStates::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize emotional state based on dinosaur type
    if (AActor* Owner = GetOwner())
    {
        FString OwnerName = Owner->GetName();
        if (OwnerName.Contains("TRex"))
        {
            EmotionalProfile.AggressionLevel = 0.7f;
            EmotionalProfile.TerritorialLevel = 0.8f;
            EmotionalProfile.FearLevel = 0.1f;
        }
        else if (OwnerName.Contains("Raptor"))
        {
            EmotionalProfile.AggressionLevel = 0.6f;
            EmotionalProfile.CuriosityLevel = 0.8f;
            EmotionalProfile.SocialBondStrength = 0.7f;
        }
        else if (OwnerName.Contains("Brachio"))
        {
            EmotionalProfile.AggressionLevel = 0.2f;
            EmotionalProfile.FearLevel = 0.4f;
            EmotionalProfile.SocialBondStrength = 0.6f;
        }
    }
}

void UNPC_DinosaurEmotionalStates::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmotionalState(DeltaTime);
    ProcessStressDecay(DeltaTime);
    HandleStateTransitions();
}

void UNPC_DinosaurEmotionalStates::SetEmotionalState(ENPC_EmotionalState NewState)
{
    if (EmotionalProfile.CurrentState != NewState)
    {
        PreviousState = EmotionalProfile.CurrentState;
        EmotionalProfile.CurrentState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed emotional state from %d to %d"), 
               *GetOwner()->GetName(), 
               (int32)PreviousState, 
               (int32)NewState);
    }
}

ENPC_EmotionalState UNPC_DinosaurEmotionalStates::GetCurrentEmotionalState() const
{
    return EmotionalProfile.CurrentState;
}

void UNPC_DinosaurEmotionalStates::ModifyStressLevel(float StressChange)
{
    int32 CurrentStressInt = (int32)EmotionalProfile.StressLevel;
    CurrentStressInt = FMath::Clamp(CurrentStressInt + FMath::RoundToInt(StressChange), 0, 4);
    EmotionalProfile.StressLevel = (ENPC_StressLevel)CurrentStressInt;
    
    // High stress can trigger aggressive or fearful states
    if (EmotionalProfile.StressLevel >= ENPC_StressLevel::High)
    {
        if (EmotionalProfile.AggressionLevel > EmotionalProfile.FearLevel)
        {
            SetEmotionalState(ENPC_EmotionalState::Aggressive);
        }
        else
        {
            SetEmotionalState(ENPC_EmotionalState::Fearful);
        }
    }
}

void UNPC_DinosaurEmotionalStates::ModifyAggression(float AggressionChange)
{
    EmotionalProfile.AggressionLevel = FMath::Clamp(EmotionalProfile.AggressionLevel + AggressionChange, 0.0f, 1.0f);
    
    if (EmotionalProfile.AggressionLevel > 0.7f)
    {
        SetEmotionalState(ENPC_EmotionalState::Aggressive);
    }
}

void UNPC_DinosaurEmotionalStates::ModifyFear(float FearChange)
{
    EmotionalProfile.FearLevel = FMath::Clamp(EmotionalProfile.FearLevel + FearChange, 0.0f, 1.0f);
    
    if (EmotionalProfile.FearLevel > 0.6f)
    {
        SetEmotionalState(ENPC_EmotionalState::Fearful);
    }
}

void UNPC_DinosaurEmotionalStates::TriggerThreatResponse(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
        return;
    
    // Calculate distance to threat
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    float ThreatProximity = FMath::Clamp(1.0f - (Distance / 3000.0f), 0.0f, 1.0f);
    
    float EffectiveThreatLevel = ThreatLevel * ThreatProximity;
    
    // Modify emotional state based on threat
    ModifyStressLevel(EffectiveThreatLevel * 2.0f);
    
    if (EmotionalProfile.AggressionLevel > EmotionalProfile.FearLevel)
    {
        ModifyAggression(EffectiveThreatLevel * 0.5f);
        SetEmotionalState(ENPC_EmotionalState::Aggressive);
    }
    else
    {
        ModifyFear(EffectiveThreatLevel * 0.7f);
        SetEmotionalState(ENPC_EmotionalState::Fearful);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s responding to threat %s with level %.2f"), 
           *GetOwner()->GetName(), 
           *ThreatActor->GetName(), 
           EffectiveThreatLevel);
}

void UNPC_DinosaurEmotionalStates::TriggerCalmingInfluence(float CalmingStrength)
{
    ModifyStressLevel(-CalmingStrength);
    EmotionalProfile.AggressionLevel = FMath::Max(0.0f, EmotionalProfile.AggressionLevel - CalmingStrength * 0.3f);
    EmotionalProfile.FearLevel = FMath::Max(0.0f, EmotionalProfile.FearLevel - CalmingStrength * 0.4f);
    
    if (EmotionalProfile.StressLevel <= ENPC_StressLevel::Mild && 
        EmotionalProfile.AggressionLevel < 0.3f && 
        EmotionalProfile.FearLevel < 0.3f)
    {
        SetEmotionalState(ENPC_EmotionalState::Calm);
    }
}

bool UNPC_DinosaurEmotionalStates::IsInAggressiveState() const
{
    return EmotionalProfile.CurrentState == ENPC_EmotionalState::Aggressive ||
           EmotionalProfile.CurrentState == ENPC_EmotionalState::Territorial ||
           EmotionalProfile.CurrentState == ENPC_EmotionalState::Hunting;
}

bool UNPC_DinosaurEmotionalStates::IsInDefensiveState() const
{
    return EmotionalProfile.CurrentState == ENPC_EmotionalState::Fearful ||
           EmotionalProfile.CurrentState == ENPC_EmotionalState::Protective ||
           EmotionalProfile.CurrentState == ENPC_EmotionalState::Alert;
}

float UNPC_DinosaurEmotionalStates::GetOverallStressLevel() const
{
    float StressValue = (float)EmotionalProfile.StressLevel / 4.0f;
    float EmotionalStress = (EmotionalProfile.AggressionLevel + EmotionalProfile.FearLevel) * 0.5f;
    return FMath::Clamp((StressValue + EmotionalStress) * 0.5f, 0.0f, 1.0f);
}

void UNPC_DinosaurEmotionalStates::UpdateEmotionalState(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastChange = CurrentTime - LastStateChangeTime;
    
    // Gradual emotional state transitions based on current levels
    if (TimeSinceLastChange > StateTransitionSpeed)
    {
        if (EmotionalProfile.CurrentState == ENPC_EmotionalState::Aggressive && 
            EmotionalProfile.AggressionLevel < 0.4f)
        {
            SetEmotionalState(ENPC_EmotionalState::Alert);
        }
        else if (EmotionalProfile.CurrentState == ENPC_EmotionalState::Fearful && 
                 EmotionalProfile.FearLevel < 0.3f)
        {
            SetEmotionalState(ENPC_EmotionalState::Alert);
        }
        else if (EmotionalProfile.CurrentState == ENPC_EmotionalState::Alert && 
                 EmotionalProfile.StressLevel <= ENPC_StressLevel::Mild)
        {
            SetEmotionalState(ENPC_EmotionalState::Calm);
        }
    }
}

void UNPC_DinosaurEmotionalStates::ProcessStressDecay(float DeltaTime)
{
    // Gradually reduce stress over time
    if (EmotionalProfile.StressLevel > ENPC_StressLevel::Relaxed)
    {
        float StressReduction = StressDecayRate * DeltaTime;
        int32 CurrentStress = (int32)EmotionalProfile.StressLevel;
        
        // Reduce stress level periodically
        static float StressDecayTimer = 0.0f;
        StressDecayTimer += DeltaTime;
        
        if (StressDecayTimer >= 5.0f) // Reduce stress every 5 seconds
        {
            ModifyStressLevel(-1.0f);
            StressDecayTimer = 0.0f;
        }
    }
    
    // Gradually reduce aggression and fear when not stimulated
    EmotionalProfile.AggressionLevel = FMath::Max(0.0f, EmotionalProfile.AggressionLevel - (DeltaTime * 0.1f));
    EmotionalProfile.FearLevel = FMath::Max(0.0f, EmotionalProfile.FearLevel - (DeltaTime * 0.15f));
}

void UNPC_DinosaurEmotionalStates::HandleStateTransitions()
{
    // Handle automatic state transitions based on emotional levels
    switch (EmotionalProfile.CurrentState)
    {
        case ENPC_EmotionalState::Calm:
            if (EmotionalProfile.StressLevel >= ENPC_StressLevel::Moderate)
            {
                SetEmotionalState(ENPC_EmotionalState::Alert);
            }
            break;
            
        case ENPC_EmotionalState::Alert:
            if (EmotionalProfile.AggressionLevel > 0.6f)
            {
                SetEmotionalState(ENPC_EmotionalState::Aggressive);
            }
            else if (EmotionalProfile.FearLevel > 0.5f)
            {
                SetEmotionalState(ENPC_EmotionalState::Fearful);
            }
            break;
            
        case ENPC_EmotionalState::Aggressive:
            if (EmotionalProfile.AggressionLevel < 0.3f && EmotionalProfile.StressLevel < ENPC_StressLevel::High)
            {
                SetEmotionalState(ENPC_EmotionalState::Alert);
            }
            break;
            
        case ENPC_EmotionalState::Fearful:
            if (EmotionalProfile.FearLevel < 0.2f && EmotionalProfile.StressLevel < ENPC_StressLevel::Moderate)
            {
                SetEmotionalState(ENPC_EmotionalState::Calm);
            }
            break;
            
        default:
            break;
    }
}