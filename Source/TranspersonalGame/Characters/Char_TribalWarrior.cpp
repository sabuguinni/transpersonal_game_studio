#include "Char_TribalWarrior.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AChar_TribalWarrior::AChar_TribalWarrior()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default values
    WarriorLevel = 1;
    TribeAffiliation = TEXT("Cretaceous Survivors");
    SurvivalExperience = 0.5f;
    HairStyle = EChar_HairStyle::Long_Braided;
    BodyBuild = EChar_BodyBuild::Muscular;
    SkinTone = EChar_SkinTone::Tanned;
    
    // Create tribal equipment components
    TribalClothingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TribalClothing"));
    BoneJewelryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneJewelry"));
    StoneSpearMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneSpear"));
    StoneAxeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneAxe"));
    
    // Attach equipment to mesh
    if (GetMesh())
    {
        TribalClothingMesh->SetupAttachment(GetMesh());
        BoneJewelryMesh->SetupAttachment(GetMesh());
        StoneSpearMesh->SetupAttachment(GetMesh());
        StoneAxeMesh->SetupAttachment(GetMesh());
    }
    
    // Configure collision
    TribalClothingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BoneJewelryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StoneSpearMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StoneAxeMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Initialize tribal equipment
    InitializeTribalEquipment();
}

void AChar_TribalWarrior::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure MetaHuman mesh for tribal appearance
    ConfigureMetaHumanMesh();
    
    // Apply initial tribal warrior preset
    ApplyTribalWarriorPreset();
    
    // Setup equipment attachment points
    SetupEquipmentAttachments();
    
    // Update materials based on character traits
    UpdateCharacterMaterials();
}

void AChar_TribalWarrior::InitializeTribalEquipment()
{
    // Try to load tribal equipment meshes
    // Note: These would be created by 3D artists or generated via Meshy
    
    // Tribal clothing (leather wraps and loincloths)
    UStaticMesh* ClothingMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Tribal/Meshes/SM_TribalClothing"));
    if (ClothingMesh)
    {
        TribalClothingMesh->SetStaticMesh(ClothingMesh);
    }
    
    // Bone jewelry (necklaces, bracelets, ornaments)
    UStaticMesh* JewelryMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Tribal/Meshes/SM_BoneJewelry"));
    if (JewelryMesh)
    {
        BoneJewelryMesh->SetStaticMesh(JewelryMesh);
    }
    
    // Stone spear weapon
    UStaticMesh* SpearMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Weapons/Primitive/Meshes/SM_StoneSpear"));
    if (SpearMesh)
    {
        StoneSpearMesh->SetStaticMesh(SpearMesh);
    }
    
    // Stone axe tool
    UStaticMesh* AxeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Weapons/Primitive/Meshes/SM_StoneAxe"));
    if (AxeMesh)
    {
        StoneAxeMesh->SetStaticMesh(AxeMesh);
    }
}

void AChar_TribalWarrior::ConfigureMetaHumanMesh()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
        return;
    
    // Try to load MetaHuman skeletal mesh
    USkeletalMesh* MetaHumanMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/MetaHumans/Common/Male/Medium/NormalWeight/Body/m_med_nrw_body"));
    
    // Fallback to UE5 mannequin if MetaHuman not available
    if (!MetaHumanMesh)
    {
        MetaHumanMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    }
    
    // Ultimate fallback to engine mannequin
    if (!MetaHumanMesh)
    {
        MetaHumanMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Manny"));
    }
    
    if (MetaHumanMesh)
    {
        MeshComp->SetSkeletalMesh(MetaHumanMesh);
        
        // Position mesh for third-person character
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        
        UE_LOG(LogTemp, Warning, TEXT("Configured MetaHuman mesh for tribal warrior"));
    }
}

void AChar_TribalWarrior::SetupEquipmentAttachments()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
        return;
    
    // Attach spear to right hand
    StoneSpearMesh->AttachToComponent(MeshComp, 
        FAttachmentTransformRules::SnapToTargetIncludingScale, 
        TEXT("hand_r"));
    
    // Attach axe to belt/hip
    StoneAxeMesh->AttachToComponent(MeshComp, 
        FAttachmentTransformRules::SnapToTargetIncludingScale, 
        TEXT("pelvis"));
    
    // Position axe at hip
    StoneAxeMesh->SetRelativeLocation(FVector(0.0f, 15.0f, -10.0f));
    StoneAxeMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void AChar_TribalWarrior::UpdateCharacterMaterials()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
        return;
    
    // Apply weathered skin material based on survival experience
    if (WeatheredSkinMaterial)
    {
        MeshComp->SetMaterial(0, WeatheredSkinMaterial);
    }
    
    // Apply tribal equipment materials
    if (TribalLeatherMaterial)
    {
        TribalClothingMesh->SetMaterial(0, TribalLeatherMaterial);
    }
    
    if (BoneMaterial)
    {
        BoneJewelryMesh->SetMaterial(0, BoneMaterial);
    }
    
    if (StoneMaterial)
    {
        StoneSpearMesh->SetMaterial(0, StoneMaterial);
        StoneAxeMesh->SetMaterial(0, StoneMaterial);
    }
}

void AChar_TribalWarrior::ApplyTribalWarriorPreset()
{
    // Configure for experienced warrior appearance
    WarriorLevel = FMath::RandRange(3, 7);
    SurvivalExperience = FMath::RandRange(0.6f, 0.9f);
    
    // Randomize tribal appearance elements
    int32 HairVariant = FMath::RandRange(0, 2);
    switch (HairVariant)
    {
        case 0: HairStyle = EChar_HairStyle::Long_Braided; break;
        case 1: HairStyle = EChar_HairStyle::Short_Wild; break;
        case 2: HairStyle = EChar_HairStyle::Mohawk; break;
    }
    
    // Set muscular build for warrior
    BodyBuild = EChar_BodyBuild::Muscular;
    
    // Randomize skin tone for tribal diversity
    int32 SkinVariant = FMath::RandRange(0, 2);
    switch (SkinVariant)
    {
        case 0: SkinTone = EChar_SkinTone::Tanned; break;
        case 1: SkinTone = EChar_SkinTone::Dark; break;
        case 2: SkinTone = EChar_SkinTone::Medium; break;
    }
    
    // Update materials to reflect new traits
    UpdateCharacterMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("Applied tribal warrior preset: Level %d, Experience %.2f"), WarriorLevel, SurvivalExperience);
}

void AChar_TribalWarrior::RandomizeTribalAppearance()
{
    // Randomize all appearance traits
    WarriorLevel = FMath::RandRange(1, 10);
    SurvivalExperience = FMath::RandRange(0.1f, 1.0f);
    
    // Random hair style
    HairStyle = static_cast<EChar_HairStyle>(FMath::RandRange(0, static_cast<int32>(EChar_HairStyle::MAX) - 1));
    
    // Random body build
    BodyBuild = static_cast<EChar_BodyBuild>(FMath::RandRange(0, static_cast<int32>(EChar_BodyBuild::MAX) - 1));
    
    // Random skin tone
    SkinTone = static_cast<EChar_SkinTone>(FMath::RandRange(0, static_cast<int32>(EChar_SkinTone::MAX) - 1));
    
    // Generate random tribe name
    TArray<FString> TribeNames = {
        TEXT("Cretaceous Survivors"),
        TEXT("Stone Spear Clan"),
        TEXT("Bone Hunters"),
        TEXT("Fire Keepers"),
        TEXT("River Walkers"),
        TEXT("Mountain Dwellers")
    };
    TribeAffiliation = TribeNames[FMath::RandRange(0, TribeNames.Num() - 1)];
    
    UpdateCharacterMaterials();
}

void AChar_TribalWarrior::SetWarriorLevel(int32 NewLevel)
{
    WarriorLevel = FMath::Clamp(NewLevel, 1, 10);
    
    // Higher level warriors have more survival experience
    SurvivalExperience = FMath::Clamp(WarriorLevel * 0.1f, 0.1f, 1.0f);
    
    // Update appearance to reflect experience
    UpdateCharacterMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("Warrior level set to %d, experience %.2f"), WarriorLevel, SurvivalExperience);
}

void AChar_TribalWarrior::ApplySurvivalWeathering(float WeatheringAmount)
{
    // Increase survival experience (weathering)
    SurvivalExperience = FMath::Clamp(SurvivalExperience + WeatheringAmount, 0.0f, 1.0f);
    
    // Update materials to show increased weathering
    UpdateCharacterMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("Applied weathering %.2f, new experience %.2f"), WeatheringAmount, SurvivalExperience);
}

void AChar_TribalWarrior::SetWeaponVisibility(bool bShowSpear, bool bShowAxe)
{
    StoneSpearMesh->SetVisibility(bShowSpear);
    StoneAxeMesh->SetVisibility(bShowAxe);
    
    UE_LOG(LogTemp, Warning, TEXT("Weapon visibility: Spear %s, Axe %s"), 
        bShowSpear ? TEXT("ON") : TEXT("OFF"),
        bShowAxe ? TEXT("ON") : TEXT("OFF"));
}