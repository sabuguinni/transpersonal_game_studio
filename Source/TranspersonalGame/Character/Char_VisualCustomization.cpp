#include "Char_VisualCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

// Material parameter names
const FName UChar_VisualCustomization::SkinColorParam = TEXT("SkinColor");
const FName UChar_VisualCustomization::RoughnessParam = TEXT("Roughness");
const FName UChar_VisualCustomization::SubsurfaceParam = TEXT("Subsurface");
const FName UChar_VisualCustomization::MuscleMassParam = TEXT("MuscleMass");

UChar_VisualCustomization::UChar_VisualCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    CharacterGender = EChar_Gender::Male;
    BodyType = EChar_BodyType::Athletic;
    HeightScale = 1.0f;
    MuscleMass = 0.5f;
    
    // Default skin tone - weathered survivor
    SkinTone.BaseColor = FLinearColor(0.7f, 0.5f, 0.35f, 1.0f);
    SkinTone.Roughness = 0.8f;
    SkinTone.Subsurface = 0.25f;
    
    // Default clothing
    CurrentClothing.ClothingName = TEXT("Primitive Hide");
}

void UChar_VisualCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Find character mesh component
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    InitializeDefaultAppearance();
    LoadAvailableAssets();
    UpdateCharacterMesh();
}

void UChar_VisualCustomization::InitializeDefaultAppearance()
{
    // Initialize available skin tones for tribal diversity
    AvailableSkinTones.Empty();
    
    // Pale (northern survivor)
    FChar_SkinTone PaleSkin;
    PaleSkin.BaseColor = FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
    PaleSkin.Roughness = 0.7f;
    PaleSkin.Subsurface = 0.4f;
    AvailableSkinTones.Add(PaleSkin);
    
    // Medium (temperate survivor)
    FChar_SkinTone MediumSkin;
    MediumSkin.BaseColor = FLinearColor(0.7f, 0.5f, 0.35f, 1.0f);
    MediumSkin.Roughness = 0.8f;
    MediumSkin.Subsurface = 0.3f;
    AvailableSkinTones.Add(MediumSkin);
    
    // Dark (tropical survivor)
    FChar_SkinTone DarkSkin;
    DarkSkin.BaseColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    DarkSkin.Roughness = 0.6f;
    DarkSkin.Subsurface = 0.2f;
    AvailableSkinTones.Add(DarkSkin);
    
    // Tanned (desert survivor)
    FChar_SkinTone TannedSkin;
    TannedSkin.BaseColor = FLinearColor(0.6f, 0.4f, 0.25f, 1.0f);
    TannedSkin.Roughness = 0.9f;
    TannedSkin.Subsurface = 0.25f;
    AvailableSkinTones.Add(TannedSkin);
    
    // Initialize available clothing sets
    AvailableClothing.Empty();
    
    // Basic hide clothing
    FChar_ClothingSet BasicHide;
    BasicHide.ClothingName = TEXT("Basic Hide");
    AvailableClothing.Add(BasicHide);
    
    // Fur clothing (cold climate)
    FChar_ClothingSet FurClothing;
    FurClothing.ClothingName = TEXT("Fur Wraps");
    AvailableClothing.Add(FurClothing);
    
    // Bone armor (warrior)
    FChar_ClothingSet BoneArmor;
    BoneArmor.ClothingName = TEXT("Bone Armor");
    AvailableClothing.Add(BoneArmor);
    
    // Tribal decorations
    FChar_ClothingSet TribalGear;
    TribalGear.ClothingName = TEXT("Tribal Decorations");
    AvailableClothing.Add(TribalGear);
}

void UChar_VisualCustomization::LoadAvailableAssets()
{
    // This would load actual skeletal meshes and materials from content
    // For now, we set up the structure for future asset integration
    
    UE_LOG(LogTemp, Log, TEXT("Character Visual Customization: Assets loading system ready"));
}

void UChar_VisualCustomization::ApplySkinTone(const FChar_SkinTone& NewSkinTone)
{
    SkinTone = NewSkinTone;
    ApplyMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Applied skin tone: R=%.2f G=%.2f B=%.2f"), 
           SkinTone.BaseColor.R, SkinTone.BaseColor.G, SkinTone.BaseColor.B);
}

void UChar_VisualCustomization::ApplyClothingSet(const FChar_ClothingSet& NewClothing)
{
    CurrentClothing = NewClothing;
    UpdateCharacterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Applied clothing set: %s"), *CurrentClothing.ClothingName);
}

void UChar_VisualCustomization::SetBodyType(EChar_BodyType NewBodyType)
{
    BodyType = NewBodyType;
    
    // Adjust muscle mass based on body type
    switch (BodyType)
    {
        case EChar_BodyType::Lean:
            MuscleMass = 0.2f;
            HeightScale = 1.05f;
            break;
        case EChar_BodyType::Athletic:
            MuscleMass = 0.5f;
            HeightScale = 1.0f;
            break;
        case EChar_BodyType::Muscular:
            MuscleMass = 0.8f;
            HeightScale = 0.98f;
            break;
        case EChar_BodyType::Stocky:
            MuscleMass = 0.6f;
            HeightScale = 0.92f;
            break;
    }
    
    UpdateCharacterMesh();
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::SetGender(EChar_Gender NewGender)
{
    CharacterGender = NewGender;
    UpdateCharacterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Set character gender: %s"), 
           CharacterGender == EChar_Gender::Male ? TEXT("Male") : TEXT("Female"));
}

void UChar_VisualCustomization::RandomizeAppearance()
{
    // Randomize skin tone
    if (AvailableSkinTones.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, AvailableSkinTones.Num() - 1);
        ApplySkinTone(AvailableSkinTones[RandomIndex]);
    }
    
    // Randomize clothing
    if (AvailableClothing.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, AvailableClothing.Num() - 1);
        ApplyClothingSet(AvailableClothing[RandomIndex]);
    }
    
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, 3);
    SetBodyType(static_cast<EChar_BodyType>(BodyTypeIndex));
    
    // Randomize gender
    CharacterGender = FMath::RandBool() ? EChar_Gender::Male : EChar_Gender::Female;
    
    // Add slight height variation
    HeightScale = FMath::RandRange(0.92f, 1.08f);
    
    UpdateCharacterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Randomized character appearance"));
}

void UChar_VisualCustomization::ApplyTribalVariation(int32 TribeIndex)
{
    // Apply specific tribal appearance based on index
    switch (TribeIndex % 4)
    {
        case 0: // Mountain tribe - pale, muscular
            if (AvailableSkinTones.Num() > 0) ApplySkinTone(AvailableSkinTones[0]);
            SetBodyType(EChar_BodyType::Muscular);
            break;
            
        case 1: // Forest tribe - medium skin, athletic
            if (AvailableSkinTones.Num() > 1) ApplySkinTone(AvailableSkinTones[1]);
            SetBodyType(EChar_BodyType::Athletic);
            break;
            
        case 2: // Desert tribe - tanned, lean
            if (AvailableSkinTones.Num() > 3) ApplySkinTone(AvailableSkinTones[3]);
            SetBodyType(EChar_BodyType::Lean);
            break;
            
        case 3: // Coastal tribe - dark skin, stocky
            if (AvailableSkinTones.Num() > 2) ApplySkinTone(AvailableSkinTones[2]);
            SetBodyType(EChar_BodyType::Stocky);
            break;
    }
    
    // Apply appropriate clothing for tribe
    if (AvailableClothing.Num() > TribeIndex % AvailableClothing.Num())
    {
        ApplyClothingSet(AvailableClothing[TribeIndex % AvailableClothing.Num()]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal variation %d"), TribeIndex);
}

void UChar_VisualCustomization::UpdateCharacterMesh()
{
    if (!CharacterMesh)
    {
        return;
    }
    
    // Apply height scaling
    FVector CurrentScale = CharacterMesh->GetComponentScale();
    CharacterMesh->SetWorldScale3D(FVector(CurrentScale.X, CurrentScale.Y, HeightScale));
    
    // Apply material parameters
    ApplyMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Updated character mesh - Height Scale: %.2f"), HeightScale);
}

void UChar_VisualCustomization::ApplyMaterialParameters()
{
    if (!CharacterMesh)
    {
        return;
    }
    
    // Get or create dynamic material instance
    UMaterialInterface* BaseMaterial = CharacterMesh->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = CharacterMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            // Apply skin tone parameters
            DynamicMaterial->SetVectorParameterValue(SkinColorParam, SkinTone.BaseColor);
            DynamicMaterial->SetScalarParameterValue(RoughnessParam, SkinTone.Roughness);
            DynamicMaterial->SetScalarParameterValue(SubsurfaceParam, SkinTone.Subsurface);
            DynamicMaterial->SetScalarParameterValue(MuscleMassParam, MuscleMass);
            
            UE_LOG(LogTemp, Log, TEXT("Applied material parameters to character"));
        }
    }
}