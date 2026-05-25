#include "CharacterCustomization.h"
#include "TranspersonalCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UCharacterCustomization::UCharacterCustomization()
{
    CharacterName = TEXT("Tribal Hunter");
    BodyType = EChar_BodyType::Athletic;
    SkinTone = EChar_SkinTone::Tan;
    BaseMesh = nullptr;
    SkinMaterial = nullptr;
    WeaponMesh = nullptr;

    // Initialize default clothing sets
    FChar_ClothingSet HunterSet;
    HunterSet.ClothingName = TEXT("Hunter");
    AvailableClothing.Add(HunterSet);

    FChar_ClothingSet GathererSet;
    GathererSet.ClothingName = TEXT("Gatherer");
    AvailableClothing.Add(GathererSet);

    FChar_ClothingSet ShamanSet;
    ShamanSet.ClothingName = TEXT("Shaman");
    AvailableClothing.Add(ShamanSet);
}

void UCharacterCustomization::ApplyCustomizationToCharacter(ATranspersonalCharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterCustomization: Invalid character reference"));
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterCustomization: Character has no mesh component"));
        return;
    }

    // Apply base mesh if available
    if (BaseMesh)
    {
        MeshComp->SetSkeletalMesh(BaseMesh);
        UE_LOG(LogTemp, Log, TEXT("CharacterCustomization: Applied base mesh %s"), *BaseMesh->GetName());
    }

    // Apply skin material
    if (SkinMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(SkinMaterial, Character);
        if (DynamicMaterial)
        {
            // Adjust material parameters based on skin tone
            switch (SkinTone)
            {
                case EChar_SkinTone::Fair:
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.2f);
                    break;
                case EChar_SkinTone::Medium:
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.4f);
                    break;
                case EChar_SkinTone::Tan:
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.6f);
                    break;
                case EChar_SkinTone::Dark:
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.8f);
                    break;
                case EChar_SkinTone::Weathered:
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.5f);
                    DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.8f);
                    break;
            }

            MeshComp->SetMaterial(0, DynamicMaterial);
            UE_LOG(LogTemp, Log, TEXT("CharacterCustomization: Applied skin material with tone %d"), (int32)SkinTone);
        }
    }

    // Apply body type scaling
    FVector BodyScale = FVector(1.0f);
    switch (BodyType)
    {
        case EChar_BodyType::Athletic:
            BodyScale = FVector(1.0f, 1.0f, 1.0f);
            break;
        case EChar_BodyType::Muscular:
            BodyScale = FVector(1.1f, 1.05f, 1.0f);
            break;
        case EChar_BodyType::Lean:
            BodyScale = FVector(0.95f, 0.95f, 1.0f);
            break;
        case EChar_BodyType::Stocky:
            BodyScale = FVector(1.05f, 1.1f, 0.98f);
            break;
    }
    
    MeshComp->SetRelativeScale3D(BodyScale);
    UE_LOG(LogTemp, Log, TEXT("CharacterCustomization: Applied body type %d with scale %s"), 
           (int32)BodyType, *BodyScale.ToString());

    UE_LOG(LogTemp, Log, TEXT("CharacterCustomization: Successfully applied customization '%s' to character"), 
           *CharacterName);
}

FChar_ClothingSet UCharacterCustomization::GetClothingByName(const FString& ClothingName) const
{
    for (const FChar_ClothingSet& ClothingSet : AvailableClothing)
    {
        if (ClothingSet.ClothingName.Equals(ClothingName, ESearchCase::IgnoreCase))
        {
            return ClothingSet;
        }
    }

    // Return default if not found
    if (AvailableClothing.Num() > 0)
    {
        return AvailableClothing[0];
    }

    return FChar_ClothingSet();
}