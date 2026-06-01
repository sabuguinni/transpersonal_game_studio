#include "Char_AppearanceManager.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UChar_AppearanceManager::UChar_AppearanceManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    SkinToneVariation = 0.3f;
    BodyScaleVariation = 0.1f;
    bRandomizeOnStart = false;
    
    // Initialize default appearance
    CurrentAppearance = FChar_AppearanceData();
}

void UChar_AppearanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePresetAppearances();
    InitializeTribalClothing();
    
    if (bRandomizeOnStart)
    {
        RandomizeAppearance();
    }
    
    // Apply current appearance to the character
    ApplyAppearanceData(CurrentAppearance);
}

void UChar_AppearanceManager::InitializePresetAppearances()
{
    PresetAppearances.Empty();
    
    // Hunter preset - lean and agile
    FChar_AppearanceData HunterPreset;
    HunterPreset.TribalRole = EChar_TribalRole::Hunter;
    HunterPreset.SkinTone = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    HunterPreset.HairColor = FLinearColor(0.15f, 0.1f, 0.05f, 1.0f);
    HunterPreset.BodyScale = 0.95f;
    HunterPreset.Gender = EChar_Gender::Male;
    HunterPreset.AgeGroup = EChar_AgeGroup::Adult;
    PresetAppearances.Add(HunterPreset);
    
    // Gatherer preset - balanced build
    FChar_AppearanceData GathererPreset;
    GathererPreset.TribalRole = EChar_TribalRole::Gatherer;
    GathererPreset.SkinTone = FLinearColor(0.75f, 0.55f, 0.35f, 1.0f);
    GathererPreset.HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    GathererPreset.BodyScale = 1.0f;
    GathererPreset.Gender = EChar_Gender::Female;
    GathererPreset.AgeGroup = EChar_AgeGroup::Adult;
    PresetAppearances.Add(GathererPreset);
    
    // Elder preset - wise and weathered
    FChar_AppearanceData ElderPreset;
    ElderPreset.TribalRole = EChar_TribalRole::Elder;
    ElderPreset.SkinTone = FLinearColor(0.65f, 0.45f, 0.25f, 1.0f);
    ElderPreset.HairColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    ElderPreset.BodyScale = 0.9f;
    ElderPreset.Gender = EChar_Gender::Male;
    ElderPreset.AgeGroup = EChar_AgeGroup::Elder;
    PresetAppearances.Add(ElderPreset);
    
    // Crafter preset - skilled hands
    FChar_AppearanceData CrafterPreset;
    CrafterPreset.TribalRole = EChar_TribalRole::Crafter;
    CrafterPreset.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    CrafterPreset.HairColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
    CrafterPreset.BodyScale = 1.05f;
    CrafterPreset.Gender = EChar_Gender::Female;
    CrafterPreset.AgeGroup = EChar_AgeGroup::Adult;
    PresetAppearances.Add(CrafterPreset);
    
    // Warrior preset - strong and scarred
    FChar_AppearanceData WarriorPreset;
    WarriorPreset.TribalRole = EChar_TribalRole::Warrior;
    WarriorPreset.SkinTone = FLinearColor(0.6f, 0.4f, 0.25f, 1.0f);
    WarriorPreset.HairColor = FLinearColor(0.1f, 0.05f, 0.02f, 1.0f);
    WarriorPreset.BodyScale = 1.1f;
    WarriorPreset.Gender = EChar_Gender::Male;
    WarriorPreset.AgeGroup = EChar_AgeGroup::Adult;
    PresetAppearances.Add(WarriorPreset);
}

void UChar_AppearanceManager::InitializeTribalClothing()
{
    AvailableClothing.Empty();
    
    // Hunter clothing - lightweight hide
    FChar_TribalClothing HunterClothing;
    HunterClothing.ClothingName = TEXT("Hunter's Hide");
    HunterClothing.SuitableForRole = EChar_TribalRole::Hunter;
    AvailableClothing.Add(HunterClothing);
    
    // Gatherer clothing - practical wraps
    FChar_TribalClothing GathererClothing;
    GathererClothing.ClothingName = TEXT("Gatherer's Wraps");
    GathererClothing.SuitableForRole = EChar_TribalRole::Gatherer;
    AvailableClothing.Add(GathererClothing);
    
    // Elder clothing - ceremonial robes
    FChar_TribalClothing ElderClothing;
    ElderClothing.ClothingName = TEXT("Elder's Robes");
    ElderClothing.SuitableForRole = EChar_TribalRole::Elder;
    AvailableClothing.Add(ElderClothing);
    
    // Crafter clothing - tool belt and apron
    FChar_TribalClothing CrafterClothing;
    CrafterClothing.ClothingName = TEXT("Crafter's Gear");
    CrafterClothing.SuitableForRole = EChar_TribalRole::Crafter;
    AvailableClothing.Add(CrafterClothing);
    
    // Warrior clothing - reinforced hide armor
    FChar_TribalClothing WarriorClothing;
    WarriorClothing.ClothingName = TEXT("Warrior's Armor");
    WarriorClothing.SuitableForRole = EChar_TribalRole::Warrior;
    AvailableClothing.Add(WarriorClothing);
}

void UChar_AppearanceManager::ApplyAppearanceData(const FChar_AppearanceData& AppearanceData)
{
    CurrentAppearance = AppearanceData;
    
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_AppearanceManager: No skeletal mesh component found"));
        return;
    }
    
    // Apply base mesh if specified
    if (AppearanceData.BaseMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = AppearanceData.BaseMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComp->SetSkeletalMesh(LoadedMesh);
        }
    }
    
    // Apply body scale
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector Scale = FVector(AppearanceData.BodyScale);
        Owner->SetActorScale3D(Scale);
    }
    
    // Update materials with appearance data
    UpdateMeshMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("UChar_AppearanceManager: Applied appearance for %s %s %s"), 
           *UEnum::GetValueAsString(AppearanceData.TribalRole),
           *UEnum::GetValueAsString(AppearanceData.Gender),
           *UEnum::GetValueAsString(AppearanceData.AgeGroup));
}

void UChar_AppearanceManager::RandomizeAppearance()
{
    FChar_AppearanceData RandomAppearance = GenerateRandomAppearance();
    ApplyAppearanceData(RandomAppearance);
}

void UChar_AppearanceManager::SetTribalRole(EChar_TribalRole NewRole)
{
    CurrentAppearance.TribalRole = NewRole;
    
    // Find preset for this role and apply some of its characteristics
    for (const FChar_AppearanceData& Preset : PresetAppearances)
    {
        if (Preset.TribalRole == NewRole)
        {
            CurrentAppearance.BodyScale = Preset.BodyScale;
            break;
        }
    }
    
    ApplyAppearanceData(CurrentAppearance);
}

void UChar_AppearanceManager::SetGender(EChar_Gender NewGender)
{
    CurrentAppearance.Gender = NewGender;
    ApplyAppearanceData(CurrentAppearance);
}

void UChar_AppearanceManager::SetAgeGroup(EChar_AgeGroup NewAgeGroup)
{
    CurrentAppearance.AgeGroup = NewAgeGroup;
    
    // Adjust appearance based on age
    if (NewAgeGroup == EChar_AgeGroup::Elder)
    {
        CurrentAppearance.HairColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray hair
        CurrentAppearance.BodyScale *= 0.95f; // Slightly smaller
    }
    else if (NewAgeGroup == EChar_AgeGroup::Young)
    {
        CurrentAppearance.BodyScale *= 0.98f; // Slightly smaller
    }
    
    ApplyAppearanceData(CurrentAppearance);
}

FChar_AppearanceData UChar_AppearanceManager::GenerateRandomAppearance(EChar_TribalRole Role)
{
    FChar_AppearanceData RandomAppearance;
    
    RandomAppearance.TribalRole = Role;
    RandomAppearance.Gender = FMath::RandBool() ? EChar_Gender::Male : EChar_Gender::Female;
    
    // Random age group with weighted distribution
    float AgeRoll = FMath::FRand();
    if (AgeRoll < 0.2f)
    {
        RandomAppearance.AgeGroup = EChar_AgeGroup::Young;
    }
    else if (AgeRoll < 0.8f)
    {
        RandomAppearance.AgeGroup = EChar_AgeGroup::Adult;
    }
    else
    {
        RandomAppearance.AgeGroup = EChar_AgeGroup::Elder;
    }
    
    RandomAppearance.SkinTone = GenerateRandomSkinTone();
    RandomAppearance.HairColor = GenerateRandomHairColor();
    RandomAppearance.BodyScale = GenerateRandomBodyScale();
    
    return RandomAppearance;
}

void UChar_AppearanceManager::ApplyTribalClothing(const FChar_TribalClothing& Clothing)
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Apply clothing mesh if available
    if (Clothing.ClothingMesh.IsValid())
    {
        USkeletalMesh* LoadedClothing = Clothing.ClothingMesh.LoadSynchronous();
        if (LoadedClothing)
        {
            // In a full implementation, this would add clothing as additional mesh components
            // For now, we log the clothing application
            UE_LOG(LogTemp, Log, TEXT("UChar_AppearanceManager: Applied clothing: %s"), *Clothing.ClothingName);
        }
    }
    
    // Apply clothing material
    if (Clothing.ClothingMaterial.IsValid())
    {
        UMaterialInterface* LoadedMaterial = Clothing.ClothingMaterial.LoadSynchronous();
        if (LoadedMaterial)
        {
            CurrentAppearance.ClothingMaterial = Clothing.ClothingMaterial;
            UpdateMeshMaterials();
        }
    }
}

TArray<FChar_TribalClothing> UChar_AppearanceManager::GetClothingForRole(EChar_TribalRole Role) const
{
    TArray<FChar_TribalClothing> RoleClothing;
    
    for (const FChar_TribalClothing& Clothing : AvailableClothing)
    {
        if (Clothing.SuitableForRole == Role)
        {
            RoleClothing.Add(Clothing);
        }
    }
    
    return RoleClothing;
}

void UChar_AppearanceManager::UpdateMeshMaterials()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Create dynamic material instances and apply appearance colors
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); MaterialIndex++)
    {
        UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Apply skin tone and hair color parameters
                DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), CurrentAppearance.SkinTone);
                DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), CurrentAppearance.HairColor);
                
                MeshComp->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
}

USkeletalMeshComponent* UChar_AppearanceManager::GetCharacterMesh() const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }
    
    return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

FLinearColor UChar_AppearanceManager::GenerateRandomSkinTone() const
{
    // Generate varied skin tones for tribal diversity
    float BaseR = FMath::FRandRange(0.4f, 0.9f);
    float BaseG = BaseR * FMath::FRandRange(0.6f, 0.8f);
    float BaseB = BaseG * FMath::FRandRange(0.4f, 0.7f);
    
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

FLinearColor UChar_AppearanceManager::GenerateRandomHairColor() const
{
    // Generate natural hair colors
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.05f, 0.02f, 0.01f, 1.0f), // Black
        FLinearColor(0.15f, 0.1f, 0.05f, 1.0f),  // Dark brown
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),    // Brown
        FLinearColor(0.4f, 0.25f, 0.1f, 1.0f),   // Light brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),    // Auburn
        FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)     // Gray (for elders)
    };
    
    int32 RandomIndex = FMath::RandRange(0, HairColors.Num() - 1);
    return HairColors[RandomIndex];
}

float UChar_AppearanceManager::GenerateRandomBodyScale() const
{
    return FMath::FRandRange(1.0f - BodyScaleVariation, 1.0f + BodyScaleVariation);
}