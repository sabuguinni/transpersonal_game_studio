#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

FCharacterProfile UCharacterDatabase::GenerateRandomCharacter(ECharacterArchetype Archetype, ETribalCulture Culture)
{
    FCharacterProfile NewProfile;
    NewProfile.Archetype = Archetype;
    NewProfile.Culture = Culture;
    
    // Generate unique visual variations
    NewProfile.VisualVariation.SkinTone = FMath::RandRange(0.0f, 1.0f);
    NewProfile.VisualVariation.BodyBuild = FMath::RandRange(0.0f, 1.0f);
    NewProfile.VisualVariation.FacialStructure = FMath::RandRange(0.0f, 1.0f);
    NewProfile.VisualVariation.HairVariation = FMath::RandRange(0.0f, 1.0f);
    
    // Scars and tattoos based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalWarrior:
            NewProfile.VisualVariation.ScarPattern = FMath::RandRange(0.3f, 0.8f);
            NewProfile.VisualVariation.TattooPattern = FMath::RandRange(0.2f, 0.7f);
            break;
        case ECharacterArchetype::TribalShaman:
            NewProfile.VisualVariation.ScarPattern = FMath::RandRange(0.0f, 0.3f);
            NewProfile.VisualVariation.TattooPattern = FMath::RandRange(0.5f, 1.0f);
            break;
        case ECharacterArchetype::TribalChild:
            NewProfile.VisualVariation.ScarPattern = FMath::RandRange(0.0f, 0.1f);
            NewProfile.VisualVariation.TattooPattern = 0.0f;
            break;
        default:
            NewProfile.VisualVariation.ScarPattern = FMath::RandRange(0.0f, 0.4f);
            NewProfile.VisualVariation.TattooPattern = FMath::RandRange(0.0f, 0.5f);
            break;
    }
    
    // Clothing sets based on culture
    switch (Culture)
    {
        case ETribalCulture::RiverPeople:
            NewProfile.VisualVariation.ClothingSet = FMath::RandRange(0, 4);
            break;
        case ETribalCulture::MountainClans:
            NewProfile.VisualVariation.ClothingSet = FMath::RandRange(5, 9);
            break;
        case ETribalCulture::ForestDwellers:
            NewProfile.VisualVariation.ClothingSet = FMath::RandRange(10, 14);
            break;
        case ETribalCulture::PlainHunters:
            NewProfile.VisualVariation.ClothingSet = FMath::RandRange(15, 19);
            break;
        case ETribalCulture::CaveClans:
            NewProfile.VisualVariation.ClothingSet = FMath::RandRange(20, 24);
            break;
    }
    
    NewProfile.VisualVariation.AccessorySet = FMath::RandRange(0, 10);
    
    // Generate basic name based on culture
    GenerateCharacterName(NewProfile);
    
    return NewProfile;
}

TArray<FCharacterProfile> UCharacterDatabase::GetCharactersByArchetype(ECharacterArchetype Archetype)
{
    TArray<FCharacterProfile> FilteredProfiles;
    
    for (const FCharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.Archetype == Archetype)
        {
            FilteredProfiles.Add(Profile);
        }
    }
    
    return FilteredProfiles;
}

TArray<FCharacterProfile> UCharacterDatabase::GetCharactersByCulture(ETribalCulture Culture)
{
    TArray<FCharacterProfile> FilteredProfiles;
    
    for (const FCharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.Culture == Culture)
        {
            FilteredProfiles.Add(Profile);
        }
    }
    
    return FilteredProfiles;
}

void UCharacterDatabase::GenerateCharacterName(FCharacterProfile& Profile)
{
    TArray<FString> NamePrefixes;
    TArray<FString> NameSuffixes;
    
    // Names based on culture
    switch (Profile.Culture)
    {
        case ETribalCulture::RiverPeople:
            NamePrefixes = {TEXT("Aqua"), TEXT("Riv"), TEXT("Flow"), TEXT("Cur"), TEXT("Tide")};
            NameSuffixes = {TEXT("ara"), TEXT("esh"), TEXT("ina"), TEXT("oro"), TEXT("uma")};
            break;
        case ETribalCulture::MountainClans:
            NamePrefixes = {TEXT("Rok"), TEXT("Ston"), TEXT("Peak"), TEXT("Crag"), TEXT("Cliff")};
            NameSuffixes = {TEXT("gar"), TEXT("dun"), TEXT("thor"), TEXT("grim"), TEXT("bold")};
            break;
        case ETribalCulture::ForestDwellers:
            NamePrefixes = {TEXT("Leaf"), TEXT("Root"), TEXT("Bark"), TEXT("Moss"), TEXT("Fern")};
            NameSuffixes = {TEXT("wyn"), TEXT("ara"), TEXT("eth"), TEXT("iel"), TEXT("orn")};
            break;
        case ETribalCulture::PlainHunters:
            NamePrefixes = {TEXT("Swift"), TEXT("Hunt"), TEXT("Track"), TEXT("Wind"), TEXT("Grass")};
            NameSuffixes = {TEXT("runner"), TEXT("eye"), TEXT("foot"), TEXT("hawk"), TEXT("wolf")};
            break;
        case ETribalCulture::CaveClans:
            NamePrefixes = {TEXT("Deep"), TEXT("Dark"), TEXT("Echo"), TEXT("Shade"), TEXT("Ember")};
            NameSuffixes = {TEXT("dweller"), TEXT("walker"), TEXT("keeper"), TEXT("watcher"), TEXT("guard")};
            break;
    }
    
    FString RandomPrefix = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num() - 1)];
    FString RandomSuffix = NameSuffixes[FMath::RandRange(0, NameSuffixes.Num() - 1)];
    
    Profile.CharacterName = RandomPrefix + RandomSuffix;
}