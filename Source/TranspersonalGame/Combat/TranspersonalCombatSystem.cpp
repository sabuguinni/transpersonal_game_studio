#include "TranspersonalCombatSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "../AI/ConsciousnessAIComponent.h"

UTranspersonalCombatSystem::UTranspersonalCombatSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentCombatState = ECombatState::Peaceful;
    ConsciousnessLevel = 0.5f;
    TranscendentThreshold = 0.8f;
    ShadowThreshold = 0.2f;

    // Initialize default attacks
    FConsciousnessAttack BasicPhysical;
    BasicPhysical.AttackType = EConsciousnessAttackType::Physical;
    BasicPhysical.Damage = 15.0f;
    BasicPhysical.ConsciousnessImpact = 2.0f;
    BasicPhysical.Range = 150.0f;
    BasicPhysical.CooldownTime = 1.5f;
    BasicPhysical.AttackName = TEXT("Physical Strike");
    BasicPhysical.Description = TEXT("A basic physical attack with minimal consciousness impact");
    AvailableAttacks.Add(BasicPhysical);

    FConsciousnessAttack EmotionalWave;
    EmotionalWave.AttackType = EConsciousnessAttackType::Emotional;
    EmotionalWave.Damage = 8.0f;
    EmotionalWave.ConsciousnessImpact = 12.0f;
    EmotionalWave.Range = 300.0f;
    EmotionalWave.CooldownTime = 3.0f;
    EmotionalWave.AttackName = TEXT("Emotional Wave");
    EmotionalWave.Description = TEXT("Sends waves of emotional energy that affect consciousness");
    AvailableAttacks.Add(EmotionalWave);
}

void UTranspersonalCombatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize defensive state based on consciousness level
    UpdateCombatBasedOnConsciousness(ConsciousnessLevel);
}

void UTranspersonalCombatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAttackCooldowns(DeltaTime);
}

void UTranspersonalCombatSystem::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState OldState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        // Adjust defensive capabilities based on state
        switch (NewState)
        {
            case ECombatState::Peaceful:
                DefensiveState.PhysicalResistance = 0.05f;
                DefensiveState.EmotionalResistance = 0.3f;
                DefensiveState.MentalResistance = 0.2f;
                DefensiveState.SpiritualResistance = 0.4f;
                break;
                
            case ECombatState::Defensive:
                DefensiveState.PhysicalResistance = 0.3f;
                DefensiveState.EmotionalResistance = 0.2f;
                DefensiveState.MentalResistance = 0.3f;
                DefensiveState.SpiritualResistance = 0.2f;
                break;
                
            case ECombatState::Aggressive:
                DefensiveState.PhysicalResistance = 0.1f;
                DefensiveState.EmotionalResistance = 0.1f;
                DefensiveState.MentalResistance = 0.1f;
                DefensiveState.SpiritualResistance = 0.1f;
                break;
                
            case ECombatState::Transcendent:
                DefensiveState.PhysicalResistance = 0.6f;
                DefensiveState.EmotionalResistance = 0.8f;
                DefensiveState.MentalResistance = 0.9f;
                DefensiveState.SpiritualResistance = 0.95f;
                break;
                
            case ECombatState::Shadow:
                DefensiveState.PhysicalResistance = 0.2f;
                DefensiveState.EmotionalResistance = 0.05f;
                DefensiveState.MentalResistance = 0.1f;
                DefensiveState.SpiritualResistance = 0.05f;
                break;
        }
        
        OnCombatStateChanged.Broadcast(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), (int32)OldState, (int32)NewState);
    }
}

bool UTranspersonalCombatSystem::ExecuteConsciousnessAttack(AActor* Target, FConsciousnessAttack Attack)
{
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot execute attack: Target is null"));
        return false;
    }

    // Check cooldown
    float* LastUsed = AttackCooldowns.Find(Attack.AttackName);
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (LastUsed && (CurrentTime - *LastUsed) < Attack.CooldownTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attack %s is on cooldown"), *Attack.AttackName);
        return false;
    }

    // Check range
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Distance > Attack.Range)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target is out of range for attack %s"), *Attack.AttackName);
        return false;
    }

    // Calculate consciousness modifier
    float ConsciousnessModifier = CalculateConsciousnessModifier(Attack.AttackType);
    
    // Apply consciousness modifier to damage
    float ModifiedDamage = Attack.Damage * ConsciousnessModifier;
    float ModifiedConsciousnessImpact = Attack.ConsciousnessImpact * ConsciousnessModifier;

    // Get target's combat system
    UTranspersonalCombatSystem* TargetCombatSystem = Target->FindComponentByClass<UTranspersonalCombatSystem>();
    if (TargetCombatSystem)
    {
        float DamageReduction = TargetCombatSystem->CalculateDamageReduction(Attack.AttackType);
        ModifiedDamage *= (1.0f - DamageReduction);
        
        if (DamageReduction > 0.0f)
        {
            TargetCombatSystem->OnConsciousnessDefense.Broadcast(Target, ModifiedDamage * DamageReduction);
        }
    }

    // Apply consciousness impact to target
    UConsciousnessAIComponent* TargetConsciousness = Target->FindComponentByClass<UConsciousnessAIComponent>();
    if (TargetConsciousness)
    {
        // Consciousness attacks can either raise or lower consciousness depending on type
        float ConsciousnessChange = ModifiedConsciousnessImpact;
        if (Attack.AttackType == EConsciousnessAttackType::Shadow)
        {
            ConsciousnessChange = -ModifiedConsciousnessImpact;
        }
        
        // This would need to be implemented in the ConsciousnessAIComponent
        // TargetConsciousness->ModifyConsciousnessLevel(ConsciousnessChange);
    }

    // Record cooldown
    AttackCooldowns.Add(Attack.AttackName, CurrentTime);

    // Broadcast attack event
    OnConsciousnessAttack.Broadcast(GetOwner(), Target, Attack);

    UE_LOG(LogTemp, Log, TEXT("Executed %s attack on %s for %f damage"), 
           *Attack.AttackName, *Target->GetName(), ModifiedDamage);

    return true;
}

void UTranspersonalCombatSystem::AddAvailableAttack(FConsciousnessAttack NewAttack)
{
    // Check if attack already exists
    for (int32 i = 0; i < AvailableAttacks.Num(); i++)
    {
        if (AvailableAttacks[i].AttackName == NewAttack.AttackName)
        {
            AvailableAttacks[i] = NewAttack; // Update existing
            return;
        }
    }
    
    AvailableAttacks.Add(NewAttack);
    UE_LOG(LogTemp, Log, TEXT("Added new attack: %s"), *NewAttack.AttackName);
}

float UTranspersonalCombatSystem::CalculateDamageReduction(EConsciousnessAttackType AttackType)
{
    float BaseReduction = 0.0f;
    
    switch (AttackType)
    {
        case EConsciousnessAttackType::Physical:
            BaseReduction = DefensiveState.PhysicalResistance;
            break;
        case EConsciousnessAttackType::Emotional:
            BaseReduction = DefensiveState.EmotionalResistance;
            break;
        case EConsciousnessAttackType::Mental:
            BaseReduction = DefensiveState.MentalResistance;
            break;
        case EConsciousnessAttackType::Spiritual:
        case EConsciousnessAttackType::Light:
            BaseReduction = DefensiveState.SpiritualResistance;
            break;
        case EConsciousnessAttackType::Shadow:
            // Shadow attacks bypass most defenses but are reduced by high consciousness
            BaseReduction = FMath::Clamp(ConsciousnessLevel - 0.5f, 0.0f, 0.5f);
            break;
    }

    // Apply shield bonus
    if (DefensiveState.bIsShielded)
    {
        BaseReduction += DefensiveState.ShieldStrength;
    }

    return FMath::Clamp(BaseReduction, 0.0f, 0.95f); // Max 95% reduction
}

void UTranspersonalCombatSystem::ActivateConsciousnessShield(float Strength, float Duration)
{
    DefensiveState.bIsShielded = true;
    DefensiveState.ShieldStrength = Strength;
    
    // Clear existing timer
    if (GetWorld()->GetTimerManager().IsTimerActive(ShieldTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(ShieldTimerHandle);
    }
    
    // Set new timer
    GetWorld()->GetTimerManager().SetTimer(ShieldTimerHandle, this, 
                                          &UTranspersonalCombatSystem::OnShieldExpired, 
                                          Duration, false);
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness shield activated: Strength %f, Duration %f"), Strength, Duration);
}

void UTranspersonalCombatSystem::DeactivateConsciousnessShield()
{
    DefensiveState.bIsShielded = false;
    DefensiveState.ShieldStrength = 0.0f;
    
    if (GetWorld()->GetTimerManager().IsTimerActive(ShieldTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(ShieldTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness shield deactivated"));
}

void UTranspersonalCombatSystem::UpdateCombatBasedOnConsciousness(float NewConsciousnessLevel)
{
    ConsciousnessLevel = FMath::Clamp(NewConsciousnessLevel, 0.0f, 1.0f);
    
    // Determine combat state based on consciousness level
    ECombatState NewState = CurrentCombatState;
    
    if (ConsciousnessLevel >= TranscendentThreshold)
    {
        NewState = ECombatState::Transcendent;
    }
    else if (ConsciousnessLevel <= ShadowThreshold)
    {
        NewState = ECombatState::Shadow;
    }
    else if (CurrentCombatState == ECombatState::Transcendent || CurrentCombatState == ECombatState::Shadow)
    {
        // Return to peaceful state when leaving extreme consciousness levels
        NewState = ECombatState::Peaceful;
    }
    
    SetCombatState(NewState);
    
    // Update available attacks based on consciousness level
    UpdateAvailableAttacksForConsciousness();
}

bool UTranspersonalCombatSystem::CanUseTranscendentAbilities() const
{
    return ConsciousnessLevel >= TranscendentThreshold && CurrentCombatState == ECombatState::Transcendent;
}

void UTranspersonalCombatSystem::UpdateAttackCooldowns(float DeltaTime)
{
    // This is handled by checking timestamps in ExecuteConsciousnessAttack
    // Could implement visual cooldown indicators here
}

void UTranspersonalCombatSystem::OnShieldExpired()
{
    DeactivateConsciousnessShield();
}

float UTranspersonalCombatSystem::CalculateConsciousnessModifier(EConsciousnessAttackType AttackType)
{
    float Modifier = 1.0f;
    
    switch (AttackType)
    {
        case EConsciousnessAttackType::Physical:
            // Physical attacks are less effective at higher consciousness
            Modifier = 1.2f - (ConsciousnessLevel * 0.4f);
            break;
            
        case EConsciousnessAttackType::Emotional:
            // Emotional attacks are most effective at mid-consciousness
            Modifier = 1.0f + (0.5f - FMath::Abs(ConsciousnessLevel - 0.5f));
            break;
            
        case EConsciousnessAttackType::Mental:
            // Mental attacks scale with consciousness
            Modifier = 0.8f + (ConsciousnessLevel * 0.6f);
            break;
            
        case EConsciousnessAttackType::Spiritual:
        case EConsciousnessAttackType::Light:
            // Spiritual/Light attacks are most effective at high consciousness
            Modifier = 0.5f + (ConsciousnessLevel * 1.0f);
            break;
            
        case EConsciousnessAttackType::Shadow:
            // Shadow attacks are most effective at low consciousness
            Modifier = 1.5f - (ConsciousnessLevel * 1.0f);
            break;
    }
    
    return FMath::Clamp(Modifier, 0.1f, 2.0f);
}

void UTranspersonalCombatSystem::UpdateAvailableAttacksForConsciousness()
{
    // Remove consciousness-specific attacks that are no longer available
    for (int32 i = AvailableAttacks.Num() - 1; i >= 0; i--)
    {
        bool ShouldRemove = false;
        
        if (AvailableAttacks[i].AttackType == EConsciousnessAttackType::Light && 
            ConsciousnessLevel < TranscendentThreshold)
        {
            ShouldRemove = true;
        }
        else if (AvailableAttacks[i].AttackName.Contains(TEXT("Transcendent")) && 
                 !CanUseTranscendentAbilities())
        {
            ShouldRemove = true;
        }
        
        if (ShouldRemove)
        {
            UE_LOG(LogTemp, Log, TEXT("Removed attack %s due to consciousness level"), 
                   *AvailableAttacks[i].AttackName);
            AvailableAttacks.RemoveAt(i);
        }
    }
    
    // Add new attacks based on consciousness level
    if (CanUseTranscendentAbilities())
    {
        // Check if we already have transcendent attacks
        bool HasTranscendentAttack = false;
        for (const FConsciousnessAttack& Attack : AvailableAttacks)
        {
            if (Attack.AttackName.Contains(TEXT("Transcendent")))
            {
                HasTranscendentAttack = true;
                break;
            }
        }
        
        if (!HasTranscendentAttack)
        {
            FConsciousnessAttack TranscendentLight;
            TranscendentLight.AttackType = EConsciousnessAttackType::Light;
            TranscendentLight.Damage = 5.0f;
            TranscendentLight.ConsciousnessImpact = 25.0f;
            TranscendentLight.Range = 500.0f;
            TranscendentLight.CooldownTime = 8.0f;
            TranscendentLight.AttackName = TEXT("Transcendent Light");
            TranscendentLight.Description = TEXT("A powerful light-based attack that elevates consciousness");
            AddAvailableAttack(TranscendentLight);
        }
    }
    
    if (ConsciousnessLevel <= ShadowThreshold)
    {
        // Add shadow attacks for low consciousness
        bool HasShadowAttack = false;
        for (const FConsciousnessAttack& Attack : AvailableAttacks)
        {
            if (Attack.AttackType == EConsciousnessAttackType::Shadow)
            {
                HasShadowAttack = true;
                break;
            }
        }
        
        if (!HasShadowAttack)
        {
            FConsciousnessAttack ShadowDrain;
            ShadowDrain.AttackType = EConsciousnessAttackType::Shadow;
            ShadowDrain.Damage = 12.0f;
            ShadowDrain.ConsciousnessImpact = -15.0f;
            ShadowDrain.Range = 200.0f;
            ShadowDrain.CooldownTime = 4.0f;
            ShadowDrain.AttackName = TEXT("Shadow Drain");
            ShadowDrain.Description = TEXT("Drains consciousness and vitality from the target");
            AddAvailableAttack(ShadowDrain);
        }
    }
}