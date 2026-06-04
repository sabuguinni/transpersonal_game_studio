#include "Char_MetaHumanManager.h"
#include "TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UChar_MetaHumanManager::UChar_MetaHumanManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxNPCsPerRole = 10;
    CharacterSpawnRadius = 2000.0f;
    
    // Initialize diversity settings
    DiversitySettings = FChar_DiversitySettings();
}

void UChar_MetaHumanManager::BeginPlay()
{
    Super::BeginPlay();
    InitializePresetDatabase();
}

ATranspersonalCharacter* UChar_MetaHumanManager::SpawnCharacterFromPreset(const FChar_CharacterPreset& Preset, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Spawn the character
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ATranspersonalCharacter* NewCharacter = World->SpawnActor<ATranspersonalCharacter>(Location, Rotation, SpawnParams);
    if (NewCharacter)
    {
        ApplyPresetToCharacter(NewCharacter, Preset);
        NewCharacter->SetActorLabel(Preset.PresetName);
    }

    return NewCharacter;
}

FChar_CharacterPreset UChar_MetaHumanManager::GenerateRandomPreset(EChar_CharacterRole DesiredRole)
{
    FChar_CharacterPreset RandomPreset;
    
    // Set role
    RandomPreset.Role = DesiredRole;
    
    // Generate random characteristics based on diversity settings
    if (DiversitySettings.bEnableGenderBalance)
    {
        RandomPreset.Gender = FMath::RandBool() ? EChar_Gender::Male : EChar_Gender::Female;
    }
    
    if (DiversitySettings.bEnableAgeDiversity)
    {
        int32 AgeRoll = FMath::RandRange(0, 100);
        if (AgeRoll < 20)
            RandomPreset.AgeGroup = EChar_AgeGroup::Young;
        else if (AgeRoll < 80)
            RandomPreset.AgeGroup = EChar_AgeGroup::Adult;
        else
            RandomPreset.AgeGroup = EChar_AgeGroup::Elder;
    }
    
    // Generate appearance
    RandomPreset.SkinTone = GenerateRandomSkinTone();
    RandomPreset.HairColor = GenerateRandomHairColor();
    RandomPreset.MeshPath = SelectRandomMeshPath();
    RandomPreset.MaterialPath = SelectRandomMaterialPath();
    RandomPreset.AccessoryPaths = SelectRandomAccessories(DesiredRole);
    
    // Set survival wear based on role
    switch (DesiredRole)
    {
        case EChar_CharacterRole::Hunter:
            RandomPreset.SurvivalWear = FMath::RandRange(0.7f, 1.0f);
            break;
        case EChar_CharacterRole::Gatherer:
            RandomPreset.SurvivalWear = FMath::RandRange(0.4f, 0.7f);
            break;
        case EChar_CharacterRole::Elder:
            RandomPreset.SurvivalWear = FMath::RandRange(0.8f, 1.0f);
            break;
        default:
            RandomPreset.SurvivalWear = FMath::RandRange(0.3f, 0.8f);
            break;
    }
    
    // Generate unique name
    FString RoleString = StaticEnum<EChar_CharacterRole>()->GetNameStringByValue((int64)DesiredRole);
    FString GenderString = StaticEnum<EChar_Gender>()->GetNameStringByValue((int64)RandomPreset.Gender);
    RandomPreset.PresetName = FString::Printf(TEXT("%s_%s_%d"), *RoleString, *GenderString, FMath::RandRange(1000, 9999));
    
    return RandomPreset;
}

void UChar_MetaHumanManager::ApplyPresetToCharacter(ATranspersonalCharacter* Character, const FChar_CharacterPreset& Preset)
{
    if (!Character)
    {
        return;
    }

    // Apply mesh if available
    if (!Preset.MeshPath.IsEmpty())
    {
        USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, *Preset.MeshPath);
        if (Mesh)
        {
            USkeletalMeshComponent* MeshComp = Character->GetMesh();
            if (MeshComp)
            {
                MeshComp->SetSkeletalMesh(Mesh);
            }
        }
    }

    // Apply material if available
    if (!Preset.MaterialPath.IsEmpty())
    {
        UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *Preset.MaterialPath);
        if (Material)
        {
            USkeletalMeshComponent* MeshComp = Character->GetMesh();
            if (MeshComp)
            {
                MeshComp->SetMaterial(0, Material);
            }
        }
    }

    // Store preset data in character for future reference
    // This would require adding preset storage to TranspersonalCharacter
}

TArray<ATranspersonalCharacter*> UChar_MetaHumanManager::SpawnDiverseNPCGroup(const FVector& CenterLocation, int32 GroupSize)
{
    TArray<ATranspersonalCharacter*> SpawnedCharacters;
    
    // Define role distribution for diverse group
    TArray<EChar_CharacterRole> RolePool = {
        EChar_CharacterRole::Elder,
        EChar_CharacterRole::Hunter,
        EChar_CharacterRole::Gatherer,
        EChar_CharacterRole::Scout,
        EChar_CharacterRole::Crafter
    };
    
    for (int32 i = 0; i < GroupSize; i++)
    {
        // Select role from pool
        EChar_CharacterRole SelectedRole = RolePool[i % RolePool.Num()];
        
        // Generate random preset
        FChar_CharacterPreset Preset = GenerateRandomPreset(SelectedRole);
        
        // Calculate spawn location in circle around center
        float Angle = (2.0f * PI * i) / GroupSize;
        float Distance = FMath::RandRange(100.0f, CharacterSpawnRadius);
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Spawn character
        ATranspersonalCharacter* NewCharacter = SpawnCharacterFromPreset(Preset, SpawnLocation);
        if (NewCharacter)
        {
            SpawnedCharacters.Add(NewCharacter);
        }
    }
    
    return SpawnedCharacters;
}

void UChar_MetaHumanManager::InitializePresetDatabase()
{
    CreateDefaultPresets();
}

FChar_CharacterPreset UChar_MetaHumanManager::GetPresetByName(const FString& PresetName) const
{
    for (const FChar_CharacterPreset& Preset : CharacterPresets)
    {
        if (Preset.PresetName == PresetName)
        {
            return Preset;
        }
    }
    
    // Return default if not found
    return FChar_CharacterPreset();
}

TArray<FChar_CharacterPreset> UChar_MetaHumanManager::GetPresetsByRole(EChar_CharacterRole Role) const
{
    TArray<FChar_CharacterPreset> FilteredPresets;
    
    for (const FChar_CharacterPreset& Preset : CharacterPresets)
    {
        if (Preset.Role == Role)
        {
            FilteredPresets.Add(Preset);
        }
    }
    
    return FilteredPresets;
}

void UChar_MetaHumanManager::PreviewCharacterPreset(const FChar_CharacterPreset& Preset)
{
    // This would be used in the editor to preview character presets
    // Implementation would depend on editor-specific functionality
    UE_LOG(LogTemp, Log, TEXT("Previewing character preset: %s"), *Preset.PresetName);
}

void UChar_MetaHumanManager::CreateDefaultPresets()
{
    CharacterPresets.Empty();
    
    // Create tribal elder preset
    FChar_CharacterPreset ElderPreset;
    ElderPreset.PresetName = TEXT("TribalElder_Male");
    ElderPreset.Role = EChar_CharacterRole::Elder;
    ElderPreset.Gender = EChar_Gender::Male;
    ElderPreset.AgeGroup = EChar_AgeGroup::Elder;
    ElderPreset.SkinTone = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    ElderPreset.HairColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    ElderPreset.SurvivalWear = 0.9f;
    CharacterPresets.Add(ElderPreset);
    
    // Create hunter preset
    FChar_CharacterPreset HunterPreset;
    HunterPreset.PresetName = TEXT("Hunter_Female");
    HunterPreset.Role = EChar_CharacterRole::Hunter;
    HunterPreset.Gender = EChar_Gender::Female;
    HunterPreset.AgeGroup = EChar_AgeGroup::Adult;
    HunterPreset.SkinTone = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
    HunterPreset.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
    HunterPreset.SurvivalWear = 0.8f;
    CharacterPresets.Add(HunterPreset);
    
    // Create gatherer preset
    FChar_CharacterPreset GathererPreset;
    GathererPreset.PresetName = TEXT("Gatherer_Male");
    GathererPreset.Role = EChar_CharacterRole::Gatherer;
    GathererPreset.Gender = EChar_Gender::Male;
    GathererPreset.AgeGroup = EChar_AgeGroup::Adult;
    GathererPreset.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    GathererPreset.HairColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    GathererPreset.SurvivalWear = 0.6f;
    CharacterPresets.Add(GathererPreset);
    
    // Create scout preset
    FChar_CharacterPreset ScoutPreset;
    ScoutPreset.PresetName = TEXT("Scout_Female");
    ScoutPreset.Role = EChar_CharacterRole::Scout;
    ScoutPreset.Gender = EChar_Gender::Female;
    ScoutPreset.AgeGroup = EChar_AgeGroup::Young;
    ScoutPreset.SkinTone = FLinearColor(0.75f, 0.55f, 0.35f, 1.0f);
    ScoutPreset.HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    ScoutPreset.SurvivalWear = 0.7f;
    CharacterPresets.Add(ScoutPreset);
    
    // Create shaman preset
    FChar_CharacterPreset ShamanPreset;
    ShamanPreset.PresetName = TEXT("Shaman_Male");
    ShamanPreset.Role = EChar_CharacterRole::Crafter;
    ShamanPreset.Gender = EChar_Gender::Male;
    ShamanPreset.AgeGroup = EChar_AgeGroup::Adult;
    ShamanPreset.SkinTone = FLinearColor(0.65f, 0.45f, 0.35f, 1.0f);
    ShamanPreset.HairColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    ShamanPreset.SurvivalWear = 0.85f;
    CharacterPresets.Add(ShamanPreset);
}

FLinearColor UChar_MetaHumanManager::GenerateRandomSkinTone() const
{
    if (!DiversitySettings.bEnableEthnicDiversity)
    {
        return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    }
    
    // Generate diverse skin tones
    float Variation = DiversitySettings.SkinToneVariation;
    float BaseR = FMath::RandRange(0.3f, 0.9f) * Variation;
    float BaseG = FMath::RandRange(0.2f, 0.7f) * Variation;
    float BaseB = FMath::RandRange(0.1f, 0.5f) * Variation;
    
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

FLinearColor UChar_MetaHumanManager::GenerateRandomHairColor() const
{
    // Common prehistoric hair colors
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.1f, 0.1f, 0.1f, 1.0f),    // Black
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),    // Dark brown
        FLinearColor(0.5f, 0.3f, 0.2f, 1.0f),    // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),    // Light brown
        FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)     // Gray (for elders)
    };
    
    return HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
}

FString UChar_MetaHumanManager::SelectRandomMeshPath() const
{
    // Return default mannequin path for now
    // In a real implementation, this would select from available MetaHuman meshes
    return TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple");
}

FString UChar_MetaHumanManager::SelectRandomMaterialPath() const
{
    // Return default material path for now
    // In a real implementation, this would select from available character materials
    return TEXT("/Game/Characters/Mannequins/Materials/M_Quinn_Body");
}

TArray<FString> UChar_MetaHumanManager::SelectRandomAccessories(EChar_CharacterRole Role) const
{
    TArray<FString> Accessories;
    
    // Role-specific accessories
    switch (Role)
    {
        case EChar_CharacterRole::Hunter:
            Accessories.Add(TEXT("/Game/Accessories/Weapons/Spear"));
            Accessories.Add(TEXT("/Game/Accessories/Clothing/LeatherArmor"));
            break;
        case EChar_CharacterRole::Gatherer:
            Accessories.Add(TEXT("/Game/Accessories/Tools/WovenBasket"));
            Accessories.Add(TEXT("/Game/Accessories/Clothing/PlantFiberCloak"));
            break;
        case EChar_CharacterRole::Elder:
            Accessories.Add(TEXT("/Game/Accessories/Jewelry/BoneNecklace"));
            Accessories.Add(TEXT("/Game/Accessories/Tools/WalkingStick"));
            break;
        case EChar_CharacterRole::Scout:
            Accessories.Add(TEXT("/Game/Accessories/Tools/SlingBag"));
            break;
        case EChar_CharacterRole::Crafter:
            Accessories.Add(TEXT("/Game/Accessories/Clothing/RitualMarkings"));
            Accessories.Add(TEXT("/Game/Accessories/Jewelry/FeatherHeadband"));
            break;
        default:
            break;
    }
    
    return Accessories;
}