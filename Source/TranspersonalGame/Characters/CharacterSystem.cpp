#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

FCharacterData UCharacterGenerationSystem::GenerateRandomCharacter(ECharacterArchetype Archetype, bool bIsMale)
{
    FCharacterData NewCharacter;
    
    // Basic setup
    NewCharacter.Archetype = Archetype;
    NewCharacter.bIsMale = bIsMale;
    
    // Generate age based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalChild:
            NewCharacter.AgeGroup = ECharacterAge::Child;
            break;
        case ECharacterArchetype::TribalElder:
            NewCharacter.AgeGroup = ECharacterAge::Elder;
            break;
        case ECharacterArchetype::TribalLeader:
        case ECharacterArchetype::TribalShaman:
            NewCharacter.AgeGroup = FMath::RandBool() ? ECharacterAge::Adult : ECharacterAge::MiddleAged;
            break;
        default:
            // Random age for other archetypes
            int32 RandomAge = FMath::RandRange(0, 4);
            NewCharacter.AgeGroup = static_cast<ECharacterAge>(RandomAge);
            break;
    }
    
    // Generate physical traits
    NewCharacter.PhysicalTraits = GeneratePhysicalTraits(Archetype, NewCharacter.AgeGroup, bIsMale);
    
    // Generate survival marks
    NewCharacter.SurvivalMarks = GenerateSurvivalMarks(Archetype, NewCharacter.AgeGroup);
    
    // Generate name
    NewCharacter.CharacterName = GenerateName(Archetype, bIsMale);
    
    // Set MetaHuman asset path (to be populated with actual assets)
    NewCharacter.MetaHumanAssetPath = FString::Printf(TEXT("/Game/MetaHumans/%s_%s"), 
        bIsMale ? TEXT("Male") : TEXT("Female"), 
        *UEnum::GetValueAsString(Archetype));
    
    return NewCharacter;
}

FCharacterData UCharacterGenerationSystem::GenerateProtagonist()
{
    FCharacterData Protagonist;
    
    Protagonist.Archetype = ECharacterArchetype::Protagonist;
    Protagonist.bIsMale = true; // Default, can be changed
    Protagonist.AgeGroup = ECharacterAge::Adult;
    Protagonist.CharacterName = TEXT("Dr. Marcus Stone"); // Placeholder
    
    // Protagonist traits - educated, not used to survival
    Protagonist.PhysicalTraits.Height = 1.0f;
    Protagonist.PhysicalTraits.Weight = 0.8f; // Slightly lean
    Protagonist.PhysicalTraits.MuscleDefinition = 0.3f; // Not very muscular
    Protagonist.PhysicalTraits.SkinTone = FLinearColor(0.85f, 0.7f, 0.6f, 1.0f);
    Protagonist.PhysicalTraits.HairColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    Protagonist.PhysicalTraits.EyeColor = FLinearColor(0.3f, 0.5f, 0.7f, 1.0f); // Blue eyes
    
    // Minimal survival marks initially
    Protagonist.SurvivalMarks.ScarIntensity = 0.0f;
    Protagonist.SurvivalMarks.WeatheringLevel = 0.1f;
    Protagonist.SurvivalMarks.DirtLevel = 0.2f;
    Protagonist.SurvivalMarks.bHasTribalMarkings = false;
    Protagonist.SurvivalMarks.bHasBattleScars = false;
    
    Protagonist.MetaHumanAssetPath = TEXT("/Game/MetaHumans/Protagonist_Marcus");
    
    Protagonist.BackgroundStory = FText::FromString(
        TEXT("Dr. Marcus Stone was a respected paleontologist from the University of Cambridge. "
             "While excavating in a remote forest, he discovered an unusual crystalline artifact. "
             "Upon touching it, he was instantly transported millions of years into the past, "
             "into a world dominated by dinosaurs. Now he must use his scientific knowledge "
             "and learn survival skills to find another crystal and return home."));
    
    Protagonist.PersonalityTraits = FText::FromString(
        TEXT("Intelligent, curious, initially naive about survival but quick to learn. "
             "Maintains scientific objectivity even in extreme situations. "
             "Tends to overthink problems but has strong problem-solving abilities."));
    
    return Protagonist;
}

TArray<FCharacterData> UCharacterGenerationSystem::GenerateTribalGroup(int32 GroupSize)
{
    TArray<FCharacterData> TribalGroup;
    
    // Ensure we have at least one leader
    TribalGroup.Add(GenerateRandomCharacter(ECharacterArchetype::TribalLeader, FMath::RandBool()));
    
    // Add other members
    for (int32 i = 1; i < GroupSize; i++)
    {
        TArray<ECharacterArchetype> PossibleArchetypes = {
            ECharacterArchetype::TribalWarrior,
            ECharacterArchetype::TribalCrafter,
            ECharacterArchetype::TribalShaman,
            ECharacterArchetype::TribalChild,
            ECharacterArchetype::TribalElder
        };
        
        ECharacterArchetype RandomArchetype = PossibleArchetypes[FMath::RandRange(0, PossibleArchetypes.Num() - 1)];
        TribalGroup.Add(GenerateRandomCharacter(RandomArchetype, FMath::RandBool()));
    }
    
    return TribalGroup;
}

FCharacterPhysicalTraits UCharacterGenerationSystem::GeneratePhysicalTraits(ECharacterArchetype Archetype, ECharacterAge Age, bool bIsMale)
{
    FCharacterPhysicalTraits Traits;
    
    // Base height and weight variations
    Traits.Height = FMath::RandRange(0.85f, 1.15f);
    Traits.Weight = FMath::RandRange(0.8f, 1.2f);
    
    // Age adjustments
    switch (Age)
    {
        case ECharacterAge::Child:
            Traits.Height *= 0.6f;
            Traits.Weight *= 0.7f;
            Traits.MuscleDefinition = 0.1f;
            break;
        case ECharacterAge::Elder:
            Traits.Height *= 0.95f;
            Traits.Weight *= 1.1f;
            Traits.MuscleDefinition = FMath::RandRange(0.2f, 0.5f);
            break;
        default:
            Traits.MuscleDefinition = FMath::RandRange(0.3f, 0.8f);
            break;
    }
    
    // Archetype adjustments
    switch (Archetype)
    {
        case ECharacterArchetype::TribalWarrior:
            Traits.MuscleDefinition = FMath::RandRange(0.7f, 1.0f);
            Traits.Height = FMath::Max(Traits.Height, 0.95f);
            break;
        case ECharacterArchetype::TribalShaman:
            Traits.MuscleDefinition = FMath::RandRange(0.2f, 0.5f);
            break;
        case ECharacterArchetype::Protagonist:
            Traits.MuscleDefinition = 0.3f; // Academic, not physical
            break;
    }
    
    // Generate diverse skin tones
    float SkinVariation = FMath::RandRange(0.0f, 1.0f);
    if (SkinVariation < 0.3f)
    {
        // Lighter skin tones
        Traits.SkinTone = FLinearColor(
            FMath::RandRange(0.8f, 0.95f),
            FMath::RandRange(0.6f, 0.8f),
            FMath::RandRange(0.5f, 0.7f),
            1.0f
        );
    }
    else if (SkinVariation < 0.7f)
    {
        // Medium skin tones
        Traits.SkinTone = FLinearColor(
            FMath::RandRange(0.6f, 0.8f),
            FMath::RandRange(0.4f, 0.6f),
            FMath::RandRange(0.3f, 0.5f),
            1.0f
        );
    }
    else
    {
        // Darker skin tones
        Traits.SkinTone = FLinearColor(
            FMath::RandRange(0.3f, 0.6f),
            FMath::RandRange(0.2f, 0.4f),
            FMath::RandRange(0.15f, 0.3f),
            1.0f
        );
    }
    
    // Hair color variations
    float HairVariation = FMath::RandRange(0.0f, 1.0f);
    if (HairVariation < 0.4f)
    {
        // Brown hair
        Traits.HairColor = FLinearColor(
            FMath::RandRange(0.2f, 0.5f),
            FMath::RandRange(0.1f, 0.3f),
            FMath::RandRange(0.05f, 0.2f),
            1.0f
        );
    }
    else if (HairVariation < 0.7f)
    {
        // Black hair
        Traits.HairColor = FLinearColor(
            FMath::RandRange(0.05f, 0.2f),
            FMath::RandRange(0.05f, 0.2f),
            FMath::RandRange(0.05f, 0.2f),
            1.0f
        );
    }
    else if (HairVariation < 0.9f)
    {
        // Blonde hair
        Traits.HairColor = FLinearColor(
            FMath::RandRange(0.6f, 0.9f),
            FMath::RandRange(0.5f, 0.8f),
            FMath::RandRange(0.3f, 0.6f),
            1.0f
        );
    }
    else
    {
        // Red hair
        Traits.HairColor = FLinearColor(
            FMath::RandRange(0.6f, 0.9f),
            FMath::RandRange(0.2f, 0.4f),
            FMath::RandRange(0.1f, 0.3f),
            1.0f
        );
    }
    
    // Eye color variations
    float EyeVariation = FMath::RandRange(0.0f, 1.0f);
    if (EyeVariation < 0.5f)
    {
        // Brown eyes
        Traits.EyeColor = FLinearColor(
            FMath::RandRange(0.3f, 0.6f),
            FMath::RandRange(0.2f, 0.4f),
            FMath::RandRange(0.1f, 0.3f),
            1.0f
        );
    }
    else if (EyeVariation < 0.8f)
    {
        // Blue eyes
        Traits.EyeColor = FLinearColor(
            FMath::RandRange(0.2f, 0.4f),
            FMath::RandRange(0.4f, 0.6f),
            FMath::RandRange(0.6f, 0.9f),
            1.0f
        );
    }
    else
    {
        // Green eyes
        Traits.EyeColor = FLinearColor(
            FMath::RandRange(0.2f, 0.4f),
            FMath::RandRange(0.5f, 0.8f),
            FMath::RandRange(0.3f, 0.5f),
            1.0f
        );
    }
    
    return Traits;
}

FCharacterSurvivalMarks UCharacterGenerationSystem::GenerateSurvivalMarks(ECharacterArchetype Archetype, ECharacterAge Age)
{
    FCharacterSurvivalMarks Marks;
    
    // Base survival marks based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalWarrior:
            Marks.ScarIntensity = FMath::RandRange(0.5f, 0.9f);
            Marks.bHasBattleScars = true;
            Marks.bHasTribalMarkings = true;
            break;
        case ECharacterArchetype::TribalLeader:
            Marks.ScarIntensity = FMath::RandRange(0.3f, 0.7f);
            Marks.bHasBattleScars = FMath::RandBool();
            Marks.bHasTribalMarkings = true;
            break;
        case ECharacterArchetype::TribalShaman:
            Marks.ScarIntensity = FMath::RandRange(0.1f, 0.4f);
            Marks.bHasTribalMarkings = true;
            break;
        case ECharacterArchetype::TribalChild:
            Marks.ScarIntensity = FMath::RandRange(0.0f, 0.2f);
            Marks.bHasTribalMarkings = FMath::RandBool();
            break;
        case ECharacterArchetype::Protagonist:
            Marks.ScarIntensity = 0.0f; // Starts clean
            break;
        default:
            Marks.ScarIntensity = FMath::RandRange(0.2f, 0.6f);
            break;
    }
    
    // Age adjustments
    switch (Age)
    {
        case ECharacterAge::Elder:
            Marks.WeatheringLevel = FMath::RandRange(0.6f, 1.0f);
            Marks.ScarIntensity = FMath::Min(Marks.ScarIntensity + 0.3f, 1.0f);
            break;
        case ECharacterAge::Child:
            Marks.WeatheringLevel = FMath::RandRange(0.0f, 0.3f);
            break;
        default:
            Marks.WeatheringLevel = FMath::RandRange(0.3f, 0.7f);
            break;
    }
    
    // Dirt level based on lifestyle
    if (Archetype == ECharacterArchetype::TribalShaman)
    {
        Marks.DirtLevel = FMath::RandRange(0.1f, 0.4f); // Cleaner
    }
    else if (Archetype == ECharacterArchetype::TribalCrafter)
    {
        Marks.DirtLevel = FMath::RandRange(0.4f, 0.8f); // Working hands
    }
    else
    {
        Marks.DirtLevel = FMath::RandRange(0.3f, 0.7f);
    }
    
    return Marks;
}

FString UCharacterGenerationSystem::GenerateName(ECharacterArchetype Archetype, bool bIsMale)
{
    TArray<FString> MaleNames;
    TArray<FString> FemaleNames;
    
    if (Archetype == ECharacterArchetype::Protagonist)
    {
        return bIsMale ? TEXT("Dr. Marcus Stone") : TEXT("Dr. Elena Stone");
    }
    
    // Tribal names with prehistoric/nature theme
    MaleNames = {
        TEXT("Kael"), TEXT("Thane"), TEXT("Brom"), TEXT("Drak"), TEXT("Grom"),
        TEXT("Tor"), TEXT("Vex"), TEXT("Zane"), TEXT("Rok"), TEXT("Jax"),
        TEXT("Flint"), TEXT("Stone"), TEXT("Bear"), TEXT("Wolf"), TEXT("Hawk")
    };
    
    FemaleNames = {
        TEXT("Lyra"), TEXT("Nyx"), TEXT("Vera"), TEXT("Zara"), TEXT("Kira"),
        TEXT("Thea"), TEXT("Mira"), TEXT("Senna"), TEXT("Ava"), TEXT("Luna"),
        TEXT("Sage"), TEXT("Ivy"), TEXT("Dawn"), TEXT("Raven"), TEXT("Willow")
    };
    
    if (bIsMale)
    {
        return MaleNames[FMath::RandRange(0, MaleNames.Num() - 1)];
    }
    else
    {
        return FemaleNames[FMath::RandRange(0, FemaleNames.Num() - 1)];
    }
}