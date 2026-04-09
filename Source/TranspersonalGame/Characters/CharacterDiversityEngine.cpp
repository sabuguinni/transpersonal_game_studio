// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CharacterDiversityEngine.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogCharacterDiversity);

UCharacterDiversityEngine::UCharacterDiversityEngine()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default diversity parameters
    InitializeDefaultDiversityParameters();
}

void UCharacterDiversityEngine::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCharacterDiversity, Log, TEXT("Character Diversity Engine initialized"));
    
    // Load diversity data from configuration
    LoadDiversityConfiguration();
}

void UCharacterDiversityEngine::InitializeDefaultDiversityParameters()
{
    // Facial feature variation ranges
    FacialFeatureVariation.FaceWidth = FVector2D(0.8f, 1.2f);
    FacialFeatureVariation.FaceHeight = FVector2D(0.85f, 1.15f);
    FacialFeatureVariation.EyeSize = FVector2D(0.7f, 1.3f);
    FacialFeatureVariation.NoseSize = FVector2D(0.8f, 1.2f);
    FacialFeatureVariation.MouthSize = FVector2D(0.85f, 1.15f);
    FacialFeatureVariation.CheekboneHeight = FVector2D(0.9f, 1.1f);
    FacialFeatureVariation.JawWidth = FVector2D(0.8f, 1.2f);
    FacialFeatureVariation.BrowRidge = FVector2D(0.7f, 1.3f);
    
    // Body type variation ranges
    BodyTypeVariation.Height = FVector2D(0.85f, 1.15f);
    BodyTypeVariation.Weight = FVector2D(0.8f, 1.2f);
    BodyTypeVariation.ShoulderWidth = FVector2D(0.85f, 1.15f);
    BodyTypeVariation.WaistSize = FVector2D(0.8f, 1.2f);
    BodyTypeVariation.LegLength = FVector2D(0.9f, 1.1f);
    BodyTypeVariation.ArmLength = FVector2D(0.9f, 1.1f);
    BodyTypeVariation.TorsoLength = FVector2D(0.9f, 1.1f);
    BodyTypeVariation.HandSize = FVector2D(0.85f, 1.15f);
    BodyTypeVariation.FootSize = FVector2D(0.85f, 1.15f);
    
    // Skin tone variation
    SkinToneVariation.BaseHue = FVector2D(0.0f, 360.0f);
    SkinToneVariation.Saturation = FVector2D(0.1f, 0.8f);
    SkinToneVariation.Brightness = FVector2D(0.2f, 0.9f);
    SkinToneVariation.Warmth = FVector2D(-0.3f, 0.3f);
    SkinToneVariation.Undertone = FVector2D(-0.2f, 0.2f);
    
    // Hair variation
    HairVariation.Color = FVector2D(0.0f, 360.0f);
    HairVariation.Thickness = FVector2D(0.5f, 2.0f);
    HairVariation.Curl = FVector2D(0.0f, 1.0f);
    HairVariation.Length = FVector2D(0.1f, 3.0f);
    HairVariation.Density = FVector2D(0.3f, 1.0f);
    
    // Age progression parameters
    AgeProgression.WrinkleIntensity = FVector2D(0.0f, 1.0f);
    AgeProgression.SkinSagging = FVector2D(0.0f, 0.8f);
    AgeProgression.HairGraying = FVector2D(0.0f, 1.0f);
    AgeProgression.PostureChange = FVector2D(0.0f, 0.3f);
    AgeProgression.MuscleDefinition = FVector2D(0.5f, 1.0f);
    
    // Clothing wear patterns
    ClothingWear.FabricWear = FVector2D(0.0f, 0.8f);
    ClothingWear.ColorFading = FVector2D(0.0f, 0.6f);
    ClothingWear.DirtAccumulation = FVector2D(0.0f, 0.7f);
    ClothingWear.TearDamage = FVector2D(0.0f, 0.4f);
    ClothingWear.Staining = FVector2D(0.0f, 0.5f);
}

FCharacterDiversityProfile UCharacterDiversityEngine::GenerateRandomDiversityProfile()
{
    FCharacterDiversityProfile Profile;
    
    // Generate random facial features within variation ranges
    Profile.FacialFeatures.FaceWidth = FMath::RandRange(FacialFeatureVariation.FaceWidth.X, FacialFeatureVariation.FaceWidth.Y);
    Profile.FacialFeatures.FaceHeight = FMath::RandRange(FacialFeatureVariation.FaceHeight.X, FacialFeatureVariation.FaceHeight.Y);
    Profile.FacialFeatures.EyeSize = FMath::RandRange(FacialFeatureVariation.EyeSize.X, FacialFeatureVariation.EyeSize.Y);
    Profile.FacialFeatures.NoseSize = FMath::RandRange(FacialFeatureVariation.NoseSize.X, FacialFeatureVariation.NoseSize.Y);
    Profile.FacialFeatures.MouthSize = FMath::RandRange(FacialFeatureVariation.MouthSize.X, FacialFeatureVariation.MouthSize.Y);
    Profile.FacialFeatures.CheekboneHeight = FMath::RandRange(FacialFeatureVariation.CheekboneHeight.X, FacialFeatureVariation.CheekboneHeight.Y);
    Profile.FacialFeatures.JawWidth = FMath::RandRange(FacialFeatureVariation.JawWidth.X, FacialFeatureVariation.JawWidth.Y);
    Profile.FacialFeatures.BrowRidge = FMath::RandRange(FacialFeatureVariation.BrowRidge.X, FacialFeatureVariation.BrowRidge.Y);
    
    // Generate random body type
    Profile.BodyType.Height = FMath::RandRange(BodyTypeVariation.Height.X, BodyTypeVariation.Height.Y);
    Profile.BodyType.Weight = FMath::RandRange(BodyTypeVariation.Weight.X, BodyTypeVariation.Weight.Y);
    Profile.BodyType.ShoulderWidth = FMath::RandRange(BodyTypeVariation.ShoulderWidth.X, BodyTypeVariation.ShoulderWidth.Y);
    Profile.BodyType.WaistSize = FMath::RandRange(BodyTypeVariation.WaistSize.X, BodyTypeVariation.WaistSize.Y);
    Profile.BodyType.LegLength = FMath::RandRange(BodyTypeVariation.LegLength.X, BodyTypeVariation.LegLength.Y);
    Profile.BodyType.ArmLength = FMath::RandRange(BodyTypeVariation.ArmLength.X, BodyTypeVariation.ArmLength.Y);
    Profile.BodyType.TorsoLength = FMath::RandRange(BodyTypeVariation.TorsoLength.X, BodyTypeVariation.TorsoLength.Y);
    Profile.BodyType.HandSize = FMath::RandRange(BodyTypeVariation.HandSize.X, BodyTypeVariation.HandSize.Y);
    Profile.BodyType.FootSize = FMath::RandRange(BodyTypeVariation.FootSize.X, BodyTypeVariation.FootSize.Y);
    
    // Generate skin tone
    Profile.SkinTone.BaseHue = FMath::RandRange(SkinToneVariation.BaseHue.X, SkinToneVariation.BaseHue.Y);
    Profile.SkinTone.Saturation = FMath::RandRange(SkinToneVariation.Saturation.X, SkinToneVariation.Saturation.Y);
    Profile.SkinTone.Brightness = FMath::RandRange(SkinToneVariation.Brightness.X, SkinToneVariation.Brightness.Y);
    Profile.SkinTone.Warmth = FMath::RandRange(SkinToneVariation.Warmth.X, SkinToneVariation.Warmth.Y);
    Profile.SkinTone.Undertone = FMath::RandRange(SkinToneVariation.Undertone.X, SkinToneVariation.Undertone.Y);
    
    // Generate hair characteristics
    Profile.HairCharacteristics.Color = FMath::RandRange(HairVariation.Color.X, HairVariation.Color.Y);
    Profile.HairCharacteristics.Thickness = FMath::RandRange(HairVariation.Thickness.X, HairVariation.Thickness.Y);
    Profile.HairCharacteristics.Curl = FMath::RandRange(HairVariation.Curl.X, HairVariation.Curl.Y);
    Profile.HairCharacteristics.Length = FMath::RandRange(HairVariation.Length.X, HairVariation.Length.Y);
    Profile.HairCharacteristics.Density = FMath::RandRange(HairVariation.Density.X, HairVariation.Density.Y);
    
    // Generate age characteristics
    float Age = FMath::RandRange(18.0f, 70.0f);
    Profile.AgeCharacteristics = GenerateAgeCharacteristics(Age);
    
    // Generate clothing wear
    Profile.ClothingWearPattern.FabricWear = FMath::RandRange(ClothingWear.FabricWear.X, ClothingWear.FabricWear.Y);
    Profile.ClothingWearPattern.ColorFading = FMath::RandRange(ClothingWear.ColorFading.X, ClothingWear.ColorFading.Y);
    Profile.ClothingWearPattern.DirtAccumulation = FMath::RandRange(ClothingWear.DirtAccumulation.X, ClothingWear.DirtAccumulation.Y);
    Profile.ClothingWearPattern.TearDamage = FMath::RandRange(ClothingWear.TearDamage.X, ClothingWear.TearDamage.Y);
    Profile.ClothingWearPattern.Staining = FMath::RandRange(ClothingWear.Staining.X, ClothingWear.Staining.Y);
    
    // Generate unique identifier
    Profile.UniqueIdentifier = FGuid::NewGuid();
    
    UE_LOG(LogCharacterDiversity, Log, TEXT("Generated diversity profile with ID: %s"), *Profile.UniqueIdentifier.ToString());
    
    return Profile;
}

FCharacterAgeCharacteristics UCharacterDiversityEngine::GenerateAgeCharacteristics(float Age)
{
    FCharacterAgeCharacteristics AgeChars;
    
    // Calculate age progression factor (0.0 = young, 1.0 = old)
    float AgeProgression = FMath::Clamp((Age - 18.0f) / 52.0f, 0.0f, 1.0f);
    
    // Apply age-based modifications
    AgeChars.WrinkleIntensity = AgeProgression * FMath::RandRange(0.8f, 1.2f);
    AgeChars.SkinSagging = AgeProgression * FMath::RandRange(0.7f, 1.1f);
    AgeChars.HairGraying = FMath::Pow(AgeProgression, 1.5f) * FMath::RandRange(0.9f, 1.1f);
    AgeChars.PostureChange = AgeProgression * FMath::RandRange(0.5f, 1.0f);
    AgeChars.MuscleDefinition = FMath::Lerp(1.0f, 0.6f, AgeProgression) * FMath::RandRange(0.9f, 1.1f);
    
    return AgeChars;
}

void UCharacterDiversityEngine::ApplyDiversityProfile(USkeletalMeshComponent* MeshComponent, const FCharacterDiversityProfile& Profile)
{
    if (!MeshComponent)
    {
        UE_LOG(LogCharacterDiversity, Warning, TEXT("Cannot apply diversity profile: MeshComponent is null"));
        return;
    }
    
    // Apply facial feature modifications
    ApplyFacialFeatures(MeshComponent, Profile.FacialFeatures);
    
    // Apply body type modifications
    ApplyBodyType(MeshComponent, Profile.BodyType);
    
    // Apply skin tone
    ApplySkinTone(MeshComponent, Profile.SkinTone);
    
    // Apply hair characteristics
    ApplyHairCharacteristics(MeshComponent, Profile.HairCharacteristics);
    
    // Apply age characteristics
    ApplyAgeCharacteristics(MeshComponent, Profile.AgeCharacteristics);
    
    UE_LOG(LogCharacterDiversity, Log, TEXT("Applied diversity profile to character mesh"));
}

void UCharacterDiversityEngine::ApplyFacialFeatures(USkeletalMeshComponent* MeshComponent, const FFacialFeatureSet& Features)
{
    if (!MeshComponent) return;
    
    // Apply morph targets for facial features
    MeshComponent->SetMorphTarget(TEXT("FaceWidth"), Features.FaceWidth - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("FaceHeight"), Features.FaceHeight - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("EyeSize"), Features.EyeSize - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("NoseSize"), Features.NoseSize - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("MouthSize"), Features.MouthSize - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("CheekboneHeight"), Features.CheekboneHeight - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("JawWidth"), Features.JawWidth - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("BrowRidge"), Features.BrowRidge - 1.0f);
}

void UCharacterDiversityEngine::ApplyBodyType(USkeletalMeshComponent* MeshComponent, const FBodyTypeSet& BodyType)
{
    if (!MeshComponent) return;
    
    // Apply body scaling
    FVector CurrentScale = MeshComponent->GetComponentScale();
    FVector NewScale = FVector(
        CurrentScale.X * BodyType.ShoulderWidth,
        CurrentScale.Y * BodyType.Weight,
        CurrentScale.Z * BodyType.Height
    );
    MeshComponent->SetWorldScale3D(NewScale);
    
    // Apply body morph targets
    MeshComponent->SetMorphTarget(TEXT("BodyWeight"), BodyType.Weight - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("WaistSize"), BodyType.WaistSize - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("LegLength"), BodyType.LegLength - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("ArmLength"), BodyType.ArmLength - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("TorsoLength"), BodyType.TorsoLength - 1.0f);
}

void UCharacterDiversityEngine::ApplySkinTone(USkeletalMeshComponent* MeshComponent, const FSkinToneSet& SkinTone)
{
    if (!MeshComponent) return;
    
    // Get or create dynamic material instance
    UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        // Convert HSV to RGB for skin tone
        FLinearColor SkinColor = FLinearColor::MakeFromHSV8(
            static_cast<uint8>(SkinTone.BaseHue),
            static_cast<uint8>(SkinTone.Saturation * 255),
            static_cast<uint8>(SkinTone.Brightness * 255)
        );
        
        // Apply skin tone parameters
        DynamicMaterial->SetVectorParameterValue(TEXT("SkinBaseColor"), SkinColor);
        DynamicMaterial->SetScalarParameterValue(TEXT("SkinWarmth"), SkinTone.Warmth);
        DynamicMaterial->SetScalarParameterValue(TEXT("SkinUndertone"), SkinTone.Undertone);
    }
}

void UCharacterDiversityEngine::ApplyHairCharacteristics(USkeletalMeshComponent* MeshComponent, const FHairCharacteristicSet& Hair)
{
    if (!MeshComponent) return;
    
    // Apply hair morph targets
    MeshComponent->SetMorphTarget(TEXT("HairThickness"), Hair.Thickness - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("HairCurl"), Hair.Curl);
    MeshComponent->SetMorphTarget(TEXT("HairLength"), Hair.Length - 1.0f);
    MeshComponent->SetMorphTarget(TEXT("HairDensity"), Hair.Density);
    
    // Apply hair color to material
    UMaterialInstanceDynamic* HairMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(1); // Assuming hair is material slot 1
    if (HairMaterial)
    {
        FLinearColor HairColor = FLinearColor::MakeFromHSV8(
            static_cast<uint8>(Hair.Color),
            200, // Default saturation for hair
            180  // Default brightness for hair
        );
        HairMaterial->SetVectorParameterValue(TEXT("HairColor"), HairColor);
    }
}

void UCharacterDiversityEngine::ApplyAgeCharacteristics(USkeletalMeshComponent* MeshComponent, const FCharacterAgeCharacteristics& Age)
{
    if (!MeshComponent) return;
    
    // Apply age-related morph targets
    MeshComponent->SetMorphTarget(TEXT("WrinkleIntensity"), Age.WrinkleIntensity);
    MeshComponent->SetMorphTarget(TEXT("SkinSagging"), Age.SkinSagging);
    MeshComponent->SetMorphTarget(TEXT("PostureChange"), Age.PostureChange);
    MeshComponent->SetMorphTarget(TEXT("MuscleDefinition"), Age.MuscleDefinition);
    
    // Apply hair graying to material
    UMaterialInstanceDynamic* HairMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(1);
    if (HairMaterial)
    {
        HairMaterial->SetScalarParameterValue(TEXT("HairGraying"), Age.HairGraying);
    }
}

void UCharacterDiversityEngine::LoadDiversityConfiguration()
{
    // Load diversity configuration from data assets or config files
    // This would typically load from a data table or configuration file
    UE_LOG(LogCharacterDiversity, Log, TEXT("Loading diversity configuration..."));
    
    // For now, use default parameters
    // In a full implementation, this would load from external data sources
}

TArray<FCharacterDiversityProfile> UCharacterDiversityEngine::GenerateDiversePopulation(int32 PopulationSize)
{
    TArray<FCharacterDiversityProfile> Population;
    Population.Reserve(PopulationSize);
    
    for (int32 i = 0; i < PopulationSize; ++i)
    {
        FCharacterDiversityProfile Profile = GenerateRandomDiversityProfile();
        Population.Add(Profile);
    }
    
    UE_LOG(LogCharacterDiversity, Log, TEXT("Generated diverse population of %d characters"), PopulationSize);
    
    return Population;
}

bool UCharacterDiversityEngine::ValidateDiversityProfile(const FCharacterDiversityProfile& Profile)
{
    // Validate that all values are within acceptable ranges
    bool bIsValid = true;
    
    // Check facial features
    if (Profile.FacialFeatures.FaceWidth < 0.5f || Profile.FacialFeatures.FaceWidth > 1.5f)
    {
        UE_LOG(LogCharacterDiversity, Warning, TEXT("Invalid face width: %f"), Profile.FacialFeatures.FaceWidth);
        bIsValid = false;
    }
    
    // Check body type
    if (Profile.BodyType.Height < 0.7f || Profile.BodyType.Height > 1.3f)
    {
        UE_LOG(LogCharacterDiversity, Warning, TEXT("Invalid height: %f"), Profile.BodyType.Height);
        bIsValid = false;
    }
    
    // Check skin tone
    if (Profile.SkinTone.Brightness < 0.0f || Profile.SkinTone.Brightness > 1.0f)
    {
        UE_LOG(LogCharacterDiversity, Warning, TEXT("Invalid skin brightness: %f"), Profile.SkinTone.Brightness);
        bIsValid = false;
    }
    
    return bIsValid;
}