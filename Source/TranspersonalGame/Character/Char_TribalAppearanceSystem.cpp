#include "Char_TribalAppearanceSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UChar_TribalAppearanceSystem::UChar_TribalAppearanceSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    TribalRole = EChar_TribalRole::Hunter;
    SkinTone = 0.5f;
    HairColor = 0.3f;
    EyeColor = 0.4f;
    Age = 25.0f;
    BodyMass = 0.5f;
    Height = 1.0f;
    
    DirtLevel = 0.3f;
    ScarLevel = 0.2f;
    TanLevel = 0.6f;
    bHasTattoos = false;
    bHasWarPaint = false;
    
    bCanLightTorches = true;
    bIsCarryingTorch = false;
    TorchLightingSkill = 0.7f;
    
    // Initialize tribal markings
    FaceMarkings.PrimaryColor = FLinearColor(0.8f, 0.2f, 0.1f, 1.0f);
    FaceMarkings.SecondaryColor = FLinearColor::White;
    FaceMarkings.PatternIndex = 0;
    FaceMarkings.Intensity = 0.8f;
    FaceMarkings.bHasScars = false;
    
    BodyMarkings = FaceMarkings;
    BodyMarkings.Intensity = 0.5f;
}

void UChar_TribalAppearanceSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner's skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (OwnerMeshComponent.IsValid())
        {
            InitializeMaterialParameters();
            GenerateRandomTribalAppearance();
        }
    }
}

void UChar_TribalAppearanceSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update torch lighting effects if carrying torch
    if (bIsCarryingTorch)
    {
        UpdateTorchLightingEffects();
    }
}

void UChar_TribalAppearanceSystem::GenerateRandomTribalAppearance()
{
    // Randomize basic appearance
    SkinTone = GetRandomFloat(0.2f, 0.9f);
    HairColor = GetRandomFloat(0.1f, 0.8f);
    EyeColor = GetRandomFloat(0.0f, 1.0f);
    Age = GetRandomFloat(18.0f, 65.0f);
    BodyMass = GetRandomFloat(0.3f, 0.8f);
    Height = GetRandomFloat(0.85f, 1.15f);
    
    // Randomize survival effects based on age and role
    float survivalFactor = FMath::Clamp((Age - 18.0f) / 47.0f, 0.0f, 1.0f);
    DirtLevel = GetRandomFloat(0.2f, 0.7f) * survivalFactor;
    ScarLevel = GetRandomFloat(0.0f, 0.5f) * survivalFactor;
    TanLevel = GetRandomFloat(0.4f, 0.9f);
    
    // Role-specific appearance
    ApplyRoleSpecificAppearance();
    
    // Generate tribal markings
    GenerateTribalMarkings();
    
    // Apply weathering based on age and role
    float weatheringIntensity = survivalFactor * 0.6f;
    ApplyWeatheringEffects(weatheringIntensity);
    
    // Update visual appearance
    UpdateMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Generated random tribal appearance for role: %d"), (int32)TribalRole);
}

void UChar_TribalAppearanceSystem::ApplyTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    ApplyRoleSpecificAppearance();
    UpdateMaterialParameters();
}

void UChar_TribalAppearanceSystem::ApplyWeatheringEffects(float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Increase dirt and weathering based on intensity
    DirtLevel = FMath::Clamp(DirtLevel + (Intensity * 0.3f), 0.0f, 1.0f);
    ScarLevel = FMath::Clamp(ScarLevel + (Intensity * 0.2f), 0.0f, 1.0f);
    TanLevel = FMath::Clamp(TanLevel + (Intensity * 0.1f), 0.0f, 1.0f);
    
    // Update clothing condition
    if (Intensity > 0.8f)
    {
        ClothingSet.WeatheringLevel = EChar_WeatheringLevel::Battered;
    }
    else if (Intensity > 0.6f)
    {
        ClothingSet.WeatheringLevel = EChar_WeatheringLevel::Weathered;
    }
    else if (Intensity > 0.4f)
    {
        ClothingSet.WeatheringLevel = EChar_WeatheringLevel::Worn;
    }
    
    UpdateMaterialParameters();
}

void UChar_TribalAppearanceSystem::UpdateClothingCondition(EChar_WeatheringLevel NewLevel)
{
    ClothingSet.WeatheringLevel = NewLevel;
    UpdateMaterialParameters();
}

void UChar_TribalAppearanceSystem::ApplyTribalMarkings(const FChar_TribalMarkings& Markings, bool bFaceMarkings)
{
    if (bFaceMarkings)
    {
        FaceMarkings = Markings;
    }
    else
    {
        BodyMarkings = Markings;
    }
    
    UpdateMaterialParameters();
}

void UChar_TribalAppearanceSystem::SetTorchCarryingState(bool bCarrying)
{
    bIsCarryingTorch = bCarrying;
    
    if (bCarrying)
    {
        UE_LOG(LogTemp, Log, TEXT("Character now carrying torch"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Character no longer carrying torch"));
    }
}

bool UChar_TribalAppearanceSystem::CanInteractWithTorch() const
{
    return bCanLightTorches && TorchLightingSkill > 0.3f;
}

void UChar_TribalAppearanceSystem::InitializeMaterialParameters()
{
    if (!OwnerMeshComponent.IsValid())
        return;
    
    // Create dynamic material instances
    int32 MaterialCount = OwnerMeshComponent->GetNumMaterials();
    DynamicMaterials.Empty();
    
    for (int32 i = 0; i < MaterialCount; i++)
    {
        UMaterialInterface* BaseMaterial = OwnerMeshComponent->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            DynamicMaterials.Add(DynamicMat);
            OwnerMeshComponent->SetMaterial(i, DynamicMat);
        }
    }
}

void UChar_TribalAppearanceSystem::UpdateMaterialParameters()
{
    for (UMaterialInstanceDynamic* DynamicMat : DynamicMaterials)
    {
        if (DynamicMat)
        {
            // Basic appearance parameters
            DynamicMat->SetScalarParameterValue(TEXT("SkinTone"), SkinTone);
            DynamicMat->SetScalarParameterValue(TEXT("HairColor"), HairColor);
            DynamicMat->SetScalarParameterValue(TEXT("EyeColor"), EyeColor);
            DynamicMat->SetScalarParameterValue(TEXT("Age"), Age / 100.0f);
            DynamicMat->SetScalarParameterValue(TEXT("BodyMass"), BodyMass);
            
            // Survival effects
            DynamicMat->SetScalarParameterValue(TEXT("DirtLevel"), DirtLevel);
            DynamicMat->SetScalarParameterValue(TEXT("ScarLevel"), ScarLevel);
            DynamicMat->SetScalarParameterValue(TEXT("TanLevel"), TanLevel);
            
            // Tribal markings
            DynamicMat->SetVectorParameterValue(TEXT("FaceMarkingColor"), FaceMarkings.PrimaryColor);
            DynamicMat->SetVectorParameterValue(TEXT("BodyMarkingColor"), BodyMarkings.PrimaryColor);
            DynamicMat->SetScalarParameterValue(TEXT("MarkingIntensity"), FaceMarkings.Intensity);
            DynamicMat->SetScalarParameterValue(TEXT("PatternIndex"), (float)FaceMarkings.PatternIndex);
            
            // Clothing weathering
            float weatheringValue = (float)ClothingSet.WeatheringLevel / 4.0f;
            DynamicMat->SetScalarParameterValue(TEXT("ClothingWeathering"), weatheringValue);
            
            // Torch lighting effects
            if (bIsCarryingTorch)
            {
                DynamicMat->SetScalarParameterValue(TEXT("TorchLighting"), 1.0f);
                DynamicMat->SetVectorParameterValue(TEXT("TorchColor"), FLinearColor(1.0f, 0.6f, 0.2f, 1.0f));
            }
            else
            {
                DynamicMat->SetScalarParameterValue(TEXT("TorchLighting"), 0.0f);
            }
        }
    }
}

void UChar_TribalAppearanceSystem::ApplyRoleSpecificAppearance()
{
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            TorchLightingSkill = GetRandomFloat(0.7f, 0.9f);
            bHasTattoos = GetRandomFloat(0.0f, 1.0f) > 0.6f;
            ScarLevel = FMath::Max(ScarLevel, 0.3f);
            break;
            
        case EChar_TribalRole::Gatherer:
            TorchLightingSkill = GetRandomFloat(0.5f, 0.8f);
            DirtLevel = FMath::Max(DirtLevel, 0.2f);
            break;
            
        case EChar_TribalRole::Shaman:
            bHasTattoos = true;
            bHasWarPaint = true;
            TorchLightingSkill = 1.0f;
            Age = FMath::Max(Age, 40.0f);
            break;
            
        case EChar_TribalRole::Elder:
            Age = FMath::Max(Age, 50.0f);
            ScarLevel = FMath::Max(ScarLevel, 0.4f);
            TorchLightingSkill = 0.9f;
            break;
            
        case EChar_TribalRole::Child:
            Age = FMath::Min(Age, 16.0f);
            ScarLevel = FMath::Min(ScarLevel, 0.1f);
            TorchLightingSkill = GetRandomFloat(0.2f, 0.5f);
            Height = GetRandomFloat(0.6f, 0.8f);
            break;
            
        case EChar_TribalRole::Chief:
            bHasTattoos = true;
            bHasWarPaint = true;
            TorchLightingSkill = 1.0f;
            ScarLevel = FMath::Max(ScarLevel, 0.3f);
            break;
            
        case EChar_TribalRole::Crafter:
            TorchLightingSkill = GetRandomFloat(0.8f, 1.0f);
            DirtLevel = FMath::Max(DirtLevel, 0.4f);
            break;
            
        case EChar_TribalRole::Scout:
            TorchLightingSkill = GetRandomFloat(0.6f, 0.8f);
            TanLevel = FMath::Max(TanLevel, 0.7f);
            break;
    }
}

void UChar_TribalAppearanceSystem::GenerateTribalMarkings()
{
    // Generate face markings
    FaceMarkings.PrimaryColor = GetRandomTribalColor();
    FaceMarkings.SecondaryColor = FLinearColor::White;
    FaceMarkings.PatternIndex = GetRandomInt(0, 7);
    FaceMarkings.Intensity = GetRandomFloat(0.5f, 1.0f);
    FaceMarkings.bHasScars = GetRandomFloat(0.0f, 1.0f) > 0.7f;
    
    // Generate body markings (usually less intense)
    BodyMarkings.PrimaryColor = FaceMarkings.PrimaryColor;
    BodyMarkings.SecondaryColor = FaceMarkings.SecondaryColor;
    BodyMarkings.PatternIndex = GetRandomInt(0, 5);
    BodyMarkings.Intensity = FaceMarkings.Intensity * 0.7f;
    BodyMarkings.bHasScars = FaceMarkings.bHasScars;
}

void UChar_TribalAppearanceSystem::ApplySurvivalWeathering()
{
    // Apply gradual weathering over time based on environment
    float weatheringRate = 0.001f; // Very slow weathering
    
    DirtLevel = FMath::Clamp(DirtLevel + weatheringRate, 0.0f, 1.0f);
    
    // Clothing degrades over time
    if (GetRandomFloat(0.0f, 1.0f) > 0.999f) // Very rare degradation
    {
        int32 currentLevel = (int32)ClothingSet.WeatheringLevel;
        if (currentLevel < 4)
        {
            ClothingSet.WeatheringLevel = (EChar_WeatheringLevel)(currentLevel + 1);
        }
    }
}

void UChar_TribalAppearanceSystem::UpdateTorchLightingEffects()
{
    // Update material parameters for torch lighting
    for (UMaterialInstanceDynamic* DynamicMat : DynamicMaterials)
    {
        if (DynamicMat)
        {
            // Flickering torch light effect
            float FlickerTime = GetWorld()->GetTimeSeconds();
            float FlickerIntensity = 0.8f + (FMath::Sin(FlickerTime * 8.0f) * 0.2f);
            
            DynamicMat->SetScalarParameterValue(TEXT("TorchFlicker"), FlickerIntensity);
        }
    }
}

void UChar_TribalAppearanceSystem::HandleTorchInteraction()
{
    // Handle torch lighting/extinguishing logic
    if (CanInteractWithTorch())
    {
        UE_LOG(LogTemp, Log, TEXT("Character can interact with torches (skill: %f)"), TorchLightingSkill);
    }
}

float UChar_TribalAppearanceSystem::GetRandomFloat(float Min, float Max) const
{
    return FMath::RandRange(Min, Max);
}

int32 UChar_TribalAppearanceSystem::GetRandomInt(int32 Min, int32 Max) const
{
    return FMath::RandRange(Min, Max);
}

FLinearColor UChar_TribalAppearanceSystem::GetRandomTribalColor() const
{
    // Generate authentic tribal colors (earth tones, ochre, charcoal)
    TArray<FLinearColor> TribalColors = {
        FLinearColor(0.8f, 0.2f, 0.1f, 1.0f), // Red ochre
        FLinearColor(0.9f, 0.7f, 0.3f, 1.0f), // Yellow ochre
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Charcoal
        FLinearColor(0.9f, 0.9f, 0.8f, 1.0f), // White clay
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown earth
        FLinearColor(0.1f, 0.3f, 0.2f, 1.0f), // Plant green
    };
    
    int32 RandomIndex = GetRandomInt(0, TribalColors.Num() - 1);
    return TribalColors[RandomIndex];
}