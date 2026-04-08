#include "CombatSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

UCombatSystem::UCombatSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default attacks
    FAttackData BasicAttack;
    BasicAttack.Damage = 15.0f;
    BasicAttack.AttackType = EAttackType::Physical;
    BasicAttack.Range = 150.0f;
    BasicAttack.Cooldown = 1.5f;
    BasicAttack.ConsciousnessImpact = 2.0f;
    
    FAttackData PsychicAttack;
    PsychicAttack.Damage = 25.0f;
    PsychicAttack.AttackType = EAttackType::Psychic;
    PsychicAttack.Range = 300.0f;
    PsychicAttack.Cooldown = 3.0f;
    PsychicAttack.ConsciousnessImpact = 10.0f;
    
    AvailableAttacks.Add(BasicAttack);
    AvailableAttacks.Add(PsychicAttack);
}

void UCombatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find consciousness component
    ConsciousnessComponent = GetOwner()->FindComponentByClass<UConsciousnessComponent>();
    
    if (!ConsciousnessComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatSystem: No ConsciousnessComponent found on %s"), *GetOwner()->GetName());
    }
}

void UCombatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateCombatBehavior(DeltaTime);
    HandleConsciousnessIntegration();
}

void UCombatSystem::UpdateCombatBehavior(float DeltaTime)
{
    if (!GetOwner())
        return;

    switch (CurrentCombatState)
    {
        case ECombatState::Peaceful:
        {
            // Look for targets
            FindNearestTarget();
            if (CurrentTarget && IsTargetInRange(DetectionRange))
            {
                SetCombatState(ECombatState::Alert);
            }
            break;
        }
        
        case ECombatState::Alert:
        {
            if (!CurrentTarget || !IsTargetInRange(DetectionRange * 1.2f))
            {
                SetCombatState(ECombatState::Peaceful);
                CurrentTarget = nullptr;
            }
            else if (IsTargetInRange(AttackRange))
            {
                SetCombatState(ECombatState::Combat);
            }
            break;
        }
        
        case ECombatState::Combat:
        {
            if (!CurrentTarget || !IsTargetInRange(DetectionRange * 1.5f))
            {
                SetCombatState(ECombatState::Alert);
            }
            else if (CanAttack() && IsTargetInRange(AttackRange))
            {
                // Select best attack based on consciousness state
                FAttackData BestAttack = SelectBestAttack();
                for (int32 i = 0; i < AvailableAttacks.Num(); i++)
                {
                    if (AvailableAttacks[i].AttackType == BestAttack.AttackType &&
                        AvailableAttacks[i].Damage == BestAttack.Damage)
                    {
                        PerformAttack(i);
                        break;
                    }
                }
            }
            
            // Check if should enter meditative state
            if (ShouldEnterMeditativeState())
            {
                SetCombatState(ECombatState::Meditating);
            }
            break;
        }
        
        case ECombatState::Retreating:
        {
            // Move away from target (implementation depends on movement system)
            if (!CurrentTarget || !IsTargetInRange(DetectionRange * 2.0f))
            {
                SetCombatState(ECombatState::Peaceful);
                CurrentTarget = nullptr;
            }
            break;
        }
        
        case ECombatState::Meditating:
        {
            // Restore consciousness and health slowly
            if (ConsciousnessComponent)
            {
                float CurrentConsciousness = ConsciousnessComponent->GetConsciousnessLevel();
                ConsciousnessComponent->ModifyConsciousness(DeltaTime * 5.0f);
            }
            
            Health = FMath::Min(Health + DeltaTime * 10.0f, MaxHealth);
            
            // Exit meditation if attacked or consciousness restored
            if (CurrentTarget && IsTargetInRange(AttackRange))
            {
                SetCombatState(ECombatState::Combat);
            }
            else if (ConsciousnessComponent && ConsciousnessComponent->GetConsciousnessLevel() > 80.0f)
            {
                SetCombatState(ECombatState::Peaceful);
            }
            break;
        }
    }
}

void UCombatSystem::HandleConsciousnessIntegration()
{
    if (!ConsciousnessComponent)
        return;
        
    float ConsciousnessLevel = ConsciousnessComponent->GetConsciousnessLevel();
    
    // Adjust behavior based on consciousness level
    if (ConsciousnessLevel < 20.0f)
    {
        // Low consciousness - more aggressive, less strategic
        DetectionRange = 600.0f;
        AttackRange = 200.0f;
    }
    else if (ConsciousnessLevel > 80.0f)
    {
        // High consciousness - more peaceful, strategic
        DetectionRange = 300.0f;
        AttackRange = 100.0f;
        
        // Chance to avoid combat entirely
        if (CurrentCombatState == ECombatState::Alert && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            SetCombatState(ECombatState::Peaceful);
            CurrentTarget = nullptr;
        }
    }
}

void UCombatSystem::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState OldState = CurrentCombatState;
        CurrentCombatState = NewState;
        OnCombatStateChanged(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), (int32)OldState, (int32)NewState);
    }
}

bool UCombatSystem::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= 1.0f; // Minimum cooldown
}

void UCombatSystem::PerformAttack(int32 AttackIndex)
{
    if (!AvailableAttacks.IsValidIndex(AttackIndex) || !CurrentTarget)
        return;
        
    const FAttackData& Attack = AvailableAttacks[AttackIndex];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if ((CurrentTime - LastAttackTime) < Attack.Cooldown)
        return;
        
    // Apply consciousness modifiers
    float ModifiedDamage = Attack.Damage * GetConsciousnessAttackModifier();
    
    // Deal damage to target
    if (UCombatSystem* TargetCombat = CurrentTarget->FindComponentByClass<UCombatSystem>())
    {
        TargetCombat->TakeDamage(ModifiedDamage, Attack.AttackType, GetOwner());
    }
    
    // Affect own consciousness
    if (ConsciousnessComponent)
    {
        ConsciousnessComponent->ModifyConsciousness(-Attack.ConsciousnessImpact);
    }
    
    LastAttackTime = CurrentTime;
    OnAttackPerformed(Attack);
    
    UE_LOG(LogTemp, Log, TEXT("Attack performed: %f damage, type %d"), ModifiedDamage, (int32)Attack.AttackType);
}

void UCombatSystem::TakeDamage(float DamageAmount, EAttackType AttackType, AActor* Attacker)
{
    float ModifiedDamage = DamageAmount * (1.0f - GetConsciousnessDefenseModifier());
    Health = FMath::Max(0.0f, Health - ModifiedDamage);
    
    // Set attacker as target if we don't have one
    if (!CurrentTarget && Attacker)
    {
        CurrentTarget = Attacker;
    }
    
    // Change to combat state if not already
    if (CurrentCombatState == ECombatState::Peaceful || CurrentCombatState == ECombatState::Meditating)
    {
        SetCombatState(ECombatState::Alert);
    }
    
    // Affect consciousness based on attack type
    if (ConsciousnessComponent)
    {
        float ConsciousnessImpact = 0.0f;
        switch (AttackType)
        {
            case EAttackType::Physical:
                ConsciousnessImpact = -2.0f;
                break;
            case EAttackType::Psychic:
                ConsciousnessImpact = -8.0f;
                break;
            case EAttackType::Spiritual:
                ConsciousnessImpact = -5.0f;
                break;
            case EAttackType::Elemental:
                ConsciousnessImpact = -3.0f;
                break;
        }
        
        ConsciousnessComponent->ModifyConsciousness(ConsciousnessImpact);
    }
    
    OnDamageTaken(ModifiedDamage, AttackType);
    
    // Check if should retreat
    if (Health < MaxHealth * 0.25f)
    {
        SetCombatState(ECombatState::Retreating);
    }
}

void UCombatSystem::FindNearestTarget()
{
    if (!GetOwner())
        return;
        
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    float NearestDistance = DetectionRange;
    AActor* NearestTarget = nullptr;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
            
        // Only target actors with combat systems (potential enemies)
        if (!Actor->FindComponentByClass<UCombatSystem>())
            continue;
            
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestTarget = Actor;
        }
    }
    
    if (NearestTarget != CurrentTarget)
    {
        CurrentTarget = NearestTarget;
        if (CurrentTarget)
        {
            OnTargetFound(CurrentTarget);
        }
    }
}

float UCombatSystem::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return -1.0f;
        
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UCombatSystem::IsTargetInRange(float Range) const
{
    float Distance = GetDistanceToTarget();
    return Distance >= 0.0f && Distance <= Range;
}

float UCombatSystem::GetConsciousnessAttackModifier() const
{
    if (!ConsciousnessComponent)
        return 1.0f;
        
    float ConsciousnessLevel = ConsciousnessComponent->GetConsciousnessLevel();
    
    // Lower consciousness = more aggressive but less precise
    // Higher consciousness = more controlled and effective
    if (ConsciousnessLevel < 30.0f)
    {
        return 1.2f; // 20% more damage but less strategic
    }
    else if (ConsciousnessLevel > 70.0f)
    {
        return 1.1f; // 10% more damage with better strategy
    }
    
    return 1.0f;
}

float UCombatSystem::GetConsciousnessDefenseModifier() const
{
    if (!ConsciousnessComponent)
        return 0.0f;
        
    float ConsciousnessLevel = ConsciousnessComponent->GetConsciousnessLevel();
    
    // Higher consciousness provides better damage resistance
    return FMath::Clamp(ConsciousnessLevel / 200.0f, 0.0f, 0.5f); // Max 50% damage reduction
}

bool UCombatSystem::ShouldEnterMeditativeState() const
{
    if (!ConsciousnessComponent)
        return false;
        
    float ConsciousnessLevel = ConsciousnessComponent->GetConsciousnessLevel();
    float HealthPercentage = Health / MaxHealth;
    
    // Enter meditation if consciousness is low and health is not critical
    return ConsciousnessLevel < 30.0f && HealthPercentage > 0.4f;
}

FAttackData UCombatSystem::SelectBestAttack() const
{
    if (AvailableAttacks.Num() == 0)
        return FAttackData();
        
    if (!ConsciousnessComponent)
        return AvailableAttacks[0];
        
    float ConsciousnessLevel = ConsciousnessComponent->GetConsciousnessLevel();
    
    // Low consciousness prefers physical attacks
    if (ConsciousnessLevel < 40.0f)
    {
        for (const FAttackData& Attack : AvailableAttacks)
        {
            if (Attack.AttackType == EAttackType::Physical)
                return Attack;
        }
    }
    // High consciousness can use psychic attacks effectively
    else if (ConsciousnessLevel > 60.0f)
    {
        for (const FAttackData& Attack : AvailableAttacks)
        {
            if (Attack.AttackType == EAttackType::Psychic)
                return Attack;
        }
    }
    
    return AvailableAttacks[0];
}