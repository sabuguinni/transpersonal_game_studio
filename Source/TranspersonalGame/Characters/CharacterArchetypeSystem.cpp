#include "CharacterArchetypeSystem.h"
#include "Engine/Engine.h"

UCharacterArchetype* UCharacterArchetypeDatabase::GetRandomArchetype(ECharacterArchetype ArchetypeFilter)
{
    TArray<UCharacterArchetype*> FilteredArchetypes;
    
    for (const TSoftObjectPtr<UCharacterArchetype>& ArchetypePtr : AllArchetypes)
    {
        if (UCharacterArchetype* Archetype = ArchetypePtr.LoadSynchronous())
        {
            if (Archetype->ArchetypeType == ArchetypeFilter)
            {
                FilteredArchetypes.Add(Archetype);
            }
        }
    }
    
    if (FilteredArchetypes.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, FilteredArchetypes.Num() - 1);
        return FilteredArchetypes[RandomIndex];
    }
    
    return nullptr;
}

TArray<UCharacterArchetype*> UCharacterArchetypeDatabase::GenerateBalancedCast(int32 NumberOfCharacters)
{
    TArray<UCharacterArchetype*> GeneratedCast;
    TArray<ECharacterEthnicity> UsedEthnicities;
    TArray<ECharacterAge> UsedAges;
    
    // Ensure diversity in the generated cast
    for (int32 i = 0; i < NumberOfCharacters; i++)
    {
        UCharacterArchetype* SelectedArchetype = nullptr;
        int32 Attempts = 0;
        const int32 MaxAttempts = 20;
        
        while (!SelectedArchetype && Attempts < MaxAttempts)
        {
            // Get random archetype from available pool
            if (AllArchetypes.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, AllArchetypes.Num() - 1);
                UCharacterArchetype* CandidateArchetype = AllArchetypes[RandomIndex].LoadSynchronous();
                
                if (CandidateArchetype)
                {
                    // Check diversity constraints
                    bool bEthnicityOK = true;
                    bool bAgeOK = true;
                    
                    if (EthnicDiversityWeight > 0.5f)
                    {
                        float EthnicityCount = UsedEthnicities.Contains(CandidateArchetype->Ethnicity) ? 
                            UsedEthnicities.Num() : 0;
                        float EthnicityRatio = EthnicityCount / FMath::Max(1.0f, (float)i);
                        bEthnicityOK = EthnicityRatio < (1.0f - EthnicDiversityWeight);
                    }
                    
                    if (AgeDistributionWeight > 0.5f)
                    {
                        float AgeCount = UsedAges.Contains(CandidateArchetype->AgeGroup) ? 
                            UsedAges.Num() : 0;
                        float AgeRatio = AgeCount / FMath::Max(1.0f, (float)i);
                        bAgeOK = AgeRatio < (1.0f - AgeDistributionWeight);
                    }
                    
                    if (bEthnicityOK && bAgeOK)
                    {
                        SelectedArchetype = CandidateArchetype;
                        UsedEthnicities.AddUnique(CandidateArchetype->Ethnicity);
                        UsedAges.AddUnique(CandidateArchetype->AgeGroup);
                    }
                }
            }
            
            Attempts++;
        }
        
        // If we couldn't find a diverse option, just pick any available
        if (!SelectedArchetype && AllArchetypes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, AllArchetypes.Num() - 1);
            SelectedArchetype = AllArchetypes[RandomIndex].LoadSynchronous();
        }
        
        if (SelectedArchetype)
        {
            GeneratedCast.Add(SelectedArchetype);
        }
    }
    
    return GeneratedCast;
}