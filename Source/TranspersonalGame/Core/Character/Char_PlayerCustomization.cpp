#include "Char_PlayerCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"

UChar_PlayerCustomization::UChar_PlayerCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default customization data
    CustomizationData = FChar_CustomizationData();
}

void UChar_PlayerCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial customization
    ApplyCustomization();
}

void UChar_PlayerCustomization::ApplyCustomization()
{
    UpdateCharacterMesh();
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::SetBodyType(EChar_BodyType NewBodyType)
{
    CustomizationData.BodyType = NewBodyType;
    UpdateCharacterMesh();
}

void UChar_PlayerCustomization::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CustomizationData.SkinTone = NewSkinTone;
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::SetClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    CustomizationData.ClothingStyle = NewClothingStyle;
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::SetSkinColor(FLinearColor NewSkinColor)
{
    CustomizationData.SkinColor = NewSkinColor;
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::SetClothingColor(FLinearColor NewClothingColor)
{
    CustomizationData.ClothingColor = NewClothingColor;
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::ToggleTribalMarkings(bool bEnable)
{
    CustomizationData.bHasTribalMarkings = bEnable;
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::ToggleScars(bool bEnable)
{
    CustomizationData.bHasScars = bEnable;
    UpdateCharacterMaterials();
}

void UChar_PlayerCustomization::LoadCustomizationData(const FChar_CustomizationData& Data)
{
    CustomizationData = Data;
    ApplyCustomization();
}

void UChar_PlayerCustomization::UpdateCharacterMesh()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Select mesh based on body type
    int32 MeshIndex = static_cast<int32>(CustomizationData.BodyType);
    if (BodyMeshes.IsValidIndex(MeshIndex) && BodyMeshes[MeshIndex].IsValid())
    {
        USkeletalMesh* NewMesh = BodyMeshes[MeshIndex].LoadSynchronous();
        if (NewMesh)
        {
            MeshComp->SetSkeletalMesh(NewMesh);
        }
    }
}

void UChar_PlayerCustomization::UpdateCharacterMaterials()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Create dynamic material instances for customization
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); MaterialIndex++)
    {
        UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Apply skin color
                DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), CustomizationData.SkinColor);
                
                // Apply clothing color
                DynamicMaterial->SetVectorParameterValue(TEXT("ClothingColor"), CustomizationData.ClothingColor);
                
                // Apply tribal markings
                DynamicMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), CustomizationData.bHasTribalMarkings ? 1.0f : 0.0f);
                
                // Apply scars
                DynamicMaterial->SetScalarParameterValue(TEXT("Scars"), CustomizationData.bHasScars ? 1.0f : 0.0f);
                
                // Apply skin tone variation
                float SkinToneValue = 0.5f;
                switch (CustomizationData.SkinTone)
                {
                    case EChar_SkinTone::Light:
                        SkinToneValue = 0.8f;
                        break;
                    case EChar_SkinTone::Medium:
                        SkinToneValue = 0.5f;
                        break;
                    case EChar_SkinTone::Dark:
                        SkinToneValue = 0.2f;
                        break;
                    case EChar_SkinTone::Weathered:
                        SkinToneValue = 0.3f;
                        break;
                }
                DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), SkinToneValue);
                
                // Apply clothing style variation
                float ClothingStyleValue = 0.0f;
                switch (CustomizationData.ClothingStyle)
                {
                    case EChar_ClothingStyle::AnimalHide:
                        ClothingStyleValue = 0.0f;
                        break;
                    case EChar_ClothingStyle::PlantFiber:
                        ClothingStyleValue = 0.33f;
                        break;
                    case EChar_ClothingStyle::Minimal:
                        ClothingStyleValue = 0.66f;
                        break;
                    case EChar_ClothingStyle::Decorated:
                        ClothingStyleValue = 1.0f;
                        break;
                }
                DynamicMaterial->SetScalarParameterValue(TEXT("ClothingStyle"), ClothingStyleValue);
                
                MeshComp->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
}

USkeletalMeshComponent* UChar_PlayerCustomization::GetCharacterMesh() const
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character)
    {
        return Character->GetMesh();
    }
    return nullptr;
}