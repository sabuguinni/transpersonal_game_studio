#include "Char_CharacterVariations.h"
#include "Engine/Engine.h"

UChar_CharacterVariations::UChar_CharacterVariations()
{
    InitializeDefaultVariations();
}

FChar_CharacterVariation UChar_CharacterVariations::GetRandomVariation() const
{
    if (AvailableVariations.Num() == 0)
    {
        return FChar_CharacterVariation();
    }

    int32 RandomIndex = FMath::RandRange(0, AvailableVariations.Num() - 1);
    return AvailableVariations[RandomIndex];
}

TArray<FChar_CharacterVariation> UChar_CharacterVariations::GetVariationsByArchetype(EChar_TribalArchetype Archetype) const
{
    TArray<FChar_CharacterVariation> FilteredVariations;
    
    for (const FChar_CharacterVariation& Variation : AvailableVariations)
    {
        if (Variation.Archetype == Archetype)
        {
            FilteredVariations.Add(Variation);
        }
    }
    
    return FilteredVariations;
}

bool UChar_CharacterVariations::GetVariationByName(const FString& Name, FChar_CharacterVariation& OutVariation) const
{
    for (const FChar_CharacterVariation& Variation : AvailableVariations)
    {
        if (Variation.CharacterName == Name)
        {
            OutVariation = Variation;
            return true;
        }
    }
    
    return false;
}

void UChar_CharacterVariations::InitializeDefaultVariations()
{
    AvailableVariations.Empty();

    // Young Hunter - Fast and agile, good with ranged weapons
    FChar_CharacterVariation YoungHunter;
    YoungHunter.Archetype = EChar_TribalArchetype::YoungHunter;
    YoungHunter.CharacterName = TEXT("Kael the Swift");
    YoungHunter.Description = FText::FromString(TEXT("A young hunter skilled in tracking and ranged combat"));
    YoungHunter.HeightScale = 0.95f;
    YoungHunter.BodyMassScale = 0.85f;
    YoungHunter.PrimaryClothing = EChar_ClothingMaterial::RaptorHide;
    YoungHunter.PrimaryWeapon = EChar_WeaponType::WoodenBow;
    YoungHunter.HealthModifier = 0.9f;
    YoungHunter.StaminaModifier = 1.3f;
    YoungHunter.HungerResistance = 1.1f;
    YoungHunter.ColdResistance = 0.9f;
    AvailableVariations.Add(YoungHunter);

    // Elder Shaman - Wise and experienced, knows survival secrets
    FChar_CharacterVariation ElderShaman;
    ElderShaman.Archetype = EChar_TribalArchetype::ElderShaman;
    ElderShaman.CharacterName = TEXT("Thara the Wise");
    ElderShaman.Description = FText::FromString(TEXT("An elder who knows the ancient ways of survival"));
    ElderShaman.HeightScale = 0.92f;
    ElderShaman.BodyMassScale = 0.95f;
    ElderShaman.PrimaryClothing = EChar_ClothingMaterial::BoneArmor;
    ElderShaman.PrimaryWeapon = EChar_WeaponType::FireTorch;
    ElderShaman.HealthModifier = 0.8f;
    ElderShaman.StaminaModifier = 0.7f;
    ElderShaman.HungerResistance = 1.4f;
    ElderShaman.ColdResistance = 1.3f;
    AvailableVariations.Add(ElderShaman);

    // Female Gatherer - Excellent at finding resources and plants
    FChar_CharacterVariation FemaleGatherer;
    FemaleGatherer.Archetype = EChar_TribalArchetype::FemaleGatherer;
    FemaleGatherer.CharacterName = TEXT("Nira the Provider");
    FemaleGatherer.Description = FText::FromString(TEXT("A skilled gatherer who knows every edible plant"));
    FemaleGatherer.HeightScale = 0.88f;
    FemaleGatherer.BodyMassScale = 0.8f;
    FemaleGatherer.PrimaryClothing = EChar_ClothingMaterial::PlantFiber;
    FemaleGatherer.PrimaryWeapon = EChar_WeaponType::FlintKnife;
    FemaleGatherer.HealthModifier = 1.0f;
    FemaleGatherer.StaminaModifier = 1.1f;
    FemaleGatherer.HungerResistance = 1.5f;
    FemaleGatherer.ColdResistance = 1.0f;
    AvailableVariations.Add(FemaleGatherer);

    // Child Scout - Small and quick, good at hiding and scouting
    FChar_CharacterVariation ChildScout;
    ChildScout.Archetype = EChar_TribalArchetype::ChildScout;
    ChildScout.CharacterName = TEXT("Pip the Quick");
    ChildScout.Description = FText::FromString(TEXT("A young scout who can slip past any predator"));
    ChildScout.HeightScale = 0.7f;
    ChildScout.BodyMassScale = 0.6f;
    ChildScout.PrimaryClothing = EChar_ClothingMaterial::RaptorHide;
    ChildScout.PrimaryWeapon = EChar_WeaponType::SlingSling;
    ChildScout.HealthModifier = 0.6f;
    ChildScout.StaminaModifier = 1.4f;
    ChildScout.HungerResistance = 0.8f;
    ChildScout.ColdResistance = 0.8f;
    AvailableVariations.Add(ChildScout);

    // Tribal Warrior - Strong and tough, excellent in melee combat
    FChar_CharacterVariation TribalWarrior;
    TribalWarrior.Archetype = EChar_TribalArchetype::TribalWarrior;
    TribalWarrior.CharacterName = TEXT("Grok the Mighty");
    TribalWarrior.Description = FText::FromString(TEXT("A powerful warrior who has faced many dinosaurs"));
    TribalWarrior.HeightScale = 1.1f;
    TribalWarrior.BodyMassScale = 1.2f;
    TribalWarrior.PrimaryClothing = EChar_ClothingMaterial::TricerHide;
    TribalWarrior.PrimaryWeapon = EChar_WeaponType::BoneClub;
    TribalWarrior.HealthModifier = 1.4f;
    TribalWarrior.StaminaModifier = 1.2f;
    TribalWarrior.HungerResistance = 1.2f;
    TribalWarrior.ColdResistance = 1.1f;
    AvailableVariations.Add(TribalWarrior);

    // Crafts Master - Expert at making tools and weapons
    FChar_CharacterVariation CraftsMaster;
    CraftsMaster.Archetype = EChar_TribalArchetype::CraftsMaster;
    CraftsMaster.CharacterName = TEXT("Vera the Maker");
    CraftsMaster.Description = FText::FromString(TEXT("A master crafter who can make tools from anything"));
    CraftsMaster.HeightScale = 0.98f;
    CraftsMaster.BodyMassScale = 1.0f;
    CraftsMaster.PrimaryClothing = EChar_ClothingMaterial::BrachiHide;
    CraftsMaster.PrimaryWeapon = EChar_WeaponType::StoneSpear;
    CraftsMaster.HealthModifier = 1.1f;
    CraftsMaster.StaminaModifier = 1.0f;
    CraftsMaster.HungerResistance = 1.2f;
    CraftsMaster.ColdResistance = 1.2f;
    AvailableVariations.Add(CraftsMaster);

    UE_LOG(LogTemp, Log, TEXT("Character Variations initialized with %d variations"), AvailableVariations.Num());
}