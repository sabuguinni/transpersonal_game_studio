#include "Char_PlayerVisualSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

AChar_PlayerVisualSystem::AChar_PlayerVisualSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create character mesh component
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    RootComponent = CharacterMesh;

    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(CharacterMesh, TEXT("hand_r"));

    // Create accessory mesh component
    AccessoryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccessoryMesh"));
    AccessoryMesh->SetupAttachment(CharacterMesh, TEXT("head"));

    // Initialize default values
    HeightScale = 1.0f;
    WidthScale = 1.0f;
    SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    // Initialize current preset
    CurrentPreset = FChar_VisualPreset();
}

void AChar_PlayerVisualSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultPresets();
    UpdateCharacterAppearance();
}

void AChar_PlayerVisualSystem::InitializeDefaultPresets()
{
    AvailablePresets.Empty();

    // Lean Hunter Preset
    FChar_VisualPreset LeanHunter;
    LeanHunter.PresetName = TEXT("Lean Hunter");
    LeanHunter.BodyType = EChar_BodyType::Lean;
    LeanHunter.SkinTone = EChar_SkinTone::Tanned;
    LeanHunter.HairStyle = EChar_HairStyle::Short;
    AvailablePresets.Add(LeanHunter);

    // Athletic Warrior Preset
    FChar_VisualPreset AthleticWarrior;
    AthleticWarrior.PresetName = TEXT("Athletic Warrior");
    AthleticWarrior.BodyType = EChar_BodyType::Athletic;
    AthleticWarrior.SkinTone = EChar_SkinTone::Medium;
    AthleticWarrior.HairStyle = EChar_HairStyle::Braided;
    AvailablePresets.Add(AthleticWarrior);

    // Stocky Gatherer Preset
    FChar_VisualPreset StockyGatherer;
    StockyGatherer.PresetName = TEXT("Stocky Gatherer");
    StockyGatherer.BodyType = EChar_BodyType::Stocky;
    StockyGatherer.SkinTone = EChar_SkinTone::Dark;
    StockyGatherer.HairStyle = EChar_HairStyle::Long;
    AvailablePresets.Add(StockyGatherer);

    // Tall Scout Preset
    FChar_VisualPreset TallScout;
    TallScout.PresetName = TEXT("Tall Scout");
    TallScout.BodyType = EChar_BodyType::Tall;
    TallScout.SkinTone = EChar_SkinTone::Light;
    TallScout.HairStyle = EChar_HairStyle::Shaved;
    AvailablePresets.Add(TallScout);

    UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Initialized %d visual presets"), AvailablePresets.Num());
}

void AChar_PlayerVisualSystem::ApplyVisualPreset(const FChar_VisualPreset& Preset)
{
    CurrentPreset = Preset;
    
    SetBodyType(Preset.BodyType);
    SetSkinTone(Preset.SkinTone);
    SetHairStyle(Preset.HairStyle);
    
    if (Preset.CharacterMesh.IsValid())
    {
        SetCharacterMesh(Preset.CharacterMesh.Get());
    }
    
    if (Preset.SkinMaterial.IsValid())
    {
        SetSkinMaterial(Preset.SkinMaterial.Get());
    }
    
    if (Preset.HairMaterial.IsValid())
    {
        SetHairMaterial(Preset.HairMaterial.Get());
    }
    
    UpdateCharacterAppearance();
    
    UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Applied preset '%s'"), *Preset.PresetName);
}

void AChar_PlayerVisualSystem::SetBodyType(EChar_BodyType NewBodyType)
{
    CurrentPreset.BodyType = NewBodyType;
    
    // Adjust scaling based on body type
    switch (NewBodyType)
    {
        case EChar_BodyType::Lean:
            HeightScale = 0.95f;
            WidthScale = 0.85f;
            break;
        case EChar_BodyType::Athletic:
            HeightScale = 1.0f;
            WidthScale = 1.0f;
            break;
        case EChar_BodyType::Stocky:
            HeightScale = 0.9f;
            WidthScale = 1.15f;
            break;
        case EChar_BodyType::Tall:
            HeightScale = 1.1f;
            WidthScale = 0.95f;
            break;
    }
    
    ApplyScaling();
}

void AChar_PlayerVisualSystem::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentPreset.SkinTone = NewSkinTone;
    
    // Set skin color based on tone
    switch (NewSkinTone)
    {
        case EChar_SkinTone::Light:
            SkinColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
            break;
        case EChar_SkinTone::Medium:
            SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
            break;
        case EChar_SkinTone::Dark:
            SkinColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
            break;
        case EChar_SkinTone::Tanned:
            SkinColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
            break;
    }
    
    ApplyMaterialParameters();
}

void AChar_PlayerVisualSystem::SetHairStyle(EChar_HairStyle NewHairStyle)
{
    CurrentPreset.HairStyle = NewHairStyle;
    
    // Set hair color variations based on style
    switch (NewHairStyle)
    {
        case EChar_HairStyle::Short:
            HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
            break;
        case EChar_HairStyle::Long:
            HairColor = FLinearColor(0.15f, 0.08f, 0.03f, 1.0f);
            break;
        case EChar_HairStyle::Braided:
            HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
            break;
        case EChar_HairStyle::Shaved:
            HairColor = FLinearColor(0.1f, 0.05f, 0.02f, 1.0f);
            break;
    }
    
    ApplyMaterialParameters();
}

void AChar_PlayerVisualSystem::SetCharacterMesh(USkeletalMesh* NewMesh)
{
    if (CharacterMesh && NewMesh)
    {
        CharacterMesh->SetSkeletalMesh(NewMesh);
        CurrentPreset.CharacterMesh = NewMesh;
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Set character mesh to %s"), *NewMesh->GetName());
    }
}

void AChar_PlayerVisualSystem::SetSkinMaterial(UMaterialInterface* NewMaterial)
{
    if (CharacterMesh && NewMaterial)
    {
        CharacterMesh->SetMaterial(0, NewMaterial);
        CurrentPreset.SkinMaterial = NewMaterial;
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Set skin material to %s"), *NewMaterial->GetName());
    }
}

void AChar_PlayerVisualSystem::SetHairMaterial(UMaterialInterface* NewMaterial)
{
    if (CharacterMesh && NewMaterial)
    {
        CharacterMesh->SetMaterial(1, NewMaterial);
        CurrentPreset.HairMaterial = NewMaterial;
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Set hair material to %s"), *NewMaterial->GetName());
    }
}

void AChar_PlayerVisualSystem::SetHeightScale(float NewScale)
{
    HeightScale = FMath::Clamp(NewScale, 0.8f, 1.2f);
    ApplyScaling();
}

void AChar_PlayerVisualSystem::SetWidthScale(float NewScale)
{
    WidthScale = FMath::Clamp(NewScale, 0.8f, 1.2f);
    ApplyScaling();
}

void AChar_PlayerVisualSystem::SetSkinColor(FLinearColor NewColor)
{
    SkinColor = NewColor;
    ApplyMaterialParameters();
}

void AChar_PlayerVisualSystem::SetHairColor(FLinearColor NewColor)
{
    HairColor = NewColor;
    ApplyMaterialParameters();
}

void AChar_PlayerVisualSystem::EquipWeapon(UStaticMesh* WeaponMeshAsset)
{
    if (WeaponMesh && WeaponMeshAsset)
    {
        WeaponMesh->SetStaticMesh(WeaponMeshAsset);
        WeaponMesh->SetVisibility(true);
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Equipped weapon %s"), *WeaponMeshAsset->GetName());
    }
}

void AChar_PlayerVisualSystem::EquipAccessory(UStaticMesh* AccessoryMeshAsset)
{
    if (AccessoryMesh && AccessoryMeshAsset)
    {
        AccessoryMesh->SetStaticMesh(AccessoryMeshAsset);
        AccessoryMesh->SetVisibility(true);
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSystem: Equipped accessory %s"), *AccessoryMeshAsset->GetName());
    }
}

void AChar_PlayerVisualSystem::UpdateCharacterAppearance()
{
    ApplyScaling();
    ApplyMaterialParameters();
}

void AChar_PlayerVisualSystem::ApplyMaterialParameters()
{
    if (!CharacterMesh)
        return;

    // Create dynamic material instances for skin and hair
    UMaterialInstanceDynamic* SkinMaterialInstance = CharacterMesh->CreateDynamicMaterialInstance(0);
    if (SkinMaterialInstance)
    {
        SkinMaterialInstance->SetVectorParameterValue(TEXT("SkinColor"), SkinColor);
        SkinMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
    }

    UMaterialInstanceDynamic* HairMaterialInstance = CharacterMesh->CreateDynamicMaterialInstance(1);
    if (HairMaterialInstance)
    {
        HairMaterialInstance->SetVectorParameterValue(TEXT("HairColor"), HairColor);
        HairMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), HairColor);
    }
}

void AChar_PlayerVisualSystem::ApplyScaling()
{
    if (CharacterMesh)
    {
        FVector NewScale = FVector(WidthScale, WidthScale, HeightScale);
        CharacterMesh->SetWorldScale3D(NewScale);
    }
}