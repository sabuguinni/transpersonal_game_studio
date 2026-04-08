#include "DinosaurArchetype.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurArchetypeLibrary::UDinosaurArchetypeLibrary()
{
    // Initialize default archetypes on construction
    InitializeDefaultArchetypes();
}

void UDinosaurArchetypeLibrary::InitializeDefaultArchetypes()
{
    CreateSmallHerbivoreArchetype();
    CreateLargeCarnivoreArchetype();
    CreatePackHunterArchetype();
    CreateGentleGiantArchetype();
    CreateSkittishPreyArchetype();
    CreateApexPredatorArchetype();
    CreateSocialHerbivoreArchetype();
    CreateSolitaryScavengerArchetype();
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurArchetypeLibrary: Initialized %d default archetypes"), Archetypes.Num());
}

FDinosaurArchetype UDinosaurArchetypeLibrary::GetArchetype(const FString& ArchetypeName)
{
    if (Archetypes.Contains(ArchetypeName))
    {
        return Archetypes[ArchetypeName];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurArchetypeLibrary: Archetype '%s' not found, returning default"), *ArchetypeName);
    
    // Return a default archetype if not found
    FDinosaurArchetype DefaultArchetype;
    DefaultArchetype.ArchetypeName = TEXT("Default");
    return DefaultArchetype;
}

FCombatPersonality UDinosaurArchetypeLibrary::CreatePersonalityVariation(const FDinosaurArchetype& Archetype, float VariationAmount)
{
    FCombatPersonality Personality = Archetype.BaseCombatPersonality;
    
    // Apply random variations within the specified range
    float Variation = VariationAmount * 0.5f; // Half range for +/- variation
    
    Personality.Aggression = FMath::Clamp(Personality.Aggression + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    Personality.Intelligence = FMath::Clamp(Personality.Intelligence + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    Personality.Patience = FMath::Clamp(Personality.Patience + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    Personality.Cooperation = FMath::Clamp(Personality.Cooperation + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    Personality.Caution = FMath::Clamp(Personality.Caution + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    
    return Personality;
}

void UDinosaurArchetypeLibrary::CreateSmallHerbivoreArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("SmallHerbivore");
    Archetype.Size = EDinosaurSize::Small;
    Archetype.Diet = EDinosaurDiet::Herbivore;
    Archetype.SocialBehavior = ESocialPattern::SmallGroup;
    Archetype.bCanBeDomesticated = true;
    Archetype.TerritoryRadius = 300.0f;
    Archetype.DetectionRange = 800.0f;
    Archetype.FleeThreshold = 0.8f; // Very skittish
    
    // Combat personality - passive and cautious
    Archetype.BaseCombatPersonality.Aggression = 0.1f;
    Archetype.BaseCombatPersonality.Intelligence = 0.6f;
    Archetype.BaseCombatPersonality.Patience = 0.3f;
    Archetype.BaseCombatPersonality.Cooperation = 0.8f;
    Archetype.BaseCombatPersonality.Caution = 0.9f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 800.0f;
    Archetype.CombatCapabilities.DetectionRadius = 800.0f;
    Archetype.CombatCapabilities.AttackRange = 100.0f;
    Archetype.CombatCapabilities.BaseDamage = 10.0f;
    Archetype.CombatCapabilities.Stamina = 80.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = false;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Opportunistic;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreateLargeCarnivoreArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("LargeCarnivore");
    Archetype.Size = EDinosaurSize::Large;
    Archetype.Diet = EDinosaurDiet::Carnivore;
    Archetype.SocialBehavior = ESocialPattern::Solitary;
    Archetype.bCanBeDomesticated = false;
    Archetype.TerritoryRadius = 2000.0f;
    Archetype.DetectionRange = 1500.0f;
    Archetype.FleeThreshold = 0.2f; // Rarely flees
    
    // Combat personality - aggressive and intelligent
    Archetype.BaseCombatPersonality.Aggression = 0.8f;
    Archetype.BaseCombatPersonality.Intelligence = 0.7f;
    Archetype.BaseCombatPersonality.Patience = 0.6f;
    Archetype.BaseCombatPersonality.Cooperation = 0.2f;
    Archetype.BaseCombatPersonality.Caution = 0.3f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 600.0f;
    Archetype.CombatCapabilities.DetectionRadius = 1500.0f;
    Archetype.CombatCapabilities.AttackRange = 300.0f;
    Archetype.CombatCapabilities.BaseDamage = 80.0f;
    Archetype.CombatCapabilities.Stamina = 150.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = true;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Ambush;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreatePackHunterArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("PackHunter");
    Archetype.Size = EDinosaurSize::Medium;
    Archetype.Diet = EDinosaurDiet::Carnivore;
    Archetype.SocialBehavior = ESocialPattern::SmallGroup;
    Archetype.bCanBeDomesticated = false;
    Archetype.TerritoryRadius = 1500.0f;
    Archetype.DetectionRange = 1200.0f;
    Archetype.FleeThreshold = 0.4f;
    
    // Combat personality - cooperative and intelligent
    Archetype.BaseCombatPersonality.Aggression = 0.7f;
    Archetype.BaseCombatPersonality.Intelligence = 0.8f;
    Archetype.BaseCombatPersonality.Patience = 0.7f;
    Archetype.BaseCombatPersonality.Cooperation = 0.9f;
    Archetype.BaseCombatPersonality.Caution = 0.5f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 700.0f;
    Archetype.CombatCapabilities.DetectionRadius = 1200.0f;
    Archetype.CombatCapabilities.AttackRange = 200.0f;
    Archetype.CombatCapabilities.BaseDamage = 45.0f;
    Archetype.CombatCapabilities.Stamina = 120.0f;
    Archetype.CombatCapabilities.bCanPackHunt = true;
    Archetype.CombatCapabilities.bCanAmbush = true;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Pack;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreateGentleGiantArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("GentleGiant");
    Archetype.Size = EDinosaurSize::Massive;
    Archetype.Diet = EDinosaurDiet::Herbivore;
    Archetype.SocialBehavior = ESocialPattern::Herd;
    Archetype.bCanBeDomesticated = true;
    Archetype.TerritoryRadius = 1000.0f;
    Archetype.DetectionRange = 600.0f;
    Archetype.FleeThreshold = 0.6f;
    
    // Combat personality - defensive but powerful
    Archetype.BaseCombatPersonality.Aggression = 0.3f;
    Archetype.BaseCombatPersonality.Intelligence = 0.5f;
    Archetype.BaseCombatPersonality.Patience = 0.8f;
    Archetype.BaseCombatPersonality.Cooperation = 0.7f;
    Archetype.BaseCombatPersonality.Caution = 0.6f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 400.0f;
    Archetype.CombatCapabilities.DetectionRadius = 600.0f;
    Archetype.CombatCapabilities.AttackRange = 400.0f;
    Archetype.CombatCapabilities.BaseDamage = 100.0f;
    Archetype.CombatCapabilities.Stamina = 200.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = false;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Territorial;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreateSkittishPreyArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("SkittishPrey");
    Archetype.Size = EDinosaurSize::Small;
    Archetype.Diet = EDinosaurDiet::Herbivore;
    Archetype.SocialBehavior = ESocialPattern::Herd;
    Archetype.bCanBeDomesticated = true;
    Archetype.TerritoryRadius = 200.0f;
    Archetype.DetectionRange = 1000.0f;
    Archetype.FleeThreshold = 0.9f; // Extremely skittish
    
    // Combat personality - very cautious and cooperative
    Archetype.BaseCombatPersonality.Aggression = 0.05f;
    Archetype.BaseCombatPersonality.Intelligence = 0.7f;
    Archetype.BaseCombatPersonality.Patience = 0.2f;
    Archetype.BaseCombatPersonality.Cooperation = 0.9f;
    Archetype.BaseCombatPersonality.Caution = 0.95f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 900.0f;
    Archetype.CombatCapabilities.DetectionRadius = 1000.0f;
    Archetype.CombatCapabilities.AttackRange = 80.0f;
    Archetype.CombatCapabilities.BaseDamage = 5.0f;
    Archetype.CombatCapabilities.Stamina = 60.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = false;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Opportunistic;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreateApexPredatorArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("ApexPredator");
    Archetype.Size = EDinosaurSize::Massive;
    Archetype.Diet = EDinosaurDiet::Carnivore;
    Archetype.SocialBehavior = ESocialPattern::Solitary;
    Archetype.bCanBeDomesticated = false;
    Archetype.TerritoryRadius = 3000.0f;
    Archetype.DetectionRange = 2000.0f;
    Archetype.FleeThreshold = 0.1f; // Almost never flees
    
    // Combat personality - extremely aggressive and intelligent
    Archetype.BaseCombatPersonality.Aggression = 0.9f;
    Archetype.BaseCombatPersonality.Intelligence = 0.8f;
    Archetype.BaseCombatPersonality.Patience = 0.8f;
    Archetype.BaseCombatPersonality.Cooperation = 0.1f;
    Archetype.BaseCombatPersonality.Caution = 0.2f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 500.0f;
    Archetype.CombatCapabilities.DetectionRadius = 2000.0f;
    Archetype.CombatCapabilities.AttackRange = 500.0f;
    Archetype.CombatCapabilities.BaseDamage = 150.0f;
    Archetype.CombatCapabilities.Stamina = 250.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = true;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Persistence;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreateSocialHerbivoreArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("SocialHerbivore");
    Archetype.Size = EDinosaurSize::Medium;
    Archetype.Diet = EDinosaurDiet::Herbivore;
    Archetype.SocialBehavior = ESocialPattern::Herd;
    Archetype.bCanBeDomesticated = true;
    Archetype.TerritoryRadius = 800.0f;
    Archetype.DetectionRange = 700.0f;
    Archetype.FleeThreshold = 0.7f;
    
    // Combat personality - cooperative and moderately cautious
    Archetype.BaseCombatPersonality.Aggression = 0.4f;
    Archetype.BaseCombatPersonality.Intelligence = 0.6f;
    Archetype.BaseCombatPersonality.Patience = 0.5f;
    Archetype.BaseCombatPersonality.Cooperation = 0.8f;
    Archetype.BaseCombatPersonality.Caution = 0.7f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 650.0f;
    Archetype.CombatCapabilities.DetectionRadius = 700.0f;
    Archetype.CombatCapabilities.AttackRange = 150.0f;
    Archetype.CombatCapabilities.BaseDamage = 30.0f;
    Archetype.CombatCapabilities.Stamina = 100.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = false;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Territorial;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}

void UDinosaurArchetypeLibrary::CreateSolitaryScavengerArchetype()
{
    FDinosaurArchetype Archetype;
    Archetype.ArchetypeName = TEXT("SolitaryScavenger");
    Archetype.Size = EDinosaurSize::Medium;
    Archetype.Diet = EDinosaurDiet::Carnivore;
    Archetype.SocialBehavior = ESocialPattern::Solitary;
    Archetype.bCanBeDomesticated = false;
    Archetype.TerritoryRadius = 1200.0f;
    Archetype.DetectionRange = 1000.0f;
    Archetype.FleeThreshold = 0.6f;
    
    // Combat personality - opportunistic and cautious
    Archetype.BaseCombatPersonality.Aggression = 0.5f;
    Archetype.BaseCombatPersonality.Intelligence = 0.7f;
    Archetype.BaseCombatPersonality.Patience = 0.6f;
    Archetype.BaseCombatPersonality.Cooperation = 0.3f;
    Archetype.BaseCombatPersonality.Caution = 0.8f;
    
    // Combat capabilities
    Archetype.CombatCapabilities.MaxSpeed = 750.0f;
    Archetype.CombatCapabilities.DetectionRadius = 1000.0f;
    Archetype.CombatCapabilities.AttackRange = 180.0f;
    Archetype.CombatCapabilities.BaseDamage = 35.0f;
    Archetype.CombatCapabilities.Stamina = 90.0f;
    Archetype.CombatCapabilities.bCanPackHunt = false;
    Archetype.CombatCapabilities.bCanAmbush = true;
    Archetype.CombatCapabilities.PreferredStrategy = EHuntingStrategy::Opportunistic;
    
    Archetypes.Add(Archetype.ArchetypeName, Archetype);
}