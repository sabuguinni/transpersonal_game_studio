#include "Char_PrimitiveHuman.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

AChar_PrimitiveHuman::AChar_PrimitiveHuman()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize character mesh (use inherited mesh component)
    CharacterMesh = GetMesh();
    if (CharacterMesh)
    {
        CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    // Create clothing component
    HideClothing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HideClothing"));
    HideClothing->SetupAttachment(CharacterMesh, TEXT("pelvis"));

    // Create jewelry component
    BoneJewelry = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneJewelry"));
    BoneJewelry->SetupAttachment(CharacterMesh, TEXT("neck_01"));

    // Create stone axe component
    StoneAxe = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneAxe"));
    StoneAxe->SetupAttachment(CharacterMesh, TEXT("hand_r"));

    // Create bone spear component
    BoneSpear = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneSpear"));
    BoneSpear->SetupAttachment(CharacterMesh, TEXT("hand_l"));

    // Initialize default values
    SkinVariation = 0;
    ClothingVariation = 0;
    bHasJewelry = true;
    bHasWeapons = true;
    BodyScale = 1.0f;
    MuscleDefinition = 0.7f;
    WeatheringLevel = 0.8f;
}

void AChar_PrimitiveHuman::BeginPlay()
{
    Super::BeginPlay();
    
    SetupCharacterMesh();
    SetupClothingAndAccessories();
    ApplyMaterialVariations();
    ConfigurePhysicalTraits();
}

void AChar_PrimitiveHuman::SetupCharacterMesh()
{
    if (!CharacterMesh)
        return;

    // Try to load a base human mesh (MetaHuman or UE5 Mannequin)
    USkeletalMesh* BaseMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
    if (!BaseMesh)
    {
        BaseMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Manny"));
    }
    
    if (BaseMesh)
    {
        CharacterMesh->SetSkeletalMesh(BaseMesh);
        UE_LOG(LogTemp, Log, TEXT("Primitive Human: Base mesh applied"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Primitive Human: No base mesh found"));
    }
}

void AChar_PrimitiveHuman::SetupClothingAndAccessories()
{
    // Hide clothing initially if no weapons/jewelry
    if (HideClothing)
    {
        HideClothing->SetVisibility(true);
        // TODO: Load hide clothing mesh when available
        // HideClothing->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Clothing/SM_HideClothing")));
    }

    if (BoneJewelry)
    {
        BoneJewelry->SetVisibility(bHasJewelry);
        // TODO: Load bone jewelry mesh when available
        // BoneJewelry->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Accessories/SM_BoneNecklace")));
    }

    if (StoneAxe)
    {
        StoneAxe->SetVisibility(bHasWeapons);
        // TODO: Load stone axe mesh when available
        // StoneAxe->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Tools/SM_StoneAxe")));
    }

    if (BoneSpear)
    {
        BoneSpear->SetVisibility(bHasWeapons);
        // TODO: Load bone spear mesh when available
        // BoneSpear->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Tools/SM_BoneSpear")));
    }
}

void AChar_PrimitiveHuman::ApplyMaterialVariations()
{
    if (!CharacterMesh)
        return;

    // Apply skin material variation if available
    if (SkinMaterials.IsValidIndex(SkinVariation))
    {
        CharacterMesh->SetMaterial(0, SkinMaterials[SkinVariation]);
    }

    // Apply clothing material variation if available
    if (HideClothing && ClothingMaterials.IsValidIndex(ClothingVariation))
    {
        HideClothing->SetMaterial(0, ClothingMaterials[ClothingVariation]);
    }
}

void AChar_PrimitiveHuman::ConfigurePhysicalTraits()
{
    if (CharacterMesh)
    {
        // Apply body scale
        FVector CurrentScale = CharacterMesh->GetRelativeScale3D();
        CharacterMesh->SetRelativeScale3D(CurrentScale * BodyScale);
        
        // TODO: Apply muscle definition and weathering through material parameters
        // This would typically be done through dynamic material instances
    }
}

void AChar_PrimitiveHuman::SetSkinVariation(int32 NewVariation)
{
    if (SkinMaterials.IsValidIndex(NewVariation))
    {
        SkinVariation = NewVariation;
        ApplyMaterialVariations();
        UE_LOG(LogTemp, Log, TEXT("Primitive Human: Skin variation changed to %d"), NewVariation);
    }
}

void AChar_PrimitiveHuman::SetClothingVariation(int32 NewVariation)
{
    if (ClothingMaterials.IsValidIndex(NewVariation))
    {
        ClothingVariation = NewVariation;
        ApplyMaterialVariations();
        UE_LOG(LogTemp, Log, TEXT("Primitive Human: Clothing variation changed to %d"), NewVariation);
    }
}

void AChar_PrimitiveHuman::ToggleJewelry(bool bEnabled)
{
    bHasJewelry = bEnabled;
    if (BoneJewelry)
    {
        BoneJewelry->SetVisibility(bHasJewelry);
    }
    UE_LOG(LogTemp, Log, TEXT("Primitive Human: Jewelry %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AChar_PrimitiveHuman::ToggleWeapons(bool bEnabled)
{
    bHasWeapons = bEnabled;
    if (StoneAxe)
    {
        StoneAxe->SetVisibility(bHasWeapons);
    }
    if (BoneSpear)
    {
        BoneSpear->SetVisibility(bHasWeapons);
    }
    UE_LOG(LogTemp, Log, TEXT("Primitive Human: Weapons %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AChar_PrimitiveHuman::RandomizeAppearance()
{
    // Randomize skin variation
    if (SkinMaterials.Num() > 0)
    {
        SetSkinVariation(FMath::RandRange(0, SkinMaterials.Num() - 1));
    }

    // Randomize clothing variation
    if (ClothingMaterials.Num() > 0)
    {
        SetClothingVariation(FMath::RandRange(0, ClothingMaterials.Num() - 1));
    }

    // Randomize accessories
    ToggleJewelry(FMath::RandBool());
    ToggleWeapons(FMath::RandBool());

    // Randomize physical traits
    BodyScale = FMath::RandRange(0.9f, 1.1f);
    MuscleDefinition = FMath::RandRange(0.5f, 1.0f);
    WeatheringLevel = FMath::RandRange(0.6f, 1.0f);

    ConfigurePhysicalTraits();
    
    UE_LOG(LogTemp, Log, TEXT("Primitive Human: Appearance randomized"));
}

FString AChar_PrimitiveHuman::GetCharacterDescription() const
{
    FString Description = TEXT("Primitive Cretaceous Human - ");
    
    Description += FString::Printf(TEXT("Skin Variation: %d, "), SkinVariation);
    Description += FString::Printf(TEXT("Clothing: %d, "), ClothingVariation);
    Description += FString::Printf(TEXT("Jewelry: %s, "), bHasJewelry ? TEXT("Yes") : TEXT("No"));
    Description += FString::Printf(TEXT("Weapons: %s, "), bHasWeapons ? TEXT("Yes") : TEXT("No"));
    Description += FString::Printf(TEXT("Scale: %.2f"), BodyScale);
    
    return Description;
}