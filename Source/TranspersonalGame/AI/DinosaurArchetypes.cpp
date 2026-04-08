#include "DinosaurArchetypes.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UDinosaurArchetypeLibrary::UDinosaurArchetypeLibrary()
{
    InitializeDefaultArchetypes();
}

FDinosaurArchetype UDinosaurArchetypeLibrary::GetArchetype(const FString& ArchetypeName)
{
    if (Archetypes.Contains(ArchetypeName))
    {
        return Archetypes[ArchetypeName];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Archetype '%s' not found, returning default"), *ArchetypeName);
    return FDinosaurArchetype();
}

FNPCPersonality UDinosaurArchetypeLibrary::CreatePersonalityVariation(const FDinosaurArchetype& Archetype, float VariationAmount)
{
    FNPCPersonality VariedPersonality = Archetype.BasePersonality;
    
    // Apply random variations within the specified range
    VariedPersonality.Curiosity = FMath::Clamp(
        VariedPersonality.Curiosity + FMath::RandRange(-VariationAmount, VariationAmount),
        0.0f, 1.0f
    );
    
    VariedPersonality.Aggression = FMath::Clamp(
        VariedPersonality.Aggression + FMath::RandRange(-VariationAmount, VariationAmount),
        0.0f, 1.0f
    );
    
    VariedPersonality.Sociability = FMath::Clamp(
        VariedPersonality.Sociability + FMath::RandRange(-VariationAmount, VariationAmount),
        0.0f, 1.0f
    );
    
    VariedPersonality.Caution = FMath::Clamp(
        VariedPersonality.Caution + FMath::RandRange(-VariationAmount, VariationAmount),
        0.0f, 1.0f
    );
    
    VariedPersonality.Energy = FMath::Clamp(
        VariedPersonality.Energy + FMath::RandRange(-VariationAmount, VariationAmount),
        0.0f, 1.0f
    );
    
    return VariedPersonality;
}

void UDinosaurArchetypeLibrary::InitializeDefaultArchetypes()
{
    Archetypes.Empty();
    
    CreateSmallHerbivoreArchetype();
    CreateLargeCarnivoreArchetype();
    CreatePackHunterArchetype();
    CreateGentleGiantArchetype();
    CreateSkittishPreyArchetype();
    CreateApexPredatorArchetype();
    CreateSocialHerbivoreArchetype();
    CreateSolitaryScavengerArchetype();
}

void UDinosaurArchetypeLibrary::CreateSmallHerbivoreArchetype()
{
    FDinosaurArchetype SmallHerbivore;
    SmallHerbivore.ArchetypeName = TEXT("SmallHerbivore");
    SmallHerbivore.Size = EDinosaurSize::Small;
    SmallHerbivore.Diet = EDinosaurDiet::Herbivore;
    SmallHerbivore.SocialBehavior = ESocialPattern::SmallGroup;
    SmallHerbivore.bCanBeDomesticated = true;
    SmallHerbivore.TerritoryRadius = 500.0f;
    SmallHerbivore.DetectionRange = 800.0f;
    SmallHerbivore.FleeThreshold = 0.7f;
    
    // Personality: Curious but cautious, moderately social
    SmallHerbivore.BasePersonality.Curiosity = 0.7f;
    SmallHerbivore.BasePersonality.Aggression = 0.2f;
    SmallHerbivore.BasePersonality.Sociability = 0.6f;
    SmallHerbivore.BasePersonality.Caution = 0.8f;
    SmallHerbivore.BasePersonality.Energy = 0.7f;
    
    // Daily routine: Active during day, rest at night
    FDailyRoutineEntry MorningGrazing;
    MorningGrazing.ActivityType = ENPCActivity::Foraging;
    MorningGrazing.StartTime = 6.0f;
    MorningGrazing.Duration = 4.0f;
    MorningGrazing.Priority = 0.8f;
    
    FDailyRoutineEntry AfternoonSocial;
    AfternoonSocial.ActivityType = ENPCActivity::Socializing;
    AfternoonSocial.StartTime = 14.0f;
    AfternoonSocial.Duration = 2.0f;
    AfternoonSocial.Priority = 0.6f;
    
    FDailyRoutineEntry EveningSafety;
    EveningSafety.ActivityType = ENPCActivity::Resting;
    EveningSafety.StartTime = 20.0f;
    EveningSafety.Duration = 8.0f;
    EveningSafety.Priority = 0.9f;
    
    SmallHerbivore.DefaultRoutine = {MorningGrazing, AfternoonSocial, EveningSafety};
    
    Archetypes.Add(SmallHerbivore.ArchetypeName, SmallHerbivore);
}

void UDinosaurArchetypeLibrary::CreateLargeCarnivoreArchetype()
{
    FDinosaurArchetype LargeCarnivore;
    LargeCarnivore.ArchetypeName = TEXT("LargeCarnivore");
    LargeCarnivore.Size = EDinosaurSize::Large;
    LargeCarnivore.Diet = EDinosaurDiet::Carnivore;
    LargeCarnivore.SocialBehavior = ESocialPattern::Solitary;
    LargeCarnivore.bCanBeDomesticated = false;
    LargeCarnivore.TerritoryRadius = 2000.0f;
    LargeCarnivore.DetectionRange = 1500.0f;
    LargeCarnivore.FleeThreshold = 0.2f;
    
    // Personality: Low curiosity, high aggression, solitary
    LargeCarnivore.BasePersonality.Curiosity = 0.3f;
    LargeCarnivore.BasePersonality.Aggression = 0.8f;
    LargeCarnivore.BasePersonality.Sociability = 0.1f;
    LargeCarnivore.BasePersonality.Caution = 0.4f;
    LargeCarnivore.BasePersonality.Energy = 0.6f;
    
    // Daily routine: Hunt at dawn and dusk
    FDailyRoutineEntry DawnHunt;
    DawnHunt.ActivityType = ENPCActivity::Hunting;
    DawnHunt.StartTime = 5.0f;
    DawnHunt.Duration = 3.0f;
    DawnHunt.Priority = 0.9f;
    
    FDailyRoutineEntry DayRest;
    DayRest.ActivityType = ENPCActivity::Resting;
    DayRest.StartTime = 10.0f;
    DayRest.Duration = 8.0f;
    DayRest.Priority = 0.7f;
    
    FDailyRoutineEntry DuskHunt;
    DuskHunt.ActivityType = ENPCActivity::Hunting;
    DuskHunt.StartTime = 18.0f;
    DuskHunt.Duration = 3.0f;
    DuskHunt.Priority = 0.9f;
    
    LargeCarnivore.DefaultRoutine = {DawnHunt, DayRest, DuskHunt};
    
    Archetypes.Add(LargeCarnivore.ArchetypeName, LargeCarnivore);
}

void UDinosaurArchetypeLibrary::CreatePackHunterArchetype()
{
    FDinosaurArchetype PackHunter;
    PackHunter.ArchetypeName = TEXT("PackHunter");
    PackHunter.Size = EDinosaurSize::Medium;
    PackHunter.Diet = EDinosaurDiet::Carnivore;
    PackHunter.SocialBehavior = ESocialPattern::SmallGroup;
    PackHunter.bCanBeDomesticated = false;
    PackHunter.TerritoryRadius = 1500.0f;
    PackHunter.DetectionRange = 1200.0f;
    PackHunter.FleeThreshold = 0.4f;
    
    // Personality: Moderate curiosity, high aggression, very social
    PackHunter.BasePersonality.Curiosity = 0.5f;
    PackHunter.BasePersonality.Aggression = 0.7f;
    PackHunter.BasePersonality.Sociability = 0.9f;
    PackHunter.BasePersonality.Caution = 0.5f;
    PackHunter.BasePersonality.Energy = 0.8f;
    
    // Daily routine: Coordinated hunting and social activities
    FDailyRoutineEntry MorningHunt;
    MorningHunt.ActivityType = ENPCActivity::Hunting;
    MorningHunt.StartTime = 7.0f;
    MorningHunt.Duration = 4.0f;
    MorningHunt.Priority = 0.9f;
    
    FDailyRoutineEntry SocialTime;
    SocialTime.ActivityType = ENPCActivity::Socializing;
    SocialTime.StartTime = 15.0f;
    SocialTime.Duration = 3.0f;
    SocialTime.Priority = 0.8f;
    
    PackHunter.DefaultRoutine = {MorningHunt, SocialTime};
    
    Archetypes.Add(PackHunter.ArchetypeName, PackHunter);
}

void UDinosaurArchetypeLibrary::CreateGentleGiantArchetype()
{
    FDinosaurArchetype GentleGiant;
    GentleGiant.ArchetypeName = TEXT("GentleGiant");
    GentleGiant.Size = EDinosaurSize::Massive;
    GentleGiant.Diet = EDinosaurDiet::Herbivore;
    GentleGiant.SocialBehavior = ESocialPattern::Herd;
    GentleGiant.bCanBeDomesticated = true;
    GentleGiant.TerritoryRadius = 3000.0f;
    GentleGiant.DetectionRange = 1000.0f;
    GentleGiant.FleeThreshold = 0.3f;
    
    // Personality: High curiosity, very low aggression, social
    GentleGiant.BasePersonality.Curiosity = 0.8f;
    GentleGiant.BasePersonality.Aggression = 0.1f;
    GentleGiant.BasePersonality.Sociability = 0.7f;
    GentleGiant.BasePersonality.Caution = 0.6f;
    GentleGiant.BasePersonality.Energy = 0.4f;
    
    // Daily routine: Long feeding sessions
    FDailyRoutineEntry AllDayGrazing;
    AllDayGrazing.ActivityType = ENPCActivity::Foraging;
    AllDayGrazing.StartTime = 6.0f;
    AllDayGrazing.Duration = 12.0f;
    AllDayGrazing.Priority = 0.8f;
    
    GentleGiant.DefaultRoutine = {AllDayGrazing};
    
    Archetypes.Add(GentleGiant.ArchetypeName, GentleGiant);
}

void UDinosaurArchetypeLibrary::CreateSkittishPreyArchetype()
{
    FDinosaurArchetype SkittishPrey;
    SkittishPrey.ArchetypeName = TEXT("SkittishPrey");
    SkittishPrey.Size = EDinosaurSize::Small;
    SkittishPrey.Diet = EDinosaurDiet::Herbivore;
    SkittishPrey.SocialBehavior = ESocialPattern::Herd;
    SkittishPrey.bCanBeDomesticated = true;
    SkittishPrey.TerritoryRadius = 800.0f;
    SkittishPrey.DetectionRange = 1500.0f;
    SkittishPrey.FleeThreshold = 0.9f;
    
    // Personality: Very cautious, low aggression, highly social
    SkittishPrey.BasePersonality.Curiosity = 0.4f;
    SkittishPrey.BasePersonality.Aggression = 0.1f;
    SkittishPrey.BasePersonality.Sociability = 0.9f;
    SkittishPrey.BasePersonality.Caution = 0.95f;
    SkittishPrey.BasePersonality.Energy = 0.8f;
    
    // Daily routine: Quick feeding with constant vigilance
    FDailyRoutineEntry QuickFeeding;
    QuickFeeding.ActivityType = ENPCActivity::Foraging;
    QuickFeeding.StartTime = 8.0f;
    QuickFeeding.Duration = 2.0f;
    QuickFeeding.Priority = 0.9f;
    
    FDailyRoutineEntry GroupSafety;
    GroupSafety.ActivityType = ENPCActivity::Socializing;
    GroupSafety.StartTime = 12.0f;
    GroupSafety.Duration = 6.0f;
    GroupSafety.Priority = 0.8f;
    
    SkittishPrey.DefaultRoutine = {QuickFeeding, GroupSafety};
    
    Archetypes.Add(SkittishPrey.ArchetypeName, SkittishPrey);
}

void UDinosaurArchetypeLibrary::CreateApexPredatorArchetype()
{
    FDinosaurArchetype ApexPredator;
    ApexPredator.ArchetypeName = TEXT("ApexPredator");
    ApexPredator.Size = EDinosaurSize::Massive;
    ApexPredator.Diet = EDinosaurDiet::Carnivore;
    ApexPredator.SocialBehavior = ESocialPattern::Solitary;
    ApexPredator.bCanBeDomesticated = false;
    ApexPredator.TerritoryRadius = 5000.0f;
    ApexPredator.DetectionRange = 2000.0f;
    ApexPredator.FleeThreshold = 0.1f;
    
    // Personality: Low curiosity, extreme aggression, completely solitary
    ApexPredator.BasePersonality.Curiosity = 0.2f;
    ApexPredator.BasePersonality.Aggression = 0.95f;
    ApexPredator.BasePersonality.Sociability = 0.0f;
    ApexPredator.BasePersonality.Caution = 0.3f;
    ApexPredator.BasePersonality.Energy = 0.7f;
    
    // Daily routine: Patrol territory and hunt
    FDailyRoutineEntry TerritoryPatrol;
    TerritoryPatrol.ActivityType = ENPCActivity::Patrolling;
    TerritoryPatrol.StartTime = 6.0f;
    TerritoryPatrol.Duration = 8.0f;
    TerritoryPatrol.Priority = 0.8f;
    
    FDailyRoutineEntry OpportunisticHunt;
    OpportunisticHunt.ActivityType = ENPCActivity::Hunting;
    OpportunisticHunt.StartTime = 16.0f;
    OpportunisticHunt.Duration = 4.0f;
    OpportunisticHunt.Priority = 0.9f;
    
    ApexPredator.DefaultRoutine = {TerritoryPatrol, OpportunisticHunt};
    
    Archetypes.Add(ApexPredator.ArchetypeName, ApexPredator);
}

void UDinosaurArchetypeLibrary::CreateSocialHerbivoreArchetype()
{
    FDinosaurArchetype SocialHerbivore;
    SocialHerbivore.ArchetypeName = TEXT("SocialHerbivore");
    SocialHerbivore.Size = EDinosaurSize::Medium;
    SocialHerbivore.Diet = EDinosaurDiet::Herbivore;
    SocialHerbivore.SocialBehavior = ESocialPattern::Herd;
    SocialHerbivore.bCanBeDomesticated = true;
    SocialHerbivore.TerritoryRadius = 1200.0f;
    SocialHerbivore.DetectionRange = 1000.0f;
    SocialHerbivore.FleeThreshold = 0.6f;
    
    // Personality: High curiosity and sociability, moderate caution
    SocialHerbivore.BasePersonality.Curiosity = 0.8f;
    SocialHerbivore.BasePersonality.Aggression = 0.3f;
    SocialHerbivore.BasePersonality.Sociability = 0.9f;
    SocialHerbivore.BasePersonality.Caution = 0.6f;
    SocialHerbivore.BasePersonality.Energy = 0.7f;
    
    // Daily routine: Group activities throughout the day
    FDailyRoutineEntry GroupForaging;
    GroupForaging.ActivityType = ENPCActivity::Foraging;
    GroupForaging.StartTime = 7.0f;
    GroupForaging.Duration = 5.0f;
    GroupForaging.Priority = 0.8f;
    
    FDailyRoutineEntry SocialBonding;
    SocialBonding.ActivityType = ENPCActivity::Socializing;
    SocialBonding.StartTime = 14.0f;
    SocialBonding.Duration = 4.0f;
    SocialBonding.Priority = 0.7f;
    
    SocialHerbivore.DefaultRoutine = {GroupForaging, SocialBonding};
    
    Archetypes.Add(SocialHerbivore.ArchetypeName, SocialHerbivore);
}

void UDinosaurArchetypeLibrary::CreateSolitaryScavengerArchetype()
{
    FDinosaurArchetype SolitaryScavenger;
    SolitaryScavenger.ArchetypeName = TEXT("SolitaryScavenger");
    SolitaryScavenger.Size = EDinosaurSize::Medium;
    SolitaryScavenger.Diet = EDinosaurDiet::Carnivore;
    SolitaryScavenger.SocialBehavior = ESocialPattern::Solitary;
    SolitaryScavenger.bCanBeDomesticated = false;
    SolitaryScavenger.TerritoryRadius = 1800.0f;
    SolitaryScavenger.DetectionRange = 1800.0f;
    SolitaryScavenger.FleeThreshold = 0.7f;
    
    // Personality: High curiosity, low aggression, opportunistic
    SolitaryScavenger.BasePersonality.Curiosity = 0.9f;
    SolitaryScavenger.BasePersonality.Aggression = 0.4f;
    SolitaryScavenger.BasePersonality.Sociability = 0.2f;
    SolitaryScavenger.BasePersonality.Caution = 0.8f;
    SolitaryScavenger.BasePersonality.Energy = 0.6f;
    
    // Daily routine: Opportunistic scavenging
    FDailyRoutineEntry ScavengingPatrol;
    ScavengingPatrol.ActivityType = ENPCActivity::Foraging;
    ScavengingPatrol.StartTime = 8.0f;
    ScavengingPatrol.Duration = 8.0f;
    ScavengingPatrol.Priority = 0.8f;
    
    FDailyRoutineEntry OpportunisticHunt;
    OpportunisticHunt.ActivityType = ENPCActivity::Hunting;
    OpportunisticHunt.StartTime = 18.0f;
    OpportunisticHunt.Duration = 2.0f;
    OpportunisticHunt.Priority = 0.6f;
    
    SolitaryScavenger.DefaultRoutine = {ScavengingPatrol, OpportunisticHunt};
    
    Archetypes.Add(SolitaryScavenger.ArchetypeName, SolitaryScavenger);
}