#include "CharacterCustomization.h"
#include "TranspersonalCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UCharacterCustomizationComponent::UCharacterCustomizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default customization data
    CustomizationData = FChar_CustomizationData();
    
    // Initialize arrays
    BodyMeshes.Empty();
    SkinMaterials.Empty();
    HairMaterials.Empty();
}

void UCharacterCustomizationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to owner character
    OwnerCharacter = Cast<ATranspersonalCharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        // Apply initial customization
        ApplyCustomization();
    }
}

void UCharacterCustomizationComponent::ApplyCustomization()
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterCustomization: No owner character found"));
        return;
    }

    UpdateMeshAndMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("CharacterCustomization: Applied customization for %s"), 
           *CustomizationData.CharacterName);
}

void UCharacterCustomizationComponent::RandomizeAppearance()
{
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, static_cast<int32>(EChar_BodyType::Heavy));
    CustomizationData.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);
    
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, static_cast<int32>(EChar_SkinTone::Weathered));
    CustomizationData.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize hair style
    int32 HairStyleIndex = FMath::RandRange(0, static_cast<int32>(EChar_HairStyle::Messy));
    CustomizationData.HairStyle = static_cast<EChar_HairStyle>(HairStyleIndex);
    
    // Randomize hair color
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.8f, 0.6f, 0.2f, 1.0f), // Blonde
        FLinearColor(0.6f, 0.3f, 0.1f, 1.0f), // Auburn
        FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)  // Gray
    };
    CustomizationData.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    
    // Randomize eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.2f, 0.4f, 0.8f, 1.0f), // Blue
        FLinearColor(0.2f, 0.6f, 0.2f, 1.0f), // Green
        FLinearColor(0.3f, 0.3f, 0.3f, 1.0f), // Gray
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f) // Dark Brown
    };
    CustomizationData.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Randomize physical attributes
    CustomizationData.Height = FMath::RandRange(0.9f, 1.1f);
    CustomizationData.Weight = FMath::RandRange(0.8f, 1.2f);
    
    // Generate random name
    TArray<FString> Names = {
        TEXT("Kael"), TEXT("Zara"), TEXT("Thane"), TEXT("Lyra"), TEXT("Gareth"),
        TEXT("Naia"), TEXT("Bren"), TEXT("Vera"), TEXT("Dex"), TEXT("Mira"),
        TEXT("Tor"), TEXT("Senna"), TEXT("Jax"), TEXT("Kira"), TEXT("Vex")
    };
    CustomizationData.CharacterName = Names[FMath::RandRange(0, Names.Num() - 1)];
    
    // Apply the randomized customization
    ApplyCustomization();
    
    UE_LOG(LogTemp, Log, TEXT("CharacterCustomization: Randomized appearance for %s"), 
           *CustomizationData.CharacterName);
}

void UCharacterCustomizationComponent::SetBodyType(EChar_BodyType NewBodyType)
{
    CustomizationData.BodyType = NewBodyType;
    UpdateMeshAndMaterials();
}

void UCharacterCustomizationComponent::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CustomizationData.SkinTone = NewSkinTone;
    UpdateMeshAndMaterials();
}

void UCharacterCustomizationComponent::SetHairStyle(EChar_HairStyle NewHairStyle)
{
    CustomizationData.HairStyle = NewHairStyle;
    UpdateMeshAndMaterials();
}

void UCharacterCustomizationComponent::SetHairColor(FLinearColor NewColor)
{
    CustomizationData.HairColor = NewColor;
    UpdateMeshAndMaterials();
}

void UCharacterCustomizationComponent::SetEyeColor(FLinearColor NewColor)
{
    CustomizationData.EyeColor = NewColor;
    UpdateMeshAndMaterials();
}

void UCharacterCustomizationComponent::UpdateMeshAndMaterials()
{
    if (!OwnerCharacter)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    // Update skeletal mesh based on body type
    USkeletalMesh* NewMesh = GetMeshForBodyType(CustomizationData.BodyType);
    if (NewMesh)
    {
        MeshComp->SetSkeletalMesh(NewMesh);
    }

    // Create dynamic material instances for customization
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
    {
        UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Set skin tone parameters
                FLinearColor SkinColor = FLinearColor::White;
                switch (CustomizationData.SkinTone)
                {
                    case EChar_SkinTone::Light:
                        SkinColor = FLinearColor(0.95f, 0.85f, 0.75f, 1.0f);
                        break;
                    case EChar_SkinTone::Medium:
                        SkinColor = FLinearColor(0.8f, 0.65f, 0.5f, 1.0f);
                        break;
                    case EChar_SkinTone::Dark:
                        SkinColor = FLinearColor(0.4f, 0.3f, 0.25f, 1.0f);
                        break;
                    case EChar_SkinTone::Tanned:
                        SkinColor = FLinearColor(0.7f, 0.55f, 0.4f, 1.0f);
                        break;
                    case EChar_SkinTone::Weathered:
                        SkinColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
                        break;
                }
                
                DynamicMaterial->SetVectorParameterValue(FName("SkinColor"), SkinColor);
                DynamicMaterial->SetVectorParameterValue(FName("HairColor"), CustomizationData.HairColor);
                DynamicMaterial->SetVectorParameterValue(FName("EyeColor"), CustomizationData.EyeColor);
                
                MeshComp->SetMaterial(i, DynamicMaterial);
            }
        }
    }

    // Apply scale based on height and weight
    FVector NewScale = FVector(
        CustomizationData.Weight,
        CustomizationData.Weight,
        CustomizationData.Height
    );
    MeshComp->SetRelativeScale3D(NewScale);
}

USkeletalMesh* UCharacterCustomizationComponent::GetMeshForBodyType(EChar_BodyType BodyType)
{
    // Return appropriate mesh based on body type
    // For now, return nullptr and use default mesh
    // In production, this would load different meshes from BodyMeshes array
    if (BodyMeshes.Num() > static_cast<int32>(BodyType))
    {
        return BodyMeshes[static_cast<int32>(BodyType)].LoadSynchronous();
    }
    
    return nullptr;
}

UMaterialInterface* UCharacterCustomizationComponent::GetMaterialForSkinTone(EChar_SkinTone SkinTone)
{
    // Return appropriate material based on skin tone
    if (SkinMaterials.Num() > static_cast<int32>(SkinTone))
    {
        return SkinMaterials[static_cast<int32>(SkinTone)].LoadSynchronous();
    }
    
    return nullptr;
}

UMaterialInterface* UCharacterCustomizationComponent::GetMaterialForHair(EChar_HairStyle HairStyle)
{
    // Return appropriate material based on hair style
    if (HairMaterials.Num() > static_cast<int32>(HairStyle))
    {
        return HairMaterials[static_cast<int32>(HairStyle)].LoadSynchronous();
    }
    
    return nullptr;
}