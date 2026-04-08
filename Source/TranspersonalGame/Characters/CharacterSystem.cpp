#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UCharacterSystem::UCharacterSystem()
{
    // Initialize archetype name tables
    ArchetypeNames.Add(ECharacterArchetype::Protagonist, {
        TEXT("Dr. Marcus Chen"), TEXT("Dr. Sarah Williams"), TEXT("Dr. Ahmed Hassan"),
        TEXT("Dr. Elena Rodriguez"), TEXT("Dr. James Thompson"), TEXT("Dr. Kenji Nakamura")
    });

    ArchetypeNames.Add(ECharacterArchetype::TribalElder, {
        TEXT("Wise Akela"), TEXT("Elder Nayara"), TEXT("Chief Kofi"), TEXT("Grandmother Aiyana"),
        TEXT("Shaman Tau"), TEXT("Elder Kesia")
    });

    ArchetypeNames.Add(ECharacterArchetype::TribalWarrior, {
        TEXT("Hunter Zuberi"), TEXT("Warrior Kaia"), TEXT("Scout Jengo"), TEXT("Guardian Amara"),
        TEXT("Tracker Bomani"), TEXT("Fighter Asha")
    });

    ArchetypeNames.Add(ECharacterArchetype::TribalHealer, {
        TEXT("Healer Nia"), TEXT("Medicine Woman Sade"), TEXT("Herbalist Kwame"),
        TEXT("Wise Woman Zara"), TEXT("Spirit Guide Jeneba"), TEXT("Healer Makena")
    });

    ArchetypeNames.Add(ECharacterArchetype::TribalChild, {
        TEXT("Little Kesi"), TEXT("Young Tau"), TEXT("Child Ama"), TEXT("Small Jengo"),
        TEXT("Little Nia"), TEXT("Young Kofi")
    });

    ArchetypeNames.Add(ECharacterArchetype::TribalCrafter, {
        TEXT("Toolmaker Bomani"), TEXT("Weaver Sade"), TEXT("Potter Amara"),
        TEXT("Carver Zuberi"), TEXT("Smith Kwame"), TEXT("Builder Kaia")
    });

    ArchetypeNames.Add(ECharacterArchetype::Survivor, {
        TEXT("Lost Explorer"), TEXT("Stranded Researcher"), TEXT("Crashed Pilot"),
        TEXT("Shipwrecked Sailor"), TEXT("Missing Hiker"), TEXT("Lost Journalist")
    });

    ArchetypeNames.Add(ECharacterArchetype::Hermit, {
        TEXT("The Wanderer"), TEXT("Forest Ghost"), TEXT("Cave Dweller"),
        TEXT("Mountain Hermit"), TEXT("Swamp Mystic"), TEXT("Desert Walker")
    });

    // Initialize ethnicity feature tables
    EthnicityFeatures.Add(ECharacterEthnicity::African, {
        TEXT("DarkSkin"), TEXT("CurlyHair"), TEXT("BroadNose"), TEXT("FullLips")
    });

    EthnicityFeatures.Add(ECharacterEthnicity::Asian, {
        TEXT("YellowSkin"), TEXT("StraightHair"), TEXT("MonolidEyes"), TEXT("SlimBuild")
    });

    EthnicityFeatures.Add(ECharacterEthnicity::Caucasian, {
        TEXT("LightSkin"), TEXT("VariedHair"), TEXT("NarrowNose"), TEXT("ThinLips")
    });

    EthnicityFeatures.Add(ECharacterEthnicity::Hispanic, {
        TEXT("OliveSkin"), TEXT("DarkHair"), TEXT("BrownEyes"), TEXT("MediumBuild")
    });

    EthnicityFeatures.Add(ECharacterEthnicity::Indigenous, {
        TEXT("CopperSkin"), TEXT("StraightDarkHair"), TEXT("HighCheekbones"), TEXT("StrongJaw")
    });

    EthnicityFeatures.Add(ECharacterEthnicity::MiddleEastern, {
        TEXT("OliveSkin"), TEXT("DarkCurlyHair"), TEXT("LargeEyes"), TEXT("ProminentNose")
    });

    EthnicityFeatures.Add(ECharacterEthnicity::Mixed, {
        TEXT("MixedFeatures"), TEXT("VariedSkin"), TEXT("BlendedTraits"), TEXT("UniqueAppearance")
    });
}

FCharacterVariationData UCharacterSystem::GenerateRandomCharacter(ECharacterArchetype Archetype)
{
    FCharacterVariationData NewCharacter;
    NewCharacter.Archetype = Archetype;

    // Random ethnicity distribution
    TArray<ECharacterEthnicity> Ethnicities = {
        ECharacterEthnicity::African,
        ECharacterEthnicity::Asian,
        ECharacterEthnicity::Caucasian,
        ECharacterEthnicity::Hispanic,
        ECharacterEthnicity::Indigenous,
        ECharacterEthnicity::MiddleEastern,
        ECharacterEthnicity::Mixed
    };
    
    NewCharacter.Ethnicity = Ethnicities[FMath::RandRange(0, Ethnicities.Num() - 1)];

    // Age distribution based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalChild:
            NewCharacter.Age = ECharacterAge::Child;
            break;
        case ECharacterArchetype::TribalElder:
            NewCharacter.Age = ECharacterAge::Elder;
            break;
        case ECharacterArchetype::Protagonist:
            NewCharacter.Age = (FMath::RandBool()) ? ECharacterAge::YoungAdult : ECharacterAge::MiddleAged;
            break;
        default:
            // Random age for other archetypes
            TArray<ECharacterAge> Ages = {
                ECharacterAge::Teenager,
                ECharacterAge::YoungAdult,
                ECharacterAge::MiddleAged
            };
            NewCharacter.Age = Ages[FMath::RandRange(0, Ages.Num() - 1)];
            break;
    }

    // Random gender
    NewCharacter.bIsMale = FMath::RandBool();

    // Physical variations
    NewCharacter.HeightVariation = FMath::FRandRange(0.2f, 0.8f);
    NewCharacter.BodyWeightVariation = FMath::FRandRange(0.3f, 0.7f);
    NewCharacter.MuscleDefinition = FMath::FRandRange(0.2f, 0.9f);
    
    // Survival wear based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            NewCharacter.SkinWear = FMath::FRandRange(0.1f, 0.4f); // Recently arrived
            NewCharacter.ScarLevel = FMath::FRandRange(0.0f, 0.2f);
            break;
        case ECharacterArchetype::TribalWarrior:
            NewCharacter.SkinWear = FMath::FRandRange(0.6f, 0.9f); // Battle-hardened
            NewCharacter.ScarLevel = FMath::FRandRange(0.4f, 0.8f);
            break;
        case ECharacterArchetype::Hermit:
            NewCharacter.SkinWear = FMath::FRandRange(0.7f, 1.0f); // Weathered
            NewCharacter.ScarLevel = FMath::FRandRange(0.3f, 0.7f);
            break;
        default:
            NewCharacter.SkinWear = FMath::FRandRange(0.3f, 0.6f);
            NewCharacter.ScarLevel = FMath::FRandRange(0.1f, 0.4f);
            break;
    }

    return NewCharacter;
}

bool UCharacterSystem::ValidateCharacterData(const FCharacterVariationData& CharacterData)
{
    // Check if all values are within valid ranges
    if (CharacterData.HeightVariation < 0.0f || CharacterData.HeightVariation > 1.0f) return false;
    if (CharacterData.BodyWeightVariation < 0.0f || CharacterData.BodyWeightVariation > 1.0f) return false;
    if (CharacterData.MuscleDefinition < 0.0f || CharacterData.MuscleDefinition > 1.0f) return false;
    if (CharacterData.SkinWear < 0.0f || CharacterData.SkinWear > 1.0f) return false;
    if (CharacterData.ScarLevel < 0.0f || CharacterData.ScarLevel > 1.0f) return false;

    // Check logical consistency
    if (CharacterData.Archetype == ECharacterArchetype::TribalChild && CharacterData.Age != ECharacterAge::Child)
    {
        return false;
    }

    if (CharacterData.Archetype == ECharacterArchetype::TribalElder && CharacterData.Age != ECharacterAge::Elder)
    {
        return false;
    }

    return true;
}

TArray<FString> UCharacterSystem::GetArchetypeClothing(ECharacterArchetype Archetype)
{
    TArray<FString> ClothingItems;

    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            ClothingItems.Add(TEXT("ModernShirt"));
            ClothingItems.Add(TEXT("CargoPants"));
            ClothingItems.Add(TEXT("HikingBoots"));
            ClothingItems.Add(TEXT("UtilityVest"));
            break;

        case ECharacterArchetype::TribalWarrior:
            ClothingItems.Add(TEXT("LeatherChest"));
            ClothingItems.Add(TEXT("TribalLoincloth"));
            ClothingItems.Add(TEXT("BoneArmor"));
            ClothingItems.Add(TEXT("FeatherHeadband"));
            break;

        case ECharacterArchetype::TribalHealer:
            ClothingItems.Add(TEXT("ShamanRobes"));
            ClothingItems.Add(TEXT("HerbPouch"));
            ClothingItems.Add(TEXT("SpiritNecklace"));
            ClothingItems.Add(TEXT("FeatheredCloak"));
            break;

        case ECharacterArchetype::TribalElder:
            ClothingItems.Add(TEXT("ElderRobes"));
            ClothingItems.Add(TEXT("WisdomStaff"));
            ClothingItems.Add(TEXT("TribalMarkings"));
            ClothingItems.Add(TEXT("CeremonialHeaddress"));
            break;

        case ECharacterArchetype::TribalChild:
            ClothingItems.Add(TEXT("SimpleCloth"));
            ClothingItems.Add(TEXT("ChildSandals"));
            ClothingItems.Add(TEXT("PlayNecklace"));
            break;

        case ECharacterArchetype::TribalCrafter:
            ClothingItems.Add(TEXT("WorkApron"));
            ClothingItems.Add(TEXT("ToolBelt"));
            ClothingItems.Add(TEXT("CraftedClothes"));
            ClothingItems.Add(TEXT("ProtectiveGloves"));
            break;

        case ECharacterArchetype::Survivor:
            ClothingItems.Add(TEXT("TornClothes"));
            ClothingItems.Add(TEXT("MakeShiftGear"));
            ClothingItems.Add(TEXT("SurvivalBoots"));
            ClothingItems.Add(TEXT("EmergencyBlanket"));
            break;

        case ECharacterArchetype::Hermit:
            ClothingItems.Add(TEXT("WeatheredRobes"));
            ClothingItems.Add(TEXT("AnimalHides"));
            ClothingItems.Add(TEXT("HandmadeBoots"));
            ClothingItems.Add(TEXT("NaturalCamouflage"));
            break;
    }

    return ClothingItems;
}

float UCharacterSystem::CalculateSurvivalWear(float DaysInWilderness, ECharacterArchetype Archetype)
{
    float BaseWear = 0.0f;
    
    // Base wear rate per archetype
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            BaseWear = 0.02f; // Modern person, not adapted
            break;
        case ECharacterArchetype::TribalWarrior:
            BaseWear = 0.005f; // Hardy, used to environment
            break;
        case ECharacterArchetype::Hermit:
            BaseWear = 0.003f; // Most adapted
            break;
        default:
            BaseWear = 0.01f;
            break;
    }

    float CalculatedWear = BaseWear * DaysInWilderness;
    return FMath::Clamp(CalculatedWear, 0.0f, 1.0f);
}