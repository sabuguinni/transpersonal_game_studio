#include "MetaHumanCharacterGenerator.h"
#include "CharacterArchetypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UMetaHumanCharacterGenerator::UMetaHumanCharacterGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    RandomSeed = FMath::RandRange(1000, 999999);
    bEnableDebugLogging = true;
}

void UMetaHumanCharacterGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize random stream with seed
    RandomStream.Initialize(RandomSeed);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("MetaHumanCharacterGenerator initialized with seed: %d"), RandomSeed);
    }
}

bool UMetaHumanCharacterGenerator::GenerateCharacterFromArchetype(const FCharacterArchetypeData& ArchetypeData, USkeletalMeshComponent* TargetMeshComponent)
{
    if (!TargetMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("GenerateCharacterFromArchetype: TargetMeshComponent is null"));
        return false;
    }

    // Generate variation parameters based on archetype
    FMetaHumanVariationParams VariationParams = GenerateVariationParams(ArchetypeData);
    
    // Generate clothing configuration
    FCharacterClothingConfig ClothingConfig = GenerateClothingConfig(ArchetypeData);
    
    // Apply variations to MetaHuman
    bool bVariationSuccess = ApplyVariationToMetaHuman(TargetMeshComponent, VariationParams);
    bool bClothingSuccess = ApplyClothingConfig(TargetMeshComponent, ClothingConfig);
    
    if (bEnableDebugLogging)
    {
        FString CharacterID = GenerateUniqueCharacterID(ArchetypeData);
        UE_LOG(LogTemp, Log, TEXT("Generated character %s - Variation: %s, Clothing: %s"), 
            *CharacterID, 
            bVariationSuccess ? TEXT("Success") : TEXT("Failed"),
            bClothingSuccess ? TEXT("Success") : TEXT("Failed"));
    }
    
    return bVariationSuccess && bClothingSuccess;
}

FMetaHumanVariationParams UMetaHumanCharacterGenerator::GenerateVariationParams(const FCharacterArchetypeData& ArchetypeData)
{
    FMetaHumanVariationParams Params;
    
    // Base random variations
    Params.FaceVariation = RandomStream.FRandRange(0.0f, 1.0f);
    Params.BodyVariation = RandomStream.FRandRange(0.0f, 1.0f);
    Params.SkinTone = RandomStream.FRandRange(0.1f, 0.9f);
    Params.HairColor = RandomStream.FRandRange(0.0f, 1.0f);
    Params.EyeColor = RandomStream.FRandRange(0.0f, 1.0f);
    
    // Apply archetype-specific modifications
    ApplyAgeModifications(Params, ArchetypeData.AgeGroup);
    ApplyBuildModifications(Params, ArchetypeData.Build);
    ApplySurvivalModifications(Params, ArchetypeData.SurvivalCondition);
    
    // Apply unique features if specified
    if (!ArchetypeData.UniqueFeatures.IsEmpty())
    {
        GenerateUniqueFeatures(Params, ArchetypeData.UniqueFeatures);
    }
    
    return Params;
}

bool UMetaHumanCharacterGenerator::ApplyVariationToMetaHuman(USkeletalMeshComponent* MeshComponent, const FMetaHumanVariationParams& Params)
{
    if (!MeshComponent)
    {
        return false;
    }
    
    // Get or create dynamic material instances for the MetaHuman
    TArray<UMaterialInterface*> Materials = MeshComponent->GetMaterials();
    
    for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); ++MaterialIndex)
    {
        UMaterialInterface* BaseMaterial = Materials[MaterialIndex];
        if (!BaseMaterial)
        {
            continue;
        }
        
        // Create dynamic material instance
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshComponent);
        if (!DynamicMaterial)
        {
            continue;
        }
        
        // Apply variation parameters to material
        DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), Params.SkinTone);
        DynamicMaterial->SetScalarParameterValue(TEXT("Age"), Params.Age);
        DynamicMaterial->SetScalarParameterValue(TEXT("Weight"), Params.Weight);
        DynamicMaterial->SetScalarParameterValue(TEXT("Muscle"), Params.Muscle);
        DynamicMaterial->SetScalarParameterValue(TEXT("WearLevel"), Params.WearLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), Params.DirtLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), Params.ScarIntensity);
        
        // Apply hair and eye color variations
        FLinearColor HairColorValue = FLinearColor::LerpUsingHSV(
            FLinearColor(0.1f, 0.05f, 0.02f), // Dark brown
            FLinearColor(0.9f, 0.8f, 0.6f),   // Blonde
            Params.HairColor
        );
        
        FLinearColor EyeColorValue = FLinearColor::LerpUsingHSV(
            FLinearColor(0.1f, 0.05f, 0.02f), // Dark brown
            FLinearColor(0.2f, 0.6f, 0.9f),   // Blue
            Params.EyeColor
        );
        
        DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), HairColorValue);
        DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), EyeColorValue);
        
        // Set the dynamic material on the mesh
        MeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
    }
    
    // Apply physical variations through morph targets or bone scaling
    ApplyPhysicalVariations(MeshComponent, Params);
    
    return true;
}

void UMetaHumanCharacterGenerator::ApplyPhysicalVariations(USkeletalMeshComponent* MeshComponent, const FMetaHumanVariationParams& Params)
{
    if (!MeshComponent || !MeshComponent->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Apply height scaling
    float HeightScale = FMath::Lerp(0.9f, 1.1f, Params.Height);
    FVector CurrentScale = MeshComponent->GetRelativeScale3D();
    MeshComponent->SetRelativeScale3D(FVector(CurrentScale.X, CurrentScale.Y, CurrentScale.Z * HeightScale));
    
    // Apply morph targets for face and body variations
    USkeletalMesh* SkeletalMesh = MeshComponent->GetSkeletalMeshAsset();
    if (SkeletalMesh)
    {
        // Apply face variation morph targets
        MeshComponent->SetMorphTarget(TEXT("FaceWidth"), FMath::Lerp(-0.5f, 0.5f, Params.FaceVariation));
        MeshComponent->SetMorphTarget(TEXT("JawWidth"), FMath::Lerp(-0.3f, 0.3f, Params.FaceVariation));
        
        // Apply body variation morph targets
        MeshComponent->SetMorphTarget(TEXT("BodyWeight"), FMath::Lerp(-0.4f, 0.4f, Params.Weight));
        MeshComponent->SetMorphTarget(TEXT("BodyMuscle"), FMath::Lerp(-0.3f, 0.5f, Params.Muscle));
        
        // Apply age-related morph targets
        MeshComponent->SetMorphTarget(TEXT("AgeWrinkles"), Params.Age * 0.8f);
        MeshComponent->SetMorphTarget(TEXT("AgeSagging"), Params.Age * 0.6f);
    }
}

FCharacterClothingConfig UMetaHumanCharacterGenerator::GenerateClothingConfig(const FCharacterArchetypeData& ArchetypeData)
{
    FCharacterClothingConfig Config;
    
    // Determine clothing style based on archetype
    switch (ArchetypeData.Archetype)
    {
        case ECharacterArchetype::Protagonist:
            Config.ClothingStyle = TEXT("Paleontologist_Field");
            Config.HasBackpack = true;
            Config.HasHat = true;
            Config.EquipmentLevel = 2;
            break;
            
        case ECharacterArchetype::Survivor_Leader:
            Config.ClothingStyle = TEXT("Military_Tactical");
            Config.HasBackpack = true;
            Config.EquipmentLevel = 3;
            break;
            
        case ECharacterArchetype::Survivor_Medic:
            Config.ClothingStyle = TEXT("Medical_Scrubs");
            Config.HasBackpack = true;
            Config.EquipmentLevel = 2;
            break;
            
        case ECharacterArchetype::Survivor_Engineer:
            Config.ClothingStyle = TEXT("Work_Coveralls");
            Config.HasGloves = true;
            Config.EquipmentLevel = 2;
            break;
            
        case ECharacterArchetype::Survivor_Hunter:
            Config.ClothingStyle = TEXT("Camouflage_Outdoor");
            Config.HasBackpack = true;
            Config.HasGloves = true;
            Config.EquipmentLevel = 2;
            break;
            
        case ECharacterArchetype::Survivor_Scholar:
            Config.ClothingStyle = TEXT("Academic_Casual");
            Config.HasBackpack = true;
            Config.EquipmentLevel = 1;
            break;
            
        case ECharacterArchetype::Survivor_Child:
            Config.ClothingStyle = TEXT("Child_Casual");
            Config.EquipmentLevel = 0;
            break;
            
        case ECharacterArchetype::Survivor_Elder:
            Config.ClothingStyle = TEXT("Elder_Comfortable");
            Config.EquipmentLevel = 1;
            break;
            
        case ECharacterArchetype::Hostile_Raider:
            Config.ClothingStyle = TEXT("Raider_Makeshift");
            Config.HasBackpack = false;
            Config.EquipmentLevel = 1;
            break;
            
        case ECharacterArchetype::Neutral_Trader:
            Config.ClothingStyle = TEXT("Trader_Practical");
            Config.HasBackpack = true;
            Config.EquipmentLevel = 2;
            break;
            
        default:
            Config.ClothingStyle = TEXT("Generic_Survivor");
            Config.EquipmentLevel = 1;
            break;
    }
    
    // Apply survival condition to clothing wear
    switch (ArchetypeData.SurvivalCondition)
    {
        case ESurvivalCondition::Fresh:
            Config.WearLevel = RandomStream.FRandRange(0.0f, 0.2f);
            Config.DirtLevel = RandomStream.FRandRange(0.0f, 0.1f);
            break;
            
        case ESurvivalCondition::Weathered:
            Config.WearLevel = RandomStream.FRandRange(0.2f, 0.5f);
            Config.DirtLevel = RandomStream.FRandRange(0.1f, 0.4f);
            break;
            
        case ESurvivalCondition::Hardened:
            Config.WearLevel = RandomStream.FRandRange(0.4f, 0.7f);
            Config.DirtLevel = RandomStream.FRandRange(0.3f, 0.6f);
            break;
            
        case ESurvivalCondition::Broken:
            Config.WearLevel = RandomStream.FRandRange(0.6f, 1.0f);
            Config.DirtLevel = RandomStream.FRandRange(0.5f, 0.9f);
            break;
    }
    
    return Config;
}

bool UMetaHumanCharacterGenerator::ApplyClothingConfig(USkeletalMeshComponent* MeshComponent, const FCharacterClothingConfig& ClothingConfig)
{
    if (!MeshComponent)
    {
        return false;
    }
    
    // Apply clothing wear and dirt to materials
    TArray<UMaterialInterface*> Materials = MeshComponent->GetMaterials();
    
    for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); ++MaterialIndex)
    {
        UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(MeshComponent->GetMaterial(MaterialIndex));
        if (!DynamicMaterial)
        {
            continue;
        }
        
        // Apply clothing condition parameters
        DynamicMaterial->SetScalarParameterValue(TEXT("ClothingWear"), ClothingConfig.WearLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("ClothingDirt"), ClothingConfig.DirtLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("EquipmentLevel"), float(ClothingConfig.EquipmentLevel) / 3.0f);
        
        // Apply equipment flags
        DynamicMaterial->SetScalarParameterValue(TEXT("HasBackpack"), ClothingConfig.HasBackpack ? 1.0f : 0.0f);
        DynamicMaterial->SetScalarParameterValue(TEXT("HasHat"), ClothingConfig.HasHat ? 1.0f : 0.0f);
        DynamicMaterial->SetScalarParameterValue(TEXT("HasGloves"), ClothingConfig.HasGloves ? 1.0f : 0.0f);
        DynamicMaterial->SetScalarParameterValue(TEXT("HasBoots"), ClothingConfig.HasBoots ? 1.0f : 0.0f);
    }
    
    return true;
}

FString UMetaHumanCharacterGenerator::GenerateUniqueCharacterID(const FCharacterArchetypeData& ArchetypeData)
{
    FString ArchetypeName = UEnum::GetValueAsString(ArchetypeData.Archetype);
    FString AgeGroupName = UEnum::GetValueAsString(ArchetypeData.AgeGroup);
    FString BuildName = UEnum::GetValueAsString(ArchetypeData.Build);
    
    // Create unique ID from archetype data and random seed
    FString UniqueID = FString::Printf(TEXT("%s_%s_%s_%d_%d"), 
        *ArchetypeName, 
        *AgeGroupName, 
        *BuildName,
        RandomSeed,
        FMath::RandRange(1000, 9999));
    
    return UniqueID;
}

void UMetaHumanCharacterGenerator::ApplyAgeModifications(FMetaHumanVariationParams& Params, ECharacterAgeGroup AgeGroup)
{
    switch (AgeGroup)
    {
        case ECharacterAgeGroup::Young:
            Params.Age = RandomStream.FRandRange(0.0f, 0.3f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(-0.2f, 0.1f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(Params.Muscle + RandomStream.FRandRange(-0.1f, 0.3f), 0.0f, 1.0f);
            break;
            
        case ECharacterAgeGroup::Adult:
            Params.Age = RandomStream.FRandRange(0.3f, 0.6f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(-0.1f, 0.2f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(Params.Muscle + RandomStream.FRandRange(-0.2f, 0.2f), 0.0f, 1.0f);
            break;
            
        case ECharacterAgeGroup::Mature:
            Params.Age = RandomStream.FRandRange(0.6f, 0.8f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(0.0f, 0.3f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(Params.Muscle + RandomStream.FRandRange(-0.3f, 0.1f), 0.0f, 1.0f);
            break;
            
        case ECharacterAgeGroup::Elder:
            Params.Age = RandomStream.FRandRange(0.8f, 1.0f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(-0.2f, 0.2f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(Params.Muscle + RandomStream.FRandRange(-0.4f, -0.1f), 0.0f, 1.0f);
            Params.ScarIntensity = FMath::Clamp(Params.ScarIntensity + RandomStream.FRandRange(0.1f, 0.3f), 0.0f, 1.0f);
            break;
    }
}

void UMetaHumanCharacterGenerator::ApplyBuildModifications(FMetaHumanVariationParams& Params, ECharacterBuild BuildType)
{
    switch (BuildType)
    {
        case ECharacterBuild::Slim:
            Params.Weight = FMath::Clamp(RandomStream.FRandRange(0.0f, 0.4f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(RandomStream.FRandRange(0.0f, 0.3f), 0.0f, 1.0f);
            Params.Height = FMath::Clamp(Params.Height + RandomStream.FRandRange(-0.1f, 0.2f), 0.0f, 1.0f);
            break;
            
        case ECharacterBuild::Average:
            Params.Weight = FMath::Clamp(RandomStream.FRandRange(0.3f, 0.7f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(RandomStream.FRandRange(0.2f, 0.6f), 0.0f, 1.0f);
            break;
            
        case ECharacterBuild::Heavy:
            Params.Weight = FMath::Clamp(RandomStream.FRandRange(0.6f, 1.0f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(RandomStream.FRandRange(0.1f, 0.5f), 0.0f, 1.0f);
            Params.Height = FMath::Clamp(Params.Height + RandomStream.FRandRange(-0.2f, 0.1f), 0.0f, 1.0f);
            break;
            
        case ECharacterBuild::Athletic:
            Params.Weight = FMath::Clamp(RandomStream.FRandRange(0.4f, 0.7f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(RandomStream.FRandRange(0.6f, 1.0f), 0.0f, 1.0f);
            Params.Height = FMath::Clamp(Params.Height + RandomStream.FRandRange(0.0f, 0.3f), 0.0f, 1.0f);
            break;
    }
}

void UMetaHumanCharacterGenerator::ApplySurvivalModifications(FMetaHumanVariationParams& Params, ESurvivalCondition Condition)
{
    switch (Condition)
    {
        case ESurvivalCondition::Fresh:
            Params.WearLevel = RandomStream.FRandRange(0.0f, 0.2f);
            Params.DirtLevel = RandomStream.FRandRange(0.0f, 0.1f);
            Params.ScarIntensity = RandomStream.FRandRange(0.0f, 0.1f);
            break;
            
        case ESurvivalCondition::Weathered:
            Params.WearLevel = RandomStream.FRandRange(0.2f, 0.5f);
            Params.DirtLevel = RandomStream.FRandRange(0.1f, 0.4f);
            Params.ScarIntensity = RandomStream.FRandRange(0.1f, 0.3f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(-0.2f, 0.0f), 0.0f, 1.0f);
            break;
            
        case ESurvivalCondition::Hardened:
            Params.WearLevel = RandomStream.FRandRange(0.4f, 0.7f);
            Params.DirtLevel = RandomStream.FRandRange(0.3f, 0.6f);
            Params.ScarIntensity = RandomStream.FRandRange(0.2f, 0.5f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(-0.3f, -0.1f), 0.0f, 1.0f);
            Params.Muscle = FMath::Clamp(Params.Muscle + RandomStream.FRandRange(0.1f, 0.3f), 0.0f, 1.0f);
            break;
            
        case ESurvivalCondition::Broken:
            Params.WearLevel = RandomStream.FRandRange(0.6f, 1.0f);
            Params.DirtLevel = RandomStream.FRandRange(0.5f, 0.9f);
            Params.ScarIntensity = RandomStream.FRandRange(0.4f, 0.8f);
            Params.Weight = FMath::Clamp(Params.Weight + RandomStream.FRandRange(-0.4f, -0.2f), 0.0f, 1.0f);
            Params.Age = FMath::Clamp(Params.Age + RandomStream.FRandRange(0.1f, 0.3f), 0.0f, 1.0f);
            break;
    }
}

void UMetaHumanCharacterGenerator::GenerateUniqueFeatures(FMetaHumanVariationParams& Params, const FString& UniqueFeatures)
{
    // Parse unique features string and apply modifications
    TArray<FString> Features;
    UniqueFeatures.ParseIntoArray(Features, TEXT(","), true);
    
    for (const FString& Feature : Features)
    {
        FString TrimmedFeature = Feature.TrimStartAndEnd();
        
        if (TrimmedFeature.Contains(TEXT("Scar")))
        {
            Params.ScarIntensity = FMath::Clamp(Params.ScarIntensity + RandomStream.FRandRange(0.3f, 0.6f), 0.0f, 1.0f);
        }
        else if (TrimmedFeature.Contains(TEXT("Muscular")))
        {
            Params.Muscle = FMath::Clamp(Params.Muscle + RandomStream.FRandRange(0.2f, 0.4f), 0.0f, 1.0f);
        }
        else if (TrimmedFeature.Contains(TEXT("Thin")))
        {
            Params.Weight = FMath::Clamp(Params.Weight - RandomStream.FRandRange(0.2f, 0.4f), 0.0f, 1.0f);
        }
        else if (TrimmedFeature.Contains(TEXT("Tall")))
        {
            Params.Height = FMath::Clamp(Params.Height + RandomStream.FRandRange(0.2f, 0.4f), 0.0f, 1.0f);
        }
        else if (TrimmedFeature.Contains(TEXT("Short")))
        {
            Params.Height = FMath::Clamp(Params.Height - RandomStream.FRandRange(0.2f, 0.4f), 0.0f, 1.0f);
        }
    }
}