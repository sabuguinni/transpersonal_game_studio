#include "Narr_DinosaurEncounterNarrative.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarr_DinosaurEncounterNarrative::UNarr_DinosaurEncounterNarrative()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;
    
    MaxHistoryEntries = 50;
    NarrativeUpdateInterval = 2.0f;
    NarrativeUpdateTimer = 0.0f;
    
    InitializeNarrativeTemplates();
}

void UNarr_DinosaurEncounterNarrative::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Encounter Narrative System initialized"));
}

void UNarr_DinosaurEncounterNarrative::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    NarrativeUpdateTimer += DeltaTime;
    
    if (NarrativeUpdateTimer >= NarrativeUpdateInterval)
    {
        NarrativeUpdateTimer = 0.0f;
        
        // Update ongoing narrative if there's an active encounter
        if (!CurrentEncounter.NarrativeTitle.IsEmpty())
        {
            UpdateEncounterNarrative(ActiveContext);
        }
    }
}

FNarr_EncounterNarrative UNarr_DinosaurEncounterNarrative::GenerateEncounterNarrative(const FNarr_DinosaurBehaviorContext& BehaviorContext)
{
    FNarr_EncounterNarrative NewEncounter;
    
    // Generate narrative title based on species and encounter type
    FString EncounterTypeStr;
    switch (BehaviorContext.EncounterType)
    {
        case ENarr_DinosaurEncounterType::FirstSighting:
            EncounterTypeStr = TEXT("First Contact");
            break;
        case ENarr_DinosaurEncounterType::PredatorHunt:
            EncounterTypeStr = TEXT("The Hunt");
            break;
        case ENarr_DinosaurEncounterType::HerbivoreGrazing:
            EncounterTypeStr = TEXT("Peaceful Grazing");
            break;
        case ENarr_DinosaurEncounterType::PackBehavior:
            EncounterTypeStr = TEXT("Pack Dynamics");
            break;
        case ENarr_DinosaurEncounterType::TerritorialDispute:
            EncounterTypeStr = TEXT("Territory War");
            break;
        default:
            EncounterTypeStr = TEXT("Encounter");
            break;
    }
    
    NewEncounter.NarrativeTitle = FString::Printf(TEXT("%s: %s"), *BehaviorContext.DinosaurSpecies, *EncounterTypeStr);
    
    // Generate opening description
    TArray<FString> SpeciesNarrative = GenerateSpeciesNarrative(BehaviorContext.DinosaurSpecies, BehaviorContext.EncounterType);
    if (SpeciesNarrative.Num() > 0)
    {
        NewEncounter.OpeningDescription = SpeciesNarrative[0];
    }
    
    // Generate environmental context
    TArray<FString> EnvironmentalNarrative = GenerateEnvironmentalNarrative(BehaviorContext.BiomeLocation, BehaviorContext.WeatherConditions);
    NewEncounter.EnvironmentalDetails = EnvironmentalNarrative;
    
    // Generate tension buildup based on encounter dynamics
    FString TensionDescription;
    switch (BehaviorContext.TensionLevel)
    {
        case ENarr_EncounterTension::Peaceful:
            TensionDescription = TEXT("The ancient creature moves with serene purpose, unaware of your presence. This moment of tranquility in the prehistoric world feels almost sacred.");
            break;
        case ENarr_EncounterTension::Cautious:
            TensionDescription = TEXT("You hold your breath, every muscle tense. The creature's head turns slightly - did it sense you? Your heart pounds as you remain perfectly still.");
            break;
        case ENarr_EncounterTension::Tense:
            TensionDescription = TEXT("The air crackles with tension. The creature's nostrils flare, testing the wind. Your survival instincts scream at you to move, but where can you go?");
            break;
        case ENarr_EncounterTension::Dangerous:
            TensionDescription = TEXT("Death stalks through the undergrowth. The predator's eyes lock onto yours with predatory intelligence. Time slows as you realize you've become prey.");
            break;
        case ENarr_EncounterTension::Terrifying:
            TensionDescription = TEXT("Pure terror floods your system. This apex predator embodies millions of years of evolutionary perfection. You are nothing but meat to this killing machine.");
            break;
        case ENarr_EncounterTension::LifeOrDeath:
            TensionDescription = TEXT("This is it. Life or death. The creature lunges with prehistoric fury. Your primitive brain takes over - fight, flight, or become fossil.");
            break;
    }
    NewEncounter.TensionBuildup = TensionDescription;
    
    // Generate climax moment
    if (BehaviorContext.bIsAggressive)
    {
        NewEncounter.ClimaxMoment = TEXT("The creature strikes with lightning speed, millions of years of predatory evolution focused into this single, terrifying moment of truth.");
    }
    else
    {
        NewEncounter.ClimaxMoment = TEXT("The moment passes like a held breath released. The ancient giant continues on its path, leaving you humbled by the encounter.");
    }
    
    // Generate player psychological response
    NewEncounter.PlayerThoughts = GeneratePlayerPsychologicalResponse(BehaviorContext.TensionLevel, EncounterHistory.Num() == 0);
    
    // Set narrative duration based on tension level
    float BaseDuration = 30.0f;
    float TensionMultiplier = 1.0f + (static_cast<float>(BehaviorContext.TensionLevel) * 0.2f);
    NewEncounter.NarrativeDuration = BaseDuration * TensionMultiplier;
    
    return NewEncounter;
}

void UNarr_DinosaurEncounterNarrative::TriggerEncounterNarrative(const FNarr_DinosaurBehaviorContext& BehaviorContext)
{
    CurrentEncounter = GenerateEncounterNarrative(BehaviorContext);
    ActiveContext = BehaviorContext;
    
    // Add to history
    EncounterHistory.Add(CurrentEncounter);
    
    // Maintain history size limit
    if (EncounterHistory.Num() > MaxHistoryEntries)
    {
        EncounterHistory.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Encounter Narrative Triggered: %s"), *CurrentEncounter.NarrativeTitle);
    
    // Broadcast narrative to game systems
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("ENCOUNTER: %s"), *CurrentEncounter.NarrativeTitle));
    }
}

void UNarr_DinosaurEncounterNarrative::UpdateEncounterNarrative(const FNarr_DinosaurBehaviorContext& UpdatedContext)
{
    // Update active context
    ActiveContext = UpdatedContext;
    
    // Recalculate tension level
    ENarr_EncounterTension NewTension = CalculateTensionLevel(UpdatedContext);
    
    // Update narrative elements if tension has changed significantly
    if (NewTension != ActiveContext.TensionLevel)
    {
        ActiveContext.TensionLevel = NewTension;
        
        // Regenerate tension buildup
        FString UpdatedTension;
        switch (NewTension)
        {
            case ENarr_EncounterTension::Peaceful:
                UpdatedTension = TEXT("The tension eases. The creature seems less interested in you now.");
                break;
            case ENarr_EncounterTension::Dangerous:
                UpdatedTension = TEXT("The situation escalates! The creature's behavior becomes more aggressive.");
                break;
            case ENarr_EncounterTension::LifeOrDeath:
                UpdatedTension = TEXT("CRITICAL! The creature is now actively hunting you!");
                break;
        }
        
        if (!UpdatedTension.IsEmpty())
        {
            CurrentEncounter.TensionBuildup = UpdatedTension;
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, UpdatedTension);
            }
        }
    }
}

ENarr_EncounterTension UNarr_DinosaurEncounterNarrative::CalculateTensionLevel(const FNarr_DinosaurBehaviorContext& BehaviorContext)
{
    float TensionScore = 0.0f;
    
    // Distance factor (closer = more tense)
    if (BehaviorContext.DistanceToPlayer < 10.0f)
        TensionScore += 3.0f;
    else if (BehaviorContext.DistanceToPlayer < 25.0f)
        TensionScore += 2.0f;
    else if (BehaviorContext.DistanceToPlayer < 50.0f)
        TensionScore += 1.0f;
    
    // Aggression factor
    if (BehaviorContext.bIsAggressive)
        TensionScore += 2.5f;
    
    // Pack behavior factor
    if (BehaviorContext.bIsInPack && BehaviorContext.PackSize > 1)
        TensionScore += 1.5f;
    
    // Encounter type factor
    switch (BehaviorContext.EncounterType)
    {
        case ENarr_DinosaurEncounterType::PredatorHunt:
            TensionScore += 3.0f;
            break;
        case ENarr_DinosaurEncounterType::TerritorialDispute:
            TensionScore += 2.0f;
            break;
        case ENarr_DinosaurEncounterType::HerbivoreGrazing:
            TensionScore -= 1.0f;
            break;
    }
    
    // Convert score to tension level
    if (TensionScore >= 6.0f)
        return ENarr_EncounterTension::LifeOrDeath;
    else if (TensionScore >= 4.5f)
        return ENarr_EncounterTension::Terrifying;
    else if (TensionScore >= 3.0f)
        return ENarr_EncounterTension::Dangerous;
    else if (TensionScore >= 1.5f)
        return ENarr_EncounterTension::Tense;
    else if (TensionScore >= 0.5f)
        return ENarr_EncounterTension::Cautious;
    else
        return ENarr_EncounterTension::Peaceful;
}

TArray<FString> UNarr_DinosaurEncounterNarrative::GenerateSpeciesNarrative(const FString& DinosaurSpecies, ENarr_DinosaurEncounterType EncounterType)
{
    TArray<FString> Narratives;
    
    if (SpeciesNarrativeTemplates.Contains(DinosaurSpecies))
    {
        return SpeciesNarrativeTemplates[DinosaurSpecies];
    }
    
    // Default narratives for unknown species
    Narratives.Add(TEXT("A magnificent prehistoric creature emerges from the ancient landscape, its presence commanding respect and awe."));
    Narratives.Add(TEXT("This living fossil moves with the grace of millions of years of evolution, a testament to nature's enduring power."));
    Narratives.Add(TEXT("Before you stands a creature from Earth's distant past, its very existence a bridge between worlds."));
    
    return Narratives;
}

TArray<FString> UNarr_DinosaurEncounterNarrative::GenerateEnvironmentalNarrative(const FString& BiomeLocation, const FString& WeatherConditions)
{
    TArray<FString> Environmental;
    
    // Biome-based environmental details
    if (BiomeLocation == TEXT("Forest"))
    {
        Environmental.Add(TEXT("Dappled sunlight filters through the prehistoric canopy, casting dancing shadows on the forest floor."));
        Environmental.Add(TEXT("The air is thick with the scent of ancient ferns and decomposing vegetation."));
    }
    else if (BiomeLocation == TEXT("Plains"))
    {
        Environmental.Add(TEXT("The vast prehistoric grasslands stretch endlessly toward the horizon."));
        Environmental.Add(TEXT("Wind whispers through tall grasses, carrying the sounds of distant creatures."));
    }
    else if (BiomeLocation == TEXT("Swamp"))
    {
        Environmental.Add(TEXT("Murky waters reflect the prehistoric sky, disturbed only by ripples of unseen movement."));
        Environmental.Add(TEXT("The humid air buzzes with insects and the calls of amphibious creatures."));
    }
    
    // Weather-based modifiers
    if (WeatherConditions == TEXT("Storm"))
    {
        Environmental.Add(TEXT("Thunder rolls across the prehistoric landscape as lightning illuminates the scene in stark detail."));
    }
    else if (WeatherConditions == TEXT("Fog"))
    {
        Environmental.Add(TEXT("Thick mist shrouds the landscape, reducing visibility and heightening every sound."));
    }
    
    return Environmental;
}

TArray<FString> UNarr_DinosaurEncounterNarrative::GeneratePlayerPsychologicalResponse(ENarr_EncounterTension TensionLevel, bool bIsFirstEncounter)
{
    TArray<FString> Thoughts;
    
    if (bIsFirstEncounter)
    {
        Thoughts.Add(TEXT("Is this real? Am I truly seeing a living dinosaur?"));
        Thoughts.Add(TEXT("Every childhood dream and nightmare about dinosaurs floods back in an instant."));
    }
    
    switch (TensionLevel)
    {
        case ENarr_EncounterTension::Peaceful:
            Thoughts.Add(TEXT("Such majesty... I feel privileged to witness this moment."));
            Thoughts.Add(TEXT("This creature has survived for millions of years. What wisdom does it carry?"));
            break;
            
        case ENarr_EncounterTension::Cautious:
            Thoughts.Add(TEXT("Stay calm. Don't make any sudden movements."));
            Thoughts.Add(TEXT("How do I get out of here without being noticed?"));
            break;
            
        case ENarr_EncounterTension::Dangerous:
            Thoughts.Add(TEXT("This is bad. Very bad. I need an escape route."));
            Thoughts.Add(TEXT("My heart is pounding so loud, can it hear me?"));
            break;
            
        case ENarr_EncounterTension::LifeOrDeath:
            Thoughts.Add(TEXT("I'm going to die. This is how I die."));
            Thoughts.Add(TEXT("Fight or flight? There's nowhere to run!"));
            break;
    }
    
    return Thoughts;
}

void UNarr_DinosaurEncounterNarrative::InitializeNarrativeTemplates()
{
    BuildSpeciesTemplates();
    BuildBiomeTemplates();
    BuildTensionModifiers();
}

void UNarr_DinosaurEncounterNarrative::BuildSpeciesTemplates()
{
    // T-Rex narratives
    TArray<FString> TRexNarratives;
    TRexNarratives.Add(TEXT("The ground trembles with each thunderous footstep. Tyrannosaurus Rex, the king of predators, emerges from the shadows with terrifying majesty."));
    TRexNarratives.Add(TEXT("Six tons of pure predatory power stands before you. The T-Rex's massive head swings in your direction, nostrils flaring as it catches your scent."));
    TRexNarratives.Add(TEXT("Evolution's perfect killing machine has found you. The T-Rex's yellow eyes gleam with predatory intelligence as it evaluates you as prey."));
    SpeciesNarrativeTemplates.Add(TEXT("TRex"), TRexNarratives);
    
    // Raptor narratives
    TArray<FString> RaptorNarratives;
    RaptorNarratives.Add(TEXT("Silent death on two legs. The Velociraptor's sickle-shaped claws gleam as it stalks through the undergrowth with lethal grace."));
    RaptorNarratives.Add(TEXT("Intelligence burns in the raptor's reptilian eyes. This is no mindless beast - it's calculating, planning, hunting."));
    RaptorNarratives.Add(TEXT("The pack hunter moves with fluid precision. Where there's one raptor, there are always more. You scan the shadows nervously."));
    SpeciesNarrativeTemplates.Add(TEXT("Raptor"), RaptorNarratives);
    
    // Brachiosaurus narratives
    TArray<FString> BrachiosaurusNarratives;
    BrachiosaurusNarratives.Add(TEXT("A gentle giant from the age of titans. The Brachiosaurus moves with slow, deliberate grace, its massive neck reaching toward the canopy."));
    BrachiosaurusNarratives.Add(TEXT("Forty tons of peaceful herbivore towers above the landscape. The Brachiosaurus's presence transforms the entire environment around it."));
    BrachiosaurusNarratives.Add(TEXT("Like a living mountain, the Brachiosaurus embodies the majesty of the prehistoric world. Its gentle eyes hold ancient wisdom."));
    SpeciesNarrativeTemplates.Add(TEXT("Brachiosaurus"), BrachiosaurusNarratives);
}

void UNarr_DinosaurEncounterNarrative::BuildBiomeTemplates()
{
    // Forest biome
    TArray<FString> ForestNarratives;
    ForestNarratives.Add(TEXT("Ancient trees tower overhead, their branches forming a prehistoric cathedral of green."));
    ForestNarratives.Add(TEXT("Shafts of golden sunlight pierce the canopy, illuminating motes of pollen and prehistoric spores."));
    BiomeNarrativeTemplates.Add(TEXT("Forest"), ForestNarratives);
    
    // Plains biome
    TArray<FString> PlainsNarratives;
    PlainsNarratives.Add(TEXT("Endless grasslands stretch to the horizon under the prehistoric sky."));
    PlainsNarratives.Add(TEXT("The wind carries the scents of wild flowers and distant herds across the open savanna."));
    BiomeNarrativeTemplates.Add(TEXT("Plains"), PlainsNarratives);
}

void UNarr_DinosaurEncounterNarrative::BuildTensionModifiers()
{
    // Peaceful modifiers
    TArray<FString> PeacefulModifiers;
    PeacefulModifiers.Add(TEXT("serenely"));
    PeacefulModifiers.Add(TEXT("gracefully"));
    PeacefulModifiers.Add(TEXT("majestically"));
    TensionNarrativeModifiers.Add(ENarr_EncounterTension::Peaceful, PeacefulModifiers);
    
    // Dangerous modifiers
    TArray<FString> DangerousModifiers;
    DangerousModifiers.Add(TEXT("menacingly"));
    DangerousModifiers.Add(TEXT("aggressively"));
    DangerousModifiers.Add(TEXT("threateningly"));
    TensionNarrativeModifiers.Add(ENarr_EncounterTension::Dangerous, DangerousModifiers);
}

FString UNarr_DinosaurEncounterNarrative::SelectNarrativeTemplate(const TArray<FString>& Templates) const
{
    if (Templates.Num() == 0)
    {
        return TEXT("A prehistoric encounter unfolds before you.");
    }
    
    int32 RandomIndex = FMath::RandRange(0, Templates.Num() - 1);
    return Templates[RandomIndex];
}

FString UNarr_DinosaurEncounterNarrative::ApplyNarrativeModifiers(const FString& BaseNarrative, const FNarr_DinosaurBehaviorContext& Context) const
{
    FString ModifiedNarrative = BaseNarrative;
    
    // Apply tension-based modifiers
    if (TensionNarrativeModifiers.Contains(Context.TensionLevel))
    {
        const TArray<FString>& Modifiers = TensionNarrativeModifiers[Context.TensionLevel];
        if (Modifiers.Num() > 0)
        {
            FString Modifier = Modifiers[FMath::RandRange(0, Modifiers.Num() - 1)];
            ModifiedNarrative = FString::Printf(TEXT("%s %s"), *ModifiedNarrative, *Modifier);
        }
    }
    
    return ModifiedNarrative;
}

float UNarr_DinosaurEncounterNarrative::CalculateNarrativeComplexity(const FNarr_DinosaurBehaviorContext& BehaviorContext) const
{
    float Complexity = 1.0f;
    
    // Pack behavior adds complexity
    if (BehaviorContext.bIsInPack)
    {
        Complexity += 0.5f * BehaviorContext.PackSize;
    }
    
    // Aggressive encounters are more complex
    if (BehaviorContext.bIsAggressive)
    {
        Complexity += 1.0f;
    }
    
    // Tension level affects complexity
    Complexity += static_cast<float>(BehaviorContext.TensionLevel) * 0.3f;
    
    return FMath::Clamp(Complexity, 1.0f, 5.0f);
}