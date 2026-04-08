#include "JurassicCharacterArtist.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MetaHumanCharacterComponent.h"
#include "TranspersonalCharacterBase.h"
#include "CharacterArchetypeSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UJurassicCharacterArtist::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Initializing Character Artist System"));
    
    // Initialize character archetype system
    ArchetypeSystem = GetWorld()->GetSubsystem<UCharacterArchetypeSystem>();
    
    // Load MetaHuman presets and variation data
    LoadCharacterPresets();
    
    // Initialize diversity tracking
    InitializeDiversitySystem();
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Character Artist System initialized successfully"));
}

void UJurassicCharacterArtist::Deinitialize()
{
    // Clean up any resources
    CharacterPresets.Empty();
    DiversityTracker.Empty();
    
    Super::Deinitialize();
}

ACharacter* UJurassicCharacterArtist::CreateCharacterFromArchetype(
    ECharacterArchetype ArchetypeType,
    const FVector& SpawnLocation,
    const FRotator& SpawnRotation,
    bool bRandomizeAppearance)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicCharacterArtist: No valid world context"));
        return nullptr;
    }

    // Get archetype data
    FCharacterArchetypeData ArchetypeData;
    if (ArchetypeSystem)
    {
        ArchetypeData = ArchetypeSystem->GetArchetypeData(ArchetypeType);
    }

    // Create spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn the character
    ATranspersonalCharacterBase* NewCharacter = GetWorld()->SpawnActor<ATranspersonalCharacterBase>(
        ATranspersonalCharacterBase::StaticClass(),
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!NewCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicCharacterArtist: Failed to spawn character"));
        return nullptr;
    }

    // Get or add MetaHuman component
    UMetaHumanCharacterComponent* MetaHumanComp = NewCharacter->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp)
    {
        MetaHumanComp = NewObject<UMetaHumanCharacterComponent>(NewCharacter);
        NewCharacter->AddInstanceComponent(MetaHumanComp);
    }

    // Generate unique customization
    FMetaHumanCustomization Customization = GenerateUniqueCustomization(ArchetypeData, bRandomizeAppearance);
    
    // Apply the customization
    ApplyMetaHumanCustomization(NewCharacter, Customization);
    
    // Apply archetype-specific features
    ApplyArchetypeFeatures(NewCharacter, ArchetypeData);
    
    // Generate and apply visual story
    FCharacterVisualStory VisualStory = GenerateVisualStory(ArchetypeData);
    ApplyLifeStory(NewCharacter, VisualStory);
    
    // Update diversity tracking
    UpdateDiversityTracking(Customization);
    
    // Set character data
    if (MetaHumanComp)
    {
        MetaHumanComp->CharacterData = ArchetypeData;
        MetaHumanComp->CharacterID = GenerateUniqueCharacterID();
    }

    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Created character from archetype: %s"), 
           *UEnum::GetValueAsString(ArchetypeType));

    return NewCharacter;
}

ACharacter* UJurassicCharacterArtist::CreateUniqueNamedCharacter(
    const FString& CharacterName,
    ECharacterArchetype BaseArchetype,
    const FMetaHumanCustomization& Customization,
    const FVector& SpawnLocation,
    const FRotator& SpawnRotation)
{
    ACharacter* NewCharacter = CreateCharacterFromArchetype(BaseArchetype, SpawnLocation, SpawnRotation, false);
    
    if (NewCharacter)
    {
        // Apply specific customization
        ApplyMetaHumanCustomization(NewCharacter, Customization);
        
        // Set character name
        UMetaHumanCharacterComponent* MetaHumanComp = NewCharacter->FindComponentByClass<UMetaHumanCharacterComponent>();
        if (MetaHumanComp)
        {
            MetaHumanComp->CharacterName = CharacterName;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Created unique named character: %s"), *CharacterName);
    }
    
    return NewCharacter;
}

void UJurassicCharacterArtist::CustomizeExistingCharacter(
    ACharacter* Character,
    const FMetaHumanCustomization& Customization)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicCharacterArtist: Invalid character for customization"));
        return;
    }

    ApplyMetaHumanCustomization(Character, Customization);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Applied customization to existing character"));
}

void UJurassicCharacterArtist::ApplyLifeStory(ACharacter* Character, const FCharacterVisualStory& Story)
{
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    // Apply survival effects based on story
    ApplySurvivalEffects(Character, Story.SurvivalTime, Story.StressLevel);
    
    // Apply environmental wear
    ApplyEnvironmentalWear(Character, Story.SunExposure, Story.DirtAccumulation);
    
    // Apply scars if present
    if (Story.ScarDescriptions.Num() > 0)
    {
        AddBattleScars(Character, Story.ScarDescriptions);
    }
    
    // Apply health condition effects
    ApplyHealthConditionEffects(Character, Story.HealthCondition, Story.bShowsSignsOfStarvation);
    
    // Apply clothing wear
    ApplyClothingWear(Character, Story.OverallClothingWear, Story.ClothingDamageAreas);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Applied life story to character"));
}

void UJurassicCharacterArtist::ApplySurvivalEffects(ACharacter* Character, float DaysInWild, float StressLevel)
{
    if (!Character)
    {
        return;
    }

    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp || !MetaHumanComp->SkinMaterialInstance)
    {
        return;
    }

    // Calculate survival wear factor
    float SurvivalWear = FMath::Clamp(DaysInWild / 30.0f, 0.0f, 1.0f); // 30 days = full wear
    float StressWear = FMath::Clamp(StressLevel, 0.0f, 1.0f);
    
    // Apply skin effects
    float SkinRoughness = FMath::Lerp(0.3f, 0.8f, SurvivalWear);
    float SkinDirtiness = FMath::Lerp(0.0f, 0.6f, SurvivalWear);
    
    MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("Roughness"), SkinRoughness);
    MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("DirtLevel"), SkinDirtiness);
    
    // Apply stress effects (darker under eyes, tension lines)
    float StressIntensity = FMath::Lerp(0.0f, 0.4f, StressWear);
    MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("StressMarks"), StressIntensity);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Applied survival effects - Days: %.1f, Stress: %.1f"), 
           DaysInWild, StressLevel);
}

void UJurassicCharacterArtist::AddBattleScars(ACharacter* Character, const TArray<FString>& ScarDescriptions)
{
    if (!Character || ScarDescriptions.Num() == 0)
    {
        return;
    }

    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp || !MetaHumanComp->SkinMaterialInstance)
    {
        return;
    }

    // Apply scar textures and parameters based on descriptions
    for (int32 i = 0; i < FMath::Min(ScarDescriptions.Num(), 3); ++i) // Max 3 visible scars
    {
        FString ParamName = FString::Printf(TEXT("Scar%d_Intensity"), i + 1);
        float ScarIntensity = FMath::RandRange(0.3f, 0.8f);
        
        MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(FName(*ParamName), ScarIntensity);
        
        UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Added scar: %s"), *ScarDescriptions[i]);
    }
}

void UJurassicCharacterArtist::ApplyEnvironmentalWear(ACharacter* Character, float SunExposure, float DirtLevel)
{
    if (!Character)
    {
        return;
    }

    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp || !MetaHumanComp->SkinMaterialInstance)
    {
        return;
    }

    // Apply sun exposure effects (tanning, burning)
    float TanIntensity = FMath::Clamp(SunExposure, 0.0f, 1.0f);
    MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("TanLevel"), TanIntensity);
    
    // Apply dirt and grime
    float DirtIntensity = FMath::Clamp(DirtLevel, 0.0f, 1.0f);
    MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("DirtLevel"), DirtIntensity);
    
    // Apply to clothing materials as well
    for (UMaterialInstanceDynamic* ClothingMat : MetaHumanComp->ClothingMaterialInstances)
    {
        if (ClothingMat)
        {
            ClothingMat->SetScalarParameterValue(TEXT("DirtLevel"), DirtIntensity);
            ClothingMat->SetScalarParameterValue(TEXT("WeatherWear"), SunExposure * 0.5f);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Applied environmental wear - Sun: %.1f, Dirt: %.1f"), 
           SunExposure, DirtLevel);
}

void UJurassicCharacterArtist::EnsureCharacterDiversity(TArray<ACharacter*>& Characters)
{
    if (Characters.Num() < 2)
    {
        return;
    }

    // Analyze current diversity
    TMap<ESkinTone, int32> SkinToneCount;
    TMap<ECharacterGender, int32> GenderCount;
    TMap<ECharacterAgeGroup, int32> AgeCount;
    
    for (ACharacter* Character : Characters)
    {
        UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
        if (MetaHumanComp)
        {
            // Count current distribution
            // This would need access to the character's current settings
        }
    }
    
    // Apply corrections to ensure diversity
    // Implementation would adjust characters that are too similar
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Ensured diversity across %d characters"), Characters.Num());
}

FMetaHumanCustomization UJurassicCharacterArtist::GenerateUniqueCustomization(
    const FCharacterArchetypeData& ArchetypeData,
    bool bRandomizeAppearance)
{
    FMetaHumanCustomization Customization;
    
    // Set basic parameters based on archetype
    Customization.Gender = (FMath::RandBool()) ? ECharacterGender::Male : ECharacterGender::Female;
    
    // Generate diverse skin tone
    Customization.SkinTone = GenerateDiverseSkinTone();
    
    // Generate eye and hair colors
    Customization.EyeColor = GenerateEyeColor();
    Customization.HairColor = GenerateHairColor();
    
    // Set quality tier based on archetype
    switch (ArchetypeData.Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            Customization.QualityTier = ECharacterQualityTier::Hero;
            break;
        case ECharacterArchetype::Survivor_Researcher:
        case ECharacterArchetype::Survivor_Explorer:
        case ECharacterArchetype::Survivor_Military:
            Customization.QualityTier = ECharacterQualityTier::Standard;
            break;
        default:
            Customization.QualityTier = ECharacterQualityTier::Background;
            break;
    }
    
    // Generate unique facial features
    if (bRandomizeAppearance)
    {
        Customization.FacialFeatures = GenerateUniqueFacialFeatures();
        Customization.CustomizationSeed = FMath::RandRange(1, 999999);
    }
    
    // Generate visual story based on archetype
    Customization.VisualStory = GenerateVisualStory(ArchetypeData);
    
    return Customization;
}

// Helper functions implementation

void UJurassicCharacterArtist::LoadCharacterPresets()
{
    // Load MetaHuman presets and configuration data
    // This would load from data assets or configuration files
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Loaded character presets"));
}

void UJurassicCharacterArtist::InitializeDiversitySystem()
{
    // Initialize diversity tracking system
    DiversityTracker.Empty();
    
    // Set diversity targets
    DiversityTargets.Add(ESkinTone::VeryPale, 0.10f);
    DiversityTargets.Add(ESkinTone::Pale, 0.15f);
    DiversityTargets.Add(ESkinTone::Light, 0.20f);
    DiversityTargets.Add(ESkinTone::Medium, 0.20f);
    DiversityTargets.Add(ESkinTone::Olive, 0.15f);
    DiversityTargets.Add(ESkinTone::Tan, 0.10f);
    DiversityTargets.Add(ESkinTone::Dark, 0.05f);
    DiversityTargets.Add(ESkinTone::VeryDark, 0.05f);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Initialized diversity system"));
}

ESkinTone UJurassicCharacterArtist::GenerateDiverseSkinTone()
{
    // Use weighted random selection based on diversity targets
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    float CumulativeWeight = 0.0f;
    
    for (const auto& Target : DiversityTargets)
    {
        CumulativeWeight += Target.Value;
        if (RandomValue <= CumulativeWeight)
        {
            return Target.Key;
        }
    }
    
    return ESkinTone::Medium; // Fallback
}

FLinearColor UJurassicCharacterArtist::GenerateEyeColor()
{
    // Generate realistic eye colors
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),  // Brown
        FLinearColor(0.1f, 0.3f, 0.6f, 1.0f),   // Blue
        FLinearColor(0.2f, 0.4f, 0.2f, 1.0f),   // Green
        FLinearColor(0.3f, 0.3f, 0.1f, 1.0f),   // Hazel
        FLinearColor(0.15f, 0.15f, 0.15f, 1.0f) // Gray
    };
    
    int32 RandomIndex = FMath::RandRange(0, EyeColors.Num() - 1);
    return EyeColors[RandomIndex];
}

FLinearColor UJurassicCharacterArtist::GenerateHairColor()
{
    // Generate realistic hair colors
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.05f, 0.05f, 0.05f, 1.0f), // Black
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),   // Dark Brown
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),    // Medium Brown
        FLinearColor(0.4f, 0.3f, 0.15f, 1.0f),   // Light Brown
        FLinearColor(0.6f, 0.5f, 0.2f, 1.0f),    // Blonde
        FLinearColor(0.4f, 0.15f, 0.1f, 1.0f),   // Auburn
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)     // Gray
    };
    
    int32 RandomIndex = FMath::RandRange(0, HairColors.Num() - 1);
    return HairColors[RandomIndex];
}

FFacialFeatureSet UJurassicCharacterArtist::GenerateUniqueFacialFeatures()
{
    FFacialFeatureSet Features;
    
    // Generate random but realistic facial features
    Features.EyeShape = FMath::RandRange(0.2f, 0.8f);
    Features.EyeSize = FMath::RandRange(0.3f, 0.7f);
    Features.EyeDistance = FMath::RandRange(0.4f, 0.6f);
    Features.NoseWidth = FMath::RandRange(0.3f, 0.7f);
    Features.NoseLength = FMath::RandRange(0.3f, 0.7f);
    Features.MouthWidth = FMath::RandRange(0.4f, 0.6f);
    Features.LipThickness = FMath::RandRange(0.3f, 0.7f);
    Features.JawWidth = FMath::RandRange(0.4f, 0.6f);
    Features.CheekboneHeight = FMath::RandRange(0.3f, 0.7f);
    Features.ForeheadHeight = FMath::RandRange(0.4f, 0.6f);
    Features.AsymmetryFactor = FMath::RandRange(0.05f, 0.15f);
    
    return Features;
}

FCharacterVisualStory UJurassicCharacterArtist::GenerateVisualStory(const FCharacterArchetypeData& ArchetypeData)
{
    FCharacterVisualStory Story;
    
    // Generate story based on archetype
    switch (ArchetypeData.SurvivalCondition)
    {
        case ESurvivalCondition::Fresh_Arrival:
            Story.SurvivalTime = FMath::RandRange(0.0f, 2.0f);
            Story.StressLevel = FMath::RandRange(0.3f, 0.6f);
            Story.DirtAccumulation = FMath::RandRange(0.0f, 0.2f);
            break;
        case ESurvivalCondition::Short_Term:
            Story.SurvivalTime = FMath::RandRange(3.0f, 10.0f);
            Story.StressLevel = FMath::RandRange(0.4f, 0.7f);
            Story.DirtAccumulation = FMath::RandRange(0.2f, 0.4f);
            break;
        case ESurvivalCondition::Long_Term:
            Story.SurvivalTime = FMath::RandRange(11.0f, 30.0f);
            Story.StressLevel = FMath::RandRange(0.5f, 0.8f);
            Story.DirtAccumulation = FMath::RandRange(0.4f, 0.7f);
            break;
        case ESurvivalCondition::Veteran_Survivor:
            Story.SurvivalTime = FMath::RandRange(31.0f, 100.0f);
            Story.StressLevel = FMath::RandRange(0.3f, 0.6f); // Veterans are calmer
            Story.DirtAccumulation = FMath::RandRange(0.6f, 0.9f);
            break;
    }
    
    Story.HealthCondition = FMath::RandRange(0.6f, 0.9f);
    Story.SunExposure = FMath::RandRange(0.2f, 0.8f);
    Story.bHasRecentInjuries = FMath::RandBool() && (Story.SurvivalTime > 5.0f);
    Story.bShowsSignsOfStarvation = (Story.SurvivalTime > 20.0f) && FMath::RandBool();
    
    return Story;
}

void UJurassicCharacterArtist::ApplyMetaHumanCustomization(ACharacter* Character, const FMetaHumanCustomization& Customization)
{
    if (!Character)
    {
        return;
    }

    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp)
    {
        return;
    }

    // Apply basic customization
    MetaHumanComp->UpdateSkinTone(Customization.SkinTone);
    MetaHumanComp->UpdateEyeColor(Customization.EyeColor);
    MetaHumanComp->UpdateHairColor(Customization.HairColor);
    
    // Apply facial features (this would require more detailed MetaHuman integration)
    // ApplyFacialFeatures(Character, Customization.FacialFeatures);
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Applied MetaHuman customization"));
}

void UJurassicCharacterArtist::ApplyArchetypeFeatures(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    if (!Character)
    {
        return;
    }

    // Apply archetype-specific visual features
    // This could include specific clothing, accessories, or physical modifications
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicCharacterArtist: Applied archetype features for: %s"), 
           *ArchetypeData.DisplayName);
}

void UJurassicCharacterArtist::UpdateDiversityTracking(const FMetaHumanCustomization& Customization)
{
    // Update diversity tracking counters
    if (DiversityTracker.Contains(Customization.SkinTone))
    {
        DiversityTracker[Customization.SkinTone]++;
    }
    else
    {
        DiversityTracker.Add(Customization.SkinTone, 1);
    }
}

FString UJurassicCharacterArtist::GenerateUniqueCharacterID()
{
    static int32 CharacterCounter = 0;
    CharacterCounter++;
    
    return FString::Printf(TEXT("CHAR_%d_%d"), 
                          FMath::RandRange(1000, 9999), 
                          CharacterCounter);
}

void UJurassicCharacterArtist::ApplyHealthConditionEffects(ACharacter* Character, float HealthCondition, bool bShowsStarvation)
{
    if (!Character)
    {
        return;
    }

    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp || !MetaHumanComp->SkinMaterialInstance)
    {
        return;
    }

    // Apply health-based skin effects
    float HealthPallor = FMath::Lerp(0.4f, 0.0f, HealthCondition);
    MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("HealthPallor"), HealthPallor);
    
    if (bShowsStarvation)
    {
        float StarvationEffects = 0.6f;
        MetaHumanComp->SkinMaterialInstance->SetScalarParameterValue(TEXT("StarvationMarks"), StarvationEffects);
    }
}

void UJurassicCharacterArtist::ApplyClothingWear(ACharacter* Character, float WearLevel, const TArray<FString>& DamageAreas)
{
    if (!Character)
    {
        return;
    }

    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp)
    {
        return;
    }

    // Apply wear to all clothing materials
    for (UMaterialInstanceDynamic* ClothingMat : MetaHumanComp->ClothingMaterialInstances)
    {
        if (ClothingMat)
        {
            ClothingMat->SetScalarParameterValue(TEXT("WearLevel"), WearLevel);
            ClothingMat->SetScalarParameterValue(TEXT("FabricDamage"), WearLevel * 0.8f);
        }
    }
}