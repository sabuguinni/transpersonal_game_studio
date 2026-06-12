#include "Char_MetaHumanManager.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"

AChar_MetaHumanManager::AChar_MetaHumanManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default character presets
    InitializeDefaultPresets();
    LoadDefaultAssets();
}

void AChar_MetaHumanManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHuman Manager initialized with %d character presets"), CharacterPresets.Num());
}

void AChar_MetaHumanManager::InitializeDefaultPresets()
{
    CharacterPresets.Empty();

    // Tribal Warrior Preset
    FChar_CharacterPreset WarriorPreset;
    WarriorPreset.CharacterType = EChar_CharacterType::TribalWarrior;
    WarriorPreset.CharacterName = TEXT("Krog the Fierce");
    WarriorPreset.SkinTone = EChar_SkinTone::Tanned;
    CharacterPresets.Add(WarriorPreset);

    // Tribal Hunter Preset
    FChar_CharacterPreset HunterPreset;
    HunterPreset.CharacterType = EChar_CharacterType::TribalHunter;
    HunterPreset.CharacterName = TEXT("Nara Swift-Arrow");
    HunterPreset.SkinTone = EChar_SkinTone::Medium;
    CharacterPresets.Add(HunterPreset);

    // Tribal Shaman Preset
    FChar_CharacterPreset ShamanPreset;
    ShamanPreset.CharacterType = EChar_CharacterType::TribalShaman;
    ShamanPreset.CharacterName = TEXT("Elder Thane");
    ShamanPreset.SkinTone = EChar_SkinTone::Weathered;
    CharacterPresets.Add(ShamanPreset);

    // Survivor Male Preset
    FChar_CharacterPreset SurvivorMalePreset;
    SurvivorMalePreset.CharacterType = EChar_CharacterType::SurvivorMale;
    SurvivorMalePreset.CharacterName = TEXT("Marcus Stone");
    SurvivorMalePreset.SkinTone = EChar_SkinTone::Light;
    CharacterPresets.Add(SurvivorMalePreset);

    // Survivor Female Preset
    FChar_CharacterPreset SurvivorFemalePreset;
    SurvivorFemalePreset.CharacterType = EChar_CharacterType::SurvivorFemale;
    SurvivorFemalePreset.CharacterName = TEXT("Elena Rivers");
    SurvivorFemalePreset.SkinTone = EChar_SkinTone::Medium;
    CharacterPresets.Add(SurvivorFemalePreset);

    // Elder Wise Preset
    FChar_CharacterPreset ElderPreset;
    ElderPreset.CharacterType = EChar_CharacterType::ElderWise;
    ElderPreset.CharacterName = TEXT("Grandmother Willow");
    ElderPreset.SkinTone = EChar_SkinTone::Weathered;
    CharacterPresets.Add(ElderPreset);

    // Young Scout Preset
    FChar_CharacterPreset ScoutPreset;
    ScoutPreset.CharacterType = EChar_CharacterType::YoungScout;
    ScoutPreset.CharacterName = TEXT("Swift Foot");
    ScoutPreset.SkinTone = EChar_SkinTone::Tanned;
    CharacterPresets.Add(ScoutPreset);
}

void AChar_MetaHumanManager::LoadDefaultAssets()
{
    // Try to load default MetaHuman assets
    // These paths would need to be updated based on actual MetaHuman content
    DefaultMaleBody = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/MetaHumans/Common/Male/Medium/NormalWeight/Body/m_med_nrw_body.m_med_nrw_body")));
    DefaultFemaleBody = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/MetaHumans/Common/Female/Medium/NormalWeight/Body/f_med_nrw_body.f_med_nrw_body")));

    // Add head variations (placeholder paths)
    MaleHeadVariations.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/MetaHumans/Common/Male/Medium/NormalWeight/Head/m_med_nrw_head_01.m_med_nrw_head_01"))));
    MaleHeadVariations.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/MetaHumans/Common/Male/Medium/NormalWeight/Head/m_med_nrw_head_02.m_med_nrw_head_02"))));
    
    FemaleHeadVariations.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/MetaHumans/Common/Female/Medium/NormalWeight/Head/f_med_nrw_head_01.f_med_nrw_head_01"))));
    FemaleHeadVariations.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/MetaHumans/Common/Female/Medium/NormalWeight/Head/f_med_nrw_head_02.f_med_nrw_head_02"))));
}

FChar_CharacterPreset AChar_MetaHumanManager::CreateRandomCharacter(EChar_CharacterType CharacterType)
{
    FChar_CharacterPreset RandomPreset;
    RandomPreset.CharacterType = CharacterType;

    // Random skin tone
    int32 SkinToneIndex = UKismetMathLibrary::RandomIntegerInRange(0, 4);
    RandomPreset.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);

    // Generate random name based on character type
    TArray<FString> Names;
    switch (CharacterType)
    {
        case EChar_CharacterType::TribalWarrior:
            Names = {TEXT("Krog"), TEXT("Thane"), TEXT("Brok"), TEXT("Grim"), TEXT("Stone")};
            break;
        case EChar_CharacterType::TribalHunter:
            Names = {TEXT("Nara"), TEXT("Swift"), TEXT("Arrow"), TEXT("Track"), TEXT("Hunt")};
            break;
        case EChar_CharacterType::TribalShaman:
            Names = {TEXT("Elder"), TEXT("Wise"), TEXT("Spirit"), TEXT("Dream"), TEXT("Vision")};
            break;
        case EChar_CharacterType::SurvivorMale:
            Names = {TEXT("Marcus"), TEXT("David"), TEXT("John"), TEXT("Michael"), TEXT("Robert")};
            break;
        case EChar_CharacterType::SurvivorFemale:
            Names = {TEXT("Elena"), TEXT("Sarah"), TEXT("Lisa"), TEXT("Maria"), TEXT("Anna")};
            break;
        default:
            Names = {TEXT("Survivor"), TEXT("Wanderer"), TEXT("Nomad")};
            break;
    }

    int32 NameIndex = UKismetMathLibrary::RandomIntegerInRange(0, Names.Num() - 1);
    RandomPreset.CharacterName = Names[NameIndex];

    // Assign appropriate meshes based on character type
    bool bIsFemale = (CharacterType == EChar_CharacterType::TribalHunter || 
                      CharacterType == EChar_CharacterType::SurvivorFemale ||
                      CharacterType == EChar_CharacterType::ElderWise);

    RandomPreset.BodyMesh = bIsFemale ? DefaultFemaleBody : DefaultMaleBody;
    RandomPreset.HeadMesh = TSoftObjectPtr<USkeletalMesh>(GetRandomHeadMesh(bIsFemale));

    return RandomPreset;
}

bool AChar_MetaHumanManager::ApplyCharacterPreset(USkeletalMeshComponent* TargetMesh, const FChar_CharacterPreset& Preset)
{
    if (!TargetMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyCharacterPreset: TargetMesh is null"));
        return false;
    }

    // Load and apply body mesh
    if (Preset.BodyMesh.IsValid())
    {
        USkeletalMesh* BodyMesh = Preset.BodyMesh.LoadSynchronous();
        if (BodyMesh)
        {
            TargetMesh->SetSkeletalMesh(BodyMesh);
        }
    }

    // Apply skin material based on skin tone
    UMaterialInterface* SkinMaterial = GetSkinMaterialForTone(Preset.SkinTone);
    if (SkinMaterial)
    {
        TargetMesh->SetMaterial(0, SkinMaterial);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied character preset: %s (%s)"), 
           *Preset.CharacterName, 
           *UEnum::GetValueAsString(Preset.CharacterType));

    return true;
}

FChar_CharacterPreset AChar_MetaHumanManager::GetPresetByType(EChar_CharacterType CharacterType)
{
    for (const FChar_CharacterPreset& Preset : CharacterPresets)
    {
        if (Preset.CharacterType == CharacterType)
        {
            return Preset;
        }
    }

    // Return default if not found
    return CreateRandomCharacter(CharacterType);
}

bool AChar_MetaHumanManager::SetCharacterSkinTone(USkeletalMeshComponent* TargetMesh, EChar_SkinTone SkinTone)
{
    if (!TargetMesh)
    {
        return false;
    }

    UMaterialInterface* SkinMaterial = GetSkinMaterialForTone(SkinTone);
    if (SkinMaterial)
    {
        TargetMesh->SetMaterial(0, SkinMaterial);
        return true;
    }

    return false;
}

bool AChar_MetaHumanManager::SetCharacterClothing(USkeletalMeshComponent* TargetMesh, int32 ClothingIndex)
{
    if (!TargetMesh || ClothingIndex < 0 || ClothingIndex >= ClothingMaterials.Num())
    {
        return false;
    }

    UMaterialInterface* ClothingMaterial = ClothingMaterials[ClothingIndex].LoadSynchronous();
    if (ClothingMaterial)
    {
        TargetMesh->SetMaterial(1, ClothingMaterial); // Assuming clothing is material slot 1
        return true;
    }

    return false;
}

TArray<FString> AChar_MetaHumanManager::GetAvailableCharacterNames()
{
    TArray<FString> Names;
    for (const FChar_CharacterPreset& Preset : CharacterPresets)
    {
        Names.Add(Preset.CharacterName);
    }
    return Names;
}

UMaterialInterface* AChar_MetaHumanManager::GetSkinMaterialForTone(EChar_SkinTone SkinTone)
{
    // This would load appropriate skin materials based on tone
    // For now, return null - materials would need to be created/imported
    switch (SkinTone)
    {
        case EChar_SkinTone::Light:
            // Return light skin material
            break;
        case EChar_SkinTone::Medium:
            // Return medium skin material
            break;
        case EChar_SkinTone::Dark:
            // Return dark skin material
            break;
        case EChar_SkinTone::Tanned:
            // Return tanned skin material
            break;
        case EChar_SkinTone::Weathered:
            // Return weathered skin material
            break;
    }

    return nullptr;
}

USkeletalMesh* AChar_MetaHumanManager::GetRandomHeadMesh(bool bIsFemale)
{
    if (bIsFemale && FemaleHeadVariations.Num() > 0)
    {
        int32 Index = UKismetMathLibrary::RandomIntegerInRange(0, FemaleHeadVariations.Num() - 1);
        return FemaleHeadVariations[Index].LoadSynchronous();
    }
    else if (!bIsFemale && MaleHeadVariations.Num() > 0)
    {
        int32 Index = UKismetMathLibrary::RandomIntegerInRange(0, MaleHeadVariations.Num() - 1);
        return MaleHeadVariations[Index].LoadSynchronous();
    }

    return nullptr;
}