#include "CharacterVisualVariationSystem.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"

UCharacterVisualVariationSystem::UCharacterVisualVariationSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize variation ranges
    MinHeightVariation = 0.85f;
    MaxHeightVariation = 1.15f;
    MinScaleVariation = 0.9f;
    MaxScaleVariation = 1.1f;
    
    InitializePresets();
}

void UCharacterVisualVariationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Character Visual Variation System initialized"));
}

void UCharacterVisualVariationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterVisualVariationSystem::ApplyCharacterVariation(const FCharacterVariationData& VariationData)
{
    CurrentCharacterVariation = VariationData;
    
    // Apply height scaling
    if (AActor* Owner = GetOwner())
    {
        FVector CurrentScale = Owner->GetActorScale3D();
        float HeightScale = VariationData.Height / 175.0f; // Normalize to average height
        Owner->SetActorScale3D(FVector(CurrentScale.X, CurrentScale.Y, HeightScale));
        
        // Update materials
        UpdateCharacterMaterials(VariationData);
    }
    
    OnCharacterVariationApplied.Broadcast(VariationData);
    
    UE_LOG(LogTemp, Log, TEXT("Applied character variation: %s"), *VariationData.CharacterName);
}

FCharacterVariationData UCharacterVisualVariationSystem::GenerateRandomCharacterVariation()
{
    FCharacterVariationData NewVariation;
    
    // Generate random physical traits
    NewVariation.Height = FMath::RandRange(160.0f, 190.0f);
    NewVariation.Weight = FMath::RandRange(50.0f, 100.0f);
    NewVariation.SkinTone = GenerateRandomSkinTone();
    NewVariation.HairColor = GenerateRandomHairColor();
    NewVariation.EyeColor = GenerateRandomEyeColor();
    NewVariation.AgeRange = GenerateRandomAge();
    NewVariation.Gender = GenerateRandomGender();
    NewVariation.Ethnicity = GenerateRandomEthnicity();
    
    // Generate unique character name
    NewVariation.CharacterName = FString::Printf(TEXT("Character_%d"), FMath::RandRange(1000, 9999));
    
    return NewVariation;
}

void UCharacterVisualVariationSystem::ApplyDinosaurVariation(const FDinosaurVariationData& VariationData)
{
    CurrentDinosaurVariation = VariationData;
    
    // Apply scale variation
    if (AActor* Owner = GetOwner())
    {
        FVector CurrentScale = Owner->GetActorScale3D();
        FVector NewScale = CurrentScale * VariationData.ScaleVariation;
        Owner->SetActorScale3D(NewScale);
        
        // Update materials
        UpdateDinosaurMaterials(VariationData);
    }
    
    OnDinosaurVariationApplied.Broadcast(VariationData);
    
    UE_LOG(LogTemp, Log, TEXT("Applied dinosaur variation: %s"), *VariationData.SpeciesName);
}

FDinosaurVariationData UCharacterVisualVariationSystem::GenerateRandomDinosaurVariation(const FString& SpeciesName)
{
    FDinosaurVariationData NewVariation;
    
    NewVariation.SpeciesName = SpeciesName;
    NewVariation.ScaleVariation = GenerateRandomScale();
    NewVariation.PrimaryColor = GenerateRandomDinosaurColor();
    NewVariation.SecondaryColor = GenerateRandomDinosaurColor();
    NewVariation.PatternColor = GenerateRandomDinosaurColor();
    NewVariation.PatternType = GenerateRandomDinosaurPattern();
    NewVariation.TemperamentType = GenerateRandomDinosaurTemperament();
    
    // Generate random physical feature variations
    NewVariation.HornSize = FMath::RandRange(0.7f, 1.3f);
    NewVariation.ClawLength = FMath::RandRange(0.8f, 1.2f);
    NewVariation.TailLength = FMath::RandRange(0.9f, 1.1f);
    NewVariation.AggressionLevel = FMath::RandRange(0.0f, 1.0f);
    
    return NewVariation;
}

void UCharacterVisualVariationSystem::EnsureDiversityInGroup(TArray<AActor*> Characters)
{
    if (Characters.Num() < 2) return;
    
    TArray<FCharacterVariationData> Variations;
    
    // Generate diverse variations for the group
    for (int32 i = 0; i < Characters.Num(); i++)
    {
        FCharacterVariationData NewVariation = GenerateRandomCharacterVariation();
        
        // Ensure diversity by checking against existing variations
        bool bNeedsDiversification = false;
        for (const FCharacterVariationData& ExistingVariation : Variations)
        {
            if (FVector::Dist(FVector(NewVariation.SkinTone), FVector(ExistingVariation.SkinTone)) < 0.2f &&
                NewVariation.Gender == ExistingVariation.Gender &&
                NewVariation.Ethnicity == ExistingVariation.Ethnicity)
            {
                bNeedsDiversification = true;
                break;
            }
        }
        
        if (bNeedsDiversification)
        {
            // Regenerate with forced diversity
            NewVariation.SkinTone = GenerateRandomSkinTone();
            NewVariation.Ethnicity = GenerateRandomEthnicity();
            NewVariation.Gender = (i % 2 == 0) ? TEXT("Male") : TEXT("Female");
        }
        
        Variations.Add(NewVariation);
        
        // Apply variation to character
        if (UCharacterVisualVariationSystem* VariationSystem = Characters[i]->FindComponentByClass<UCharacterVisualVariationSystem>())
        {
            VariationSystem->ApplyCharacterVariation(NewVariation);
        }
    }
}

bool UCharacterVisualVariationSystem::ValidateCharacterDiversity(const TArray<FCharacterVariationData>& CharacterData)
{
    if (CharacterData.Num() < 2) return true;
    
    int32 MaleCount = 0;
    int32 FemaleCount = 0;
    TSet<FString> UniqueEthnicities;
    
    for (const FCharacterVariationData& Character : CharacterData)
    {
        if (Character.Gender == TEXT("Male")) MaleCount++;
        else if (Character.Gender == TEXT("Female")) FemaleCount++;
        
        UniqueEthnicities.Add(Character.Ethnicity);
    }
    
    // Check for reasonable gender balance (not more than 70% of one gender)
    float MaleRatio = static_cast<float>(MaleCount) / CharacterData.Num();
    bool bGenderBalanced = (MaleRatio >= 0.3f && MaleRatio <= 0.7f);
    
    // Check for ethnic diversity (at least 2 different ethnicities in groups of 4+)
    bool bEthnicDiversity = (CharacterData.Num() < 4) || (UniqueEthnicities.Num() >= 2);
    
    return bGenderBalanced && bEthnicDiversity;
}

void UCharacterVisualVariationSystem::UpdateCharacterMaterials(const FCharacterVariationData& VariationData)
{
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
            {
                for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
                {
                    if (UMaterialInterface* Material = MeshComp->GetMaterial(i))
                    {
                        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
                        if (DynamicMaterial)
                        {
                            ApplyMaterialParameters(DynamicMaterial, VariationData);
                            MeshComp->SetMaterial(i, DynamicMaterial);
                        }
                    }
                }
            }
        }
    }
}

void UCharacterVisualVariationSystem::UpdateDinosaurMaterials(const FDinosaurVariationData& VariationData)
{
    if (AActor* Owner = GetOwner())
    {
        TArray<USkeletalMeshComponent*> MeshComponents;
        Owner->GetComponents<USkeletalMeshComponent>(MeshComponents);
        
        for (USkeletalMeshComponent* MeshComp : MeshComponents)
        {
            for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
            {
                if (UMaterialInterface* Material = MeshComp->GetMaterial(i))
                {
                    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
                    if (DynamicMaterial)
                    {
                        ApplyDinosaurMaterialParameters(DynamicMaterial, VariationData);
                        MeshComp->SetMaterial(i, DynamicMaterial);
                    }
                }
            }
        }
    }
}

void UCharacterVisualVariationSystem::InitializePresets()
{
    // Skin tone presets for diverse representation
    SkinTonePresets.Empty();
    SkinTonePresets.Add(FLinearColor(0.95f, 0.87f, 0.73f, 1.0f)); // Light
    SkinTonePresets.Add(FLinearColor(0.85f, 0.72f, 0.56f, 1.0f)); // Medium Light
    SkinTonePresets.Add(FLinearColor(0.76f, 0.60f, 0.42f, 1.0f)); // Medium
    SkinTonePresets.Add(FLinearColor(0.65f, 0.48f, 0.32f, 1.0f)); // Medium Dark
    SkinTonePresets.Add(FLinearColor(0.45f, 0.32f, 0.22f, 1.0f)); // Dark
    SkinTonePresets.Add(FLinearColor(0.35f, 0.25f, 0.18f, 1.0f)); // Very Dark
    
    // Hair color presets
    HairColorPresets.Empty();
    HairColorPresets.Add(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));   // Black
    HairColorPresets.Add(FLinearColor(0.3f, 0.2f, 0.1f, 1.0f));   // Dark Brown
    HairColorPresets.Add(FLinearColor(0.5f, 0.3f, 0.2f, 1.0f));   // Brown
    HairColorPresets.Add(FLinearColor(0.7f, 0.5f, 0.3f, 1.0f));   // Light Brown
    HairColorPresets.Add(FLinearColor(0.9f, 0.8f, 0.6f, 1.0f));   // Blonde
    HairColorPresets.Add(FLinearColor(0.8f, 0.4f, 0.2f, 1.0f));   // Red
    HairColorPresets.Add(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));   // Gray
    
    // Eye color presets
    EyeColorPresets.Empty();
    EyeColorPresets.Add(FLinearColor(0.2f, 0.1f, 0.05f, 1.0f));  // Brown
    EyeColorPresets.Add(FLinearColor(0.3f, 0.5f, 0.7f, 1.0f));   // Blue
    EyeColorPresets.Add(FLinearColor(0.4f, 0.6f, 0.3f, 1.0f));   // Green
    EyeColorPresets.Add(FLinearColor(0.5f, 0.4f, 0.2f, 1.0f));   // Hazel
    EyeColorPresets.Add(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));   // Gray
    
    // Ethnicity types
    EthnicityTypes.Empty();
    EthnicityTypes.Add(TEXT("European"));
    EthnicityTypes.Add(TEXT("African"));
    EthnicityTypes.Add(TEXT("Asian"));
    EthnicityTypes.Add(TEXT("Hispanic"));
    EthnicityTypes.Add(TEXT("Middle Eastern"));
    EthnicityTypes.Add(TEXT("Native American"));
    EthnicityTypes.Add(TEXT("Pacific Islander"));
    EthnicityTypes.Add(TEXT("Mixed"));
    
    // Dinosaur color presets
    DinosaurColorPresets.Empty();
    DinosaurColorPresets.Add(FLinearColor(0.4f, 0.6f, 0.3f, 1.0f)); // Forest Green
    DinosaurColorPresets.Add(FLinearColor(0.6f, 0.4f, 0.2f, 1.0f)); // Brown
    DinosaurColorPresets.Add(FLinearColor(0.7f, 0.7f, 0.5f, 1.0f)); // Sandy
    DinosaurColorPresets.Add(FLinearColor(0.5f, 0.3f, 0.2f, 1.0f)); // Dark Brown
    DinosaurColorPresets.Add(FLinearColor(0.3f, 0.5f, 0.4f, 1.0f)); // Teal
    DinosaurColorPresets.Add(FLinearColor(0.6f, 0.5f, 0.3f, 1.0f)); // Tan
    
    // Dinosaur pattern types
    DinosaurPatternTypes.Empty();
    DinosaurPatternTypes.Add(TEXT("Stripes"));
    DinosaurPatternTypes.Add(TEXT("Spots"));
    DinosaurPatternTypes.Add(TEXT("Solid"));
    DinosaurPatternTypes.Add(TEXT("Mottled"));
    DinosaurPatternTypes.Add(TEXT("Banded"));
    
    // Dinosaur temperaments
    DinosaurTemperaments.Empty();
    DinosaurTemperaments.Add(TEXT("Docile"));
    DinosaurTemperaments.Add(TEXT("Aggressive"));
    DinosaurTemperaments.Add(TEXT("Curious"));
    DinosaurTemperaments.Add(TEXT("Territorial"));
    DinosaurTemperaments.Add(TEXT("Skittish"));
    DinosaurTemperaments.Add(TEXT("Protective"));
}

FLinearColor UCharacterVisualVariationSystem::GenerateRandomSkinTone()
{
    if (SkinTonePresets.Num() > 0)
    {
        return SkinTonePresets[FMath::RandRange(0, SkinTonePresets.Num() - 1)];
    }
    return FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
}

FLinearColor UCharacterVisualVariationSystem::GenerateRandomHairColor()
{
    if (HairColorPresets.Num() > 0)
    {
        return HairColorPresets[FMath::RandRange(0, HairColorPresets.Num() - 1)];
    }
    return FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
}

FLinearColor UCharacterVisualVariationSystem::GenerateRandomEyeColor()
{
    if (EyeColorPresets.Num() > 0)
    {
        return EyeColorPresets[FMath::RandRange(0, EyeColorPresets.Num() - 1)];
    }
    return FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);
}

FString UCharacterVisualVariationSystem::GenerateRandomEthnicity()
{
    if (EthnicityTypes.Num() > 0)
    {
        return EthnicityTypes[FMath::RandRange(0, EthnicityTypes.Num() - 1)];
    }
    return TEXT("Mixed");
}

FString UCharacterVisualVariationSystem::GenerateRandomGender()
{
    return (FMath::RandBool()) ? TEXT("Male") : TEXT("Female");
}

int32 UCharacterVisualVariationSystem::GenerateRandomAge()
{
    return FMath::RandRange(18, 65);
}

FLinearColor UCharacterVisualVariationSystem::GenerateRandomDinosaurColor()
{
    if (DinosaurColorPresets.Num() > 0)
    {
        return DinosaurColorPresets[FMath::RandRange(0, DinosaurColorPresets.Num() - 1)];
    }
    return FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
}

FString UCharacterVisualVariationSystem::GenerateRandomDinosaurPattern()
{
    if (DinosaurPatternTypes.Num() > 0)
    {
        return DinosaurPatternTypes[FMath::RandRange(0, DinosaurPatternTypes.Num() - 1)];
    }
    return TEXT("Solid");
}

FString UCharacterVisualVariationSystem::GenerateRandomDinosaurTemperament()
{
    if (DinosaurTemperaments.Num() > 0)
    {
        return DinosaurTemperaments[FMath::RandRange(0, DinosaurTemperaments.Num() - 1)];
    }
    return TEXT("Docile");
}

float UCharacterVisualVariationSystem::GenerateRandomScale()
{
    return FMath::RandRange(MinScaleVariation, MaxScaleVariation);
}

void UCharacterVisualVariationSystem::ApplyMaterialParameters(UMaterialInterface* Material, const FCharacterVariationData& VariationData)
{
    if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material))
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("SkinTone"), VariationData.SkinTone);
        DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), VariationData.HairColor);
        DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), VariationData.EyeColor);
        DynamicMaterial->SetScalarParameterValue(TEXT("Age"), static_cast<float>(VariationData.AgeRange));
    }
}

void UCharacterVisualVariationSystem::ApplyDinosaurMaterialParameters(UMaterialInterface* Material, const FDinosaurVariationData& VariationData)
{
    if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material))
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), VariationData.PrimaryColor);
        DynamicMaterial->SetVectorParameterValue(TEXT("SecondaryColor"), VariationData.SecondaryColor);
        DynamicMaterial->SetVectorParameterValue(TEXT("PatternColor"), VariationData.PatternColor);
        DynamicMaterial->SetScalarParameterValue(TEXT("HornSize"), VariationData.HornSize);
        DynamicMaterial->SetScalarParameterValue(TEXT("ClawLength"), VariationData.ClawLength);
        DynamicMaterial->SetScalarParameterValue(TEXT("AggressionLevel"), VariationData.AggressionLevel);
    }
}