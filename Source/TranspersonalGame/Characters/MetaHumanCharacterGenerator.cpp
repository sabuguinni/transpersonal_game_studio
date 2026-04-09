#include "MetaHumanCharacterGenerator.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/TranspersonalGame.h"

UMetaHumanCharacterGenerator::UMetaHumanCharacterGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    RandomSeed = FMath::Rand();
    bEnableDebugLogging = false;
}

void UMetaHumanCharacterGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize random stream with seed
    RandomStream.Initialize(RandomSeed);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("MetaHumanCharacterGenerator initialized with seed: %d"), RandomSeed);
}

bool UMetaHumanCharacterGenerator::GenerateCharacterFromArchetype(const FCharacterArchetypeData& ArchetypeData, USkeletalMeshComponent* TargetMeshComponent)
{
    if (!TargetMeshComponent)
    {
        UE_LOG(LogTranspersonalGame, Error, TEXT("GenerateCharacterFromArchetype: Invalid target mesh component"));
        return false;
    }

    // Generate variation parameters based on archetype
    FMetaHumanVariationParams VariationParams = GenerateVariationParams(ArchetypeData);
    
    // Apply variations to the MetaHuman
    if (!ApplyVariationToMetaHuman(TargetMeshComponent, VariationParams))
    {
        UE_LOG(LogTranspersonalGame, Error, TEXT("Failed to apply variations to MetaHuman for archetype: %s"), *ArchetypeData.ArchetypeName);
        return false;
    }

    // Generate and apply clothing configuration
    FCharacterClothingConfig ClothingConfig = GenerateClothingConfig(ArchetypeData);
    if (!ApplyClothingConfig(TargetMeshComponent, ClothingConfig))
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Failed to apply clothing config for archetype: %s"), *ArchetypeData.ArchetypeName);
    }

    if (bEnableDebugLogging)
    {
        UE_LOG(LogTranspersonalGame, Log, TEXT("Successfully generated character from archetype: %s"), *ArchetypeData.ArchetypeName);
    }

    return true;
}

FMetaHumanVariationParams UMetaHumanCharacterGenerator::GenerateVariationParams(const FCharacterArchetypeData& ArchetypeData)
{
    FMetaHumanVariationParams Params;
    
    // Base randomization within reasonable bounds
    Params.FaceVariation = RandomStream.FRandRange(0.2f, 0.8f);
    Params.BodyVariation = RandomStream.FRandRange(0.2f, 0.8f);
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

    // Get or create dynamic material instances for each material slot
    TArray<UMaterialInterface*> Materials = MeshComponent->GetMaterials();
    
    for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); MaterialIndex++)
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

        // Apply skin tone variations
        DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), Params.SkinTone);
        DynamicMaterial->SetScalarParameterValue(TEXT("SkinVariation"), Params.FaceVariation);
        
        // Apply age-related parameters
        DynamicMaterial->SetScalarParameterValue(TEXT("Age"), Params.Age);
        DynamicMaterial->SetScalarParameterValue(TEXT("WrinkleIntensity"), Params.Age * 0.7f);
        
        // Apply survival condition parameters
        DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), Params.DirtLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("WearLevel"), Params.WearLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), Params.ScarIntensity);
        
        // Apply color variations
        FLinearColor HairColorValue = FLinearColor::LerpUsingHSV(
            FLinearColor(0.1f, 0.05f, 0.02f, 1.0f), // Dark brown
            FLinearColor(0.9f, 0.8f, 0.6f, 1.0f),   // Blonde
            Params.HairColor
        );
        DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), HairColorValue);
        
        FLinearColor EyeColorValue = FLinearColor::LerpUsingHSV(
            FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Brown
            FLinearColor(0.3f, 0.6f, 0.9f, 1.0f),  // Blue
            Params.EyeColor
        );
        DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), EyeColorValue);
        
        // Set the dynamic material
        MeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
    }

    // Apply physical modifications through morph targets if available
    if (USkeletalMesh* SkeletalMesh = MeshComponent->GetSkeletalMeshAsset())
    {
        // Apply height scaling
        float HeightScale = FMath::Lerp(0.95f, 1.05f, Params.Height);
        MeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, HeightScale));
        
        // Apply weight/muscle morph targets if they exist
        MeshComponent->SetMorphTarget(TEXT("Weight"), Params.Weight);
        MeshComponent->SetMorphTarget(TEXT("Muscle"), Params.Muscle);
        MeshComponent->SetMorphTarget(TEXT("Age"), Params.Age);
    }

    return true;
}

FCharacterClothingConfig UMetaHumanCharacterGenerator::GenerateClothingConfig(const FCharacterArchetypeData& ArchetypeData)
{
    FCharacterClothingConfig Config;
    
    // Determine clothing style based on archetype
    switch (ArchetypeData.Archetype)
    {
        case ECharacterArchetype::Survivor:
            Config.ClothingStyle = TEXT("Rugged_Survival");
            Config.HasBackpack = true;
            Config.HasBoots = true;
            Config.EquipmentLevel = 2;
            break;
            
        case ECharacterArchetype::Explorer:
            Config.ClothingStyle = TEXT("Explorer_Gear");
            Config.HasBackpack = true;
            Config.HasHat = true;
            Config.HasGloves = true;
            Config.HasBoots = true;
            Config.EquipmentLevel = 3;
            break;
            
        case ECharacterArchetype::Scientist:
            Config.ClothingStyle = TEXT("Field_Research");
            Config.HasBackpack = true;
            Config.HasGloves = true;
            Config.HasBoots = true;
            Config.EquipmentLevel = 2;
            break;
            
        case ECharacterArchetype::Tribal:
            Config.ClothingStyle = TEXT("Primitive_Tribal");
            Config.HasBackpack = false;
            Config.HasBoots = false;
            Config.EquipmentLevel = 0;
            break;
            
        default:
            Config.ClothingStyle = TEXT("Basic_Clothing");
            Config.EquipmentLevel = 1;
            break;
    }
    
    // Adjust for survival condition
    switch (ArchetypeData.SurvivalCondition)
    {
        case ESurvivalCondition::Thriving:
            // No modifications needed
            break;
            
        case ESurvivalCondition::Struggling:
            Config.EquipmentLevel = FMath::Max(0, Config.EquipmentLevel - 1);
            break;
            
        case ESurvivalCondition::Desperate:
            Config.EquipmentLevel = 0;
            Config.HasBackpack = false;
            Config.HasHat = false;
            Config.HasGloves = false;
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

    // This would typically involve:
    // 1. Loading appropriate clothing meshes
    // 2. Applying clothing materials
    // 3. Setting up equipment attachments
    // 4. Configuring clothing physics
    
    // For now, we'll apply material modifications to simulate clothing changes
    TArray<UMaterialInterface*> Materials = MeshComponent->GetMaterials();
    
    for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); MaterialIndex++)
    {
        UMaterialInterface* BaseMaterial = Materials[MaterialIndex];
        if (!BaseMaterial)
        {
            continue;
        }

        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshComponent);
        if (!DynamicMaterial)
        {
            continue;
        }

        // Apply equipment level modifications
        float EquipmentQuality = static_cast<float>(ClothingConfig.EquipmentLevel) / 3.0f;
        DynamicMaterial->SetScalarParameterValue(TEXT("EquipmentQuality"), EquipmentQuality);
        
        // Apply wear based on equipment level
        float WearLevel = 1.0f - EquipmentQuality;
        DynamicMaterial->SetScalarParameterValue(TEXT("ClothingWear"), WearLevel);
        
        MeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
    }

    if (bEnableDebugLogging)
    {
        UE_LOG(LogTranspersonalGame, Log, TEXT("Applied clothing config: %s (Equipment Level: %d)"), 
               *ClothingConfig.ClothingStyle, ClothingConfig.EquipmentLevel);
    }

    return true;
}

FString UMetaHumanCharacterGenerator::GenerateUniqueCharacterID(const FCharacterArchetypeData& ArchetypeData)
{
    // Generate a unique ID based on archetype, timestamp, and random elements
    FString ArchetypeName = UEnum::GetValueAsString(ArchetypeData.Archetype);
    FString TimeStamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    int32 RandomSuffix = RandomStream.RandRange(1000, 9999);
    
    return FString::Printf(TEXT("%s_%s_%d"), *ArchetypeName, *TimeStamp, RandomSuffix);
}

void UMetaHumanCharacterGenerator::ApplyAgeModifications(FMetaHumanVariationParams& Params, ECharacterAgeGroup AgeGroup)
{
    switch (AgeGroup)
    {
        case ECharacterAgeGroup::Young:
            Params.Age = RandomStream.FRandRange(0.0f, 0.3f);
            Params.ScarIntensity *= 0.3f; // Young people have fewer scars
            Params.WearLevel *= 0.7f; // Less wear on clothing
            break;
            
        case ECharacterAgeGroup::Adult:
            Params.Age = RandomStream.FRandRange(0.3f, 0.6f);
            // No modifications needed for adults
            break;
            
        case ECharacterAgeGroup::MiddleAged:
            Params.Age = RandomStream.FRandRange(0.6f, 0.8f);
            Params.ScarIntensity *= 1.2f; // More life experience
            break;
            
        case ECharacterAgeGroup::Elder:
            Params.Age = RandomStream.FRandRange(0.8f, 1.0f);
            Params.ScarIntensity *= 1.5f; // Lifetime of experiences
            Params.WearLevel *= 1.3f; // Well-worn clothing
            break;
    }
}

void UMetaHumanCharacterGenerator::ApplyBuildModifications(FMetaHumanVariationParams& Params, ECharacterBuild BuildType)
{
    switch (BuildType)
    {
        case ECharacterBuild::Thin:
            Params.Weight = RandomStream.FRandRange(0.0f, 0.3f);
            Params.Muscle = RandomStream.FRandRange(0.0f, 0.4f);
            break;
            
        case ECharacterBuild::Average:
            Params.Weight = RandomStream.FRandRange(0.3f, 0.7f);
            Params.Muscle = RandomStream.FRandRange(0.3f, 0.7f);
            break;
            
        case ECharacterBuild::Heavy:
            Params.Weight = RandomStream.FRandRange(0.7f, 1.0f);
            Params.Muscle = RandomStream.FRandRange(0.2f, 0.6f);
            break;
            
        case ECharacterBuild::Muscular:
            Params.Weight = RandomStream.FRandRange(0.5f, 0.8f);
            Params.Muscle = RandomStream.FRandRange(0.7f, 1.0f);
            break;
    }
}

void UMetaHumanCharacterGenerator::ApplySurvivalModifications(FMetaHumanVariationParams& Params, ESurvivalCondition Condition)
{
    switch (Condition)
    {
        case ESurvivalCondition::Thriving:
            Params.DirtLevel = RandomStream.FRandRange(0.0f, 0.2f);
            Params.WearLevel = RandomStream.FRandRange(0.0f, 0.3f);
            Params.ScarIntensity *= 0.8f;
            break;
            
        case ESurvivalCondition::Struggling:
            Params.DirtLevel = RandomStream.FRandRange(0.3f, 0.6f);
            Params.WearLevel = RandomStream.FRandRange(0.4f, 0.7f);
            Params.ScarIntensity *= 1.2f;
            break;
            
        case ESurvivalCondition::Desperate:
            Params.DirtLevel = RandomStream.FRandRange(0.7f, 1.0f);
            Params.WearLevel = RandomStream.FRandRange(0.8f, 1.0f);
            Params.ScarIntensity *= 1.5f;
            Params.Weight *= 0.7f; // Malnutrition effects
            break;
    }
}

void UMetaHumanCharacterGenerator::GenerateUniqueFeatures(FMetaHumanVariationParams& Params, const FString& UniqueFeatures)
{
    // Parse unique features string and apply modifications
    TArray<FString> Features;
    UniqueFeatures.ParseIntoArray(Features, TEXT(","));
    
    for (const FString& Feature : Features)
    {
        FString TrimmedFeature = Feature.TrimStartAndEnd();
        
        if (TrimmedFeature.Contains(TEXT("scar")))
        {
            Params.ScarIntensity = FMath::Max(Params.ScarIntensity, 0.6f);
        }
        else if (TrimmedFeature.Contains(TEXT("tan")))
        {
            Params.SkinTone = FMath::Min(Params.SkinTone + 0.3f, 1.0f);
        }
        else if (TrimmedFeature.Contains(TEXT("pale")))
        {
            Params.SkinTone = FMath::Max(Params.SkinTone - 0.3f, 0.0f);
        }
        else if (TrimmedFeature.Contains(TEXT("weathered")))
        {
            Params.Age = FMath::Min(Params.Age + 0.2f, 1.0f);
            Params.WearLevel = FMath::Min(Params.WearLevel + 0.3f, 1.0f);
        }
    }
}