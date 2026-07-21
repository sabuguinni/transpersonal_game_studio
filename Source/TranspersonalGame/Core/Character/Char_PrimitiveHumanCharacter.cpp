#include "Char_PrimitiveHumanCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AChar_PrimitiveHumanCharacter::AChar_PrimitiveHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set up character mesh
    CharacterMesh = GetMesh();
    if (CharacterMesh)
    {
        CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        
        // Try to load default mannequin mesh
        static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
        if (MeshAsset.Succeeded())
        {
            CharacterMesh->SetSkeletalMesh(MeshAsset.Object);
        }
    }

    // Create bone tool belt component
    BoneToolBelt = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneToolBelt"));
    BoneToolBelt->SetupAttachment(CharacterMesh, TEXT("pelvis"));
    BoneToolBelt->SetRelativeLocation(FVector(0.0f, 15.0f, 0.0f));

    // Create spear weapon component
    SpearWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearWeapon"));
    SpearWeapon->SetupAttachment(CharacterMesh, TEXT("hand_r"));
    SpearWeapon->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
    SpearWeapon->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));

    // Initialize customization settings
    CustomizationSettings.SkinTone = EChar_SkinTone::Medium;
    CustomizationSettings.ClothingStyle = EChar_ClothingStyle::BasicLeather;
    CustomizationSettings.FacePaintColor = FLinearColor::Red;
    CustomizationSettings.bHasFacePaint = true;
    CustomizationSettings.MuscleMass = 0.7f;
    CustomizationSettings.WeatheringLevel = 0.5f;

    // Initialize character stats
    SurvivalExperience = 50.0f;
    TribalRank = 1.0f;
    CraftingSkill = 25.0f;
    HuntingSkill = 30.0f;

    // Set default collision
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
}

void AChar_PrimitiveHumanCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultEquipment();
    ApplyCustomization(CustomizationSettings);
}

void AChar_PrimitiveHumanCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update weathering over time
    if (GetWorld() && GetWorld()->GetTimeSeconds() - LastCustomizationTime > 60.0f)
    {
        CustomizationSettings.WeatheringLevel = FMath::Clamp(
            CustomizationSettings.WeatheringLevel + (DeltaTime * 0.001f), 
            0.0f, 1.0f
        );
        LastCustomizationTime = GetWorld()->GetTimeSeconds();
    }
}

void AChar_PrimitiveHumanCharacter::ApplyCustomization(const FChar_CharacterCustomization& NewCustomization)
{
    CustomizationSettings = NewCustomization;
    
    UpdateCharacterMaterials();
    UpdateClothing();
    UpdateFacePaint();
    
    bIsCustomizationApplied = true;
    LastCustomizationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Applied customization to character: %s"), *GetCharacterDescription());
}

void AChar_PrimitiveHumanCharacter::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CustomizationSettings.SkinTone = NewSkinTone;
    UpdateCharacterMaterials();
}

void AChar_PrimitiveHumanCharacter::SetClothingStyle(EChar_ClothingStyle NewStyle)
{
    CustomizationSettings.ClothingStyle = NewStyle;
    UpdateClothing();
}

void AChar_PrimitiveHumanCharacter::SetFacePaint(bool bEnabled, FLinearColor Color)
{
    CustomizationSettings.bHasFacePaint = bEnabled;
    CustomizationSettings.FacePaintColor = Color;
    UpdateFacePaint();
}

void AChar_PrimitiveHumanCharacter::AddSurvivalExperience(float Amount)
{
    SurvivalExperience = FMath::Clamp(SurvivalExperience + Amount, 0.0f, 100.0f);
    
    // Increase weathering with experience
    CustomizationSettings.WeatheringLevel = FMath::Clamp(
        SurvivalExperience * 0.01f, 
        0.0f, 1.0f
    );
    
    UpdateCharacterMaterials();
}

void AChar_PrimitiveHumanCharacter::SetTribalRank(float NewRank)
{
    TribalRank = FMath::Clamp(NewRank, 1.0f, 10.0f);
    
    // Higher rank = better equipment
    if (TribalRank > 5.0f)
    {
        CustomizationSettings.ClothingStyle = EChar_ClothingStyle::BoneArmor;
        UpdateClothing();
    }
}

void AChar_PrimitiveHumanCharacter::EquipBoneTools()
{
    if (BoneToolBelt)
    {
        BoneToolBelt->SetVisibility(true);
        UE_LOG(LogTemp, Log, TEXT("Equipped bone tools on character"));
    }
}

void AChar_PrimitiveHumanCharacter::EquipSpear()
{
    if (SpearWeapon)
    {
        SpearWeapon->SetVisibility(true);
        UE_LOG(LogTemp, Log, TEXT("Equipped spear on character"));
    }
}

FString AChar_PrimitiveHumanCharacter::GetCharacterDescription() const
{
    FString SkinToneStr;
    switch (CustomizationSettings.SkinTone)
    {
        case EChar_SkinTone::Light: SkinToneStr = TEXT("Light"); break;
        case EChar_SkinTone::Medium: SkinToneStr = TEXT("Medium"); break;
        case EChar_SkinTone::Dark: SkinToneStr = TEXT("Dark"); break;
        case EChar_SkinTone::Weathered: SkinToneStr = TEXT("Weathered"); break;
        default: SkinToneStr = TEXT("Unknown"); break;
    }

    FString ClothingStr;
    switch (CustomizationSettings.ClothingStyle)
    {
        case EChar_ClothingStyle::BasicLeather: ClothingStr = TEXT("Basic Leather"); break;
        case EChar_ClothingStyle::FurWraps: ClothingStr = TEXT("Fur Wraps"); break;
        case EChar_ClothingStyle::BoneArmor: ClothingStr = TEXT("Bone Armor"); break;
        case EChar_ClothingStyle::TribalGear: ClothingStr = TEXT("Tribal Gear"); break;
        default: ClothingStr = TEXT("Unknown"); break;
    }

    return FString::Printf(TEXT("Primitive Human - Skin: %s, Clothing: %s, Experience: %.1f, Rank: %.1f"), 
        *SkinToneStr, *ClothingStr, SurvivalExperience, TribalRank);
}

void AChar_PrimitiveHumanCharacter::TestCharacterCustomization()
{
    // Cycle through different customizations for testing
    static int32 TestIndex = 0;
    
    switch (TestIndex % 4)
    {
        case 0:
            CustomizationSettings.SkinTone = EChar_SkinTone::Light;
            CustomizationSettings.ClothingStyle = EChar_ClothingStyle::BasicLeather;
            break;
        case 1:
            CustomizationSettings.SkinTone = EChar_SkinTone::Medium;
            CustomizationSettings.ClothingStyle = EChar_ClothingStyle::FurWraps;
            break;
        case 2:
            CustomizationSettings.SkinTone = EChar_SkinTone::Dark;
            CustomizationSettings.ClothingStyle = EChar_ClothingStyle::BoneArmor;
            break;
        case 3:
            CustomizationSettings.SkinTone = EChar_SkinTone::Weathered;
            CustomizationSettings.ClothingStyle = EChar_ClothingStyle::TribalGear;
            break;
    }
    
    TestIndex++;
    ApplyCustomization(CustomizationSettings);
}

void AChar_PrimitiveHumanCharacter::UpdateCharacterMaterials()
{
    if (!CharacterMesh) return;

    // Create dynamic material instance for skin
    if (SkinMaterials.Num() > 0)
    {
        int32 MaterialIndex = static_cast<int32>(CustomizationSettings.SkinTone);
        if (SkinMaterials.IsValidIndex(MaterialIndex))
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(
                SkinMaterials[MaterialIndex], this
            );
            
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("MuscleMass"), CustomizationSettings.MuscleMass);
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), CustomizationSettings.WeatheringLevel);
                CharacterMesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void AChar_PrimitiveHumanCharacter::UpdateClothing()
{
    if (!CharacterMesh) return;

    // Apply clothing material based on style
    if (ClothingMaterials.Num() > 0)
    {
        int32 MaterialIndex = static_cast<int32>(CustomizationSettings.ClothingStyle);
        if (ClothingMaterials.IsValidIndex(MaterialIndex))
        {
            CharacterMesh->SetMaterial(1, ClothingMaterials[MaterialIndex]);
        }
    }
}

void AChar_PrimitiveHumanCharacter::UpdateFacePaint()
{
    if (!CharacterMesh || !FacePaintMaterial) return;

    if (CustomizationSettings.bHasFacePaint)
    {
        UMaterialInstanceDynamic* DynamicPaint = UMaterialInstanceDynamic::Create(
            FacePaintMaterial, this
        );
        
        if (DynamicPaint)
        {
            DynamicPaint->SetVectorParameterValue(TEXT("PaintColor"), CustomizationSettings.FacePaintColor);
            CharacterMesh->SetMaterial(2, DynamicPaint);
        }
    }
    else
    {
        CharacterMesh->SetMaterial(2, nullptr);
    }
}

void AChar_PrimitiveHumanCharacter::SetupDefaultEquipment()
{
    // Set up basic bone tools
    if (BoneToolBelt)
    {
        // Try to load a basic static mesh for tools
        static ConstructorHelpers::FObjectFinder<UStaticMesh> ToolMesh(TEXT("/Engine/BasicShapes/Cube"));
        if (ToolMesh.Succeeded())
        {
            BoneToolBelt->SetStaticMesh(ToolMesh.Object);
            BoneToolBelt->SetWorldScale3D(FVector(0.1f, 0.5f, 0.1f));
        }
    }

    // Set up basic spear
    if (SpearWeapon)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> SpearMesh(TEXT("/Engine/BasicShapes/Cylinder"));
        if (SpearMesh.Succeeded())
        {
            SpearWeapon->SetStaticMesh(SpearMesh.Object);
            SpearWeapon->SetWorldScale3D(FVector(0.05f, 0.05f, 2.0f));
        }
    }
}