// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "CharacterDiversityEngine.h"
#include "MetaHumanIntegrationSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMetaHumanIntegration, Log, All);

class USkeletalMeshComponent;
class UAnimBlueprint;
class UMaterialInterface;

/**
 * MetaHuman body type variations
 */
UENUM(BlueprintType)
enum class EMetaHumanBodyType : uint8
{
    Masculine_Small     UMETA(DisplayName = "Masculine Small"),
    Masculine_Medium    UMETA(DisplayName = "Masculine Medium"),
    Masculine_Large     UMETA(DisplayName = "Masculine Large"),
    Feminine_Small      UMETA(DisplayName = "Feminine Small"),
    Feminine_Medium     UMETA(DisplayName = "Feminine Medium"),
    Feminine_Large      UMETA(DisplayName = "Feminine Large"),
    Androgynous_Small   UMETA(DisplayName = "Androgynous Small"),
    Androgynous_Medium  UMETA(DisplayName = "Androgynous Medium"),
    Androgynous_Large   UMETA(DisplayName = "Androgynous Large")
};

/**
 * MetaHuman facial expression presets
 */
UENUM(BlueprintType)
enum class EMetaHumanExpression : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Happy           UMETA(DisplayName = "Happy"),
    Sad             UMETA(DisplayName = "Sad"),
    Angry           UMETA(DisplayName = "Angry"),
    Surprised       UMETA(DisplayName = "Surprised"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Disgusted       UMETA(DisplayName = "Disgusted"),
    Contemplative   UMETA(DisplayName = "Contemplative"),
    Determined      UMETA(DisplayName = "Determined"),
    Suspicious      UMETA(DisplayName = "Suspicious")
};

/**
 * MetaHuman clothing style categories
 */
UENUM(BlueprintType)
enum class EMetaHumanClothingStyle : uint8
{
    Modern_Casual       UMETA(DisplayName = "Modern Casual"),
    Modern_Formal       UMETA(DisplayName = "Modern Formal"),
    Survival_Gear       UMETA(DisplayName = "Survival Gear"),
    Primitive_Clothing  UMETA(DisplayName = "Primitive Clothing"),
    Tribal_Wear         UMETA(DisplayName = "Tribal Wear"),
    Scientific_Attire   UMETA(DisplayName = "Scientific Attire"),
    Hunter_Outfit       UMETA(DisplayName = "Hunter Outfit"),
    Mystic_Robes        UMETA(DisplayName = "Mystic Robes"),
    Weathered_Clothes   UMETA(DisplayName = "Weathered Clothes")
};

/**
 * MetaHuman configuration data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMetaHumanConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    EMetaHumanBodyType BodyType = EMetaHumanBodyType::Masculine_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    EMetaHumanExpression DefaultExpression = EMetaHumanExpression::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EMetaHumanClothingStyle ClothingStyle = EMetaHumanClothingStyle::Modern_Casual;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TMap<FString, float> MorphTargetValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TMap<FString, FLinearColor> MaterialParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimBlueprint> CustomAnimBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    FCharacterDiversityProfile DiversityProfile;

    FMetaHumanConfiguration()
    {
        // Initialize default morph target values
        MorphTargetValues.Add(TEXT("FaceWidth"), 0.0f);
        MorphTargetValues.Add(TEXT("FaceHeight"), 0.0f);
        MorphTargetValues.Add(TEXT("EyeSize"), 0.0f);
        MorphTargetValues.Add(TEXT("NoseSize"), 0.0f);
        MorphTargetValues.Add(TEXT("MouthSize"), 0.0f);
        MorphTargetValues.Add(TEXT("Age"), 0.0f);
        
        // Initialize default material parameters
        MaterialParameters.Add(TEXT("SkinRoughness"), FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
        MaterialParameters.Add(TEXT("SkinSubsurface"), FLinearColor(0.3f, 0.1f, 0.1f, 1.0f));
        MaterialParameters.Add(TEXT("HairRoughness"), FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
    }
};

/**
 * MetaHuman asset library entry
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMetaHumanAssetEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    EMetaHumanBodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TMap<FString, float> DefaultMorphTargets;
};

/**
 * MetaHuman Integration System
 * Manages MetaHuman character creation, customization, and integration with game systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMetaHumanIntegrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UMetaHumanIntegrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core MetaHuman Functions
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool CreateMetaHuman(USkeletalMeshComponent* MeshComponent, const FMetaHumanConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool ApplyMetaHumanConfiguration(USkeletalMeshComponent* MeshComponent, const FMetaHumanConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    FMetaHumanConfiguration GenerateRandomConfiguration(EMetaHumanBodyType BodyType = EMetaHumanBodyType::Masculine_Medium);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool SwapMetaHumanBodyType(USkeletalMeshComponent* MeshComponent, EMetaHumanBodyType NewBodyType);

    // Customization Functions
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Customization")
    void SetMorphTargetValue(USkeletalMeshComponent* MeshComponent, const FString& MorphTargetName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Customization")
    void SetMaterialParameter(USkeletalMeshComponent* MeshComponent, int32 MaterialIndex, const FString& ParameterName, const FLinearColor& Value);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Customization")
    void ApplyExpression(USkeletalMeshComponent* MeshComponent, EMetaHumanExpression Expression, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Customization")
    void ApplyClothingStyle(USkeletalMeshComponent* MeshComponent, EMetaHumanClothingStyle ClothingStyle);

    // Diversity Integration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Diversity")
    void ApplyDiversityProfile(USkeletalMeshComponent* MeshComponent, const FCharacterDiversityProfile& DiversityProfile);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Diversity")
    FCharacterDiversityProfile GenerateDiversityProfileForBodyType(EMetaHumanBodyType BodyType);

    // Asset Management
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Assets")
    bool LoadMetaHumanAssetLibrary();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Assets")
    TArray<FString> GetAvailableMetaHumans(EMetaHumanBodyType BodyType = EMetaHumanBodyType::Masculine_Medium);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Assets")
    FMetaHumanAssetEntry GetMetaHumanAsset(const FString& AssetName);

    // Animation Integration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Animation")
    bool SetupMetaHumanAnimation(USkeletalMeshComponent* MeshComponent, const FMetaHumanConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Animation")
    void UpdateFacialAnimation(USkeletalMeshComponent* MeshComponent, float DeltaTime);

    // Lighting Integration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Lighting")
    void UpdateMetaHumanLighting(USkeletalMeshComponent* MeshComponent, const FVector& LightDirection, const FLinearColor& LightColor);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Utility")
    bool ValidateMetaHumanConfiguration(const FMetaHumanConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Utility")
    FString GetBodyTypeDisplayName(EMetaHumanBodyType BodyType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman|Utility")
    EMetaHumanBodyType GetRandomBodyType();

protected:
    // Asset Library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Assets")
    TMap<FString, FMetaHumanAssetEntry> MetaHumanAssetLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Assets")
    TSoftObjectPtr<UDataTable> MetaHumanAssetTable;

    // Expression Configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Expressions")
    TMap<EMetaHumanExpression, TMap<FString, float>> ExpressionMorphTargets;

    // Clothing Configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Clothing")
    TMap<EMetaHumanClothingStyle, TArray<TSoftObjectPtr<USkeletalMesh>>> ClothingMeshes;

    // Material Templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Materials")
    TMap<EMetaHumanBodyType, TSoftObjectPtr<UMaterialInterface>> SkinMaterialTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Materials")
    TSoftObjectPtr<UMaterialInterface> HairMaterialTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Materials")
    TSoftObjectPtr<UMaterialInterface> EyeMaterialTemplate;

    // Diversity Engine Reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Diversity")
    TObjectPtr<UCharacterDiversityEngine> DiversityEngine;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Configuration")
    bool bAutoLoadAssets = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Configuration")
    bool bEnableRealTimeLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Configuration")
    bool bEnableFacialAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman|Configuration")
    float MorphTargetBlendSpeed = 2.0f;

private:
    // Internal Functions
    void InitializeExpressionMorphTargets();
    void InitializeClothingConfigurations();
    void InitializeMaterialTemplates();
    
    USkeletalMesh* GetSkeletalMeshForBodyType(EMetaHumanBodyType BodyType);
    UAnimBlueprint* GetAnimBlueprintForBodyType(EMetaHumanBodyType BodyType);
    
    void ApplyMorphTargetsFromMap(USkeletalMeshComponent* MeshComponent, const TMap<FString, float>& MorphTargets);
    void BlendMorphTarget(USkeletalMeshComponent* MeshComponent, const FString& MorphTargetName, float TargetValue, float BlendSpeed);
    
    UMaterialInstanceDynamic* GetOrCreateDynamicMaterial(USkeletalMeshComponent* MeshComponent, int32 MaterialIndex);
    
    bool LoadAssetFromSoftPtr(const TSoftObjectPtr<UObject>& SoftPtr);
    
    // Cached Data
    TMap<USkeletalMeshComponent*, TMap<FString, float>> CurrentMorphTargetValues;
    TMap<USkeletalMeshComponent*, TArray<UMaterialInstanceDynamic*>> DynamicMaterials;
    
    bool bIsInitialized = false;
};