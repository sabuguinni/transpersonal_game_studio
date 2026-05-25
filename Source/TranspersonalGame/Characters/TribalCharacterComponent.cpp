#include "TribalCharacterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UTribalCharacterComponent::UTribalCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    AppearanceData = FChar_AppearanceData();
    
    // Initialize pointers
    BaseSkinMaterial = nullptr;
    BaseClothingMaterial = nullptr;
    DynamicSkinMaterial = nullptr;
    DynamicClothingMaterial = nullptr;
    CharacterMesh = nullptr;
}

void UTribalCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the character mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    InitializeMaterials();
    ApplyAppearance(AppearanceData);
}

void UTribalCharacterComponent::ApplyAppearance(const FChar_AppearanceData& NewAppearance)
{
    AppearanceData = NewAppearance;
    UpdateMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Applied appearance: SkinTone=%d, BodyType=%d, ClothingStyle=%d"), 
           (int32)AppearanceData.SkinTone, (int32)AppearanceData.BodyType, (int32)AppearanceData.ClothingStyle);
}

void UTribalCharacterComponent::RandomizeAppearance()
{
    // Randomize all appearance parameters
    AppearanceData.SkinTone = static_cast<EChar_SkinTone>(FMath::RandRange(0, 4));
    AppearanceData.BodyType = static_cast<EChar_BodyType>(FMath::RandRange(0, 4));
    AppearanceData.ClothingStyle = static_cast<EChar_ClothingStyle>(FMath::RandRange(0, 4));
    AppearanceData.TribalMarking = static_cast<EChar_TribalMarking>(FMath::RandRange(0, 5));
    
    AppearanceData.ScarIntensity = FMath::RandRange(0.0f, 0.8f);
    AppearanceData.WeatheringIntensity = FMath::RandRange(0.2f, 0.9f);
    AppearanceData.MuscleDefinition = FMath::RandRange(0.4f, 1.0f);
    
    // Random tribal marking color (earth tones)
    float Hue = FMath::RandRange(0.0f, 60.0f); // Red to yellow range
    float Saturation = FMath::RandRange(0.6f, 1.0f);
    float Value = FMath::RandRange(0.3f, 0.8f);
    AppearanceData.TribalMarkingColor = FLinearColor::MakeFromHSV8(Hue, Saturation * 255, Value * 255);
    
    ApplyAppearance(AppearanceData);
}

void UTribalCharacterComponent::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    AppearanceData.SkinTone = NewSkinTone;
    UpdateSkinMaterial();
}

void UTribalCharacterComponent::SetBodyType(EChar_BodyType NewBodyType)
{
    AppearanceData.BodyType = NewBodyType;
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::SetClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    AppearanceData.ClothingStyle = NewClothingStyle;
    UpdateClothingMaterial();
}

void UTribalCharacterComponent::SetTribalMarking(EChar_TribalMarking NewMarking, FLinearColor MarkingColor)
{
    AppearanceData.TribalMarking = NewMarking;
    AppearanceData.TribalMarkingColor = MarkingColor;
    UpdateSkinMaterial();
}

void UTribalCharacterComponent::UpdateMaterialParameters()
{
    UpdateSkinMaterial();
    UpdateClothingMaterial();
}

void UTribalCharacterComponent::PreviewRandomAppearance()
{
    RandomizeAppearance();
    UE_LOG(LogTemp, Warning, TEXT("Previewing random appearance for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UTribalCharacterComponent::InitializeMaterials()
{
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterMesh not found for TribalCharacterComponent"));
        return;
    }
    
    // Create dynamic material instances if base materials are set
    if (BaseSkinMaterial)
    {
        DynamicSkinMaterial = UMaterialInstanceDynamic::Create(BaseSkinMaterial, this);
        if (DynamicSkinMaterial)
        {
            CharacterMesh->SetMaterial(0, DynamicSkinMaterial); // Assuming skin is material slot 0
        }
    }
    
    if (BaseClothingMaterial)
    {
        DynamicClothingMaterial = UMaterialInstanceDynamic::Create(BaseClothingMaterial, this);
        if (DynamicClothingMaterial)
        {
            CharacterMesh->SetMaterial(1, DynamicClothingMaterial); // Assuming clothing is material slot 1
        }
    }
}

void UTribalCharacterComponent::UpdateSkinMaterial()
{
    if (!DynamicSkinMaterial)
        return;
    
    // Set skin tone color
    FLinearColor SkinColor = GetSkinToneColor(AppearanceData.SkinTone);
    DynamicSkinMaterial->SetVectorParameterValue(TEXT("SkinColor"), SkinColor);
    
    // Set weathering and scars
    DynamicSkinMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), AppearanceData.WeatheringIntensity);
    DynamicSkinMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), AppearanceData.ScarIntensity);
    DynamicSkinMaterial->SetScalarParameterValue(TEXT("MuscleDefinition"), AppearanceData.MuscleDefinition);
    
    // Set tribal marking parameters
    DynamicSkinMaterial->SetScalarParameterValue(TEXT("TribalMarkingType"), static_cast<float>((int32)AppearanceData.TribalMarking));
    DynamicSkinMaterial->SetVectorParameterValue(TEXT("TribalMarkingColor"), AppearanceData.TribalMarkingColor);
    
    // Body type scaling
    float BodyScale = GetBodyTypeScale(AppearanceData.BodyType);
    DynamicSkinMaterial->SetScalarParameterValue(TEXT("BodyScale"), BodyScale);
}

void UTribalCharacterComponent::UpdateClothingMaterial()
{
    if (!DynamicClothingMaterial)
        return;
    
    // Set clothing style parameters
    DynamicClothingMaterial->SetScalarParameterValue(TEXT("ClothingStyle"), static_cast<float>((int32)AppearanceData.ClothingStyle));
    DynamicClothingMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), AppearanceData.WeatheringIntensity);
    
    // Clothing color based on style
    FLinearColor ClothingColor;
    switch (AppearanceData.ClothingStyle)
    {
        case EChar_ClothingStyle::Minimal:
            ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Brown leather
            break;
        case EChar_ClothingStyle::Hunter:
            ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Dark brown
            break;
        case EChar_ClothingStyle::Gatherer:
            ClothingColor = FLinearColor(0.5f, 0.5f, 0.3f, 1.0f); // Olive
            break;
        case EChar_ClothingStyle::Warrior:
            ClothingColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f); // Dark leather
            break;
        case EChar_ClothingStyle::Shaman:
            ClothingColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f); // Lighter ceremonial
            break;
        default:
            ClothingColor = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
            break;
    }
    
    DynamicClothingMaterial->SetVectorParameterValue(TEXT("ClothingColor"), ClothingColor);
}

FLinearColor UTribalCharacterComponent::GetSkinToneColor(EChar_SkinTone SkinTone) const
{
    switch (SkinTone)
    {
        case EChar_SkinTone::PaleTanned:
            return FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
        case EChar_SkinTone::MediumTanned:
            return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        case EChar_SkinTone::DarkTanned:
            return FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
        case EChar_SkinTone::WeatheredBronze:
            return FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
        case EChar_SkinTone::DeepBronze:
            return FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        default:
            return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    }
}

float UTribalCharacterComponent::GetBodyTypeScale(EChar_BodyType BodyType) const
{
    switch (BodyType)
    {
        case EChar_BodyType::Lean:
            return 0.9f;
        case EChar_BodyType::Athletic:
            return 1.0f;
        case EChar_BodyType::Muscular:
            return 1.1f;
        case EChar_BodyType::Stocky:
            return 1.05f;
        case EChar_BodyType::Weathered:
            return 0.95f;
        default:
            return 1.0f;
    }
}