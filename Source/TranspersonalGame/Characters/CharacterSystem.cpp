#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

TMap<ECharacterArchetype, UCharacterArchetypeData*> UCharacterGenerator::ArchetypeDatabase;

FCharacterVariation UCharacterGenerator::GenerateRandomVariation(ECharacterArchetype Archetype, ECharacterGender Gender, ECharacterAge Age, ECharacterEthnicity Ethnicity)
{
    FCharacterVariation Variation;

    // Base variation ranges based on ethnicity
    switch (Ethnicity)
    {
        case ECharacterEthnicity::African:
            Variation.SkinTone = FMath::RandRange(0.1f, 0.4f);
            Variation.EyeColor = FMath::RandRange(0.0f, 0.2f); // Darker eyes
            Variation.HairColor = FMath::RandRange(0.0f, 0.3f); // Darker hair
            break;
            
        case ECharacterEthnicity::Asian:
            Variation.SkinTone = FMath::RandRange(0.3f, 0.6f);
            Variation.EyeColor = FMath::RandRange(0.0f, 0.3f);
            Variation.HairColor = FMath::RandRange(0.0f, 0.2f);
            break;
            
        case ECharacterEthnicity::European:
            Variation.SkinTone = FMath::RandRange(0.6f, 0.9f);
            Variation.EyeColor = FMath::RandRange(0.2f, 0.8f); // More variety
            Variation.HairColor = FMath::RandRange(0.1f, 0.9f); // Full range
            break;
            
        case ECharacterEthnicity::Indigenous:
            Variation.SkinTone = FMath::RandRange(0.4f, 0.7f);
            Variation.EyeColor = FMath::RandRange(0.0f, 0.3f);
            Variation.HairColor = FMath::RandRange(0.0f, 0.3f);
            break;
            
        case ECharacterEthnicity::MiddleEastern:
            Variation.SkinTone = FMath::RandRange(0.4f, 0.7f);
            Variation.EyeColor = FMath::RandRange(0.0f, 0.4f);
            Variation.HairColor = FMath::RandRange(0.0f, 0.4f);
            break;
            
        case ECharacterEthnicity::Mixed:
            Variation.SkinTone = FMath::RandRange(0.2f, 0.8f);
            Variation.EyeColor = FMath::RandRange(0.0f, 0.7f);
            Variation.HairColor = FMath::RandRange(0.0f, 0.8f);
            break;
    }

    // Age-based modifications
    switch (Age)
    {
        case ECharacterAge::Child:
            Variation.Height = FMath::RandRange(0.1f, 0.3f);
            Variation.Weight = FMath::RandRange(0.1f, 0.3f);
            Variation.Muscle = FMath::RandRange(0.0f, 0.2f);
            Variation.SkinWeathering = 0.0f;
            break;
            
        case ECharacterAge::Teenager:
            Variation.Height = FMath::RandRange(0.3f, 0.7f);
            Variation.Weight = FMath::RandRange(0.2f, 0.6f);
            Variation.Muscle = FMath::RandRange(0.1f, 0.5f);
            Variation.SkinWeathering = FMath::RandRange(0.0f, 0.1f);
            break;
            
        case ECharacterAge::YoungAdult:
            Variation.Height = FMath::RandRange(0.4f, 0.9f);
            Variation.Weight = FMath::RandRange(0.3f, 0.8f);
            Variation.Muscle = FMath::RandRange(0.2f, 0.8f);
            Variation.SkinWeathering = FMath::RandRange(0.0f, 0.2f);
            break;
            
        case ECharacterAge::Adult:
            Variation.Height = FMath::RandRange(0.4f, 0.9f);
            Variation.Weight = FMath::RandRange(0.3f, 0.9f);
            Variation.Muscle = FMath::RandRange(0.3f, 0.9f);
            Variation.SkinWeathering = FMath::RandRange(0.1f, 0.4f);
            break;
            
        case ECharacterAge::MiddleAged:
            Variation.Height = FMath::RandRange(0.4f, 0.9f);
            Variation.Weight = FMath::RandRange(0.4f, 0.9f);
            Variation.Muscle = FMath::RandRange(0.2f, 0.7f);
            Variation.SkinWeathering = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case ECharacterAge::Elder:
            Variation.Height = FMath::RandRange(0.3f, 0.8f);
            Variation.Weight = FMath::RandRange(0.2f, 0.8f);
            Variation.Muscle = FMath::RandRange(0.1f, 0.5f);
            Variation.SkinWeathering = FMath::RandRange(0.5f, 0.9f);
            break;
    }

    // Gender-based modifications
    if (Gender == ECharacterGender::Male)
    {
        Variation.Muscle += 0.1f; // Males tend to be more muscular
        Variation.FaceWidth += 0.1f;
        Variation.JawWidth += 0.15f;
    }
    else if (Gender == ECharacterGender::Female)
    {
        Variation.Weight -= 0.05f; // Slight adjustment
        Variation.FaceWidth -= 0.05f;
        Variation.JawWidth -= 0.1f;
    }

    // Archetype-based modifications
    switch (Archetype)
    {
        case ECharacterArchetype::TribalHunter:
            Variation.Muscle += 0.2f;
            Variation.Weight -= 0.1f; // Lean and strong
            Variation.SunDamage += 0.3f; // Outdoor exposure
            break;
            
        case ECharacterArchetype::TribalElder:
            Variation.SkinWeathering += 0.2f;
            Variation.Weight += 0.1f;
            break;
            
        case ECharacterArchetype::WildHermit:
            Variation.SkinWeathering += 0.4f;
            Variation.SunDamage += 0.5f;
            Variation.Weight -= 0.2f; // Malnourished
            break;
            
        case ECharacterArchetype::Paleontologist:
            Variation.SunDamage += 0.2f; // Field work
            break;
    }

    // Facial feature randomization
    Variation.FaceWidth = FMath::Clamp(Variation.FaceWidth + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    Variation.JawWidth = FMath::Clamp(Variation.JawWidth + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    Variation.NoseSize = FMath::RandRange(0.2f, 0.8f);
    Variation.EyeSize = FMath::RandRange(0.3f, 0.7f);

    // Clamp all values
    Variation.SkinTone = FMath::Clamp(Variation.SkinTone, 0.0f, 1.0f);
    Variation.EyeColor = FMath::Clamp(Variation.EyeColor, 0.0f, 1.0f);
    Variation.HairColor = FMath::Clamp(Variation.HairColor, 0.0f, 1.0f);
    Variation.Height = FMath::Clamp(Variation.Height, 0.0f, 1.0f);
    Variation.Weight = FMath::Clamp(Variation.Weight, 0.0f, 1.0f);
    Variation.Muscle = FMath::Clamp(Variation.Muscle, 0.0f, 1.0f);
    Variation.SkinWeathering = FMath::Clamp(Variation.SkinWeathering, 0.0f, 1.0f);
    Variation.SunDamage = FMath::Clamp(Variation.SunDamage, 0.0f, 1.0f);

    // Generate battle scars
    Variation.BattleScars = GenerateBattleScars(Archetype, Age);

    return Variation;
}

FCharacterClothing UCharacterGenerator::SelectClothingForArchetype(ECharacterArchetype Archetype, float WearLevel)
{
    FCharacterClothing Clothing;
    
    Clothing.ClothingWear = WearLevel;
    Clothing.DirtLevel = FMath::RandRange(0.2f, 0.8f);
    
    switch (Archetype)
    {
        case ECharacterArchetype::Paleontologist:
            // Modern field gear, but worn and damaged
            Clothing.ClothingWear = FMath::RandRange(0.6f, 0.9f);
            Clothing.DirtLevel = FMath::RandRange(0.5f, 0.8f);
            Clothing.BloodStains = FMath::RandRange(0.1f, 0.3f);
            break;
            
        case ECharacterArchetype::TribalHunter:
            // Animal hide, bone accessories
            Clothing.ClothingWear = FMath::RandRange(0.3f, 0.7f);
            Clothing.DirtLevel = FMath::RandRange(0.4f, 0.7f);
            Clothing.BloodStains = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case ECharacterArchetype::TribalElder:
            // Ceremonial clothing, better maintained
            Clothing.ClothingWear = FMath::RandRange(0.1f, 0.4f);
            Clothing.DirtLevel = FMath::RandRange(0.1f, 0.4f);
            Clothing.BloodStains = FMath::RandRange(0.0f, 0.1f);
            break;
            
        case ECharacterArchetype::WildHermit:
            // Extremely worn, makeshift clothing
            Clothing.ClothingWear = FMath::RandRange(0.7f, 1.0f);
            Clothing.DirtLevel = FMath::RandRange(0.8f, 1.0f);
            Clothing.BloodStains = FMath::RandRange(0.2f, 0.5f);
            break;
            
        case ECharacterArchetype::TribalShaman:
            // Decorated with feathers, bones, paint
            Clothing.ClothingWear = FMath::RandRange(0.2f, 0.5f);
            Clothing.DirtLevel = FMath::RandRange(0.3f, 0.6f);
            break;
    }
    
    return Clothing;
}

TArray<FString> UCharacterGenerator::GenerateBattleScars(ECharacterArchetype Archetype, ECharacterAge Age)
{
    TArray<FString> Scars;
    
    // Base scar probability based on age
    float ScarProbability = 0.0f;
    switch (Age)
    {
        case ECharacterAge::Child:
            ScarProbability = 0.1f;
            break;
        case ECharacterAge::Teenager:
            ScarProbability = 0.2f;
            break;
        case ECharacterAge::YoungAdult:
            ScarProbability = 0.4f;
            break;
        case ECharacterAge::Adult:
            ScarProbability = 0.6f;
            break;
        case ECharacterAge::MiddleAged:
            ScarProbability = 0.7f;
            break;
        case ECharacterAge::Elder:
            ScarProbability = 0.8f;
            break;
    }
    
    // Archetype modifiers
    switch (Archetype)
    {
        case ECharacterArchetype::TribalHunter:
            ScarProbability += 0.3f;
            break;
        case ECharacterArchetype::WildHermit:
            ScarProbability += 0.4f;
            break;
        case ECharacterArchetype::InjuredSurvivor:
            ScarProbability += 0.5f;
            break;
        case ECharacterArchetype::TribalElder:
            ScarProbability += 0.2f;
            break;
        case ECharacterArchetype::Paleontologist:
            ScarProbability += 0.1f;
            break;
    }
    
    ScarProbability = FMath::Clamp(ScarProbability, 0.0f, 1.0f);
    
    // Possible scar types
    TArray<FString> PossibleScars = {
        "Claw_Mark_Face_Left",
        "Claw_Mark_Face_Right", 
        "Claw_Mark_Arm_Left",
        "Claw_Mark_Arm_Right",
        "Bite_Mark_Shoulder",
        "Burn_Scar_Hand",
        "Cut_Scar_Forehead",
        "Puncture_Wound_Leg",
        "Scratch_Mark_Chest",
        "Old_Wound_Back"
    };
    
    // Generate scars based on probability
    for (const FString& ScarType : PossibleScars)
    {
        if (FMath::RandRange(0.0f, 1.0f) < ScarProbability * 0.3f) // Each scar has 30% of base probability
        {
            Scars.Add(ScarType);
        }
    }
    
    return Scars;
}

float UCharacterGenerator::CalculateWeatheringLevel(ECharacterArchetype Archetype, ECharacterAge Age)
{
    float BaseWeathering = 0.0f;
    
    // Age-based weathering
    switch (Age)
    {
        case ECharacterAge::Child:
            BaseWeathering = 0.0f;
            break;
        case ECharacterAge::Teenager:
            BaseWeathering = 0.1f;
            break;
        case ECharacterAge::YoungAdult:
            BaseWeathering = 0.2f;
            break;
        case ECharacterAge::Adult:
            BaseWeathering = 0.4f;
            break;
        case ECharacterAge::MiddleAged:
            BaseWeathering = 0.6f;
            break;
        case ECharacterAge::Elder:
            BaseWeathering = 0.8f;
            break;
    }
    
    // Archetype modifiers
    switch (Archetype)
    {
        case ECharacterArchetype::WildHermit:
            BaseWeathering += 0.3f;
            break;
        case ECharacterArchetype::TribalHunter:
            BaseWeathering += 0.2f;
            break;
        case ECharacterArchetype::InjuredSurvivor:
            BaseWeathering += 0.25f;
            break;
        case ECharacterArchetype::Paleontologist:
            BaseWeathering += 0.15f;
            break;
        case ECharacterArchetype::TribalElder:
            BaseWeathering += 0.1f;
            break;
    }
    
    return FMath::Clamp(BaseWeathering, 0.0f, 1.0f);
}