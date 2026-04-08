#include "CharacterArchetypeSystem.h"
#include "Engine/DataTable.h"

FCharacterArchetypeData UCharacterArchetypeDataAsset::GetArchetypeData(ECharacterArchetype Archetype) const
{
    if (!ArchetypeDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchetypeDataTable is null in UCharacterArchetypeDataAsset"));
        return FCharacterArchetypeData();
    }

    FString ArchetypeName = UEnum::GetValueAsString(Archetype);
    FCharacterArchetypeData* FoundData = ArchetypeDataTable->FindRow<FCharacterArchetypeData>(FName(*ArchetypeName), TEXT(""));
    
    if (FoundData)
    {
        return *FoundData;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Could not find archetype data for %s"), *ArchetypeName);
    return FCharacterArchetypeData();
}

FCharacterArchetypeData UCharacterArchetypeDataAsset::GenerateRandomCharacter(ECharacterArchetype Archetype) const
{
    FCharacterArchetypeData BaseData = GetArchetypeData(Archetype);
    
    // Randomize physical traits
    if (BaseData.PhysicalTraits.Gender == ECharacterGender::Random)
    {
        BaseData.PhysicalTraits.Gender = FMath::RandBool() ? ECharacterGender::Male : ECharacterGender::Female;
    }
    
    // Randomize skin tone within reasonable range
    int32 SkinToneVariation = FMath::RandRange(0, 4);
    BaseData.PhysicalTraits.SkinTone = static_cast<ESkinTone>(SkinToneVariation);
    
    // Randomize height within archetype-appropriate range
    float HeightVariation = FMath::RandRange(-0.1f, 0.1f);
    BaseData.PhysicalTraits.HeightScale = FMath::Clamp(BaseData.PhysicalTraits.HeightScale + HeightVariation, 0.8f, 1.2f);
    
    // Randomize hair color
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Dark Brown
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f),  // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),  // Light Brown
        FLinearColor(0.8f, 0.6f, 0.2f, 1.0f),  // Blonde
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f),  // Gray
        FLinearColor(0.8f, 0.2f, 0.1f, 1.0f)   // Red
    };
    
    if (HairColors.Num() > 0)
    {
        BaseData.PhysicalTraits.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    }
    
    // Randomize eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Brown
        FLinearColor(0.1f, 0.3f, 0.6f, 1.0f),  // Blue
        FLinearColor(0.2f, 0.4f, 0.2f, 1.0f),  // Green
        FLinearColor(0.3f, 0.3f, 0.3f, 1.0f),  // Gray
        FLinearColor(0.1f, 0.1f, 0.1f, 1.0f)   // Black
    };
    
    if (EyeColors.Num() > 0)
    {
        BaseData.PhysicalTraits.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    }
    
    // Add random scars based on archetype (survivors should have more)
    TArray<FString> PossibleScars = {
        TEXT("Facial Scar"),
        TEXT("Arm Scar"),
        TEXT("Leg Scar"),
        TEXT("Hand Scar"),
        TEXT("Claw Marks"),
        TEXT("Burn Mark")
    };
    
    float ScarChance = 0.3f; // Base 30% chance
    if (Archetype == ECharacterArchetype::TribalWarrior || 
        Archetype == ECharacterArchetype::HostileRaider ||
        Archetype == ECharacterArchetype::WildHermit)
    {
        ScarChance = 0.7f; // 70% chance for combat archetypes
    }
    
    BaseData.PhysicalTraits.Scars.Empty();
    for (const FString& Scar : PossibleScars)
    {
        if (FMath::RandRange(0.0f, 1.0f) < ScarChance)
        {
            BaseData.PhysicalTraits.Scars.Add(Scar);
        }
    }
    
    // Randomize clothing wear based on archetype
    if (Archetype == ECharacterArchetype::WildHermit || 
        Archetype == ECharacterArchetype::LostSurvivor)
    {
        BaseData.Clothing.WearLevel = FMath::RandRange(0.6f, 1.0f); // Very worn
    }
    else if (Archetype == ECharacterArchetype::TribalLeader || 
             Archetype == ECharacterArchetype::TribalShaman)
    {
        BaseData.Clothing.WearLevel = FMath::RandRange(0.0f, 0.3f); // Well maintained
    }
    else
    {
        BaseData.Clothing.WearLevel = FMath::RandRange(0.2f, 0.7f); // Normal wear
    }
    
    // Randomize personality traits slightly
    BaseData.Personality.Aggressiveness += FMath::RandRange(-0.2f, 0.2f);
    BaseData.Personality.Trustworthiness += FMath::RandRange(-0.2f, 0.2f);
    BaseData.Personality.Intelligence += FMath::RandRange(-0.2f, 0.2f);
    BaseData.Personality.Sociability += FMath::RandRange(-0.2f, 0.2f);
    
    // Clamp personality values
    BaseData.Personality.Aggressiveness = FMath::Clamp(BaseData.Personality.Aggressiveness, 0.0f, 1.0f);
    BaseData.Personality.Trustworthiness = FMath::Clamp(BaseData.Personality.Trustworthiness, 0.0f, 1.0f);
    BaseData.Personality.Intelligence = FMath::Clamp(BaseData.Personality.Intelligence, 0.0f, 1.0f);
    BaseData.Personality.Sociability = FMath::Clamp(BaseData.Personality.Sociability, 0.0f, 1.0f);
    
    return BaseData;
}