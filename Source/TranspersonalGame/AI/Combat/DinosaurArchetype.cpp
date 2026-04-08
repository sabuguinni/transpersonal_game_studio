#include "DinosaurArchetype.h"

UDinosaurArchetype::UDinosaurArchetype()
{
    // Default values
    DinosaurName = TEXT("Unknown Dinosaur");
    ScientificName = TEXT("Dinosaurus unknownus");
    Size = EDinosaurSize::Medium;
    BehaviorType = EDinosaurBehaviorType::Opportunist;
    PrimaryAttackPattern = EAttackPattern::DirectCharge;
    
    // Default combat stats
    CombatStats.Health = 100.0f;
    CombatStats.AttackDamage = 25.0f;
    CombatStats.AttackRange = 200.0f;
    CombatStats.MovementSpeed = 400.0f;
    CombatStats.DetectionRange = 1500.0f;
    CombatStats.AttackCooldown = 2.0f;
    CombatStats.FleeHealthThreshold = 0.3f;
    
    // Default tactical behavior
    TacticalBehavior.AggressionLevel = 0.7f;
    TacticalBehavior.CautiousDistance = 800.0f;
    TacticalBehavior.OptimalAttackDistance = 300.0f;
    TacticalBehavior.bUsesAmbushTactics = false;
    TacticalBehavior.bCanCoordinateWithPack = false;
    TacticalBehavior.bTerritorialBehavior = false;
    TacticalBehavior.TerritoryRadius = 1000.0f;
    TacticalBehavior.PatrolRadius = 500.0f;
}

float UDinosaurArchetype::GetThreatLevel() const
{
    float ThreatLevel = 1.0f;
    
    // Base threat on size
    switch (Size)
    {
        case EDinosaurSize::Small:
            ThreatLevel = 0.3f;
            break;
        case EDinosaurSize::Medium:
            ThreatLevel = 0.6f;
            break;
        case EDinosaurSize::Large:
            ThreatLevel = 0.8f;
            break;
        case EDinosaurSize::Massive:
            ThreatLevel = 1.0f;
            break;
    }
    
    // Modify based on behavior type
    switch (BehaviorType)
    {
        case EDinosaurBehaviorType::Ambush:
            ThreatLevel *= 1.2f; // Ambush predators are more dangerous
            break;
        case EDinosaurBehaviorType::Pack:
            ThreatLevel *= 1.5f; // Pack hunters are very dangerous
            break;
        case EDinosaurBehaviorType::Apex:
            ThreatLevel *= 1.3f; // Apex predators are formidable
            break;
        case EDinosaurBehaviorType::Territorial:
            ThreatLevel *= 1.1f; // Slightly more dangerous when defending territory
            break;
        case EDinosaurBehaviorType::Opportunist:
            ThreatLevel *= 0.9f; // Slightly less dangerous as they're cautious
            break;
    }
    
    // Factor in aggression level
    ThreatLevel *= TacticalBehavior.AggressionLevel;
    
    return FMath::Clamp(ThreatLevel, 0.1f, 2.0f);
}

FPrimaryAssetId UDinosaurArchetype::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(FPrimaryAssetType("DinosaurArchetype"), GetFName());
}