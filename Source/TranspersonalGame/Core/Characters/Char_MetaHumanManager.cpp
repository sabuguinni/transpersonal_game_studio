#include "Char_MetaHumanManager.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/UnrealMathUtility.h"

UChar_MetaHumanManager::UChar_MetaHumanManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    BodyMeshComponent = nullptr;
    HeadMeshComponent = nullptr;
}

void UChar_MetaHumanManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultPresets();
    CreateDynamicMaterials();
}

void UChar_MetaHumanManager::InitializeDefaultPresets()
{
    SetupTribalWarriorPresets();
    SetupSurvivorPresets();
    SetupElderPresets();
}

void UChar_MetaHumanManager::SetupTribalWarriorPresets()
{
    TribalWarriorPresets.Empty();

    // Preset 1: Young Warrior
    FChar_MetaHumanPreset YoungWarrior;
    YoungWarrior.PresetName = TEXT("Young Warrior");
    YoungWarrior.SkinTone = FLinearColor(0.75f, 0.55f, 0.35f, 1.0f);
    YoungWarrior.HairColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
    YoungWarrior.EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    YoungWarrior.WeatheringLevel = 0.2f;
    YoungWarrior.ScarIntensity = 0.1f;
    TribalWarriorPresets.Add(YoungWarrior);

    // Preset 2: Battle-Scarred Veteran
    FChar_MetaHumanPreset Veteran;
    Veteran.PresetName = TEXT("Battle-Scarred Veteran");
    Veteran.SkinTone = FLinearColor(0.65f, 0.45f, 0.3f, 1.0f);
    Veteran.HairColor = FLinearColor(0.4f, 0.35f, 0.3f, 1.0f);
    Veteran.EyeColor = FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);
    Veteran.WeatheringLevel = 0.8f;
    Veteran.ScarIntensity = 0.9f;
    TribalWarriorPresets.Add(Veteran);

    // Preset 3: Hunter
    FChar_MetaHumanPreset Hunter;
    Hunter.PresetName = TEXT("Hunter");
    Hunter.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    Hunter.HairColor = FLinearColor(0.5f, 0.3f, 0.2f, 1.0f);
    Hunter.EyeColor = FLinearColor(0.1f, 0.5f, 0.2f, 1.0f);
    Hunter.WeatheringLevel = 0.6f;
    Hunter.ScarIntensity = 0.4f;
    TribalWarriorPresets.Add(Hunter);

    // Preset 4: Tribal Chief
    FChar_MetaHumanPreset Chief;
    Chief.PresetName = TEXT("Tribal Chief");
    Chief.SkinTone = FLinearColor(0.7f, 0.5f, 0.35f, 1.0f);
    Chief.HairColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    Chief.EyeColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    Chief.WeatheringLevel = 0.7f;
    Chief.ScarIntensity = 0.6f;
    TribalWarriorPresets.Add(Chief);

    // Preset 5: Scout
    FChar_MetaHumanPreset Scout;
    Scout.PresetName = TEXT("Scout");
    Scout.SkinTone = FLinearColor(0.85f, 0.65f, 0.45f, 1.0f);
    Scout.HairColor = FLinearColor(0.3f, 0.25f, 0.15f, 1.0f);
    Scout.EyeColor = FLinearColor(0.5f, 0.4f, 0.2f, 1.0f);
    Scout.WeatheringLevel = 0.4f;
    Scout.ScarIntensity = 0.2f;
    TribalWarriorPresets.Add(Scout);
}

void UChar_MetaHumanManager::SetupSurvivorPresets()
{
    SurvivorPresets.Empty();

    // Preset 1: Desperate Survivor
    FChar_MetaHumanPreset Desperate;
    Desperate.PresetName = TEXT("Desperate Survivor");
    Desperate.SkinTone = FLinearColor(0.6f, 0.4f, 0.25f, 1.0f);
    Desperate.HairColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    Desperate.EyeColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
    Desperate.WeatheringLevel = 0.9f;
    Desperate.ScarIntensity = 0.7f;
    SurvivorPresets.Add(Desperate);

    // Preset 2: Resourceful Gatherer
    FChar_MetaHumanPreset Gatherer;
    Gatherer.PresetName = TEXT("Resourceful Gatherer");
    Gatherer.SkinTone = FLinearColor(0.75f, 0.55f, 0.4f, 1.0f);
    Gatherer.HairColor = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
    Gatherer.EyeColor = FLinearColor(0.2f, 0.5f, 0.3f, 1.0f);
    Gatherer.WeatheringLevel = 0.5f;
    Gatherer.ScarIntensity = 0.3f;
    SurvivorPresets.Add(Gatherer);

    // Preset 3: Cave Dweller
    FChar_MetaHumanPreset CaveDweller;
    CaveDweller.PresetName = TEXT("Cave Dweller");
    CaveDweller.SkinTone = FLinearColor(0.65f, 0.45f, 0.3f, 1.0f);
    CaveDweller.HairColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
    CaveDweller.EyeColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    CaveDweller.WeatheringLevel = 0.8f;
    CaveDweller.ScarIntensity = 0.5f;
    SurvivorPresets.Add(CaveDweller);
}

void UChar_MetaHumanManager::SetupElderPresets()
{
    ElderPresets.Empty();

    // Preset 1: Wise Elder
    FChar_MetaHumanPreset WiseElder;
    WiseElder.PresetName = TEXT("Wise Elder");
    WiseElder.SkinTone = FLinearColor(0.7f, 0.5f, 0.35f, 1.0f);
    WiseElder.HairColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    WiseElder.EyeColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
    WiseElder.WeatheringLevel = 0.9f;
    WiseElder.ScarIntensity = 0.8f;
    ElderPresets.Add(WiseElder);

    // Preset 2: Tribal Shaman
    FChar_MetaHumanPreset Shaman;
    Shaman.PresetName = TEXT("Knowledge Keeper");
    Shaman.SkinTone = FLinearColor(0.65f, 0.45f, 0.3f, 1.0f);
    Shaman.HairColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
    Shaman.EyeColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f);
    Shaman.WeatheringLevel = 0.7f;
    Shaman.ScarIntensity = 0.6f;
    ElderPresets.Add(Shaman);
}

void UChar_MetaHumanManager::ApplyPreset(const FChar_MetaHumanPreset& Preset)
{
    if (!BodyMeshComponent || !HeadMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("MetaHumanManager: Mesh components not initialized"));
        return;
    }

    // Apply meshes if specified
    if (Preset.BodyMesh.IsValid())
    {
        BodyMeshComponent->SetSkeletalMesh(Preset.BodyMesh.LoadSynchronous());
    }

    if (Preset.HeadMesh.IsValid())
    {
        HeadMeshComponent->SetSkeletalMesh(Preset.HeadMesh.LoadSynchronous());
    }

    // Apply material parameters
    SetSkinTone(Preset.SkinTone);
    SetHairColor(Preset.HairColor);
    SetEyeColor(Preset.EyeColor);
    SetWeatheringLevel(Preset.WeatheringLevel);
    SetScarIntensity(Preset.ScarIntensity);

    UE_LOG(LogTemp, Log, TEXT("MetaHumanManager: Applied preset %s"), *Preset.PresetName);
}

void UChar_MetaHumanManager::RandomizeAppearance(EChar_CharacterType CharacterType)
{
    FChar_MetaHumanPreset RandomPreset = GetRandomPreset(CharacterType);
    
    // Add some randomization to the preset
    RandomPreset.SkinTone = GenerateRandomSkinTone();
    RandomPreset.HairColor = GenerateRandomHairColor();
    RandomPreset.EyeColor = GenerateRandomEyeColor();
    RandomPreset.WeatheringLevel = FMath::RandRange(0.2f, 0.9f);
    RandomPreset.ScarIntensity = FMath::RandRange(0.1f, 0.8f);

    ApplyPreset(RandomPreset);
}

FChar_MetaHumanPreset UChar_MetaHumanManager::GetRandomPreset(EChar_CharacterType CharacterType)
{
    switch (CharacterType)
    {
        case EChar_CharacterType::TribalWarrior:
            if (TribalWarriorPresets.Num() > 0)
                return TribalWarriorPresets[FMath::RandRange(0, TribalWarriorPresets.Num() - 1)];
            break;
        case EChar_CharacterType::Survivor:
            if (SurvivorPresets.Num() > 0)
                return SurvivorPresets[FMath::RandRange(0, SurvivorPresets.Num() - 1)];
            break;
        case EChar_CharacterType::Elder:
            if (ElderPresets.Num() > 0)
                return ElderPresets[FMath::RandRange(0, ElderPresets.Num() - 1)];
            break;
    }

    // Return default preset if no presets available
    return FChar_MetaHumanPreset();
}

void UChar_MetaHumanManager::InitializeCharacterMeshes(USkeletalMeshComponent* InBodyMesh, USkeletalMeshComponent* InHeadMesh)
{
    BodyMeshComponent = InBodyMesh;
    HeadMeshComponent = InHeadMesh;
    CreateDynamicMaterials();
}

void UChar_MetaHumanManager::CreateDynamicMaterials()
{
    DynamicMaterials.Empty();

    if (BodyMeshComponent)
    {
        for (int32 i = 0; i < BodyMeshComponent->GetNumMaterials(); i++)
        {
            UMaterialInterface* BaseMaterial = BodyMeshComponent->GetMaterial(i);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                BodyMeshComponent->SetMaterial(i, DynMaterial);
                DynamicMaterials.Add(DynMaterial);
            }
        }
    }

    if (HeadMeshComponent)
    {
        for (int32 i = 0; i < HeadMeshComponent->GetNumMaterials(); i++)
        {
            UMaterialInterface* BaseMaterial = HeadMeshComponent->GetMaterial(i);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                HeadMeshComponent->SetMaterial(i, DynMaterial);
                DynamicMaterials.Add(DynMaterial);
            }
        }
    }
}

void UChar_MetaHumanManager::SetSkinTone(FLinearColor NewSkinTone)
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(TEXT("SkinTone"), NewSkinTone);
        }
    }
}

void UChar_MetaHumanManager::SetHairColor(FLinearColor NewHairColor)
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(TEXT("HairColor"), NewHairColor);
        }
    }
}

void UChar_MetaHumanManager::SetEyeColor(FLinearColor NewEyeColor)
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(TEXT("EyeColor"), NewEyeColor);
        }
    }
}

void UChar_MetaHumanManager::SetWeatheringLevel(float WeatheringLevel)
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->SetScalarParameterValue(TEXT("WeatheringLevel"), WeatheringLevel);
        }
    }
}

void UChar_MetaHumanManager::SetScarIntensity(float ScarIntensity)
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->SetScalarParameterValue(TEXT("ScarIntensity"), ScarIntensity);
        }
    }
}

void UChar_MetaHumanManager::UpdateMaterialParameters()
{
    // Force update all dynamic materials
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->RecacheUniformExpressions(false);
        }
    }
}

FLinearColor UChar_MetaHumanManager::GenerateRandomSkinTone()
{
    float BaseR = FMath::RandRange(0.6f, 0.9f);
    float BaseG = FMath::RandRange(0.4f, 0.7f);
    float BaseB = FMath::RandRange(0.25f, 0.5f);
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

FLinearColor UChar_MetaHumanManager::GenerateRandomHairColor()
{
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.1f, 0.05f, 0.02f, 1.0f), // Black
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),   // Dark Brown
        FLinearColor(0.5f, 0.3f, 0.2f, 1.0f),   // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),   // Light Brown
        FLinearColor(0.7f, 0.6f, 0.4f, 1.0f),   // Dirty Blonde
        FLinearColor(0.8f, 0.7f, 0.6f, 1.0f)    // Gray (weathered)
    };
    
    return HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
}

FLinearColor UChar_MetaHumanManager::GenerateRandomEyeColor()
{
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Dark Brown
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),  // Brown
        FLinearColor(0.1f, 0.3f, 0.2f, 1.0f),  // Hazel
        FLinearColor(0.2f, 0.4f, 0.6f, 1.0f),  // Blue
        FLinearColor(0.1f, 0.4f, 0.2f, 1.0f),  // Green
        FLinearColor(0.4f, 0.4f, 0.4f, 1.0f)   // Gray
    };
    
    return EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
}