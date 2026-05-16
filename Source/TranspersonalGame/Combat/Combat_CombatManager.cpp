#include "Combat_CombatManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UCombat_CombatManager::UCombat_CombatManager()
{
    CombatWorld = nullptr;
    BaseDamageMultiplier = 1.0f;
    CombatTimeout = 30.0f;
    PackCoordinationRadius = 1500.0f;
}

void UCombat_CombatManager::InitializeCombatManager(UWorld* InWorld)
{
    CombatWorld = InWorld;
    RegisteredCombatants.Empty();
    CombatPairs.Empty();
    CombatTimers.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Manager initialized for world"));
}

void UCombat_CombatManager::RegisterCombatant(APawn* Combatant, ECombat_CombatRole Role)
{
    if (!Combatant || !CombatWorld)
    {
        return;
    }

    RegisteredCombatants.Add(Combatant, Role);
    UE_LOG(LogTemp, Warning, TEXT("Registered combatant: %s with role %d"), 
           *Combatant->GetName(), static_cast<int32>(Role));
}

void UCombat_CombatManager::UnregisterCombatant(APawn* Combatant)
{
    if (!Combatant)
    {
        return;
    }

    // End any active combat
    EndCombat(Combatant);
    
    // Remove from registration
    RegisteredCombatants.Remove(Combatant);
    
    UE_LOG(LogTemp, Warning, TEXT("Unregistered combatant: %s"), *Combatant->GetName());
}

bool UCombat_CombatManager::StartCombat(APawn* Attacker, APawn* Target)
{
    if (!Attacker || !Target || !CombatWorld)
    {
        return false;
    }

    // Check if both are registered
    if (!RegisteredCombatants.Contains(Attacker) || !RegisteredCombatants.Contains(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start combat: unregistered combatants"));
        return false;
    }

    // Set combat pairs
    CombatPairs.Add(Attacker, Target);
    CombatPairs.Add(Target, Attacker);
    
    // Set combat timers
    float CurrentTime = CombatWorld->GetTimeSeconds();
    CombatTimers.Add(Attacker, CurrentTime);
    CombatTimers.Add(Target, CurrentTime);

    // Notify AI systems
    NotifyAIOfCombatState(Attacker, true);
    NotifyAIOfCombatState(Target, true);

    // Broadcast event
    OnCombatStarted.Broadcast(Attacker, Target);

    UE_LOG(LogTemp, Warning, TEXT("Combat started between %s and %s"), 
           *Attacker->GetName(), *Target->GetName());

    return true;
}

void UCombat_CombatManager::EndCombat(APawn* Combatant)
{
    if (!Combatant || !CombatWorld)
    {
        return;
    }

    // Find combat partner
    APawn** PartnerPtr = CombatPairs.Find(Combatant);
    APawn* Partner = PartnerPtr ? *PartnerPtr : nullptr;

    // Remove combat pairs
    CombatPairs.Remove(Combatant);
    if (Partner)
    {
        CombatPairs.Remove(Partner);
    }

    // Remove timers
    CombatTimers.Remove(Combatant);
    if (Partner)
    {
        CombatTimers.Remove(Partner);
    }

    // Notify AI systems
    NotifyAIOfCombatState(Combatant, false);
    if (Partner)
    {
        NotifyAIOfCombatState(Partner, false);
    }

    // Broadcast event
    if (Partner)
    {
        OnCombatEnded.Broadcast(Combatant, Partner);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat ended for %s"), *Combatant->GetName());
}

float UCombat_CombatManager::CalculateDamage(APawn* Attacker, APawn* Target, ECombat_AttackType AttackType)
{
    if (!Attacker || !Target)
    {
        return 0.0f;
    }

    float BaseDamage = 10.0f; // Default base damage

    // Modify damage based on attack type
    switch (AttackType)
    {
        case ECombat_AttackType::Bite:
            BaseDamage = 25.0f;
            break;
        case ECombat_AttackType::Claw:
            BaseDamage = 15.0f;
            break;
        case ECombat_AttackType::Tail:
            BaseDamage = 20.0f;
            break;
        case ECombat_AttackType::Charge:
            BaseDamage = 30.0f;
            break;
        case ECombat_AttackType::Stomp:
            BaseDamage = 35.0f;
            break;
        default:
            BaseDamage = 10.0f;
            break;
    }

    // Apply species modifier
    float SpeciesModifier = CalculateSpeciesDamageModifier(Attacker, Target);
    
    // Apply base multiplier
    float FinalDamage = BaseDamage * SpeciesModifier * BaseDamageMultiplier;

    return FinalDamage;
}

void UCombat_CombatManager::ApplyDamage(APawn* Attacker, APawn* Target, float Damage)
{
    if (!Attacker || !Target || Damage <= 0.0f)
    {
        return;
    }

    // Apply damage to target (basic implementation)
    // In a full system, this would interface with health components
    UE_LOG(LogTemp, Warning, TEXT("%s deals %.1f damage to %s"), 
           *Attacker->GetName(), Damage, *Target->GetName());

    // Broadcast damage event
    OnDamageDealt.Broadcast(Attacker, Target, Damage);

    // Check if target should flee or die
    // This would be expanded with proper health/survival systems
}

bool UCombat_CombatManager::IsInCombat(APawn* Combatant) const
{
    return CombatPairs.Contains(Combatant);
}

APawn* UCombat_CombatManager::GetCombatTarget(APawn* Combatant) const
{
    if (const APawn* const* TargetPtr = CombatPairs.Find(Combatant))
    {
        return *TargetPtr;
    }
    return nullptr;
}

TArray<APawn*> UCombat_CombatManager::GetNearbyEnemies(APawn* Combatant, float Radius) const
{
    TArray<APawn*> NearbyEnemies;
    
    if (!Combatant || !CombatWorld)
    {
        return NearbyEnemies;
    }

    FVector CombatantLocation = Combatant->GetActorLocation();
    ECombat_CombatRole CombatantRole = RegisteredCombatants.FindRef(Combatant);

    for (const auto& Pair : RegisteredCombatants)
    {
        APawn* OtherCombatant = Pair.Key;
        ECombat_CombatRole OtherRole = Pair.Value;

        if (OtherCombatant == Combatant)
        {
            continue;
        }

        // Check if they are enemies (different roles)
        bool bAreEnemies = (CombatantRole == ECombat_CombatRole::Predator && OtherRole == ECombat_CombatRole::Prey) ||
                          (CombatantRole == ECombat_CombatRole::Prey && OtherRole == ECombat_CombatRole::Predator);

        if (bAreEnemies)
        {
            float Distance = FVector::Dist(CombatantLocation, OtherCombatant->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyEnemies.Add(OtherCombatant);
            }
        }
    }

    return NearbyEnemies;
}

float UCombat_CombatManager::GetCombatThreatLevel(APawn* Combatant) const
{
    if (!Combatant)
    {
        return 0.0f;
    }

    TArray<APawn*> NearbyEnemies = GetNearbyEnemies(Combatant, 2000.0f);
    float ThreatLevel = 0.0f;

    for (APawn* Enemy : NearbyEnemies)
    {
        float Distance = FVector::Dist(Combatant->GetActorLocation(), Enemy->GetActorLocation());
        float DistanceThreat = FMath::Max(0.0f, 1.0f - (Distance / 2000.0f));
        ThreatLevel += DistanceThreat;
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UCombat_CombatManager::CoordinatePackAttack(const TArray<APawn*>& PackMembers, APawn* Target)
{
    if (!Target || PackMembers.Num() == 0)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Coordinating pack attack with %d members against %s"), 
           PackMembers.Num(), *Target->GetName());

    // Start combat for all pack members
    for (APawn* PackMember : PackMembers)
    {
        if (PackMember && RegisteredCombatants.Contains(PackMember))
        {
            StartCombat(PackMember, Target);
        }
    }
}

void UCombat_CombatManager::UpdateCombatAI(float DeltaTime)
{
    if (!CombatWorld)
    {
        return;
    }

    CleanupExpiredCombat();

    // Update combat AI logic for all active combatants
    float CurrentTime = CombatWorld->GetTimeSeconds();
    
    for (const auto& Pair : CombatPairs)
    {
        APawn* Combatant = Pair.Key;
        APawn* Target = Pair.Value;
        
        if (Combatant && Target)
        {
            // Calculate distance and update AI behavior
            float Distance = FVector::Dist(Combatant->GetActorLocation(), Target->GetActorLocation());
            
            // If too far apart, end combat
            if (Distance > 5000.0f)
            {
                EndCombat(Combatant);
            }
        }
    }
}

void UCombat_CombatManager::CleanupExpiredCombat()
{
    if (!CombatWorld)
    {
        return;
    }

    float CurrentTime = CombatWorld->GetTimeSeconds();
    TArray<APawn*> ExpiredCombatants;

    for (const auto& Pair : CombatTimers)
    {
        APawn* Combatant = Pair.Key;
        float StartTime = Pair.Value;

        if (CurrentTime - StartTime > CombatTimeout)
        {
            ExpiredCombatants.Add(Combatant);
        }
    }

    for (APawn* ExpiredCombatant : ExpiredCombatants)
    {
        EndCombat(ExpiredCombatant);
    }
}

float UCombat_CombatManager::CalculateSpeciesDamageModifier(APawn* Attacker, APawn* Target)
{
    // Basic species damage modifiers
    // In a full system, this would check actual species types
    
    FString AttackerName = Attacker->GetName().ToLower();
    FString TargetName = Target->GetName().ToLower();

    // T-Rex deals extra damage to everything
    if (AttackerName.Contains(TEXT("trex")) || AttackerName.Contains(TEXT("rex")))
    {
        return 2.0f;
    }

    // Raptors deal extra damage in packs
    if (AttackerName.Contains(TEXT("raptor")))
    {
        TArray<APawn*> NearbyAllies = GetNearbyEnemies(Attacker, PackCoordinationRadius);
        int32 PackSize = 1;
        for (APawn* Ally : NearbyAllies)
        {
            if (Ally->GetName().ToLower().Contains(TEXT("raptor")))
            {
                PackSize++;
            }
        }
        return 1.0f + (PackSize * 0.2f); // +20% per pack member
    }

    // Large herbivores deal reduced damage
    if (AttackerName.Contains(TEXT("brachio")) || AttackerName.Contains(TEXT("trike")))
    {
        return 0.7f;
    }

    return 1.0f; // Default modifier
}

void UCombat_CombatManager::NotifyAIOfCombatState(APawn* Combatant, bool bInCombat)
{
    if (!Combatant)
    {
        return;
    }

    // This would interface with AI controllers and behavior trees
    // For now, just log the state change
    UE_LOG(LogTemp, Warning, TEXT("AI Combat State: %s is %s"), 
           *Combatant->GetName(), bInCombat ? TEXT("entering combat") : TEXT("leaving combat"));
}