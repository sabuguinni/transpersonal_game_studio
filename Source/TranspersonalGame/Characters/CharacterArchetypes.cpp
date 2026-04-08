// CharacterArchetypes.cpp
// Transpersonal Game Studio - Character Art System
// Implementation of character generation system

#include "CharacterArchetypes.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimBlueprint.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/DateTime.h"

FCharacterVariationData UCharacterGenerator::GenerateRandomVariation(
    const UCharacterArchetypeData* ArchetypeData,
    int32 Seed)
{
    if (!ArchetypeData)
    {
        return FCharacterVariationData();
    }
    
    // Use provided seed or generate from current time
    if (Seed == -1)
    {
        Seed = FDateTime::Now().GetTicks();
    }
    
    FRandomStream RandomStream(Seed);
    FCharacterVariationData Variation = ArchetypeData->BaseVariation;
    float Range = ArchetypeData->VariationRange;
    
    // FACIAL FEATURE VARIATIONS
    Variation.FaceWidth = FMath::Clamp(
        Variation.FaceWidth + RandomStream.FRandRange(-Range, Range),
        0.0f, 1.0f
    );
    
    Variation.EyeSize = FMath::Clamp(
        Variation.EyeSize + RandomStream.FRandRange(-Range, Range),
        0.0f, 1.0f
    );
    
    Variation.NoseSize = FMath::Clamp(
        Variation.NoseSize + RandomStream.FRandRange(-Range, Range),
        0.0f, 1.0f
    );
    
    Variation.MouthSize = FMath::Clamp(
        Variation.MouthSize + RandomStream.FRandRange(-Range, Range),
        0.0f, 1.0f
    );
    
    Variation.JawStrength = FMath::Clamp(
        Variation.JawStrength + RandomStream.FRandRange(-Range, Range),
        0.0f, 1.0f
    );
    
    // BODY PROPORTIONS
    Variation.Height = FMath::Clamp(
        Variation.Height + RandomStream.FRandRange(-Range * 0.2f, Range * 0.2f),
        0.8f, 1.2f
    );
    
    Variation.BodyMass = FMath::Clamp(
        Variation.BodyMass + RandomStream.FRandRange(-Range * 0.3f, Range * 0.3f),
        0.8f, 1.2f
    );
    
    // COLOR VARIATIONS
    float ColorVariation = Range * 0.1f;
    
    // Skin tone variation
    Variation.SkinTone.R = FMath::Clamp(
        Variation.SkinTone.R + RandomStream.FRandRange(-ColorVariation, ColorVariation),
        0.1f, 1.0f
    );
    Variation.SkinTone.G = FMath::Clamp(
        Variation.SkinTone.G + RandomStream.FRandRange(-ColorVariation, ColorVariation),
        0.1f, 1.0f
    );
    Variation.SkinTone.B = FMath::Clamp(
        Variation.SkinTone.B + RandomStream.FRandRange(-ColorVariation, ColorVariation),
        0.1f, 1.0f
    );
    
    // Hair color variation
    Variation.HairColor.R = FMath::Clamp(
        Variation.HairColor.R + RandomStream.FRandRange(-ColorVariation, ColorVariation),
        0.0f, 1.0f
    );
    Variation.HairColor.G = FMath::Clamp(
        Variation.HairColor.G + RandomStream.FRandRange(-ColorVariation, ColorVariation),
        0.0f, 1.0f
    );
    Variation.HairColor.B = FMath::Clamp(
        Variation.HairColor.B + RandomStream.FRandRange(-ColorVariation, ColorVariation),
        0.0f, 1.0f
    );
    
    // Eye color variation (more dramatic)
    if (RandomStream.FRand() < 0.3f) // 30% chance for different eye color
    {
        TArray<FLinearColor> EyeColors = {
            FLinearColor(0.2f, 0.4f, 0.6f, 1.0f), // Blue
            FLinearColor(0.3f, 0.5f, 0.2f, 1.0f), // Green
            FLinearColor(0.4f, 0.3f, 0.2f, 1.0f), // Brown
            FLinearColor(0.5f, 0.4f, 0.3f, 1.0f), // Hazel
            FLinearColor(0.3f, 0.3f, 0.3f, 1.0f)  // Gray
        };
        
        Variation.EyeColor = EyeColors[RandomStream.RandRange(0, EyeColors.Num() - 1)];
    }
    
    // SURVIVAL WEAR (based on archetype)
    switch (ArchetypeData->Archetype)
    {
        case ECharacterArchetype::Paleontologist:
            // Protagonist starts clean but gets dirty over time
            Variation.SkinDamage = RandomStream.FRandRange(0.0f, 0.1f);
            Variation.ClothingWear = RandomStream.FRandRange(0.0f, 0.3f);
            Variation.Dirtiness = RandomStream.FRandRange(0.0f, 0.4f);
            break;
            
        case ECharacterArchetype::TribalHunter:
        case ECharacterArchetype::TribalElder:
            // Tribal characters show more survival wear
            Variation.SkinDamage = RandomStream.FRandRange(0.2f, 0.6f);
            Variation.ClothingWear = RandomStream.FRandRange(0.4f, 0.8f);
            Variation.Dirtiness = RandomStream.FRandRange(0.3f, 0.7f);
            break;
            
        case ECharacterArchetype::Survivor:
            // Survivors are heavily worn
            Variation.SkinDamage = RandomStream.FRandRange(0.3f, 0.8f);
            Variation.ClothingWear = RandomStream.FRandRange(0.5f, 1.0f);
            Variation.Dirtiness = RandomStream.FRandRange(0.4f, 0.9f);
            break;
            
        default:
            // Default moderate wear
            Variation.SkinDamage = RandomStream.FRandRange(0.1f, 0.4f);
            Variation.ClothingWear = RandomStream.FRandRange(0.2f, 0.6f);
            Variation.Dirtiness = RandomStream.FRandRange(0.2f, 0.5f);
            break;
    }
    
    return Variation;
}

bool UCharacterGenerator::ApplyVariationToCharacter(
    ACharacter* Character,
    const FCharacterVariationData& Variation)
{
    if (!Character || !Character->GetMesh())
    {
        return false;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    
    // APPLY BODY SCALING
    FVector CurrentScale = MeshComp->GetRelativeScale3D();
    FVector NewScale = FVector(
        CurrentScale.X * Variation.Height,
        CurrentScale.Y * Variation.BodyMass,
        CurrentScale.Z * Variation.Height
    );
    MeshComp->SetRelativeScale3D(NewScale);
    
    // APPLY MATERIAL PARAMETERS
    // This would typically involve creating dynamic material instances
    // and setting parameters for skin tone, wear, etc.
    
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); MaterialIndex++)
    {
        UMaterialInterface* Material = MeshComp->GetMaterial(MaterialIndex);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = 
                MeshComp->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
            
            if (DynamicMaterial)
            {
                // Set skin tone
                DynamicMaterial->SetVectorParameterValue(
                    FName("SkinTone"), 
                    Variation.SkinTone
                );
                
                // Set wear parameters
                DynamicMaterial->SetScalarParameterValue(
                    FName("SkinDamage"), 
                    Variation.SkinDamage
                );
                
                DynamicMaterial->SetScalarParameterValue(
                    FName("ClothingWear"), 
                    Variation.ClothingWear
                );
                
                DynamicMaterial->SetScalarParameterValue(
                    FName("Dirtiness"), 
                    Variation.Dirtiness
                );
                
                // Set eye color
                DynamicMaterial->SetVectorParameterValue(
                    FName("EyeColor"), 
                    Variation.EyeColor
                );
                
                // Set hair color
                DynamicMaterial->SetVectorParameterValue(
                    FName("HairColor"), 
                    Variation.HairColor
                );
            }
        }
    }
    
    return true;
}

FString UCharacterGenerator::GenerateCharacterID(
    ECharacterArchetype Archetype,
    const FCharacterVariationData& Variation)
{
    // Create a unique hash based on character features
    // This allows the recognition system to identify specific individuals
    
    FString ArchetypeString = StaticEnum<ECharacterArchetype>()->GetNameStringByValue(
        static_cast<int64>(Archetype)
    );
    
    // Combine key variation parameters into a hash
    uint32 Hash = 0;
    Hash = HashCombine(Hash, GetTypeHash(Variation.FaceWidth));
    Hash = HashCombine(Hash, GetTypeHash(Variation.EyeSize));
    Hash = HashCombine(Hash, GetTypeHash(Variation.NoseSize));
    Hash = HashCombine(Hash, GetTypeHash(Variation.Height));
    Hash = HashCombine(Hash, GetTypeHash(Variation.SkinTone.ToString()));
    Hash = HashCombine(Hash, GetTypeHash(Variation.HairColor.ToString()));
    
    return FString::Printf(TEXT("%s_%08X"), *ArchetypeString, Hash);
}