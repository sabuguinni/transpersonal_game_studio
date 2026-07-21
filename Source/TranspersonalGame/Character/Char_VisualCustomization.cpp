#include "Char_VisualCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UChar_VisualCustomization::UChar_VisualCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    CharacterMesh = nullptr;
}

void UChar_VisualCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the character mesh component
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CharacterMesh)
        {
            InitializeDefaultPresets();
            ApplyVisualPreset(CurrentPreset);
        }
    }
}

void UChar_VisualCustomization::InitializeDefaultPresets()
{
    AvailablePresets.Empty();

    // Tribal Warrior Preset
    FChar_VisualPreset WarriorPreset;
    WarriorPreset.PresetName = TEXT("Tribal Warrior");
    WarriorPreset.SkinTone = EChar_SkinTone::Tanned;
    WarriorPreset.HairStyle = EChar_HairStyle::Long;
    WarriorPreset.ClothingStyle = EChar_ClothingStyle::Leather;
    WarriorPreset.FacePaintColor = FLinearColor::Red;
    AvailablePresets.Add(WarriorPreset);

    // Hunter Preset
    FChar_VisualPreset HunterPreset;
    HunterPreset.PresetName = TEXT("Hunter");
    HunterPreset.SkinTone = EChar_SkinTone::Medium;
    HunterPreset.HairStyle = EChar_HairStyle::Braided;
    HunterPreset.ClothingStyle = EChar_ClothingStyle::Fur;
    HunterPreset.FacePaintColor = FLinearColor::Blue;
    AvailablePresets.Add(HunterPreset);

    // Gatherer Preset
    FChar_VisualPreset GathererPreset;
    GathererPreset.PresetName = TEXT("Gatherer");
    GathererPreset.SkinTone = EChar_SkinTone::Light;
    GathererPreset.HairStyle = EChar_HairStyle::Short;
    GathererPreset.ClothingStyle = EChar_ClothingStyle::Woven;
    GathererPreset.FacePaintColor = FLinearColor::Green;
    AvailablePresets.Add(GathererPreset);

    // Shaman Preset
    FChar_VisualPreset ShamanPreset;
    ShamanPreset.PresetName = TEXT("Tribal Elder");
    ShamanPreset.SkinTone = EChar_SkinTone::Dark;
    ShamanPreset.HairStyle = EChar_HairStyle::Long;
    ShamanPreset.ClothingStyle = EChar_ClothingStyle::Minimal;
    ShamanPreset.FacePaintColor = FLinearColor::Yellow;
    AvailablePresets.Add(ShamanPreset);

    // Set default preset
    if (AvailablePresets.Num() > 0)
    {
        CurrentPreset = AvailablePresets[0];
    }
}

void UChar_VisualCustomization::ApplyVisualPreset(const FChar_VisualPreset& Preset)
{
    CurrentPreset = Preset;
    UpdateCharacterAppearance();
    
    UE_LOG(LogTemp, Log, TEXT("Applied visual preset: %s"), *Preset.PresetName);
}

void UChar_VisualCustomization::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentPreset.SkinTone = NewSkinTone;
    UpdateCharacterAppearance();
}

void UChar_VisualCustomization::SetHairStyle(EChar_HairStyle NewHairStyle)
{
    CurrentPreset.HairStyle = NewHairStyle;
    UpdateCharacterAppearance();
}

void UChar_VisualCustomization::SetClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    CurrentPreset.ClothingStyle = NewClothingStyle;
    UpdateCharacterAppearance();
}

void UChar_VisualCustomization::SetFacePaintColor(FLinearColor NewColor)
{
    CurrentPreset.FacePaintColor = NewColor;
    UpdateCharacterAppearance();
}

void UChar_VisualCustomization::UpdateCharacterAppearance()
{
    if (!CharacterMesh)
    {
        return;
    }

    // Load character mesh if specified
    if (CurrentPreset.CharacterMesh.IsValid())
    {
        LoadCharacterMesh(CurrentPreset.CharacterMesh);
    }

    // Apply material parameters
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::LoadCharacterMesh(TSoftObjectPtr<USkeletalMesh> MeshAsset)
{
    if (!CharacterMesh || !MeshAsset.IsValid())
    {
        return;
    }

    USkeletalMesh* LoadedMesh = MeshAsset.LoadSynchronous();
    if (LoadedMesh)
    {
        CharacterMesh->SetSkeletalMesh(LoadedMesh);
        UE_LOG(LogTemp, Log, TEXT("Loaded character mesh: %s"), *LoadedMesh->GetName());
    }
}

void UChar_VisualCustomization::ApplyMaterialParameters()
{
    if (!CharacterMesh)
    {
        return;
    }

    // Create dynamic material instances for customization
    for (int32 MaterialIndex = 0; MaterialIndex < CharacterMesh->GetNumMaterials(); MaterialIndex++)
    {
        UMaterialInterface* BaseMaterial = CharacterMesh->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Apply skin tone
                FLinearColor SkinColor = FLinearColor::White;
                switch (CurrentPreset.SkinTone)
                {
                    case EChar_SkinTone::Light:
                        SkinColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
                        break;
                    case EChar_SkinTone::Medium:
                        SkinColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
                        break;
                    case EChar_SkinTone::Dark:
                        SkinColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
                        break;
                    case EChar_SkinTone::Tanned:
                        SkinColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
                        break;
                }

                DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), SkinColor);
                DynamicMaterial->SetVectorParameterValue(TEXT("FacePaintColor"), CurrentPreset.FacePaintColor);
                
                CharacterMesh->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied material parameters for skin tone and face paint"));
}