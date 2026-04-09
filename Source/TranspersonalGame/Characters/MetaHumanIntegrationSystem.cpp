// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "MetaHumanIntegrationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY(LogMetaHumanIntegration);

UMetaHumanIntegrationSystem::UMetaHumanIntegrationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30 FPS for smooth morph target blending
    
    bIsInitialized = false;
}

void UMetaHumanIntegrationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("MetaHuman Integration System initializing..."));
    
    // Initialize expression morph targets
    InitializeExpressionMorphTargets();
    
    // Initialize clothing configurations
    InitializeClothingConfigurations();
    
    // Initialize material templates
    InitializeMaterialTemplates();
    
    // Load asset library if auto-load is enabled
    if (bAutoLoadAssets)
    {
        LoadMetaHumanAssetLibrary();
    }
    
    // Initialize diversity engine
    if (!DiversityEngine)
    {
        DiversityEngine = NewObject<UCharacterDiversityEngine>(this);
    }
    
    bIsInitialized = true;
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("MetaHuman Integration System initialized successfully"));
}

void UMetaHumanIntegrationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized) return;
    
    // Update morph target blending for all tracked mesh components
    for (auto& MorphTargetPair : CurrentMorphTargetValues)
    {
        USkeletalMeshComponent* MeshComponent = MorphTargetPair.Key;
        if (!IsValid(MeshComponent)) continue;
        
        const TMap<FString, float>& TargetValues = MorphTargetPair.Value;
        
        for (const auto& MorphPair : TargetValues)
        {
            const FString& MorphTargetName = MorphPair.Key;
            float TargetValue = MorphPair.Value;
            float CurrentValue = MeshComponent->GetMorphTarget(FName(*MorphTargetName));
            
            if (!FMath::IsNearlyEqual(CurrentValue, TargetValue, 0.01f))
            {
                BlendMorphTarget(MeshComponent, MorphTargetName, TargetValue, MorphTargetBlendSpeed);
            }
        }
    }
    
    // Update facial animation if enabled
    if (bEnableFacialAnimation)
    {
        for (auto& MorphTargetPair : CurrentMorphTargetValues)
        {
            USkeletalMeshComponent* MeshComponent = MorphTargetPair.Key;
            if (IsValid(MeshComponent))
            {
                UpdateFacialAnimation(MeshComponent, DeltaTime);
            }
        }
    }
}

bool UMetaHumanIntegrationSystem::CreateMetaHuman(USkeletalMeshComponent* MeshComponent, const FMetaHumanConfiguration& Configuration)
{
    if (!MeshComponent)
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Cannot create MetaHuman: MeshComponent is null"));
        return false;
    }
    
    if (!ValidateMetaHumanConfiguration(Configuration))
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Invalid MetaHuman configuration"));
        return false;
    }
    
    // Get skeletal mesh for body type
    USkeletalMesh* SkeletalMesh = GetSkeletalMeshForBodyType(Configuration.BodyType);
    if (!SkeletalMesh)
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Failed to load skeletal mesh for body type: %d"), (int32)Configuration.BodyType);
        return false;
    }
    
    // Set skeletal mesh
    MeshComponent->SetSkeletalMesh(SkeletalMesh);
    
    // Apply configuration
    if (!ApplyMetaHumanConfiguration(MeshComponent, Configuration))
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Failed to apply MetaHuman configuration"));
        return false;
    }
    
    // Setup animation
    if (!SetupMetaHumanAnimation(MeshComponent, Configuration))
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Failed to setup MetaHuman animation"));
    }
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Successfully created MetaHuman with body type: %s"), 
           *GetBodyTypeDisplayName(Configuration.BodyType));
    
    return true;
}

bool UMetaHumanIntegrationSystem::ApplyMetaHumanConfiguration(USkeletalMeshComponent* MeshComponent, const FMetaHumanConfiguration& Configuration)
{
    if (!MeshComponent)
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Cannot apply configuration: MeshComponent is null"));
        return false;
    }
    
    // Apply morph targets
    ApplyMorphTargetsFromMap(MeshComponent, Configuration.MorphTargetValues);
    
    // Apply material parameters
    for (const auto& MaterialParam : Configuration.MaterialParameters)
    {
        const FString& ParameterName = MaterialParam.Key;
        const FLinearColor& Value = MaterialParam.Value;
        
        // Apply to all materials
        for (int32 i = 0; i < MeshComponent->GetNumMaterials(); ++i)
        {
            SetMaterialParameter(MeshComponent, i, ParameterName, Value);
        }
    }
    
    // Apply skin tone
    SetMaterialParameter(MeshComponent, 0, TEXT("SkinBaseColor"), Configuration.SkinTone);
    
    // Apply hair color
    SetMaterialParameter(MeshComponent, 1, TEXT("HairColor"), Configuration.HairColor);
    
    // Apply eye color
    SetMaterialParameter(MeshComponent, 2, TEXT("EyeColor"), Configuration.EyeColor);
    
    // Apply default expression
    ApplyExpression(MeshComponent, Configuration.DefaultExpression);
    
    // Apply clothing style
    ApplyClothingStyle(MeshComponent, Configuration.ClothingStyle);
    
    // Apply diversity profile if provided
    if (DiversityEngine && Configuration.DiversityProfile.UniqueIdentifier.IsValid())
    {
        ApplyDiversityProfile(MeshComponent, Configuration.DiversityProfile);
    }
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Applied MetaHuman configuration successfully"));
    
    return true;
}

FMetaHumanConfiguration UMetaHumanIntegrationSystem::GenerateRandomConfiguration(EMetaHumanBodyType BodyType)
{
    FMetaHumanConfiguration Configuration;
    
    Configuration.BodyType = BodyType;
    
    // Generate random skin tone
    float SkinHue = FMath::RandRange(20.0f, 40.0f); // Skin tone range
    float SkinSaturation = FMath::RandRange(0.2f, 0.8f);
    float SkinBrightness = FMath::RandRange(0.3f, 0.9f);
    Configuration.SkinTone = FLinearColor::MakeFromHSV8(
        static_cast<uint8>(SkinHue),
        static_cast<uint8>(SkinSaturation * 255),
        static_cast<uint8>(SkinBrightness * 255)
    );
    
    // Generate random hair color
    float HairHue = FMath::RandRange(0.0f, 360.0f);
    float HairSaturation = FMath::RandRange(0.3f, 1.0f);
    float HairBrightness = FMath::RandRange(0.1f, 0.8f);
    Configuration.HairColor = FLinearColor::MakeFromHSV8(
        static_cast<uint8>(HairHue),
        static_cast<uint8>(HairSaturation * 255),
        static_cast<uint8>(HairBrightness * 255)
    );
    
    // Generate random eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.4f, 0.8f, 1.0f), // Blue
        FLinearColor(0.3f, 0.6f, 0.3f, 1.0f), // Green
        FLinearColor(0.4f, 0.3f, 0.1f, 1.0f), // Brown
        FLinearColor(0.6f, 0.6f, 0.6f, 1.0f), // Gray
        FLinearColor(0.2f, 0.5f, 0.4f, 1.0f)  // Hazel
    };
    Configuration.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Generate random expression
    Configuration.DefaultExpression = static_cast<EMetaHumanExpression>(FMath::RandRange(0, (int32)EMetaHumanExpression::Suspicious));
    
    // Generate random clothing style
    Configuration.ClothingStyle = static_cast<EMetaHumanClothingStyle>(FMath::RandRange(0, (int32)EMetaHumanClothingStyle::Weathered_Clothes));
    
    // Generate random morph target values
    Configuration.MorphTargetValues.Add(TEXT("FaceWidth"), FMath::RandRange(-0.5f, 0.5f));
    Configuration.MorphTargetValues.Add(TEXT("FaceHeight"), FMath::RandRange(-0.5f, 0.5f));
    Configuration.MorphTargetValues.Add(TEXT("EyeSize"), FMath::RandRange(-0.3f, 0.3f));
    Configuration.MorphTargetValues.Add(TEXT("NoseSize"), FMath::RandRange(-0.4f, 0.4f));
    Configuration.MorphTargetValues.Add(TEXT("MouthSize"), FMath::RandRange(-0.3f, 0.3f));
    Configuration.MorphTargetValues.Add(TEXT("Age"), FMath::RandRange(0.0f, 0.8f));
    
    // Generate diversity profile
    if (DiversityEngine)
    {
        Configuration.DiversityProfile = DiversityEngine->GenerateRandomDiversityProfile();
    }
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Generated random MetaHuman configuration"));
    
    return Configuration;
}

bool UMetaHumanIntegrationSystem::SwapMetaHumanBodyType(USkeletalMeshComponent* MeshComponent, EMetaHumanBodyType NewBodyType)
{
    if (!MeshComponent)
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Cannot swap body type: MeshComponent is null"));
        return false;
    }
    
    // Get new skeletal mesh
    USkeletalMesh* NewSkeletalMesh = GetSkeletalMeshForBodyType(NewBodyType);
    if (!NewSkeletalMesh)
    {
        UE_LOG(LogMetaHumanIntegration, Error, TEXT("Failed to load skeletal mesh for body type: %d"), (int32)NewBodyType);
        return false;
    }
    
    // Store current morph target values
    TMap<FString, float> CurrentMorphTargets;
    if (CurrentMorphTargetValues.Contains(MeshComponent))
    {
        CurrentMorphTargets = CurrentMorphTargetValues[MeshComponent];
    }
    
    // Set new skeletal mesh
    MeshComponent->SetSkeletalMesh(NewSkeletalMesh);
    
    // Get new animation blueprint
    UAnimBlueprint* NewAnimBlueprint = GetAnimBlueprintForBodyType(NewBodyType);
    if (NewAnimBlueprint)
    {
        MeshComponent->SetAnimInstanceClass(NewAnimBlueprint->GetAnimBlueprintGeneratedClass());
    }
    
    // Reapply morph targets
    ApplyMorphTargetsFromMap(MeshComponent, CurrentMorphTargets);
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Swapped MetaHuman body type to: %s"), *GetBodyTypeDisplayName(NewBodyType));
    
    return true;
}

void UMetaHumanIntegrationSystem::SetMorphTargetValue(USkeletalMeshComponent* MeshComponent, const FString& MorphTargetName, float Value)
{
    if (!MeshComponent)
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Cannot set morph target: MeshComponent is null"));
        return;
    }
    
    // Clamp value to valid range
    Value = FMath::Clamp(Value, -1.0f, 1.0f);
    
    // Store target value for blending
    if (!CurrentMorphTargetValues.Contains(MeshComponent))
    {
        CurrentMorphTargetValues.Add(MeshComponent, TMap<FString, float>());
    }
    CurrentMorphTargetValues[MeshComponent].Add(MorphTargetName, Value);
    
    // Apply immediately if blending is disabled
    if (MorphTargetBlendSpeed <= 0.0f)
    {
        MeshComponent->SetMorphTarget(FName(*MorphTargetName), Value);
    }
}

void UMetaHumanIntegrationSystem::SetMaterialParameter(USkeletalMeshComponent* MeshComponent, int32 MaterialIndex, const FString& ParameterName, const FLinearColor& Value)
{
    if (!MeshComponent)
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Cannot set material parameter: MeshComponent is null"));
        return;
    }
    
    UMaterialInstanceDynamic* DynamicMaterial = GetOrCreateDynamicMaterial(MeshComponent, MaterialIndex);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(FName(*ParameterName), Value);
    }
}

void UMetaHumanIntegrationSystem::ApplyExpression(USkeletalMeshComponent* MeshComponent, EMetaHumanExpression Expression, float Intensity)
{
    if (!MeshComponent || !ExpressionMorphTargets.Contains(Expression))
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Cannot apply expression: Invalid parameters"));
        return;
    }
    
    // Reset all expression morph targets
    for (const auto& ExpressionPair : ExpressionMorphTargets)
    {
        for (const auto& MorphPair : ExpressionPair.Value)
        {
            SetMorphTargetValue(MeshComponent, MorphPair.Key, 0.0f);
        }
    }
    
    // Apply selected expression
    const TMap<FString, float>& ExpressionMorphs = ExpressionMorphTargets[Expression];
    for (const auto& MorphPair : ExpressionMorphs)
    {
        float Value = MorphPair.Value * FMath::Clamp(Intensity, 0.0f, 1.0f);
        SetMorphTargetValue(MeshComponent, MorphPair.Key, Value);
    }
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Applied expression: %d with intensity: %.2f"), (int32)Expression, Intensity);
}

void UMetaHumanIntegrationSystem::ApplyClothingStyle(USkeletalMeshComponent* MeshComponent, EMetaHumanClothingStyle ClothingStyle)
{
    if (!MeshComponent || !ClothingMeshes.Contains(ClothingStyle))
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Cannot apply clothing style: Invalid parameters"));
        return;
    }
    
    // This would typically involve swapping clothing meshes or applying clothing materials
    // For now, we'll apply material parameters to simulate different clothing styles
    
    switch (ClothingStyle)
    {
        case EMetaHumanClothingStyle::Survival_Gear:
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingRoughness"), FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingColor"), FLinearColor(0.3f, 0.4f, 0.2f, 1.0f));
            break;
            
        case EMetaHumanClothingStyle::Primitive_Clothing:
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingRoughness"), FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingColor"), FLinearColor(0.6f, 0.4f, 0.2f, 1.0f));
            break;
            
        case EMetaHumanClothingStyle::Tribal_Wear:
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingRoughness"), FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingColor"), FLinearColor(0.8f, 0.3f, 0.1f, 1.0f));
            break;
            
        case EMetaHumanClothingStyle::Scientific_Attire:
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingRoughness"), FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingColor"), FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
            break;
            
        default:
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingRoughness"), FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
            SetMaterialParameter(MeshComponent, 3, TEXT("ClothingColor"), FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
            break;
    }
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Applied clothing style: %d"), (int32)ClothingStyle);
}

void UMetaHumanIntegrationSystem::ApplyDiversityProfile(USkeletalMeshComponent* MeshComponent, const FCharacterDiversityProfile& DiversityProfile)
{
    if (!MeshComponent || !DiversityEngine)
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Cannot apply diversity profile: Invalid parameters"));
        return;
    }
    
    DiversityEngine->ApplyDiversityProfile(MeshComponent, DiversityProfile);
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Applied diversity profile: %s"), *DiversityProfile.UniqueIdentifier.ToString());
}

FCharacterDiversityProfile UMetaHumanIntegrationSystem::GenerateDiversityProfileForBodyType(EMetaHumanBodyType BodyType)
{
    if (!DiversityEngine)
    {
        UE_LOG(LogMetaHumanIntegration, Warning, TEXT("Cannot generate diversity profile: DiversityEngine is null"));
        return FCharacterDiversityProfile();
    }
    
    FCharacterDiversityProfile Profile = DiversityEngine->GenerateRandomDiversityProfile();
    
    // Adjust profile based on body type
    switch (BodyType)
    {
        case EMetaHumanBodyType::Masculine_Large:
            Profile.BodyType.Height *= 1.1f;
            Profile.BodyType.ShoulderWidth *= 1.2f;
            Profile.BodyType.Weight *= 1.15f;
            break;
            
        case EMetaHumanBodyType::Feminine_Small:
            Profile.BodyType.Height *= 0.9f;
            Profile.BodyType.ShoulderWidth *= 0.8f;
            Profile.BodyType.Weight *= 0.85f;
            break;
            
        case EMetaHumanBodyType::Androgynous_Medium:
            // Keep default values
            break;
            
        default:
            break;
    }
    
    return Profile;
}

bool UMetaHumanIntegrationSystem::LoadMetaHumanAssetLibrary()
{
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Loading MetaHuman asset library..."));
    
    // Load from data table if available
    if (MetaHumanAssetTable.IsValid())
    {
        UDataTable* DataTable = MetaHumanAssetTable.LoadSynchronous();
        if (DataTable)
        {
            TArray<FMetaHumanAssetEntry*> AssetEntries;
            DataTable->GetAllRows<FMetaHumanAssetEntry>(TEXT("MetaHumanAssetLibrary"), AssetEntries);
            
            for (FMetaHumanAssetEntry* Entry : AssetEntries)
            {
                if (Entry)
                {
                    MetaHumanAssetLibrary.Add(Entry->AssetName, *Entry);
                }
            }
            
            UE_LOG(LogMetaHumanIntegration, Log, TEXT("Loaded %d MetaHuman assets from data table"), AssetEntries.Num());
            return true;
        }
    }
    
    // Fallback: Create default asset entries
    CreateDefaultAssetEntries();
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Created default MetaHuman asset library"));
    return true;
}

void UMetaHumanIntegrationSystem::InitializeExpressionMorphTargets()
{
    // Initialize expression morph target mappings
    
    // Neutral expression (all zeros)
    ExpressionMorphTargets.Add(EMetaHumanExpression::Neutral, TMap<FString, float>());
    
    // Happy expression
    TMap<FString, float> HappyMorphs;
    HappyMorphs.Add(TEXT("MouthSmile"), 0.8f);
    HappyMorphs.Add(TEXT("EyeSquint"), 0.3f);
    HappyMorphs.Add(TEXT("CheekRaise"), 0.5f);
    ExpressionMorphTargets.Add(EMetaHumanExpression::Happy, HappyMorphs);
    
    // Sad expression
    TMap<FString, float> SadMorphs;
    SadMorphs.Add(TEXT("MouthFrown"), 0.7f);
    SadMorphs.Add(TEXT("BrowDown"), 0.6f);
    SadMorphs.Add(TEXT("EyeClose"), 0.2f);
    ExpressionMorphTargets.Add(EMetaHumanExpression::Sad, SadMorphs);
    
    // Angry expression
    TMap<FString, float> AngryMorphs;
    AngryMorphs.Add(TEXT("BrowDown"), 0.8f);
    AngryMorphs.Add(TEXT("MouthPress"), 0.6f);
    AngryMorphs.Add(TEXT("NostrilFlare"), 0.5f);
    ExpressionMorphTargets.Add(EMetaHumanExpression::Angry, AngryMorphs);
    
    // Surprised expression
    TMap<FString, float> SurprisedMorphs;
    SurprisedMorphs.Add(TEXT("BrowUp"), 0.8f);
    SurprisedMorphs.Add(TEXT("EyeWide"), 0.7f);
    SurprisedMorphs.Add(TEXT("MouthOpen"), 0.5f);
    ExpressionMorphTargets.Add(EMetaHumanExpression::Surprised, SurprisedMorphs);
    
    // Fearful expression
    TMap<FString, float> FearfulMorphs;
    FearfulMorphs.Add(TEXT("BrowUp"), 0.6f);
    FearfulMorphs.Add(TEXT("EyeWide"), 0.8f);
    FearfulMorphs.Add(TEXT("MouthOpen"), 0.3f);
    ExpressionMorphTargets.Add(EMetaHumanExpression::Fearful, FearfulMorphs);
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Initialized expression morph targets"));
}

void UMetaHumanIntegrationSystem::InitializeClothingConfigurations()
{
    // Initialize clothing mesh configurations
    // This would typically load actual clothing mesh assets
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Initialized clothing configurations"));
}

void UMetaHumanIntegrationSystem::InitializeMaterialTemplates()
{
    // Initialize material template references
    // These would typically be loaded from asset references
    
    UE_LOG(LogMetaHumanIntegration, Log, TEXT("Initialized material templates"));
}