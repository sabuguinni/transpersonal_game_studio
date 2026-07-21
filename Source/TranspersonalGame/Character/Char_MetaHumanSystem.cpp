#include "Char_MetaHumanSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UChar_MetaHumanSystem::UChar_MetaHumanSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    MetaHumanMesh = nullptr;
    
    // Initialize default character data
    CharacterData = FChar_TribalCharacterData();
    
    InitializeDefaultMorphTargets();
}

void UChar_MetaHumanSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        MetaHumanMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (MetaHumanMesh)
        {
            ApplyTribalCharacterData(CharacterData);
        }
    }
}

void UChar_MetaHumanSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_MetaHumanSystem::ApplyTribalCharacterData(const FChar_TribalCharacterData& NewCharacterData)
{
    CharacterData = NewCharacterData;
    
    if (!MetaHumanMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("MetaHumanMesh not found for character: %s"), *CharacterData.CharacterName);
        return;
    }
    
    // Apply physical characteristics
    ApplyMorphTargets();
    
    // Setup clothing and materials
    SetupTribalClothing();
    
    // Apply weathering effects based on role
    ApplyWeatheringEffects();
    
    // Attach equipment
    AttachTribalEquipment();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal character data for: %s"), *CharacterData.CharacterName);
}

void UChar_MetaHumanSystem::RandomizeTribalAppearance()
{
    FChar_TribalCharacterData RandomData = GenerateRandomTribalData();
    ApplyTribalCharacterData(RandomData);
}

void UChar_MetaHumanSystem::SetTribalRole(EChar_TribalRole NewRole)
{
    CharacterData.TribalRole = NewRole;
    
    // Adjust equipment and appearance based on role
    switch (NewRole)
    {
        case EChar_TribalRole::Hunter:
            CharacterData.PhysicalBuild = EChar_PhysicalBuild::Athletic;
            CharacterData.EquipmentList = {TEXT("Spear"), TEXT("Stone Knife"), TEXT("Hide Pouch")};
            break;
            
        case EChar_TribalRole::Gatherer:
            CharacterData.PhysicalBuild = EChar_PhysicalBuild::Lean;
            CharacterData.EquipmentList = {TEXT("Gathering Basket"), TEXT("Digging Stick"), TEXT("Plant Pouch")};
            break;
            
        case EChar_TribalRole::Elder:
            CharacterData.PhysicalBuild = EChar_PhysicalBuild::Stocky;
            CharacterData.EquipmentList = {TEXT("Walking Staff"), TEXT("Wisdom Beads"), TEXT("Ceremonial Cloak")};
            break;
            
        case EChar_TribalRole::Scout:
            CharacterData.PhysicalBuild = EChar_PhysicalBuild::Tall;
            CharacterData.EquipmentList = {TEXT("Sling"), TEXT("Signal Horn"), TEXT("Travel Pack")};
            break;
            
        case EChar_TribalRole::Crafter:
            CharacterData.PhysicalBuild = EChar_PhysicalBuild::Short;
            CharacterData.EquipmentList = {TEXT("Stone Tools"), TEXT("Crafting Materials"), TEXT("Work Apron")};
            break;
            
        case EChar_TribalRole::Healer:
            CharacterData.PhysicalBuild = EChar_PhysicalBuild::Lean;
            CharacterData.EquipmentList = {TEXT("Medicine Pouch"), TEXT("Healing Herbs"), TEXT("Bone Needles")};
            break;
    }
    
    ApplyTribalCharacterData(CharacterData);
}

void UChar_MetaHumanSystem::ApplyMorphTargets()
{
    if (!MetaHumanMesh || !MetaHumanMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Apply morph targets based on physical build
    float HeightModifier = (CharacterData.Height - 170.0f) / 170.0f;
    float WeightModifier = (CharacterData.Weight - 70.0f) / 70.0f;
    
    // Apply height scaling
    if (MorphTargetValues.Contains(TEXT("Height")))
    {
        MorphTargetValues[TEXT("Height")] = FMath::Clamp(HeightModifier, -0.3f, 0.3f);
    }
    
    // Apply build-specific morph targets
    switch (CharacterData.PhysicalBuild)
    {
        case EChar_PhysicalBuild::Athletic:
            MorphTargetValues.Add(TEXT("Muscle_Definition"), 0.7f);
            MorphTargetValues.Add(TEXT("Body_Fat"), 0.2f);
            break;
            
        case EChar_PhysicalBuild::Lean:
            MorphTargetValues.Add(TEXT("Muscle_Definition"), 0.4f);
            MorphTargetValues.Add(TEXT("Body_Fat"), 0.1f);
            break;
            
        case EChar_PhysicalBuild::Stocky:
            MorphTargetValues.Add(TEXT("Muscle_Definition"), 0.5f);
            MorphTargetValues.Add(TEXT("Body_Fat"), 0.6f);
            break;
            
        case EChar_PhysicalBuild::Tall:
            MorphTargetValues.Add(TEXT("Height_Stretch"), 0.8f);
            MorphTargetValues.Add(TEXT("Limb_Length"), 0.7f);
            break;
            
        case EChar_PhysicalBuild::Short:
            MorphTargetValues.Add(TEXT("Height_Compress"), 0.6f);
            MorphTargetValues.Add(TEXT("Torso_Compact"), 0.5f);
            break;
    }
    
    // Apply all morph target values
    for (const auto& MorphPair : MorphTargetValues)
    {
        MetaHumanMesh->SetMorphTarget(FName(*MorphPair.Key), MorphPair.Value);
    }
}

void UChar_MetaHumanSystem::SetSkinMaterial(int32 MaterialIndex)
{
    if (!MetaHumanMesh || MaterialIndex >= SkinMaterials.Num())
    {
        return;
    }
    
    if (SkinMaterials[MaterialIndex])
    {
        MetaHumanMesh->SetMaterial(0, SkinMaterials[MaterialIndex]);
    }
}

void UChar_MetaHumanSystem::SetClothingMaterial(int32 MaterialIndex)
{
    if (!MetaHumanMesh || MaterialIndex >= ClothingMaterials.Num())
    {
        return;
    }
    
    if (ClothingMaterials[MaterialIndex])
    {
        // Apply to clothing material slots (typically slots 1-3)
        for (int32 i = 1; i <= 3; i++)
        {
            MetaHumanMesh->SetMaterial(i, ClothingMaterials[MaterialIndex]);
        }
    }
}

void UChar_MetaHumanSystem::AttachTribalEquipment()
{
    if (!MetaHumanMesh)
    {
        return;
    }
    
    // Attach equipment based on the character's equipment list
    for (const FString& Equipment : CharacterData.EquipmentList)
    {
        // This would typically load and attach static mesh components
        // For now, we'll log the equipment being attached
        UE_LOG(LogTemp, Log, TEXT("Attaching equipment: %s to character: %s"), 
               *Equipment, *CharacterData.CharacterName);
    }
}

bool UChar_MetaHumanSystem::ValidateCharacterSetup() const
{
    if (!MetaHumanMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("MetaHumanMesh is null for character: %s"), *CharacterData.CharacterName);
        return false;
    }
    
    if (!MetaHumanMesh->GetSkeletalMeshAsset())
    {
        UE_LOG(LogTemp, Error, TEXT("SkeletalMeshAsset is null for character: %s"), *CharacterData.CharacterName);
        return false;
    }
    
    if (CharacterData.CharacterName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Character name is empty"));
        return false;
    }
    
    return true;
}

void UChar_MetaHumanSystem::InitializeDefaultMorphTargets()
{
    // Initialize common morph targets with default values
    MorphTargetValues.Add(TEXT("Height"), 0.0f);
    MorphTargetValues.Add(TEXT("Weight"), 0.0f);
    MorphTargetValues.Add(TEXT("Muscle_Definition"), 0.5f);
    MorphTargetValues.Add(TEXT("Body_Fat"), 0.3f);
    MorphTargetValues.Add(TEXT("Face_Width"), 0.0f);
    MorphTargetValues.Add(TEXT("Eye_Size"), 0.0f);
    MorphTargetValues.Add(TEXT("Nose_Size"), 0.0f);
    MorphTargetValues.Add(TEXT("Jaw_Width"), 0.0f);
    MorphTargetValues.Add(TEXT("Weathering"), 0.0f);
    MorphTargetValues.Add(TEXT("Scars"), 0.0f);
}

void UChar_MetaHumanSystem::SetupTribalClothing()
{
    if (!MetaHumanMesh)
    {
        return;
    }
    
    // Setup clothing based on tribal role and environment
    FString ClothingType = CharacterData.ClothingStyle;
    
    // Apply role-specific clothing modifications
    switch (CharacterData.TribalRole)
    {
        case EChar_TribalRole::Hunter:
            ClothingType = TEXT("Hunter_Leather");
            break;
            
        case EChar_TribalRole::Gatherer:
            ClothingType = TEXT("Gatherer_Woven");
            break;
            
        case EChar_TribalRole::Elder:
            ClothingType = TEXT("Elder_Ceremonial");
            break;
            
        case EChar_TribalRole::Scout:
            ClothingType = TEXT("Scout_Lightweight");
            break;
            
        case EChar_TribalRole::Crafter:
            ClothingType = TEXT("Crafter_Practical");
            break;
            
        case EChar_TribalRole::Healer:
            ClothingType = TEXT("Healer_Decorated");
            break;
    }
    
    CharacterData.ClothingStyle = ClothingType;
    UE_LOG(LogTemp, Log, TEXT("Setup clothing type: %s for character: %s"), 
           *ClothingType, *CharacterData.CharacterName);
}

void UChar_MetaHumanSystem::ApplyWeatheringEffects()
{
    if (!MetaHumanMesh)
    {
        return;
    }
    
    // Apply weathering based on role and age
    float WeatheringAmount = 0.0f;
    float ScarAmount = 0.0f;
    
    switch (CharacterData.TribalRole)
    {
        case EChar_TribalRole::Hunter:
            WeatheringAmount = 0.7f;
            ScarAmount = 0.6f;
            break;
            
        case EChar_TribalRole::Scout:
            WeatheringAmount = 0.8f;
            ScarAmount = 0.4f;
            break;
            
        case EChar_TribalRole::Elder:
            WeatheringAmount = 0.9f;
            ScarAmount = 0.3f;
            break;
            
        case EChar_TribalRole::Gatherer:
            WeatheringAmount = 0.5f;
            ScarAmount = 0.2f;
            break;
            
        case EChar_TribalRole::Crafter:
            WeatheringAmount = 0.4f;
            ScarAmount = 0.3f;
            break;
            
        case EChar_TribalRole::Healer:
            WeatheringAmount = 0.3f;
            ScarAmount = 0.1f;
            break;
    }
    
    MorphTargetValues[TEXT("Weathering")] = WeatheringAmount;
    MorphTargetValues[TEXT("Scars")] = ScarAmount;
    
    // Apply the morph targets
    MetaHumanMesh->SetMorphTarget(FName(TEXT("Weathering")), WeatheringAmount);
    MetaHumanMesh->SetMorphTarget(FName(TEXT("Scars")), ScarAmount);
}

FChar_TribalCharacterData UChar_MetaHumanSystem::GenerateRandomTribalData()
{
    FChar_TribalCharacterData RandomData;
    
    // Random role
    int32 RoleIndex = FMath::RandRange(0, 5);
    RandomData.TribalRole = static_cast<EChar_TribalRole>(RoleIndex);
    
    // Random physical build
    int32 BuildIndex = FMath::RandRange(0, 4);
    RandomData.PhysicalBuild = static_cast<EChar_PhysicalBuild>(BuildIndex);
    
    // Random physical characteristics
    RandomData.Height = FMath::RandRange(150.0f, 190.0f);
    RandomData.Weight = FMath::RandRange(55.0f, 85.0f);
    
    // Random appearance
    TArray<FString> SkinTones = {TEXT("Light"), TEXT("Medium"), TEXT("Dark"), TEXT("Tanned")};
    TArray<FString> HairColors = {TEXT("Black"), TEXT("Dark Brown"), TEXT("Brown"), TEXT("Light Brown")};
    TArray<FString> EyeColors = {TEXT("Brown"), TEXT("Dark Brown"), TEXT("Hazel"), TEXT("Green")};
    
    RandomData.SkinTone = SkinTones[FMath::RandRange(0, SkinTones.Num() - 1)];
    RandomData.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    RandomData.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Generate name based on role
    TArray<FString> HunterNames = {TEXT("Kael"), TEXT("Thorne"), TEXT("Bram"), TEXT("Dex")};
    TArray<FString> GathererNames = {TEXT("Lyra"), TEXT("Sage"), TEXT("Fern"), TEXT("Iris")};
    TArray<FString> ElderNames = {TEXT("Aldric"), TEXT("Vera"), TEXT("Magnus"), TEXT("Cora")};
    TArray<FString> ScoutNames = {TEXT("Swift"), TEXT("Keen"), TEXT("Dash"), TEXT("Scout")};
    TArray<FString> CrafterNames = {TEXT("Smith"), TEXT("Clay"), TEXT("Forge"), TEXT("Craft")};
    TArray<FString> HealerNames = {TEXT("Mend"), TEXT("Heal"), TEXT("Sage"), TEXT("Wise")};
    
    switch (RandomData.TribalRole)
    {
        case EChar_TribalRole::Hunter:
            RandomData.CharacterName = HunterNames[FMath::RandRange(0, HunterNames.Num() - 1)];
            break;
        case EChar_TribalRole::Gatherer:
            RandomData.CharacterName = GathererNames[FMath::RandRange(0, GathererNames.Num() - 1)];
            break;
        case EChar_TribalRole::Elder:
            RandomData.CharacterName = ElderNames[FMath::RandRange(0, ElderNames.Num() - 1)];
            break;
        case EChar_TribalRole::Scout:
            RandomData.CharacterName = ScoutNames[FMath::RandRange(0, ScoutNames.Num() - 1)];
            break;
        case EChar_TribalRole::Crafter:
            RandomData.CharacterName = CrafterNames[FMath::RandRange(0, CrafterNames.Num() - 1)];
            break;
        case EChar_TribalRole::Healer:
            RandomData.CharacterName = HealerNames[FMath::RandRange(0, HealerNames.Num() - 1)];
            break;
    }
    
    return RandomData;
}