#include "Char_MetaHumanManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/SkeletalMesh.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

UChar_MetaHumanManager::UChar_MetaHumanManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    CharacterMesh = nullptr;
    
    InitializeDefaultTraits();
}

void UChar_MetaHumanManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CharacterMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Found character mesh component"));
            ApplyCharacterTraits(CurrentTraits);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MetaHumanManager: No skeletal mesh component found on owner"));
        }
    }
}

void UChar_MetaHumanManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_MetaHumanManager::ApplyCharacterTraits(const FChar_CharacterTraits& Traits)
{
    CurrentTraits = Traits;
    
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("MetaHumanManager: Cannot apply traits - no character mesh"));
        return;
    }
    
    ApplyPhysicalTraits(Traits);
    ApplyClothingAndAccessories(Traits);
    UpdateMaterialParameters(Traits);
    
    UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Applied character traits for %s"), *Traits.CharacterName);
}

FChar_CharacterTraits UChar_MetaHumanManager::GenerateRandomTraits()
{
    FChar_CharacterTraits RandomTraits;
    
    // Random body type
    int32 BodyTypeIndex = FMath::RandRange(0, 4);
    RandomTraits.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);
    
    // Random skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    RandomTraits.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Random physical attributes
    RandomTraits.Height = FMath::RandRange(0.85f, 1.15f);
    RandomTraits.Weight = FMath::RandRange(0.8f, 1.2f);
    
    // Random clothing style
    int32 ClothingIndex = FMath::RandRange(0, 4);
    RandomTraits.ClothingStyle = static_cast<EChar_ClothingStyle>(ClothingIndex);
    
    // Random accessories
    RandomTraits.bHasTribalPaint = FMath::RandBool();
    RandomTraits.bHasBoneJewelry = FMath::RandBool();
    RandomTraits.bHasScars = FMath::RandBool();
    
    // Generate names
    TArray<FString> FirstNames = {TEXT("Krog"), TEXT("Ula"), TEXT("Thak"), TEXT("Nara"), TEXT("Brok"), TEXT("Zara")};
    TArray<FString> TribeNames = {TEXT("Stone Spear"), TEXT("Cave Bear"), TEXT("River Walker"), TEXT("Sky Watcher"), TEXT("Bone Crusher")};
    
    RandomTraits.CharacterName = FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)];
    RandomTraits.TribeName = TribeNames[FMath::RandRange(0, TribeNames.Num() - 1)];
    
    return RandomTraits;
}

void UChar_MetaHumanManager::SetBodyType(EChar_BodyType NewBodyType)
{
    CurrentTraits.BodyType = NewBodyType;
    ApplyCharacterTraits(CurrentTraits);
}

void UChar_MetaHumanManager::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentTraits.SkinTone = NewSkinTone;
    ApplyCharacterTraits(CurrentTraits);
}

void UChar_MetaHumanManager::SetClothingStyle(EChar_ClothingStyle NewStyle)
{
    CurrentTraits.ClothingStyle = NewStyle;
    ApplyCharacterTraits(CurrentTraits);
}

void UChar_MetaHumanManager::LoadMetaHumanPreset(const FString& PresetName)
{
    // Find preset by name
    for (const FChar_CharacterTraits& Preset : PresetTraits)
    {
        if (Preset.CharacterName == PresetName)
        {
            ApplyCharacterTraits(Preset);
            UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Loaded preset %s"), *PresetName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MetaHumanManager: Preset %s not found"), *PresetName);
}

void UChar_MetaHumanManager::SaveCurrentConfiguration(const FString& ConfigName)
{
    FChar_CharacterTraits ConfigToSave = CurrentTraits;
    ConfigToSave.CharacterName = ConfigName;
    
    // Remove existing config with same name
    PresetTraits.RemoveAll([ConfigName](const FChar_CharacterTraits& Trait)
    {
        return Trait.CharacterName == ConfigName;
    });
    
    PresetTraits.Add(ConfigToSave);
    UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Saved configuration %s"), *ConfigName);
}

TArray<FString> UChar_MetaHumanManager::GetAvailablePresets()
{
    TArray<FString> PresetNames;
    for (const FChar_CharacterTraits& Preset : PresetTraits)
    {
        PresetNames.Add(Preset.CharacterName);
    }
    return PresetNames;
}

void UChar_MetaHumanManager::CreateCharacterVariant(const FChar_CharacterTraits& BaseTraits, int32 VariationSeed)
{
    FMath::RandInit(VariationSeed);
    
    FChar_CharacterTraits VariantTraits = BaseTraits;
    
    // Apply random variations
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance to change body type
    {
        int32 NewBodyType = FMath::RandRange(0, 4);
        VariantTraits.BodyType = static_cast<EChar_BodyType>(NewBodyType);
    }
    
    if (FMath::RandRange(0.0f, 1.0f) < 0.5f) // 50% chance to change skin tone
    {
        int32 NewSkinTone = FMath::RandRange(0, 4);
        VariantTraits.SkinTone = static_cast<EChar_SkinTone>(NewSkinTone);
    }
    
    // Slight variations in physical attributes
    VariantTraits.Height += FMath::RandRange(-0.1f, 0.1f);
    VariantTraits.Weight += FMath::RandRange(-0.1f, 0.1f);
    
    // Random accessories
    VariantTraits.bHasTribalPaint = FMath::RandBool();
    VariantTraits.bHasBoneJewelry = FMath::RandBool();
    VariantTraits.bHasScars = FMath::RandBool();
    
    ApplyCharacterTraits(VariantTraits);
}

void UChar_MetaHumanManager::PopulateNPCsWithDiversity(int32 NPCCount, float DiversityFactor)
{
    UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Populating %d NPCs with diversity factor %f"), NPCCount, DiversityFactor);
    
    for (int32 i = 0; i < NPCCount; i++)
    {
        FChar_CharacterTraits NPCTraits = GenerateRandomTraits();
        
        // Apply diversity factor (higher = more variation)
        if (DiversityFactor > 0.5f)
        {
            NPCTraits.Height = FMath::RandRange(0.7f, 1.3f);
            NPCTraits.Weight = FMath::RandRange(0.6f, 1.4f);
        }
        
        // Store NPC traits for later use
        PresetTraits.Add(NPCTraits);
    }
}

void UChar_MetaHumanManager::InitializeDefaultTraits()
{
    // Create default character traits
    CurrentTraits.BodyType = EChar_BodyType::Average;
    CurrentTraits.SkinTone = EChar_SkinTone::Medium;
    CurrentTraits.Height = 1.0f;
    CurrentTraits.Weight = 1.0f;
    CurrentTraits.ClothingStyle = EChar_ClothingStyle::AnimalHide;
    CurrentTraits.bHasTribalPaint = false;
    CurrentTraits.bHasBoneJewelry = false;
    CurrentTraits.bHasScars = false;
    CurrentTraits.CharacterName = TEXT("Default");
    CurrentTraits.TribeName = TEXT("Wanderer");
    
    // Initialize preset traits
    PresetTraits.Empty();
    
    // Add some default presets
    FChar_CharacterTraits HunterPreset;
    HunterPreset.BodyType = EChar_BodyType::Muscular;
    HunterPreset.SkinTone = EChar_SkinTone::Dark;
    HunterPreset.ClothingStyle = EChar_ClothingStyle::AnimalHide;
    HunterPreset.bHasScars = true;
    HunterPreset.CharacterName = TEXT("Tribal Hunter");
    HunterPreset.TribeName = TEXT("Stone Spear");
    PresetTraits.Add(HunterPreset);
    
    FChar_CharacterTraits ShamanPreset;
    ShamanPreset.BodyType = EChar_BodyType::Elderly;
    ShamanPreset.SkinTone = EChar_SkinTone::Weathered;
    ShamanPreset.ClothingStyle = EChar_ClothingStyle::BoneArmor;
    ShamanPreset.bHasTribalPaint = true;
    ShamanPreset.bHasBoneJewelry = true;
    ShamanPreset.CharacterName = TEXT("Cave Shaman");
    ShamanPreset.TribeName = TEXT("Bone Crusher");
    PresetTraits.Add(ShamanPreset);
}

void UChar_MetaHumanManager::ApplyPhysicalTraits(const FChar_CharacterTraits& Traits)
{
    if (!CharacterMesh)
        return;
    
    // Apply scale based on height and weight
    FVector NewScale = FVector(Traits.Weight, Traits.Weight, Traits.Height);
    
    // Adjust scale based on body type
    switch (Traits.BodyType)
    {
        case EChar_BodyType::Lean:
            NewScale.X *= 0.9f;
            NewScale.Y *= 0.9f;
            break;
        case EChar_BodyType::Muscular:
            NewScale.X *= 1.1f;
            NewScale.Y *= 1.1f;
            break;
        case EChar_BodyType::Stocky:
            NewScale.X *= 1.2f;
            NewScale.Y *= 1.2f;
            NewScale.Z *= 0.95f;
            break;
        case EChar_BodyType::Elderly:
            NewScale.Z *= 0.95f;
            break;
        default:
            break;
    }
    
    CharacterMesh->SetWorldScale3D(NewScale);
}

void UChar_MetaHumanManager::ApplyClothingAndAccessories(const FChar_CharacterTraits& Traits)
{
    // This would typically involve swapping skeletal meshes or materials
    // For now, we'll log the intended changes
    UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Applying clothing style %d"), static_cast<int32>(Traits.ClothingStyle));
    
    if (Traits.bHasTribalPaint)
    {
        UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Adding tribal paint"));
    }
    
    if (Traits.bHasBoneJewelry)
    {
        UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Adding bone jewelry"));
    }
    
    if (Traits.bHasScars)
    {
        UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Adding scars"));
    }
}

void UChar_MetaHumanManager::UpdateMaterialParameters(const FChar_CharacterTraits& Traits)
{
    if (!CharacterMesh)
        return;
    
    // Get the current material and create dynamic instance
    UMaterialInterface* CurrentMaterial = CharacterMesh->GetMaterial(0);
    if (CurrentMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = CharacterMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            // Apply skin tone color
            FLinearColor SkinColor;
            switch (Traits.SkinTone)
            {
                case EChar_SkinTone::Light:
                    SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
                    break;
                case EChar_SkinTone::Medium:
                    SkinColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
                    break;
                case EChar_SkinTone::Dark:
                    SkinColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
                    break;
                case EChar_SkinTone::Weathered:
                    SkinColor = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
                    break;
                case EChar_SkinTone::Pale:
                    SkinColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
                    break;
                default:
                    SkinColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
                    break;
            }
            
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), SkinColor);
            UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Applied skin color material parameters"));
        }
    }
}