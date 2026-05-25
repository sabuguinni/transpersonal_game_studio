#include "TribalCharacterComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"

UTribalCharacterComponent::UTribalCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    Appearance.SkinTone = EChar_SkinTone::Medium;
    Appearance.BodyBuild = EChar_BodyBuild::Muscular;
    Appearance.SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    Appearance.bHasTribalScars = true;
    Appearance.bHasBoneJewelry = true;
    Appearance.MuscleMass = 1.0f;
    Appearance.Age = 25.0f;
    
    // Initialize default stats
    Strength = 75.0f;
    Agility = 60.0f;
    Wisdom = 45.0f;
    SurvivalSkill = 80.0f;
    
    // Initialize equipment
    EquippedTools.Add(TEXT("Stone Spear"));
    EquippedTools.Add(TEXT("Bone Knife"));
    PrimaryWeapon = TEXT("Stone Spear");
    ClothingStyle = TEXT("Hide Wrap");
}

void UTribalCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateEquipmentForRole();
    UpdateCharacterStats();
}

void UTribalCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTribalCharacterComponent::RandomizeAppearance()
{
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    Appearance.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize body build
    int32 BodyBuildIndex = FMath::RandRange(0, 4);
    Appearance.BodyBuild = static_cast<EChar_BodyBuild>(BodyBuildIndex);
    
    // Randomize skin color based on tone
    switch(Appearance.SkinTone)
    {
        case EChar_SkinTone::Light:
            Appearance.SkinColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
            break;
        case EChar_SkinTone::Medium:
            Appearance.SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
            break;
        case EChar_SkinTone::Dark:
            Appearance.SkinColor = FLinearColor(0.5f, 0.3f, 0.2f, 1.0f);
            break;
        case EChar_SkinTone::Bronze:
            Appearance.SkinColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
            break;
        case EChar_SkinTone::Weathered:
            Appearance.SkinColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
            break;
    }
    
    // Randomize features
    Appearance.bHasTribalScars = FMath::RandBool();
    Appearance.bHasBoneJewelry = FMath::RandBool();
    Appearance.bHasFeathers = FMath::RandRange(0, 100) < 30; // 30% chance
    Appearance.MuscleMass = FMath::FRandRange(0.7f, 1.3f);
    Appearance.Age = FMath::FRandRange(18.0f, 65.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterComponent: Randomized appearance"));
}

void UTribalCharacterComponent::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    UpdateEquipmentForRole();
    UpdateCharacterStats();
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterComponent: Set role to %s"), *GetRoleDescription());
}

void UTribalCharacterComponent::ApplyAppearanceToMesh(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("TribalCharacterComponent: MeshComponent is null"));
        return;
    }
    
    ApplySkinTone(MeshComponent);
    ApplyBodyModifications(MeshComponent);
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterComponent: Applied appearance to mesh"));
}

void UTribalCharacterComponent::UpdateCharacterStats()
{
    // Adjust stats based on role
    switch(TribalRole)
    {
        case EChar_TribalRole::Hunter:
            Strength = FMath::FRandRange(70.0f, 90.0f);
            Agility = FMath::FRandRange(75.0f, 95.0f);
            Wisdom = FMath::FRandRange(40.0f, 60.0f);
            SurvivalSkill = FMath::FRandRange(80.0f, 100.0f);
            break;
            
        case EChar_TribalRole::Gatherer:
            Strength = FMath::FRandRange(50.0f, 70.0f);
            Agility = FMath::FRandRange(60.0f, 80.0f);
            Wisdom = FMath::FRandRange(60.0f, 80.0f);
            SurvivalSkill = FMath::FRandRange(70.0f, 90.0f);
            break;
            
        case EChar_TribalRole::Shaman:
            Strength = FMath::FRandRange(40.0f, 60.0f);
            Agility = FMath::FRandRange(45.0f, 65.0f);
            Wisdom = FMath::FRandRange(85.0f, 100.0f);
            SurvivalSkill = FMath::FRandRange(75.0f, 95.0f);
            break;
            
        case EChar_TribalRole::Warrior:
            Strength = FMath::FRandRange(80.0f, 100.0f);
            Agility = FMath::FRandRange(70.0f, 90.0f);
            Wisdom = FMath::FRandRange(35.0f, 55.0f);
            SurvivalSkill = FMath::FRandRange(65.0f, 85.0f);
            break;
            
        case EChar_TribalRole::Elder:
            Strength = FMath::FRandRange(30.0f, 50.0f);
            Agility = FMath::FRandRange(25.0f, 45.0f);
            Wisdom = FMath::FRandRange(90.0f, 100.0f);
            SurvivalSkill = FMath::FRandRange(85.0f, 100.0f);
            Appearance.Age = FMath::FRandRange(55.0f, 80.0f);
            break;
            
        case EChar_TribalRole::Child:
            Strength = FMath::FRandRange(15.0f, 35.0f);
            Agility = FMath::FRandRange(50.0f, 70.0f);
            Wisdom = FMath::FRandRange(20.0f, 40.0f);
            SurvivalSkill = FMath::FRandRange(25.0f, 45.0f);
            Appearance.Age = FMath::FRandRange(8.0f, 16.0f);
            break;
    }
    
    // Adjust body build based on role and age
    if (Appearance.Age > 55.0f)
    {
        Appearance.BodyBuild = EChar_BodyBuild::Elderly;
    }
    else if (Appearance.Age < 16.0f)
    {
        Appearance.BodyBuild = EChar_BodyBuild::Child;
    }
}

FString UTribalCharacterComponent::GetRoleDescription() const
{
    switch(TribalRole)
    {
        case EChar_TribalRole::Hunter:
            return TEXT("Hunter - Skilled in tracking and hunting prehistoric beasts");
        case EChar_TribalRole::Gatherer:
            return TEXT("Gatherer - Expert in finding food, herbs, and materials");
        case EChar_TribalRole::Shaman:
            return TEXT("Shaman - Keeper of tribal knowledge and healing arts");
        case EChar_TribalRole::Warrior:
            return TEXT("Warrior - Protector of the tribe, skilled in combat");
        case EChar_TribalRole::Elder:
            return TEXT("Elder - Wise leader with decades of survival experience");
        case EChar_TribalRole::Child:
            return TEXT("Child - Young tribal member learning survival skills");
        default:
            return TEXT("Unknown Role");
    }
}

TArray<FString> UTribalCharacterComponent::GetAppropriateTools() const
{
    TArray<FString> Tools;
    
    switch(TribalRole)
    {
        case EChar_TribalRole::Hunter:
            Tools.Add(TEXT("Stone Spear"));
            Tools.Add(TEXT("Bone Knife"));
            Tools.Add(TEXT("Wooden Bow"));
            Tools.Add(TEXT("Stone Arrows"));
            break;
            
        case EChar_TribalRole::Gatherer:
            Tools.Add(TEXT("Woven Basket"));
            Tools.Add(TEXT("Digging Stick"));
            Tools.Add(TEXT("Stone Knife"));
            Tools.Add(TEXT("Carrying Pouch"));
            break;
            
        case EChar_TribalRole::Shaman:
            Tools.Add(TEXT("Bone Staff"));
            Tools.Add(TEXT("Medicine Pouch"));
            Tools.Add(TEXT("Ritual Stones"));
            Tools.Add(TEXT("Herb Bundle"));
            break;
            
        case EChar_TribalRole::Warrior:
            Tools.Add(TEXT("Stone Axe"));
            Tools.Add(TEXT("Wooden Club"));
            Tools.Add(TEXT("Bone Spear"));
            Tools.Add(TEXT("Hide Shield"));
            break;
            
        case EChar_TribalRole::Elder:
            Tools.Add(TEXT("Walking Staff"));
            Tools.Add(TEXT("Memory Stones"));
            Tools.Add(TEXT("Wisdom Pouch"));
            break;
            
        case EChar_TribalRole::Child:
            Tools.Add(TEXT("Small Stick"));
            Tools.Add(TEXT("Learning Stones"));
            Tools.Add(TEXT("Practice Spear"));
            break;
    }
    
    return Tools;
}

void UTribalCharacterComponent::ApplySkinTone(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
        return;
        
    // Create dynamic material instance for skin
    UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
    if (BaseMaterial)
    {
        SkinMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (SkinMaterial)
        {
            SkinMaterial->SetVectorParameterValue(TEXT("SkinColor"), Appearance.SkinColor);
            SkinMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), Appearance.bHasTribalScars ? 1.0f : 0.0f);
            MeshComponent->SetMaterial(0, SkinMaterial);
        }
    }
}

void UTribalCharacterComponent::ApplyBodyModifications(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
        return;
        
    // Apply muscle mass scaling
    FVector Scale = MeshComponent->GetComponentScale();
    Scale.X *= Appearance.MuscleMass;
    Scale.Y *= Appearance.MuscleMass;
    MeshComponent->SetWorldScale3D(Scale);
    
    // Apply age-based modifications
    if (Appearance.Age > 55.0f)
    {
        // Elder modifications - slightly hunched posture
        FRotator CurrentRotation = MeshComponent->GetComponentRotation();
        CurrentRotation.Pitch += 5.0f; // Slight forward lean
        MeshComponent->SetWorldRotation(CurrentRotation);
    }
}

void UTribalCharacterComponent::UpdateEquipmentForRole()
{
    EquippedTools = GetAppropriateTools();
    
    if (EquippedTools.Num() > 0)
    {
        PrimaryWeapon = EquippedTools[0];
    }
    
    // Set appropriate clothing style
    switch(TribalRole)
    {
        case EChar_TribalRole::Hunter:
        case EChar_TribalRole::Warrior:
            ClothingStyle = TEXT("Hunter Hide Wrap");
            break;
        case EChar_TribalRole::Gatherer:
            ClothingStyle = TEXT("Simple Hide Dress");
            break;
        case EChar_TribalRole::Shaman:
            ClothingStyle = TEXT("Decorated Robe");
            break;
        case EChar_TribalRole::Elder:
            ClothingStyle = TEXT("Elder Ceremonial Wrap");
            break;
        case EChar_TribalRole::Child:
            ClothingStyle = TEXT("Simple Child Wrap");
            break;
    }
}