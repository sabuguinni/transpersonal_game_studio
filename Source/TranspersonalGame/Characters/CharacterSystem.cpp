#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

FCharacterArchetypeData UCharacterSystem::GetArchetypeData(ECharacterArchetype Archetype)
{
    if (!ArchetypeDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchetypeDataTable is null"));
        return FCharacterArchetypeData();
    }

    FString RowName = UEnum::GetValueAsString(Archetype);
    FCharacterArchetypeData* FoundData = ArchetypeDataTable->FindRow<FCharacterArchetypeData>(FName(*RowName), TEXT(""));
    
    if (FoundData)
    {
        return *FoundData;
    }

    UE_LOG(LogTemp, Warning, TEXT("Archetype data not found for: %s"), *RowName);
    return FCharacterArchetypeData();
}

UMetaHumanCharacterBP* UCharacterSystem::GenerateRandomCharacter(ECharacterArchetype Archetype)
{
    if (BaseMetaHumans.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No base MetaHumans available"));
        return nullptr;
    }

    // Seleciona MetaHuman base aleatório
    int32 RandomIndex = FMath::RandRange(0, BaseMetaHumans.Num() - 1);
    UMetaHumanCharacterBP* BaseCharacter = BaseMetaHumans[RandomIndex].LoadSynchronous();

    if (!BaseCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load MetaHuman at index %d"), RandomIndex);
        return nullptr;
    }

    // TODO: Aplicar variações baseadas no arquétipo
    // Isto será implementado quando tivermos o sistema de variação do MetaHuman
    
    return BaseCharacter;
}

TArray<FCharacterVariation> UCharacterSystem::GenerateVariationsForArchetype(ECharacterArchetype Archetype, int32 Count)
{
    TArray<FCharacterVariation> Variations;
    FCharacterArchetypeData ArchetypeData = GetArchetypeData(Archetype);

    for (int32 i = 0; i < Count; i++)
    {
        FCharacterVariation NewVariation;
        NewVariation.VariationName = FString::Printf(TEXT("%s_Variation_%d"), *ArchetypeData.ArchetypeName, i + 1);
        
        // Gerar variações aleatórias dentro de ranges apropriados
        NewVariation.SkinTone = FMath::RandRange(0.1f, 0.9f);
        NewVariation.EyeColor = FMath::RandRange(0.0f, 1.0f);
        NewVariation.HairColor = FMath::RandRange(0.0f, 1.0f);
        NewVariation.FacialStructure = FMath::RandRange(0.2f, 0.8f);
        NewVariation.BodyMass = FMath::RandRange(0.3f, 0.7f);
        NewVariation.Height = FMath::RandRange(0.4f, 0.6f);

        // Aplicar modificadores baseados no arquétipo
        switch (Archetype)
        {
            case ECharacterArchetype::TribalWarrior:
                NewVariation.BodyMass = FMath::Clamp(NewVariation.BodyMass + 0.2f, 0.0f, 1.0f);
                break;
            case ECharacterArchetype::TribalElder:
                NewVariation.Height = FMath::Clamp(NewVariation.Height - 0.1f, 0.0f, 1.0f);
                break;
            case ECharacterArchetype::TribalScout:
                NewVariation.BodyMass = FMath::Clamp(NewVariation.BodyMass - 0.1f, 0.0f, 1.0f);
                NewVariation.Height = FMath::Clamp(NewVariation.Height + 0.1f, 0.0f, 1.0f);
                break;
        }

        Variations.Add(NewVariation);
    }

    return Variations;
}