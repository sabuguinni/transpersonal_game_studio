// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CharacterGenerator.h"
#include "CharacterSystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

void UCharacterGenerator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    GenerationSeed = FMath::Rand();
    InitializeMetaHumanPresets();
    LoadArchetypeData();
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("CharacterGenerator initialized with seed: %d"), GenerationSeed);
}

FCharacterDefinition UCharacterGenerator::GenerateRandomCharacter(ECharacterArchetype TargetArchetype)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterGenerator not initialized"));
        return FCharacterDefinition();
    }

    FCharacterDefinition NewCharacter;
    NewCharacter.Archetype = TargetArchetype;
    NewCharacter.CharacterID = FGuid::NewGuid();
    
    // Generate basic traits
    NewCharacter.Ethnicity = SelectRandomEthnicity();
    NewCharacter.Age = SelectRandomAge(TargetArchetype);
    NewCharacter.bIsMale = SelectRandomGender();
    NewCharacter.BodyType = SelectRandomBodyType(NewCharacter.Age, 0);
    
    // Generate visual traits
    NewCharacter.VisualTraits = GenerateVisualTraits(TargetArchetype, NewCharacter.Ethnicity, NewCharacter.Age, NewCharacter.bIsMale);
    
    // Generate personality and backstory
    NewCharacter.Personality = GeneratePersonality(TargetArchetype, 0);
    NewCharacter.Backstory = GenerateBackstory(TargetArchetype, NewCharacter.Personality);
    
    // Apply archetype-specific traits
    ApplyArchetypeSpecificTraits(NewCharacter.VisualTraits, TargetArchetype);
    
    // Validate and ensure consistency
    ValidateVisualCoherence(NewCharacter);
    EnsureNarrativeConsistency(NewCharacter);
    
    OnCharacterGenerated.Broadcast(NewCharacter);
    
    return NewCharacter;
}

FCharacterDefinition UCharacterGenerator::GenerateProtagonist()
{
    FCharacterDefinition Protagonist = GenerateRandomCharacter(ECharacterArchetype::Protagonist_Paleontologist);
    
    // Protagonist-specific customization
    Protagonist.VisualTraits.Intelligence = 0.9f;
    Protagonist.VisualTraits.PhysicalFitness = 0.7f;
    Protagonist.Personality.Curiosity = 0.95f;
    Protagonist.Personality.Determination = 0.9f;
    
    return Protagonist;
}

TArray<FCharacterDefinition> UCharacterGenerator::GenerateNPCGroup(int32 Count, bool bEnsureDiversity)
{
    TArray<FCharacterDefinition> NPCGroup;
    NPCGroup.Reserve(Count);
    
    for (int32 i = 0; i < Count; i++)
    {
        ECharacterArchetype RandomArchetype = static_cast<ECharacterArchetype>(GetRandomInt(0, static_cast<int32>(ECharacterArchetype::MAX) - 1));
        FCharacterDefinition NPC = GenerateRandomCharacter(RandomArchetype);
        NPCGroup.Add(NPC);
    }
    
    if (bEnsureDiversity)
    {
        EnsureGroupDiversity(NPCGroup);
    }
    
    return NPCGroup;
}

FCharacterDefinition UCharacterGenerator::GenerateTribalCharacter(bool bIsElder)
{
    ECharacterArchetype TribalArchetype = bIsElder ? ECharacterArchetype::Tribal_Elder : ECharacterArchetype::Tribal_Hunter;
    FCharacterDefinition TribalCharacter = GenerateRandomCharacter(TribalArchetype);
    
    // Tribal-specific traits
    TribalCharacter.VisualTraits.SkinWeathering = 0.8f;
    TribalCharacter.VisualTraits.MuscleDefinition = bIsElder ? 0.6f : 0.9f;
    TribalCharacter.Personality.Wisdom = bIsElder ? 0.95f : 0.7f;
    
    return TribalCharacter;
}

FCharacterDefinition UCharacterGenerator::GenerateSurvivorCharacter(int32 DaysInJurassic)
{
    FCharacterDefinition Survivor = GenerateRandomCharacter(ECharacterArchetype::Survivor_Veteran);
    
    // Apply survival weathering based on days survived
    ApplySurvivalWeathering(Survivor.VisualTraits, DaysInJurassic);
    
    // Adjust personality based on survival experience
    float SurvivalFactor = FMath::Clamp(DaysInJurassic / 365.0f, 0.0f, 1.0f);
    Survivor.Personality.Resilience = FMath::Lerp(0.5f, 0.95f, SurvivalFactor);
    Survivor.Personality.Paranoia = FMath::Lerp(0.1f, 0.7f, SurvivalFactor);
    
    return Survivor;
}

bool UCharacterGenerator::ValidateCharacterDefinition(const FCharacterDefinition& Character)
{
    // Basic validation checks
    if (!Character.CharacterID.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Character has invalid GUID"));
        return false;
    }
    
    if (Character.VisualTraits.Height <= 0.0f || Character.VisualTraits.Height > 3.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character has invalid height: %f"), Character.VisualTraits.Height);
        return false;
    }
    
    return true;
}

void UCharacterGenerator::EnsureGroupDiversity(TArray<FCharacterDefinition>& Characters)
{
    if (Characters.Num() < 2) return;
    
    // Count current diversity metrics
    TMap<ECharacterEthnicity, int32> EthnicityCount;
    TMap<ECharacterAge, int32> AgeCount;
    int32 MaleCount = 0;
    
    for (const auto& Character : Characters)
    {
        EthnicityCount.FindOrAdd(Character.Ethnicity)++;
        AgeCount.FindOrAdd(Character.Age)++;
        if (Character.bIsMale) MaleCount++;
    }
    
    // Adjust characters to improve diversity if needed
    float TargetDiversityRatio = 0.7f;
    int32 TargetDiverseCount = FMath::RoundToInt(Characters.Num() * TargetDiversityRatio);
    
    // Implementation of diversity balancing logic would go here
    // For now, we log the current state
    UE_LOG(LogTemp, Log, TEXT("Group diversity: %d ethnicities, %d age groups, %d%% male"), 
           EthnicityCount.Num(), AgeCount.Num(), (MaleCount * 100) / Characters.Num());
}

USkeletalMesh* UCharacterGenerator::CreateMetaHumanFromDefinition(const FCharacterDefinition& Character)
{
    // This would integrate with MetaHuman Creator API
    // For now, return nullptr as placeholder
    UE_LOG(LogTemp, Log, TEXT("Creating MetaHuman for character: %s"), *Character.CharacterID.ToString());
    return nullptr;
}

void UCharacterGenerator::ApplyVisualTraitsToMetaHuman(USkeletalMesh* MetaHumanMesh, const FCharacterVisualTraits& Traits)
{
    if (!MetaHumanMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot apply traits to null MetaHuman mesh"));
        return;
    }
    
    // Implementation would modify MetaHuman parameters based on traits
    UE_LOG(LogTemp, Log, TEXT("Applying visual traits to MetaHuman"));
}

// Private helper methods implementation

FCharacterVisualTraits UCharacterGenerator::GenerateVisualTraits(ECharacterArchetype Archetype, ECharacterEthnicity Ethnicity, ECharacterAge Age, bool bIsMale)
{
    FCharacterVisualTraits Traits;
    
    // Base height and build based on gender and age
    if (bIsMale)
    {
        Traits.Height = GetRandomFloat(1.65f, 1.95f);
        Traits.MuscleDefinition = GetRandomFloat(0.4f, 0.9f);
    }
    else
    {
        Traits.Height = GetRandomFloat(1.55f, 1.80f);
        Traits.MuscleDefinition = GetRandomFloat(0.3f, 0.7f);
    }
    
    // Age-based adjustments
    switch (Age)
    {
        case ECharacterAge::Young:
            Traits.SkinWeathering = GetRandomFloat(0.0f, 0.2f);
            Traits.PhysicalFitness = GetRandomFloat(0.7f, 1.0f);
            break;
        case ECharacterAge::Adult:
            Traits.SkinWeathering = GetRandomFloat(0.1f, 0.4f);
            Traits.PhysicalFitness = GetRandomFloat(0.5f, 0.9f);
            break;
        case ECharacterAge::Elder:
            Traits.SkinWeathering = GetRandomFloat(0.4f, 0.8f);
            Traits.PhysicalFitness = GetRandomFloat(0.3f, 0.6f);
            break;
    }
    
    // Random variation
    Traits.Intelligence = GetRandomFloat(0.3f, 0.9f);
    Traits.Charisma = GetRandomFloat(0.2f, 0.8f);
    
    return Traits;
}

FCharacterPersonality UCharacterGenerator::GeneratePersonality(ECharacterArchetype Archetype, int32 SurvivalDays)
{
    FCharacterPersonality Personality;
    
    // Base personality traits
    Personality.Courage = GetRandomFloat(0.2f, 0.8f);
    Personality.Intelligence = GetRandomFloat(0.3f, 0.9f);
    Personality.Empathy = GetRandomFloat(0.2f, 0.8f);
    Personality.Determination = GetRandomFloat(0.4f, 0.9f);
    Personality.Curiosity = GetRandomFloat(0.3f, 0.8f);
    Personality.Resilience = GetRandomFloat(0.4f, 0.8f);
    Personality.Leadership = GetRandomFloat(0.1f, 0.7f);
    Personality.Paranoia = GetRandomFloat(0.0f, 0.3f);
    Personality.Wisdom = GetRandomFloat(0.2f, 0.6f);
    
    // Archetype-specific adjustments
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            Personality.Intelligence += 0.2f;
            Personality.Curiosity += 0.3f;
            break;
        case ECharacterArchetype::Tribal_Elder:
            Personality.Wisdom += 0.4f;
            Personality.Leadership += 0.3f;
            break;
        case ECharacterArchetype::Survivor_Veteran:
            Personality.Resilience += 0.3f;
            Personality.Paranoia += 0.2f;
            break;
    }
    
    // Clamp all values to valid range
    Personality.Courage = FMath::Clamp(Personality.Courage, 0.0f, 1.0f);
    Personality.Intelligence = FMath::Clamp(Personality.Intelligence, 0.0f, 1.0f);
    Personality.Empathy = FMath::Clamp(Personality.Empathy, 0.0f, 1.0f);
    Personality.Determination = FMath::Clamp(Personality.Determination, 0.0f, 1.0f);
    Personality.Curiosity = FMath::Clamp(Personality.Curiosity, 0.0f, 1.0f);
    Personality.Resilience = FMath::Clamp(Personality.Resilience, 0.0f, 1.0f);
    Personality.Leadership = FMath::Clamp(Personality.Leadership, 0.0f, 1.0f);
    Personality.Paranoia = FMath::Clamp(Personality.Paranoia, 0.0f, 1.0f);
    Personality.Wisdom = FMath::Clamp(Personality.Wisdom, 0.0f, 1.0f);
    
    return Personality;
}

FCharacterBackstory UCharacterGenerator::GenerateBackstory(ECharacterArchetype Archetype, const FCharacterPersonality& Personality)
{
    FCharacterBackstory Backstory;
    
    // Generate backstory based on archetype and personality
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            Backstory.Profession = TEXT("Paleontologist");
            Backstory.Origin = TEXT("Academic researcher");
            Backstory.Motivation = TEXT("Scientific discovery and survival");
            break;
        case ECharacterArchetype::Tribal_Hunter:
            Backstory.Profession = TEXT("Hunter");
            Backstory.Origin = TEXT("Indigenous tribe");
            Backstory.Motivation = TEXT("Protect tribe and territory");
            break;
        case ECharacterArchetype::Survivor_Veteran:
            Backstory.Profession = TEXT("Survivor");
            Backstory.Origin = TEXT("Previous expedition member");
            Backstory.Motivation = TEXT("Find way home");
            break;
        default:
            Backstory.Profession = TEXT("Unknown");
            Backstory.Origin = TEXT("Mysterious past");
            Backstory.Motivation = TEXT("Survival");
            break;
    }
    
    // Add personality-influenced backstory elements
    if (Personality.Intelligence > 0.7f)
    {
        Backstory.Skills.Add(TEXT("Problem Solving"));
    }
    if (Personality.Leadership > 0.6f)
    {
        Backstory.Skills.Add(TEXT("Leadership"));
    }
    if (Personality.Resilience > 0.7f)
    {
        Backstory.Skills.Add(TEXT("Survival"));
    }
    
    return Backstory;
}

ECharacterEthnicity UCharacterGenerator::SelectRandomEthnicity()
{
    // Weighted random selection for diversity
    TArray<TPair<ECharacterEthnicity, float>> EthnicityWeights = {
        {ECharacterEthnicity::Caucasian, 0.25f},
        {ECharacterEthnicity::African, 0.20f},
        {ECharacterEthnicity::Asian, 0.20f},
        {ECharacterEthnicity::Hispanic, 0.15f},
        {ECharacterEthnicity::MiddleEastern, 0.10f},
        {ECharacterEthnicity::Indigenous, 0.10f}
    };
    
    float RandomValue = GetRandomFloat(0.0f, 1.0f);
    float CumulativeWeight = 0.0f;
    
    for (const auto& Weight : EthnicityWeights)
    {
        CumulativeWeight += Weight.Value;
        if (RandomValue <= CumulativeWeight)
        {
            return Weight.Key;
        }
    }
    
    return ECharacterEthnicity::Caucasian; // Fallback
}

ECharacterAge UCharacterGenerator::SelectRandomAge(ECharacterArchetype Archetype)
{
    // Age distribution based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::Tribal_Elder:
            return ECharacterAge::Elder;
        case ECharacterArchetype::Protagonist_Paleontologist:
            return GetRandomBool(0.8f) ? ECharacterAge::Adult : ECharacterAge::Young;
        default:
            {
                float RandomValue = GetRandomFloat(0.0f, 1.0f);
                if (RandomValue < 0.3f) return ECharacterAge::Young;
                if (RandomValue < 0.8f) return ECharacterAge::Adult;
                return ECharacterAge::Elder;
            }
    }
}

bool UCharacterGenerator::SelectRandomGender()
{
    return GetRandomBool(0.5f); // 50/50 male/female distribution
}

ECharacterBodyType UCharacterGenerator::SelectRandomBodyType(ECharacterAge Age, int32 SurvivalDays)
{
    // Body type influenced by age and survival experience
    TArray<ECharacterBodyType> PossibleTypes = {
        ECharacterBodyType::Slim,
        ECharacterBodyType::Average,
        ECharacterBodyType::Athletic,
        ECharacterBodyType::Stocky
    };
    
    // Survival tends toward leaner builds
    if (SurvivalDays > 30)
    {
        PossibleTypes.RemoveAll([](ECharacterBodyType Type) {
            return Type == ECharacterBodyType::Stocky;
        });
    }
    
    // Elders less likely to be athletic
    if (Age == ECharacterAge::Elder)
    {
        PossibleTypes.RemoveAll([](ECharacterBodyType Type) {
            return Type == ECharacterBodyType::Athletic;
        });
    }
    
    int32 RandomIndex = GetRandomInt(0, PossibleTypes.Num() - 1);
    return PossibleTypes[RandomIndex];
}

void UCharacterGenerator::ApplySurvivalWeathering(FCharacterVisualTraits& Traits, int32 DaysInJurassic)
{
    float SurvivalFactor = FMath::Clamp(DaysInJurassic / 365.0f, 0.0f, 1.0f);
    
    // Increase weathering based on survival time
    Traits.SkinWeathering = FMath::Lerp(Traits.SkinWeathering, 0.8f, SurvivalFactor * 0.5f);
    
    // Decrease fitness slightly due to harsh conditions
    Traits.PhysicalFitness = FMath::Lerp(Traits.PhysicalFitness, 0.6f, SurvivalFactor * 0.3f);
    
    // Increase muscle definition from survival activities
    Traits.MuscleDefinition = FMath::Lerp(Traits.MuscleDefinition, 0.8f, SurvivalFactor * 0.4f);
}

void UCharacterGenerator::ApplyArchetypeSpecificTraits(FCharacterVisualTraits& Traits, ECharacterArchetype Archetype)
{
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            Traits.Intelligence = FMath::Max(Traits.Intelligence, 0.7f);
            break;
        case ECharacterArchetype::Tribal_Hunter:
            Traits.PhysicalFitness = FMath::Max(Traits.PhysicalFitness, 0.8f);
            Traits.MuscleDefinition = FMath::Max(Traits.MuscleDefinition, 0.7f);
            break;
        case ECharacterArchetype::Tribal_Elder:
            Traits.Wisdom = FMath::Max(Traits.Intelligence, 0.8f);
            Traits.SkinWeathering = FMath::Max(Traits.SkinWeathering, 0.6f);
            break;
    }
}

void UCharacterGenerator::ValidateVisualCoherence(FCharacterDefinition& Character)
{
    // Ensure visual traits are coherent with each other
    if (Character.Age == ECharacterAge::Elder && Character.VisualTraits.PhysicalFitness > 0.8f)
    {
        Character.VisualTraits.PhysicalFitness = FMath::Clamp(Character.VisualTraits.PhysicalFitness, 0.3f, 0.7f);
    }
    
    if (Character.VisualTraits.MuscleDefinition > 0.8f && Character.VisualTraits.PhysicalFitness < 0.5f)
    {
        Character.VisualTraits.PhysicalFitness = FMath::Max(Character.VisualTraits.PhysicalFitness, 0.6f);
    }
}

void UCharacterGenerator::EnsureNarrativeConsistency(FCharacterDefinition& Character)
{
    // Ensure personality traits match backstory and archetype
    if (Character.Archetype == ECharacterArchetype::Tribal_Elder && Character.Personality.Wisdom < 0.6f)
    {
        Character.Personality.Wisdom = FMath::Max(Character.Personality.Wisdom, 0.7f);
    }
    
    if (Character.Archetype == ECharacterArchetype::Protagonist_Paleontologist && Character.Personality.Intelligence < 0.6f)
    {
        Character.Personality.Intelligence = FMath::Max(Character.Personality.Intelligence, 0.7f);
    }
}

float UCharacterGenerator::GetRandomFloat(float Min, float Max)
{
    return FMath::RandRange(Min, Max);
}

int32 UCharacterGenerator::GetRandomInt(int32 Min, int32 Max)
{
    return FMath::RandRange(Min, Max);
}

bool UCharacterGenerator::GetRandomBool(float TrueProbability)
{
    return FMath::RandRange(0.0f, 1.0f) < TrueProbability;
}

void UCharacterGenerator::InitializeMetaHumanPresets()
{
    // Initialize MetaHuman preset paths
    CachedMetaHumanPresets.Add(TEXT("/Game/MetaHumans/Presets/Caucasian_Male_Adult"));
    CachedMetaHumanPresets.Add(TEXT("/Game/MetaHumans/Presets/Caucasian_Female_Adult"));
    CachedMetaHumanPresets.Add(TEXT("/Game/MetaHumans/Presets/African_Male_Adult"));
    CachedMetaHumanPresets.Add(TEXT("/Game/MetaHumans/Presets/African_Female_Adult"));
    CachedMetaHumanPresets.Add(TEXT("/Game/MetaHumans/Presets/Asian_Male_Adult"));
    CachedMetaHumanPresets.Add(TEXT("/Game/MetaHumans/Presets/Asian_Female_Adult"));
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d MetaHuman presets"), CachedMetaHumanPresets.Num());
}

void UCharacterGenerator::LoadArchetypeData()
{
    // Load archetype data from data tables
    // This would typically load from UDataTable assets
    UE_LOG(LogTemp, Log, TEXT("Loading character archetype data"));
}