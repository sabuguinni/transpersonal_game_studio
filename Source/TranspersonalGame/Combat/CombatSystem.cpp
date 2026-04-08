// CombatSystem.cpp
// Implementação do sistema de combate consciente
#include "CombatSystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatSystem::UCombatSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentCombatState = ECombatState::Peaceful;
    CombatIntensity = 0.0f;
    
    InitializeConsciousnessModifiers();
}

void UCombatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat stats based on consciousness level
    CombatStats.ConsciousnessLevel = 1.0f;
    CombatStats.KarmicBalance = 0.0f;
}

void UCombatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessKarmicBalance(DeltaTime);
    
    // Gradually reduce combat intensity when not in active combat
    if (CurrentCombatState != ECombatState::Aggressive && CombatIntensity > 0.0f)
    {
        CombatIntensity = FMath::FInterpTo(CombatIntensity, 0.0f, DeltaTime, 2.0f);
        
        if (CombatIntensity < 0.1f)
        {
            CombatIntensity = 0.0f;
            if (CurrentCombatState != ECombatState::Peaceful)
            {
                SetCombatState(ECombatState::Peaceful, 0.0f);
            }
        }
    }
}

void UCombatSystem::SetCombatState(ECombatState NewState, float Intensity)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        CombatIntensity = FMath::Clamp(Intensity, 0.0f, 10.0f);
        
        FString StateChangeReason = FString::Printf(TEXT("Combat state changed from %d to %d"), 
            (int32)PreviousState, (int32)NewState);
        
        OnCombatStateChanged.Broadcast(NewState, CombatIntensity, StateChangeReason);
        
        UE_LOG(LogTemp, Log, TEXT("Combat State Changed: %s"), *StateChangeReason);
    }
}

bool UCombatSystem::ExecuteSpiritualAttack(EAttackType AttackType, AActor* Target, float Power)
{
    if (!Target || !CanEngageInCombat(Target))
    {
        return false;
    }
    
    // Get target's combat system
    UCombatSystem* TargetCombat = Target->FindComponentByClass<UCombatSystem>();
    if (!TargetCombat)
    {
        return false;
    }
    
    // Calculate spiritual damage based on consciousness states
    EConsciousnessState AttackerState = EConsciousnessState::Ordinary; // Would get from consciousness system
    EConsciousnessState DefenderState = EConsciousnessState::Ordinary; // Would get from target's consciousness
    
    float Damage = CalculateSpiritualDamage(AttackType, Power, AttackerState, DefenderState);
    
    // Apply karmic consequences
    bool bHarmedInnocent = (TargetCombat->GetCombatState() == ECombatState::Peaceful);
    ApplyKarmicConsequences(AttackType, bHarmedInnocent, Damage);
    
    // Determine if attack is transformative (helps target grow spiritually)
    bool bTransformative = (AttackType == EAttackType::Transformative) || 
                          (CombatStats.ConsciousnessLevel > 5.0f && FMath::RandRange(0.0f, 1.0f) < 0.3f);
    
    float SpiritualImpact = Damage * CombatStats.SpiritualResonance * 0.01f;
    
    OnSpiritualCombat.Broadcast(AttackType, Damage, SpiritualImpact, bTransformative);
    
    // Set combat state to aggressive
    SetCombatState(ECombatState::Aggressive, Power);
    
    UE_LOG(LogTemp, Log, TEXT("Spiritual Attack: Type=%d, Damage=%.2f, Transformative=%s"), 
        (int32)AttackType, Damage, bTransformative ? TEXT("True") : TEXT("False"));
    
    return true;
}

float UCombatSystem::CalculateSpiritualDamage(EAttackType AttackType, float BasePower, 
    EConsciousnessState AttackerState, EConsciousnessState DefenderState)
{
    float Damage = BasePower * CombatStats.PhysicalPower * 0.01f;
    
    // Modify damage based on attack type
    switch (AttackType)
    {
        case EAttackType::Physical:
            Damage *= 1.0f;
            break;
        case EAttackType::Energetic:
            Damage *= (CombatStats.SpiritualResonance * 0.02f);
            break;
        case EAttackType::Spiritual:
            Damage *= (CombatStats.ConsciousnessLevel * 0.5f);
            break;
        case EAttackType::Transformative:
            Damage *= 0.5f; // Lower damage but higher spiritual impact
            break;
    }
    
    // Apply consciousness modifiers
    float AttackerBonus = CalculateConsciousnessBonus(AttackerState);
    float DefenderReduction = CalculateConsciousnessBonus(DefenderState) * 0.5f;
    
    Damage *= (1.0f + AttackerBonus - DefenderReduction);
    
    // Karmic influence
    if (CombatStats.KarmicBalance < 0.0f)
    {
        Damage *= (1.0f + FMath::Abs(CombatStats.KarmicBalance) * 0.1f); // Negative karma increases damage
    }
    else
    {
        Damage *= (1.0f - CombatStats.KarmicBalance * 0.05f); // Positive karma reduces damage
    }
    
    return FMath::Max(Damage, 1.0f);
}

bool UCombatSystem::CanEngageInCombat(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    // Check if target has combat system
    UCombatSystem* TargetCombat = Target->FindComponentByClass<UCombatSystem>();
    if (!TargetCombat)
    {
        return false;
    }
    
    // High consciousness beings may refuse to fight peaceful targets
    if (CombatStats.ConsciousnessLevel > 7.0f && 
        TargetCombat->GetCombatState() == ECombatState::Peaceful)
    {
        return false;
    }
    
    // Check distance and other conditions
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance < 1000.0f; // Combat range
}

void UCombatSystem::UpdateCombatFromConsciousness(EConsciousnessState ConsciousnessState, float Level)
{
    CombatStats.ConsciousnessLevel = Level;
    
    // Higher consciousness reduces aggressive tendencies
    if (Level > 5.0f && CurrentCombatState == ECombatState::Aggressive)
    {
        SetCombatState(ECombatState::Defensive, CombatIntensity * 0.5f);
    }
    
    // Transcendent consciousness enters transcendent combat state
    if (ConsciousnessState == EConsciousnessState::Unity && Level > 8.0f)
    {
        SetCombatState(ECombatState::Transcendent, 1.0f);
    }
    
    // Update spiritual resonance based on consciousness
    CombatStats.SpiritualResonance = 50.0f + (Level * 10.0f);
}

void UCombatSystem::ApplyKarmicConsequences(EAttackType AttackType, bool bHarmedInnocent, float Damage)
{
    float KarmicImpact = 0.0f;
    
    if (bHarmedInnocent)
    {
        // Negative karma for harming peaceful beings
        KarmicImpact = -(Damage * 0.1f);
        
        if (AttackType == EAttackType::Transformative)
        {
            KarmicImpact *= 0.3f; // Reduced penalty for transformative attacks
        }
    }
    else
    {
        // Slight positive karma for defensive actions
        if (CurrentCombatState == ECombatState::Defensive)
        {
            KarmicImpact = Damage * 0.02f;
        }
    }
    
    // Transformative attacks generate positive karma
    if (AttackType == EAttackType::Transformative)
    {
        KarmicImpact += Damage * 0.05f;
    }
    
    CombatStats.KarmicBalance += KarmicImpact;
    CombatStats.KarmicBalance = FMath::Clamp(CombatStats.KarmicBalance, -100.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Karmic Impact: %.2f, New Balance: %.2f"), 
        KarmicImpact, CombatStats.KarmicBalance);
}

void UCombatSystem::ModifyCombatStats(const FCombatStats& StatModifiers)
{
    CombatStats.PhysicalPower += StatModifiers.PhysicalPower;
    CombatStats.SpiritualResonance += StatModifiers.SpiritualResonance;
    CombatStats.ConsciousnessLevel += StatModifiers.ConsciousnessLevel;
    CombatStats.KarmicBalance += StatModifiers.KarmicBalance;
    
    // Clamp values
    CombatStats.PhysicalPower = FMath::Max(CombatStats.PhysicalPower, 1.0f);
    CombatStats.SpiritualResonance = FMath::Max(CombatStats.SpiritualResonance, 1.0f);
    CombatStats.ConsciousnessLevel = FMath::Max(CombatStats.ConsciousnessLevel, 1.0f);
    CombatStats.KarmicBalance = FMath::Clamp(CombatStats.KarmicBalance, -100.0f, 100.0f);
}

void UCombatSystem::InitializeConsciousnessModifiers()
{
    ConsciousnessCombatModifiers.Add(EConsciousnessState::Ordinary, 0.0f);
    ConsciousnessCombatModifiers.Add(EConsciousnessState::Expanded, 0.2f);
    ConsciousnessCombatModifiers.Add(EConsciousnessState::Transcendent, 0.5f);
    ConsciousnessCombatModifiers.Add(EConsciousnessState::Unity, 1.0f);
    ConsciousnessCombatModifiers.Add(EConsciousnessState::Cosmic, 2.0f);
}

float UCombatSystem::CalculateConsciousnessBonus(EConsciousnessState State) const
{
    if (const float* Modifier = ConsciousnessCombatModifiers.Find(State))
    {
        return *Modifier;
    }
    return 0.0f;
}

void UCombatSystem::ProcessKarmicBalance(float DeltaTime)
{
    // Gradually decay karmic balance towards neutral
    if (FMath::Abs(CombatStats.KarmicBalance) > 0.1f)
    {
        float DecayAmount = KarmicDecayRate * DeltaTime;
        if (CombatStats.KarmicBalance > 0.0f)
        {
            CombatStats.KarmicBalance = FMath::Max(0.0f, CombatStats.KarmicBalance - DecayAmount);
        }
        else
        {
            CombatStats.KarmicBalance = FMath::Min(0.0f, CombatStats.KarmicBalance + DecayAmount);
        }
    }
}