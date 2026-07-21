#include "Char_PrimitiveHumanCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AChar_PrimitiveHumanCharacter::AChar_PrimitiveHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create equipment components
    AnimalHideClothing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnimalHideClothing"));
    AnimalHideClothing->SetupAttachment(GetMesh(), TEXT("spine_03"));

    PrimitiveWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveWeapon"));
    PrimitiveWeapon->SetupAttachment(GetMesh(), TEXT("hand_r"));

    TribalAccessories = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TribalAccessories"));
    TribalAccessories->SetupAttachment(GetMesh(), TEXT("head"));

    // Set default character customization
    CharacterCustomization.SkinTone = EChar_SkinTone::Medium;
    CharacterCustomization.WeatheringLevel = EChar_WeatheringLevel::Weathered;
    CharacterCustomization.TribalMarkings = EChar_TribalMarkings::Hunter;
    CharacterCustomization.MuscleMass = 1.0f;
    CharacterCustomization.ScarVisibility = 0.5f;
    CharacterCustomization.MarkingColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);

    // Initialize material arrays
    SkinToneMaterials.SetNum(5); // For each skin tone enum
    WeatheringMaterials.SetNum(4); // For each weathering level
    TribalMarkingMaterials.SetNum(6); // For each tribal marking type
}

void AChar_PrimitiveHumanCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMaterials();
    ApplyCharacterCustomization(CharacterCustomization);
}

void AChar_PrimitiveHumanCharacter::InitializeMaterials()
{
    // Create dynamic material instances
    if (GetMesh() && GetMesh()->GetMaterial(0))
    {
        DynamicSkinMaterial = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);
        if (DynamicSkinMaterial)
        {
            GetMesh()->SetMaterial(0, DynamicSkinMaterial);
        }
    }

    if (AnimalHideClothing && AnimalHideClothing->GetMaterial(0))
    {
        DynamicClothingMaterial = UMaterialInstanceDynamic::Create(AnimalHideClothing->GetMaterial(0), this);
        if (DynamicClothingMaterial)
        {
            AnimalHideClothing->SetMaterial(0, DynamicClothingMaterial);
        }
    }
}

void AChar_PrimitiveHumanCharacter::ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization)
{
    CharacterCustomization = NewCustomization;
    UpdateCharacterMaterials();
}

void AChar_PrimitiveHumanCharacter::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CharacterCustomization.SkinTone = NewSkinTone;
    ApplySkinToneToMaterial();
}

void AChar_PrimitiveHumanCharacter::SetWeatheringLevel(EChar_WeatheringLevel NewWeatheringLevel)
{
    CharacterCustomization.WeatheringLevel = NewWeatheringLevel;
    ApplyWeatheringToMaterial();
}

void AChar_PrimitiveHumanCharacter::SetTribalMarkings(EChar_TribalMarkings NewMarkings)
{
    CharacterCustomization.TribalMarkings = NewMarkings;
    ApplyTribalMarkingsToMaterial();
}

void AChar_PrimitiveHumanCharacter::SetMuscleMass(float NewMuscleMass)
{
    CharacterCustomization.MuscleMass = FMath::Clamp(NewMuscleMass, 0.5f, 2.0f);
    UpdateMuscleDefinition();
}

void AChar_PrimitiveHumanCharacter::SetScarVisibility(float NewScarVisibility)
{
    CharacterCustomization.ScarVisibility = FMath::Clamp(NewScarVisibility, 0.0f, 1.0f);
    UpdateScarring();
}

void AChar_PrimitiveHumanCharacter::SetMarkingColor(FLinearColor NewColor)
{
    CharacterCustomization.MarkingColor = NewColor;
    ApplyTribalMarkingsToMaterial();
}

void AChar_PrimitiveHumanCharacter::EquipPrimitiveWeapon(UStaticMesh* WeaponMesh)
{
    if (PrimitiveWeapon && WeaponMesh)
    {
        PrimitiveWeapon->SetStaticMesh(WeaponMesh);
        PrimitiveWeapon->SetVisibility(true);
    }
}

void AChar_PrimitiveHumanCharacter::EquipTribalAccessories(UStaticMesh* AccessoryMesh)
{
    if (TribalAccessories && AccessoryMesh)
    {
        TribalAccessories->SetStaticMesh(AccessoryMesh);
        TribalAccessories->SetVisibility(true);
    }
}

void AChar_PrimitiveHumanCharacter::UpdateClothing(UStaticMesh* ClothingMesh)
{
    if (AnimalHideClothing && ClothingMesh)
    {
        AnimalHideClothing->SetStaticMesh(ClothingMesh);
        AnimalHideClothing->SetVisibility(true);
    }
}

void AChar_PrimitiveHumanCharacter::UpdateCharacterMaterials()
{
    ApplySkinToneToMaterial();
    ApplyWeatheringToMaterial();
    ApplyTribalMarkingsToMaterial();
    UpdateMuscleDefinition();
    UpdateScarring();
}

void AChar_PrimitiveHumanCharacter::ApplySkinToneToMaterial()
{
    if (!DynamicSkinMaterial) return;

    FLinearColor SkinColor;
    switch (CharacterCustomization.SkinTone)
    {
        case EChar_SkinTone::VeryLight:
            SkinColor = FLinearColor(0.95f, 0.85f, 0.75f, 1.0f);
            break;
        case EChar_SkinTone::Light:
            SkinColor = FLinearColor(0.85f, 0.75f, 0.65f, 1.0f);
            break;
        case EChar_SkinTone::Medium:
            SkinColor = FLinearColor(0.75f, 0.65f, 0.55f, 1.0f);
            break;
        case EChar_SkinTone::Dark:
            SkinColor = FLinearColor(0.55f, 0.45f, 0.35f, 1.0f);
            break;
        case EChar_SkinTone::VeryDark:
            SkinColor = FLinearColor(0.35f, 0.25f, 0.20f, 1.0f);
            break;
        default:
            SkinColor = FLinearColor(0.75f, 0.65f, 0.55f, 1.0f);
            break;
    }

    DynamicSkinMaterial->SetVectorParameterValue(TEXT("SkinTone"), SkinColor);
}

void AChar_PrimitiveHumanCharacter::ApplyWeatheringToMaterial()
{
    if (!DynamicSkinMaterial) return;

    float WeatheringIntensity;
    switch (CharacterCustomization.WeatheringLevel)
    {
        case EChar_WeatheringLevel::Fresh:
            WeatheringIntensity = 0.1f;
            break;
        case EChar_WeatheringLevel::Weathered:
            WeatheringIntensity = 0.4f;
            break;
        case EChar_WeatheringLevel::Scarred:
            WeatheringIntensity = 0.7f;
            break;
        case EChar_WeatheringLevel::BattleWorn:
            WeatheringIntensity = 1.0f;
            break;
        default:
            WeatheringIntensity = 0.4f;
            break;
    }

    DynamicSkinMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), WeatheringIntensity);
}

void AChar_PrimitiveHumanCharacter::ApplyTribalMarkingsToMaterial()
{
    if (!DynamicSkinMaterial) return;

    float MarkingVisibility = (CharacterCustomization.TribalMarkings == EChar_TribalMarkings::None) ? 0.0f : 1.0f;
    float MarkingPattern = static_cast<float>(CharacterCustomization.TribalMarkings) / 5.0f; // Normalize to 0-1

    DynamicSkinMaterial->SetScalarParameterValue(TEXT("MarkingVisibility"), MarkingVisibility);
    DynamicSkinMaterial->SetScalarParameterValue(TEXT("MarkingPattern"), MarkingPattern);
    DynamicSkinMaterial->SetVectorParameterValue(TEXT("MarkingColor"), CharacterCustomization.MarkingColor);
}

void AChar_PrimitiveHumanCharacter::UpdateMuscleDefinition()
{
    if (!DynamicSkinMaterial) return;

    DynamicSkinMaterial->SetScalarParameterValue(TEXT("MuscleDefinition"), CharacterCustomization.MuscleMass);
    
    // Also affect the mesh scale slightly for muscle mass
    FVector CurrentScale = GetMesh()->GetRelativeScale3D();
    float ScaleFactor = 0.95f + (CharacterCustomization.MuscleMass * 0.1f); // Range from 0.95 to 1.15
    GetMesh()->SetRelativeScale3D(FVector(ScaleFactor, ScaleFactor, CurrentScale.Z));
}

void AChar_PrimitiveHumanCharacter::UpdateScarring()
{
    if (!DynamicSkinMaterial) return;

    DynamicSkinMaterial->SetScalarParameterValue(TEXT("ScarVisibility"), CharacterCustomization.ScarVisibility);
}