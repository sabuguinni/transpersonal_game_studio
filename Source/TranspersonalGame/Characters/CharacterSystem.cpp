#include "CharacterSystem.h"
#include "Engine/Engine.h"

FCharacterDefinition UCharacterDatabase::GetCharacterByName(const FString& CharacterName) const
{
    for (const FCharacterDefinition& Character : Characters)
    {
        if (Character.CharacterName == CharacterName)
        {
            return Character;
        }
    }
    
    // Return empty character if not found
    return FCharacterDefinition();
}

TArray<FCharacterDefinition> UCharacterDatabase::GetCharactersByArchetype(ECharacterArchetype Archetype) const
{
    TArray<FCharacterDefinition> FilteredCharacters;
    
    for (const FCharacterDefinition& Character : Characters)
    {
        if (Character.Archetype == Archetype)
        {
            FilteredCharacters.Add(Character);
        }
    }
    
    return FilteredCharacters;
}

FCharacterDefinition UCharacterDatabase::GetRandomCharacterByArchetype(ECharacterArchetype Archetype) const
{
    TArray<FCharacterDefinition> MatchingCharacters = GetCharactersByArchetype(Archetype);
    
    if (MatchingCharacters.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No characters found for archetype"));
        return FCharacterDefinition();
    }
    
    // Calculate total weight
    int32 TotalWeight = 0;
    for (const FCharacterDefinition& Character : MatchingCharacters)
    {
        TotalWeight += Character.SpawnWeight;
    }
    
    // Select random character based on weight
    int32 RandomWeight = FMath::RandRange(1, TotalWeight);
    int32 CurrentWeight = 0;
    
    for (const FCharacterDefinition& Character : MatchingCharacters)
    {
        CurrentWeight += Character.SpawnWeight;
        if (RandomWeight <= CurrentWeight)
        {
            return Character;
        }
    }
    
    // Fallback to first character
    return MatchingCharacters[0];
}