#include "Char_PrimitiveHumanCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AChar_PrimitiveHumanCharacter::AChar_PrimitiveHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize character customization defaults
    CharacterCustomization.SkinTone = EChar_SkinTone::Medium;
    CharacterCustomization.ClothingStyle = EChar_ClothingStyle::BasicHide;
    CharacterCustomization.BodyMassIndex = 1.0f;
    CharacterCustomization.Height = 1.0f;
    CharacterCustomization.bHasTribalMarkings = false;
    CharacterCustomization.bHasBoneJewelry = true;
    CharacterCustomization.bHasScars = false;

    // Initialize character stats
    TribalRank = 1.0f;
    SurvivalExperience = 0.0f;
    CraftingSkill = 1.0f;
    HuntingSkill = 1.0f;

    // Create weapon components
    StoneAxeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneAxeComponent"));
    StoneAxeComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));
    StoneAxeComponent->SetVisibility(false);

    SpearComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearComponent"));
    SpearComponent->SetupAttachment(GetMesh(), TEXT("hand_l"));
    SpearComponent->SetVisibility(false);

    // Create clothing components
    HideClothingComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HideClothingComponent"));
    HideClothingComponent->SetupAttachment(GetMesh());

    BoneJewelryComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneJewelryComponent"));
    BoneJewelryComponent->SetupAttachment(GetMesh());
    BoneJewelryComponent->SetVisibility(CharacterCustomization.bHasBoneJewelry);

    // Set default mesh to UE5 Mannequin if available
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Manny"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

void AChar_PrimitiveHumanCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial customization
    ApplyCharacterCustomization(CharacterCustomization);
}

void AChar_PrimitiveHumanCharacter::ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization)
{
    CharacterCustomization = NewCustomization;
    
    UpdateMeshMaterials();
    UpdateClothingMesh();
    UpdateJewelryVisibility();
    
    // Apply body scaling
    FVector NewScale = FVector(
        CharacterCustomization.BodyMassIndex,
        CharacterCustomization.BodyMassIndex,
        CharacterCustomization.Height
    );
    GetMesh()->SetRelativeScale3D(NewScale);
}

void AChar_PrimitiveHumanCharacter::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CharacterCustomization.SkinTone = NewSkinTone;
    UpdateMeshMaterials();
}

void AChar_PrimitiveHumanCharacter::SetClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    CharacterCustomization.ClothingStyle = NewClothingStyle;
    UpdateClothingMesh();
}

void AChar_PrimitiveHumanCharacter::ToggleTribalMarkings(bool bEnabled)
{
    CharacterCustomization.bHasTribalMarkings = bEnabled;
    UpdateMeshMaterials();
}

void AChar_PrimitiveHumanCharacter::ToggleBoneJewelry(bool bEnabled)
{
    CharacterCustomization.bHasBoneJewelry = bEnabled;
    UpdateJewelryVisibility();
}

void AChar_PrimitiveHumanCharacter::ToggleScars(bool bEnabled)
{
    CharacterCustomization.bHasScars = bEnabled;
    UpdateMeshMaterials();
}

void AChar_PrimitiveHumanCharacter::EquipStoneAxe(bool bEquipped)
{
    StoneAxeComponent->SetVisibility(bEquipped);
}

void AChar_PrimitiveHumanCharacter::EquipSpear(bool bEquipped)
{
    SpearComponent->SetVisibility(bEquipped);
}

FString AChar_PrimitiveHumanCharacter::GetCharacterDescription() const
{
    FString Description = TEXT("Primitive Human - ");
    
    // Add skin tone description
    switch (CharacterCustomization.SkinTone)
    {
        case EChar_SkinTone::Light:
            Description += TEXT("Light skin, ");
            break;
        case EChar_SkinTone::Medium:
            Description += TEXT("Medium skin, ");
            break;
        case EChar_SkinTone::Dark:
            Description += TEXT("Dark skin, ");
            break;
        case EChar_SkinTone::Weathered:
            Description += TEXT("Weathered skin, ");
            break;
    }
    
    // Add clothing description
    switch (CharacterCustomization.ClothingStyle)
    {
        case EChar_ClothingStyle::BasicHide:
            Description += TEXT("basic hide clothing");
            break;
        case EChar_ClothingStyle::FurTrim:
            Description += TEXT("fur-trimmed clothing");
            break;
        case EChar_ClothingStyle::BoneArmor:
            Description += TEXT("bone armor");
            break;
        case EChar_ClothingStyle::Tribal:
            Description += TEXT("tribal clothing");
            break;
    }
    
    // Add features
    if (CharacterCustomization.bHasTribalMarkings)
    {
        Description += TEXT(", tribal markings");
    }
    
    if (CharacterCustomization.bHasBoneJewelry)
    {
        Description += TEXT(", bone jewelry");
    }
    
    if (CharacterCustomization.bHasScars)
    {
        Description += TEXT(", battle scars");
    }
    
    return Description;
}

float AChar_PrimitiveHumanCharacter::GetOverallSkillLevel() const
{
    return (CraftingSkill + HuntingSkill + SurvivalExperience) / 3.0f;
}

void AChar_PrimitiveHumanCharacter::UpdateMeshMaterials()
{
    // This would typically load and apply different materials based on skin tone,
    // tribal markings, and scars. For now, we'll log the changes.
    if (GEngine)
    {
        FString SkinToneStr;
        switch (CharacterCustomization.SkinTone)
        {
            case EChar_SkinTone::Light: SkinToneStr = TEXT("Light"); break;
            case EChar_SkinTone::Medium: SkinToneStr = TEXT("Medium"); break;
            case EChar_SkinTone::Dark: SkinToneStr = TEXT("Dark"); break;
            case EChar_SkinTone::Weathered: SkinToneStr = TEXT("Weathered"); break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Updated skin tone to: %s"), *SkinToneStr));
    }
}

void AChar_PrimitiveHumanCharacter::UpdateClothingMesh()
{
    // This would typically swap out clothing meshes based on the clothing style
    if (HideClothingComponent && GEngine)
    {
        FString ClothingStr;
        switch (CharacterCustomization.ClothingStyle)
        {
            case EChar_ClothingStyle::BasicHide: ClothingStr = TEXT("Basic Hide"); break;
            case EChar_ClothingStyle::FurTrim: ClothingStr = TEXT("Fur Trim"); break;
            case EChar_ClothingStyle::BoneArmor: ClothingStr = TEXT("Bone Armor"); break;
            case EChar_ClothingStyle::Tribal: ClothingStr = TEXT("Tribal"); break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
            FString::Printf(TEXT("Updated clothing to: %s"), *ClothingStr));
    }
}

void AChar_PrimitiveHumanCharacter::UpdateJewelryVisibility()
{
    if (BoneJewelryComponent)
    {
        BoneJewelryComponent->SetVisibility(CharacterCustomization.bHasBoneJewelry);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Bone jewelry: %s"), 
                CharacterCustomization.bHasBoneJewelry ? TEXT("Visible") : TEXT("Hidden")));
        }
    }
}