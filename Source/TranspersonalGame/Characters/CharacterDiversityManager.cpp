#include "CharacterDiversityManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

void UCharacterDiversityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeNameDatabases();
    InitializeDiversityTargets();
    
    UE_LOG(LogTemp, Log, TEXT("Character Diversity Manager initialized"));
}

void UCharacterDiversityManager::Deinitialize()
{
    ActiveCharacters.Empty();
    GeneratedCharacters.Empty();
    
    Super::Deinitialize();
}

FMetaHumanCustomization UCharacterDiversityManager::GenerateDiverseCharacter(ECharacterArchetype BaseArchetype)
{
    FMetaHumanCustomization NewCustomization;
    
    // Select balanced demographics
    NewCustomization.Gender = SelectBalancedGender();
    NewCustomization.SkinTone = SelectBalancedSkinTone();
    
    // Generate unique facial features
    NewCustomization.FacialFeatures = GenerateUniqueFacialFeatures(NewCustomization.SkinTone, NewCustomization.Gender);
    
    // Generate random colors with some archetype influence
    switch (BaseArchetype)
    {
    case ECharacterArchetype::Scientist:
        // Scientists might have more conservative appearance
        NewCustomization.HairColor = FLinearColor(
            FMath::RandRange(0.2f, 0.8f),
            FMath::RandRange(0.1f, 0.6f),
            FMath::RandRange(0.0f, 0.4f),
            1.0f
        );
        break;
        
    case ECharacterArchetype::Survivor:
        // Survivors might show more wear and natural colors
        NewCustomization.HairColor = FLinearColor(
            FMath::RandRange(0.1f, 0.7f),
            FMath::RandRange(0.05f, 0.5f),
            FMath::RandRange(0.0f, 0.3f),
            1.0f
        );
        break;
        
    default:
        // Random natural hair colors
        NewCustomization.HairColor = FLinearColor(
            FMath::RandRange(0.1f, 0.9f),
            FMath::RandRange(0.05f, 0.7f),
            FMath::RandRange(0.0f, 0.5f),
            1.0f
        );
        break;
    }
    
    // Generate eye color
    float EyeColorRand = FMath::RandRange(0.0f, 1.0f);
    if (EyeColorRand < 0.4f) // Brown eyes (most common)
    {
        NewCustomization.EyeColor = FLinearColor(0.3f, 0.15f, 0.05f, 1.0f);
    }
    else if (EyeColorRand < 0.7f) // Blue eyes
    {
        NewCustomization.EyeColor = FLinearColor(0.1f, 0.3f, 0.8f, 1.0f);
    }
    else if (EyeColorRand < 0.85f) // Green eyes
    {
        NewCustomization.EyeColor = FLinearColor(0.2f, 0.6f, 0.3f, 1.0f);
    }
    else // Hazel eyes
    {
        NewCustomization.EyeColor = FLinearColor(0.4f, 0.3f, 0.1f, 1.0f);
    }
    
    // Set quality tier based on archetype importance
    switch (BaseArchetype)
    {
    case ECharacterArchetype::Protagonist:
        NewCustomization.QualityTier = ECharacterQualityTier::Hero;
        break;
    case ECharacterArchetype::Scientist:
    case ECharacterArchetype::Survivor:
        NewCustomization.QualityTier = ECharacterQualityTier::Standard;
        break;
    default:
        NewCustomization.QualityTier = ECharacterQualityTier::Background;
        break;
    }
    
    // Generate unique seed
    NewCustomization.CustomizationSeed = FMath::RandRange(1, 999999);
    NewCustomization.bUseRandomVariation = true;
    
    // Ensure uniqueness
    int32 Attempts = 0;
    while (!ValidateCharacterUniqueness(NewCustomization) && Attempts < 50)
    {
        // Modify features slightly to ensure uniqueness
        AddVariationToFeatures(NewCustomization.FacialFeatures, 0.1f);
        NewCustomization.CustomizationSeed = FMath::RandRange(1, 999999);
        Attempts++;
    }
    
    if (Attempts >= 50)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not generate unique character after 50 attempts"));
    }
    
    return NewCustomization;
}

bool UCharacterDiversityManager::ValidateCharacterUniqueness(const FMetaHumanCustomization& Customization)
{
    for (const FMetaHumanCustomization& ExistingChar : GeneratedCharacters)
    {
        float Similarity = CalculateFacialSimilarity(Customization.FacialFeatures, ExistingChar.FacialFeatures);
        
        // Also check basic demographics
        if (ExistingChar.Gender == Customization.Gender &&
            ExistingChar.SkinTone == Customization.SkinTone &&
            Similarity > MaximumAllowedSimilarity)
        {
            return false; // Too similar
        }
    }
    
    return true; // Unique enough
}

void UCharacterDiversityManager::RegisterCharacter(ACharacter* Character, const FMetaHumanCustomization& Customization)
{
    if (!Character) return;
    
    ActiveCharacters.Add(Character, Customization);
    GeneratedCharacters.Add(Customization);
    
    // Update diversity stats
    DiversityStats.TotalCharacters++;
    DiversityStats.GenderCounts[Customization.Gender]++;
    DiversityStats.SkinToneCounts[Customization.SkinTone]++;
    
    UE_LOG(LogTemp, Log, TEXT("Registered character. Total: %d"), DiversityStats.TotalCharacters);
}

void UCharacterDiversityManager::UnregisterCharacter(ACharacter* Character)
{
    if (!Character) return;
    
    if (FMetaHumanCustomization* Customization = ActiveCharacters.Find(Character))
    {
        DiversityStats.TotalCharacters--;
        DiversityStats.GenderCounts[Customization->Gender]--;
        DiversityStats.SkinToneCounts[Customization->SkinTone]--;
        
        ActiveCharacters.Remove(Character);
    }
}

FCharacterDiversityStats UCharacterDiversityManager::GetDiversityStats() const
{
    return DiversityStats;
}

FString UCharacterDiversityManager::GenerateUniqueName(ECharacterGender Gender, const FString& CulturalBackground)
{
    FString GeneratedName;
    int32 Attempts = 0;
    
    do
    {
        GeneratedName = GenerateRandomName(Gender);
        Attempts++;
    }
    while (!IsNameAvailable(GeneratedName) && Attempts < 100);
    
    if (Attempts >= 100)
    {
        // Fallback: add number suffix
        GeneratedName = GenerateRandomName(Gender) + FString::Printf(TEXT(" %d"), FMath::RandRange(1, 999));
    }
    
    DiversityStats.UsedCharacterNames.Add(GeneratedName);
    return GeneratedName;
}

bool UCharacterDiversityManager::IsNameAvailable(const FString& Name) const
{
    return !DiversityStats.UsedCharacterNames.Contains(Name);
}

FFacialFeatureSet UCharacterDiversityManager::GenerateUniqueFacialFeatures(ESkinTone SkinTone, ECharacterGender Gender)
{
    FFacialFeatureSet Features;
    
    // Generate base features with some demographic tendencies
    Features.EyeShape = FMath::RandRange(0.1f, 0.9f);
    Features.EyeSize = FMath::RandRange(0.3f, 0.8f);
    Features.EyeDistance = FMath::RandRange(0.4f, 0.7f);
    
    Features.NoseWidth = FMath::RandRange(0.2f, 0.8f);
    Features.NoseLength = FMath::RandRange(0.3f, 0.8f);
    
    Features.MouthWidth = FMath::RandRange(0.3f, 0.8f);
    Features.LipThickness = FMath::RandRange(0.2f, 0.8f);
    
    Features.JawWidth = FMath::RandRange(0.3f, 0.8f);
    Features.CheekboneHeight = FMath::RandRange(0.2f, 0.9f);
    Features.ForeheadHeight = FMath::RandRange(0.3f, 0.8f);
    
    // Add subtle asymmetry for realism
    Features.AsymmetryFactor = FMath::RandRange(0.02f, 0.15f);
    
    // Gender-based adjustments
    if (Gender == ECharacterGender::Male)
    {
        Features.JawWidth += 0.1f;
        Features.LipThickness -= 0.05f;
    }
    else
    {
        Features.EyeSize += 0.05f;
        Features.LipThickness += 0.05f;
    }
    
    // Clamp all values
    Features.EyeShape = FMath::Clamp(Features.EyeShape, 0.0f, 1.0f);
    Features.EyeSize = FMath::Clamp(Features.EyeSize, 0.0f, 1.0f);
    Features.EyeDistance = FMath::Clamp(Features.EyeDistance, 0.0f, 1.0f);
    Features.NoseWidth = FMath::Clamp(Features.NoseWidth, 0.0f, 1.0f);
    Features.NoseLength = FMath::Clamp(Features.NoseLength, 0.0f, 1.0f);
    Features.MouthWidth = FMath::Clamp(Features.MouthWidth, 0.0f, 1.0f);
    Features.LipThickness = FMath::Clamp(Features.LipThickness, 0.0f, 1.0f);
    Features.JawWidth = FMath::Clamp(Features.JawWidth, 0.0f, 1.0f);
    Features.CheekboneHeight = FMath::Clamp(Features.CheekboneHeight, 0.0f, 1.0f);
    Features.ForeheadHeight = FMath::Clamp(Features.ForeheadHeight, 0.0f, 1.0f);
    
    return Features;
}

float UCharacterDiversityManager::CalculateFacialSimilarity(const FFacialFeatureSet& Features1, const FFacialFeatureSet& Features2)
{
    float TotalDifference = 0.0f;
    int32 FeatureCount = 0;
    
    TotalDifference += FMath::Abs(Features1.EyeShape - Features2.EyeShape);
    TotalDifference += FMath::Abs(Features1.EyeSize - Features2.EyeSize);
    TotalDifference += FMath::Abs(Features1.EyeDistance - Features2.EyeDistance);
    TotalDifference += FMath::Abs(Features1.NoseWidth - Features2.NoseWidth);
    TotalDifference += FMath::Abs(Features1.NoseLength - Features2.NoseLength);
    TotalDifference += FMath::Abs(Features1.MouthWidth - Features2.MouthWidth);
    TotalDifference += FMath::Abs(Features1.LipThickness - Features2.LipThickness);
    TotalDifference += FMath::Abs(Features1.JawWidth - Features2.JawWidth);
    TotalDifference += FMath::Abs(Features1.CheekboneHeight - Features2.CheekboneHeight);
    TotalDifference += FMath::Abs(Features1.ForeheadHeight - Features2.ForeheadHeight);
    
    FeatureCount = 10;
    
    float AverageDifference = TotalDifference / FeatureCount;
    float Similarity = 1.0f - AverageDifference; // Convert difference to similarity
    
    return FMath::Clamp(Similarity, 0.0f, 1.0f);
}

void UCharacterDiversityManager::EnsureMinimumVariation(TArray<FMetaHumanCustomization>& Characters)
{
    for (int32 i = 0; i < Characters.Num(); i++)
    {
        for (int32 j = i + 1; j < Characters.Num(); j++)
        {
            float Similarity = CalculateFacialSimilarity(Characters[i].FacialFeatures, Characters[j].FacialFeatures);
            
            if (Similarity > MaximumAllowedSimilarity)
            {
                // Add variation to the second character
                AddVariationToFeatures(Characters[j].FacialFeatures, 0.2f);
            }
        }
    }
}

void UCharacterDiversityManager::FixDiversityImbalance()
{
    // Check gender balance
    int32 MaleCount = DiversityStats.GenderCounts[ECharacterGender::Male];
    int32 FemaleCount = DiversityStats.GenderCounts[ECharacterGender::Female];
    
    if (DiversityStats.TotalCharacters > 0)
    {
        float FemaleRatio = static_cast<float>(FemaleCount) / DiversityStats.TotalCharacters;
        
        if (FMath::Abs(FemaleRatio - TargetFemaleRatio) > 0.2f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Gender imbalance detected. Female ratio: %.2f, Target: %.2f"), 
                   FemaleRatio, TargetFemaleRatio);
        }
    }
}

ECharacterGender UCharacterDiversityManager::SelectBalancedGender()
{
    if (DiversityStats.TotalCharacters == 0)
    {
        return FMath::RandBool() ? ECharacterGender::Male : ECharacterGender::Female;
    }
    
    int32 MaleCount = DiversityStats.GenderCounts[ECharacterGender::Male];
    int32 FemaleCount = DiversityStats.GenderCounts[ECharacterGender::Female];
    
    float CurrentFemaleRatio = static_cast<float>(FemaleCount) / DiversityStats.TotalCharacters;
    
    // If we're below target female ratio, prefer female
    if (CurrentFemaleRatio < TargetFemaleRatio - 0.1f)
    {
        return ECharacterGender::Female;
    }
    // If we're above target female ratio, prefer male
    else if (CurrentFemaleRatio > TargetFemaleRatio + 0.1f)
    {
        return ECharacterGender::Male;
    }
    // Otherwise random
    else
    {
        return FMath::RandBool() ? ECharacterGender::Male : ECharacterGender::Female;
    }
}

ESkinTone UCharacterDiversityManager::SelectBalancedSkinTone()
{
    // For now, use weighted random selection
    // In a real implementation, this would balance against targets
    
    TArray<ESkinTone> SkinTones = {
        ESkinTone::VeryPale,
        ESkinTone::Pale,
        ESkinTone::Light,
        ESkinTone::Medium,
        ESkinTone::Olive,
        ESkinTone::Tan,
        ESkinTone::Dark,
        ESkinTone::VeryDark
    };
    
    // Weighted selection (adjust weights for desired distribution)
    TArray<float> Weights = { 0.1f, 0.15f, 0.2f, 0.2f, 0.15f, 0.1f, 0.05f, 0.05f };
    
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    float CumulativeWeight = 0.0f;
    
    for (int32 i = 0; i < SkinTones.Num(); i++)
    {
        CumulativeWeight += Weights[i];
        if (RandomValue <= CumulativeWeight)
        {
            return SkinTones[i];
        }
    }
    
    return ESkinTone::Medium; // Fallback
}

void UCharacterDiversityManager::AddVariationToFeatures(FFacialFeatureSet& Features, float VariationAmount)
{
    Features.EyeShape = FMath::Clamp(Features.EyeShape + FMath::RandRange(-VariationAmount, VariationAmount), 0.0f, 1.0f);
    Features.EyeSize = FMath::Clamp(Features.EyeSize + FMath::RandRange(-VariationAmount, VariationAmount), 0.0f, 1.0f);
    Features.NoseWidth = FMath::Clamp(Features.NoseWidth + FMath::RandRange(-VariationAmount, VariationAmount), 0.0f, 1.0f);
    Features.MouthWidth = FMath::Clamp(Features.MouthWidth + FMath::RandRange(-VariationAmount, VariationAmount), 0.0f, 1.0f);
    Features.JawWidth = FMath::Clamp(Features.JawWidth + FMath::RandRange(-VariationAmount, VariationAmount), 0.0f, 1.0f);
}

bool UCharacterDiversityManager::IsTooSimilarToExisting(const FFacialFeatureSet& Features)
{
    for (const FMetaHumanCustomization& ExistingChar : GeneratedCharacters)
    {
        float Similarity = CalculateFacialSimilarity(Features, ExistingChar.FacialFeatures);
        if (Similarity > MaximumAllowedSimilarity)
        {
            return true;
        }
    }
    return false;
}

FString UCharacterDiversityManager::GenerateRandomName(ECharacterGender Gender)
{
    FString FirstName;
    
    if (Gender == ECharacterGender::Male && MaleNames.Num() > 0)
    {
        FirstName = MaleNames[FMath::RandRange(0, MaleNames.Num() - 1)];
    }
    else if (Gender == ECharacterGender::Female && FemaleNames.Num() > 0)
    {
        FirstName = FemaleNames[FMath::RandRange(0, FemaleNames.Num() - 1)];
    }
    else
    {
        // Fallback names
        FirstName = FString::Printf(TEXT("Person%d"), FMath::RandRange(1, 999));
    }
    
    FString LastName;
    if (Surnames.Num() > 0)
    {
        LastName = Surnames[FMath::RandRange(0, Surnames.Num() - 1)];
    }
    else
    {
        LastName = FString::Printf(TEXT("Survivor%d"), FMath::RandRange(1, 999));
    }
    
    return FirstName + TEXT(" ") + LastName;
}

void UCharacterDiversityManager::InitializeNameDatabases()
{
    // Initialize with some basic names
    MaleNames = {
        TEXT("Alex"), TEXT("Marcus"), TEXT("David"), TEXT("James"), TEXT("Robert"),
        TEXT("Michael"), TEXT("William"), TEXT("Richard"), TEXT("Thomas"), TEXT("Christopher"),
        TEXT("Daniel"), TEXT("Matthew"), TEXT("Anthony"), TEXT("Mark"), TEXT("Donald"),
        TEXT("Steven"), TEXT("Paul"), TEXT("Andrew"), TEXT("Joshua"), TEXT("Kenneth")
    };
    
    FemaleNames = {
        TEXT("Sarah"), TEXT("Jennifer"), TEXT("Lisa"), TEXT("Karen"), TEXT("Nancy"),
        TEXT("Betty"), TEXT("Helen"), TEXT("Sandra"), TEXT("Donna"), TEXT("Carol"),
        TEXT("Ruth"), TEXT("Sharon"), TEXT("Michelle"), TEXT("Laura"), TEXT("Emily"),
        TEXT("Kimberly"), TEXT("Deborah"), TEXT("Dorothy"), TEXT("Amy"), TEXT("Angela")
    };
    
    Surnames = {
        TEXT("Smith"), TEXT("Johnson"), TEXT("Williams"), TEXT("Brown"), TEXT("Jones"),
        TEXT("Garcia"), TEXT("Miller"), TEXT("Davis"), TEXT("Rodriguez"), TEXT("Martinez"),
        TEXT("Hernandez"), TEXT("Lopez"), TEXT("Gonzalez"), TEXT("Wilson"), TEXT("Anderson"),
        TEXT("Thomas"), TEXT("Taylor"), TEXT("Moore"), TEXT("Jackson"), TEXT("Martin")
    };
}

void UCharacterDiversityManager::InitializeDiversityTargets()
{
    // Initialize target skin tone distribution
    TargetSkinToneDistribution.Add(ESkinTone::VeryPale, 0.08f);
    TargetSkinToneDistribution.Add(ESkinTone::Pale, 0.15f);
    TargetSkinToneDistribution.Add(ESkinTone::Light, 0.20f);
    TargetSkinToneDistribution.Add(ESkinTone::Medium, 0.22f);
    TargetSkinToneDistribution.Add(ESkinTone::Olive, 0.15f);
    TargetSkinToneDistribution.Add(ESkinTone::Tan, 0.10f);
    TargetSkinToneDistribution.Add(ESkinTone::Dark, 0.06f);
    TargetSkinToneDistribution.Add(ESkinTone::VeryDark, 0.04f);
}

void UCharacterDiversityManager::LoadNamesFromDataTable()
{
    // TODO: Load names from data table assets for better localization support
    UE_LOG(LogTemp, Log, TEXT("Name database loading from data tables not yet implemented"));
}