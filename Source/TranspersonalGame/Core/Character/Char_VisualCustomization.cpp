#include "Char_VisualCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

UChar_VisualCustomization::UChar_VisualCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    InitializeDefaultColors();
}

void UChar_VisualCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply default customization on start
    ApplyCustomizationToMesh();
}

void UChar_VisualCustomization::InitializeDefaultColors()
{
    // Initialize skin tone variations for diverse tribal populations
    SkinTones.Empty();
    SkinTones.Add(FLinearColor(0.95f, 0.8f, 0.6f, 1.0f));   // Very Light
    SkinTones.Add(FLinearColor(0.85f, 0.7f, 0.5f, 1.0f));   // Light
    SkinTones.Add(FLinearColor(0.75f, 0.6f, 0.45f, 1.0f));  // Medium Light
    SkinTones.Add(FLinearColor(0.65f, 0.5f, 0.35f, 1.0f));  // Medium
    SkinTones.Add(FLinearColor(0.55f, 0.4f, 0.3f, 1.0f));   // Medium Dark
    SkinTones.Add(FLinearColor(0.45f, 0.35f, 0.25f, 1.0f)); // Dark
    SkinTones.Add(FLinearColor(0.35f, 0.25f, 0.2f, 1.0f));  // Very Dark

    // Initialize hair color variations
    HairColors.Empty();
    HairColors.Add(FLinearColor(0.05f, 0.02f, 0.01f, 1.0f)); // Black
    HairColors.Add(FLinearColor(0.15f, 0.08f, 0.04f, 1.0f)); // Dark Brown
    HairColors.Add(FLinearColor(0.3f, 0.15f, 0.08f, 1.0f));  // Brown
    HairColors.Add(FLinearColor(0.5f, 0.3f, 0.15f, 1.0f));   // Light Brown
    HairColors.Add(FLinearColor(0.7f, 0.5f, 0.25f, 1.0f));   // Blonde
    HairColors.Add(FLinearColor(0.4f, 0.15f, 0.08f, 1.0f));  // Auburn
    HairColors.Add(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));    // Gray (elder)

    // Initialize eye color variations
    EyeColors.Empty();
    EyeColors.Add(FLinearColor(0.2f, 0.1f, 0.05f, 1.0f));    // Dark Brown
    EyeColors.Add(FLinearColor(0.4f, 0.25f, 0.1f, 1.0f));    // Brown
    EyeColors.Add(FLinearColor(0.6f, 0.4f, 0.2f, 1.0f));     // Hazel
    EyeColors.Add(FLinearColor(0.3f, 0.5f, 0.2f, 1.0f));     // Green
    EyeColors.Add(FLinearColor(0.2f, 0.4f, 0.7f, 1.0f));     // Blue
    EyeColors.Add(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));     // Gray
}

void UChar_VisualCustomization::RandomizeAllFeatures()
{
    RandomizeFacialFeatures();
    RandomizeBodyProportions();
    RandomizeColors();
    ApplyCustomizationToMesh();
}

void UChar_VisualCustomization::RandomizeFacialFeatures()
{
    // Randomize facial features within realistic ranges
    FacialFeatures.NoseBridgeHeight = FMath::RandRange(-0.3f, 0.3f);
    FacialFeatures.JawWidth = FMath::RandRange(-0.2f, 0.2f);
    FacialFeatures.CheekboneProminence = FMath::RandRange(-0.25f, 0.25f);
    FacialFeatures.EyeSpacing = FMath::RandRange(-0.15f, 0.15f);
    FacialFeatures.BrowRidgeProminence = FMath::RandRange(-0.2f, 0.3f);

    UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Randomized facial features"));
}

void UChar_VisualCustomization::RandomizeBodyProportions()
{
    // Randomize body proportions within realistic human variation
    BodyProportions.HeightScale = FMath::RandRange(0.85f, 1.15f);
    BodyProportions.TorsoLength = FMath::RandRange(0.9f, 1.1f);
    BodyProportions.ArmLength = FMath::RandRange(0.95f, 1.05f);
    BodyProportions.LegLength = FMath::RandRange(0.9f, 1.1f);
    BodyProportions.ShoulderWidth = FMath::RandRange(0.8f, 1.2f);

    UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Randomized body proportions"));
}

void UChar_VisualCustomization::RandomizeColors()
{
    if (SkinTones.Num() > 0)
    {
        CurrentSkinToneIndex = FMath::RandRange(0, SkinTones.Num() - 1);
    }

    if (HairColors.Num() > 0)
    {
        CurrentHairColorIndex = FMath::RandRange(0, HairColors.Num() - 1);
    }

    if (EyeColors.Num() > 0)
    {
        CurrentEyeColorIndex = FMath::RandRange(0, EyeColors.Num() - 1);
    }

    UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Randomized colors - Skin: %d, Hair: %d, Eyes: %d"), 
           CurrentSkinToneIndex, CurrentHairColorIndex, CurrentEyeColorIndex);
}

void UChar_VisualCustomization::ApplyCustomizationToMesh()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("VisualCustomization: No skeletal mesh component found"));
        return;
    }

    ApplyFacialFeatures();
    ApplyBodyProportions();
    ApplyColorVariations();

    UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Applied all customizations to mesh"));
}

void UChar_VisualCustomization::ApplyFacialFeatures()
{
    // Implementation would modify morph targets or bone transforms
    // For now, log the intended changes
    UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Applied facial features - Nose: %.2f, Jaw: %.2f, Cheeks: %.2f"), 
           FacialFeatures.NoseBridgeHeight, FacialFeatures.JawWidth, FacialFeatures.CheekboneProminence);
}

void UChar_VisualCustomization::ApplyBodyProportions()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (MeshComp)
    {
        // Apply height scale to the entire mesh
        FVector CurrentScale = MeshComp->GetComponentScale();
        CurrentScale.Z = BodyProportions.HeightScale;
        MeshComp->SetWorldScale3D(CurrentScale);

        UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Applied body proportions - Height: %.2f"), 
               BodyProportions.HeightScale);
    }
}

void UChar_VisualCustomization::ApplyColorVariations()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Create dynamic material instances for color customization
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
    {
        UMaterialInterface* Material = MeshComp->GetMaterial(i);
        if (Material)
        {
            UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynMaterial)
            {
                // Apply color parameters
                DynMaterial->SetVectorParameterValue(TEXT("SkinTone"), GetCurrentSkinTone());
                DynMaterial->SetVectorParameterValue(TEXT("HairColor"), GetCurrentHairColor());
                DynMaterial->SetVectorParameterValue(TEXT("EyeColor"), GetCurrentEyeColor());

                MeshComp->SetMaterial(i, DynMaterial);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VisualCustomization: Applied color variations"));
}

FLinearColor UChar_VisualCustomization::GetCurrentSkinTone() const
{
    if (SkinTones.IsValidIndex(CurrentSkinToneIndex))
    {
        return SkinTones[CurrentSkinToneIndex];
    }
    return FLinearColor(0.75f, 0.6f, 0.45f, 1.0f); // Default medium skin tone
}

FLinearColor UChar_VisualCustomization::GetCurrentHairColor() const
{
    if (HairColors.IsValidIndex(CurrentHairColorIndex))
    {
        return HairColors[CurrentHairColorIndex];
    }
    return FLinearColor(0.3f, 0.15f, 0.08f, 1.0f); // Default brown hair
}

FLinearColor UChar_VisualCustomization::GetCurrentEyeColor() const
{
    if (EyeColors.IsValidIndex(CurrentEyeColorIndex))
    {
        return EyeColors[CurrentEyeColorIndex];
    }
    return FLinearColor(0.4f, 0.25f, 0.1f, 1.0f); // Default brown eyes
}

void UChar_VisualCustomization::SetSkinTone(int32 Index)
{
    if (SkinTones.IsValidIndex(Index))
    {
        CurrentSkinToneIndex = Index;
        ApplyColorVariations();
    }
}

void UChar_VisualCustomization::SetHairColor(int32 Index)
{
    if (HairColors.IsValidIndex(Index))
    {
        CurrentHairColorIndex = Index;
        ApplyColorVariations();
    }
}

void UChar_VisualCustomization::SetEyeColor(int32 Index)
{
    if (EyeColors.IsValidIndex(Index))
    {
        CurrentEyeColorIndex = Index;
        ApplyColorVariations();
    }
}

USkeletalMeshComponent* UChar_VisualCustomization::GetCharacterMesh() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetMesh();
    }
    return nullptr;
}