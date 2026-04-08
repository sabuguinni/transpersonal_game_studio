#include "CharacterArchetypes.h"
#include "Engine/Engine.h"

FCharacterArchetypeData UCharacterArchetypeDataAsset::GetArchetypeByID(const FString& ArchetypeID) const
{
    for (const FCharacterArchetypeData& Archetype : CharacterArchetypes)
    {
        if (Archetype.ArchetypeID == ArchetypeID)
        {
            return Archetype;
        }
    }
    
    // Return empty archetype if not found
    FCharacterArchetypeData EmptyArchetype;
    UE_LOG(LogTemp, Warning, TEXT("Character archetype with ID '%s' not found"), *ArchetypeID);
    return EmptyArchetype;
}

TArray<FCharacterArchetypeData> UCharacterArchetypeDataAsset::GetArchetypesByType(ECharacterArchetype ArchetypeType) const
{
    TArray<FCharacterArchetypeData> FilteredArchetypes;
    
    for (const FCharacterArchetypeData& Archetype : CharacterArchetypes)
    {
        if (Archetype.Archetype == ArchetypeType)
        {
            FilteredArchetypes.Add(Archetype);
        }
    }
    
    return FilteredArchetypes;
}

FCharacterArchetypeData UCharacterArchetypeDataAsset::GetRandomArchetype(ECharacterArchetype FilterType) const
{
    TArray<FCharacterArchetypeData> CandidateArchetypes;
    
    // Filter by type if specified
    if (FilterType != ECharacterArchetype::MAX)
    {
        CandidateArchetypes = GetArchetypesByType(FilterType);
    }
    else
    {
        CandidateArchetypes = CharacterArchetypes;
    }
    
    if (CandidateArchetypes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No character archetypes available for selection"));
        return FCharacterArchetypeData();
    }
    
    // Calculate total weight
    float TotalWeight = 0.0f;
    for (const FCharacterArchetypeData& Archetype : CandidateArchetypes)
    {
        TotalWeight += Archetype.SpawnWeight;
    }
    
    if (TotalWeight <= 0.0f)
    {
        // Fallback to uniform random selection
        int32 RandomIndex = FMath::RandRange(0, CandidateArchetypes.Num() - 1);
        return CandidateArchetypes[RandomIndex];
    }
    
    // Weighted random selection
    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const FCharacterArchetypeData& Archetype : CandidateArchetypes)
    {
        CurrentWeight += Archetype.SpawnWeight;
        if (RandomValue <= CurrentWeight)
        {
            return Archetype;
        }
    }
    
    // Fallback (should not reach here)
    return CandidateArchetypes.Last();
}