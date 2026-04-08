#include "CharacterSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create MetaHuman component placeholder
    // Note: This will be replaced with actual MetaHuman component when available
    MetaHumanComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanComponent"));
    
    // Set default character data
    CharacterData = FCharacterVariationData();
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial character variation
    UpdateMetaHumanAppearance();
}

void ATranspersonalCharacter::ApplyCharacterVariation(const FCharacterVariationData& VariationData)
{
    CharacterData = VariationData;
    UpdateMetaHumanAppearance();
    
    UE_LOG(LogTemp, Log, TEXT("Applied character variation for: %s"), *CharacterData.CharacterName);
}

void ATranspersonalCharacter::GenerateRandomVariation(ECharacterType Type)
{
    CharacterData.CharacterType = Type;
    
    // Random gender
    CharacterData.Gender = (FMath::RandBool()) ? ECharacterGender::Male : ECharacterGender::Female;
    
    // Random age distribution weighted towards adults
    float AgeRoll = FMath::FRand();
    if (AgeRoll < 0.2f)
    {
        CharacterData.Age = ECharacterAge::Young;
    }
    else if (AgeRoll < 0.7f)
    {
        CharacterData.Age = ECharacterAge::Adult;
    }
    else
    {
        CharacterData.Age = ECharacterAge::Elder;
    }
    
    // Random physical variations
    CharacterData.HeightVariation = FMath::RandRange(-0.1f, 0.1f);
    CharacterData.BodyMassVariation = FMath::RandRange(-0.2f, 0.2f);
    
    // Random facial features
    CharacterData.EyeSize = FMath::RandRange(-0.3f, 0.3f);
    CharacterData.NoseSize = FMath::RandRange(-0.3f, 0.3f);
    CharacterData.JawWidth = FMath::RandRange(-0.3f, 0.3f);
    CharacterData.CheekboneHeight = FMath::RandRange(-0.3f, 0.3f);
    
    // Random skin tone based on realistic human variation
    float SkinBase = FMath::RandRange(0.3f, 0.9f);
    CharacterData.SkinTone = FLinearColor(
        SkinBase,
        SkinBase * 0.8f,
        SkinBase * 0.6f,
        1.0f
    );
    
    // Random hair
    CharacterData.HairStyleIndex = FMath::RandRange(0, FMath::Max(0, HairStyles.Num() - 1));
    
    // Natural hair colors
    TArray<FLinearColor> NaturalHairColors = {
        FLinearColor(0.05f, 0.02f, 0.01f, 1.0f), // Black
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),   // Dark Brown
        FLinearColor(0.4f, 0.25f, 0.1f, 1.0f),   // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),    // Light Brown
        FLinearColor(0.8f, 0.6f, 0.3f, 1.0f),    // Blonde
        FLinearColor(0.7f, 0.3f, 0.1f, 1.0f),    // Red
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)     // Gray (for elders)
    };
    
    int32 HairColorIndex = FMath::RandRange(0, NaturalHairColors.Num() - 1);
    
    // Elders more likely to have gray hair
    if (CharacterData.Age == ECharacterAge::Elder && FMath::FRand() < 0.4f)
    {
        HairColorIndex = NaturalHairColors.Num() - 1; // Gray
    }
    
    CharacterData.HairColor = NaturalHairColors[HairColorIndex];
    
    // Random clothing appropriate to character type
    CharacterData.ClothingSetIndex = FMath::RandRange(0, FMath::Max(0, ClothingSets.Num() - 1));
    
    // Generate name based on type and gender
    GenerateCharacterName();
    
    UpdateMetaHumanAppearance();
}

void ATranspersonalCharacter::GenerateCharacterName()
{
    // Prehistoric-themed names that sound natural but not modern
    TArray<FString> MaleNames = {
        TEXT("Kael"), TEXT("Thane"), TEXT("Bren"), TEXT("Drak"), TEXT("Gor"),
        TEXT("Holt"), TEXT("Jax"), TEXT("Kane"), TEXT("Lok"), TEXT("Nox"),
        TEXT("Ox"), TEXT("Pike"), TEXT("Rex"), TEXT("Tor"), TEXT("Vex")
    };
    
    TArray<FString> FemaleNames = {
        TEXT("Ayla"), TEXT("Bria"), TEXT("Cora"), TEXT("Dara"), TEXT("Eira"),
        TEXT("Faye"), TEXT("Gaia"), TEXT("Hira"), TEXT("Iona"), TEXT("Kira"),
        TEXT("Lara"), TEXT("Mira"), TEXT("Nara"), TEXT("Ora"), TEXT("Vera")
    };
    
    if (CharacterData.Gender == ECharacterGender::Male)
    {
        CharacterData.CharacterName = MaleNames[FMath::RandRange(0, MaleNames.Num() - 1)];
    }
    else
    {
        CharacterData.CharacterName = FemaleNames[FMath::RandRange(0, FemaleNames.Num() - 1)];
    }
}

void ATranspersonalCharacter::SetCharacterName(const FString& NewName)
{
    CharacterData.CharacterName = NewName;
}

void ATranspersonalCharacter::UpdateMetaHumanAppearance()
{
    ApplyBodyVariations();
    ApplyFacialVariations();
    ApplyClothing();
}

void ATranspersonalCharacter::ApplyBodyVariations()
{
    if (!MetaHumanComponent)
        return;
    
    // Select appropriate body mesh based on gender
    USkeletalMesh* SelectedBodyMesh = nullptr;
    
    if (CharacterData.Gender == ECharacterGender::Male && MaleMetaHumanBodies.Num() > 0)
    {
        int32 BodyIndex = FMath::RandRange(0, MaleMetaHumanBodies.Num() - 1);
        SelectedBodyMesh = MaleMetaHumanBodies[BodyIndex];
    }
    else if (CharacterData.Gender == ECharacterGender::Female && FemaleMetaHumanBodies.Num() > 0)
    {
        int32 BodyIndex = FMath::RandRange(0, FemaleMetaHumanBodies.Num() - 1);
        SelectedBodyMesh = FemaleMetaHumanBodies[BodyIndex];
    }
    
    if (SelectedBodyMesh)
    {
        GetMesh()->SetSkeletalMesh(SelectedBodyMesh);
        
        // Apply height and mass variations
        FVector CurrentScale = GetMesh()->GetRelativeScale3D();
        float HeightScale = 1.0f + CharacterData.HeightVariation;
        float MassScale = 1.0f + CharacterData.BodyMassVariation;
        
        GetMesh()->SetRelativeScale3D(FVector(MassScale, MassScale, HeightScale));
        
        // Apply skin tone to material
        UMaterialInstanceDynamic* DynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinTone"), CharacterData.SkinTone);
        }
    }
}

void ATranspersonalCharacter::ApplyFacialVariations()
{
    // This will be implemented when MetaHuman facial morphing is available
    // For now, log the intended variations
    UE_LOG(LogTemp, Log, TEXT("Facial variations for %s: Eye=%.2f, Nose=%.2f, Jaw=%.2f, Cheek=%.2f"),
        *CharacterData.CharacterName,
        CharacterData.EyeSize,
        CharacterData.NoseSize,
        CharacterData.JawWidth,
        CharacterData.CheekboneHeight);
}

void ATranspersonalCharacter::ApplyClothing()
{
    // Apply clothing and hair based on character type and indices
    if (HairStyles.IsValidIndex(CharacterData.HairStyleIndex))
    {
        // Hair will be applied as additional skeletal mesh component
        UE_LOG(LogTemp, Log, TEXT("Applied hair style %d to %s"), 
            CharacterData.HairStyleIndex, *CharacterData.CharacterName);
    }
    
    if (ClothingSets.IsValidIndex(CharacterData.ClothingSetIndex))
    {
        // Clothing will be applied through material swapping or additional mesh components
        UE_LOG(LogTemp, Log, TEXT("Applied clothing set %d to %s"), 
            CharacterData.ClothingSetIndex, *CharacterData.CharacterName);
    }
}