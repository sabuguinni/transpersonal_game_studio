#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "CharacterArchetypes.h"
#include "MetaHumanCharacterComponent.h"
#include "JurassicCharacterArtist.generated.h"

class UMetaHumanIdentity;
class USkeletalMesh;
class UMaterialInterface;
class UGroomAsset;
class ACharacter;

/**
 * Enums for Character Artist System
 */
UENUM(BlueprintType)
enum class ESkinTone : uint8
{
    VeryPale        UMETA(DisplayName = "Very Pale"),
    Pale            UMETA(DisplayName = "Pale"),
    Light           UMETA(DisplayName = "Light"),
    Medium          UMETA(DisplayName = "Medium"),
    Olive           UMETA(DisplayName = "Olive"),
    Tan             UMETA(DisplayName = "Tan"),
    Dark            UMETA(DisplayName = "Dark"),
    VeryDark        UMETA(DisplayName = "Very Dark"),
    
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male            UMETA(DisplayName = "Male"),
    Female          UMETA(DisplayName = "Female"),
    
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EFacialFeatureIntensity : uint8
{
    Subtle          UMETA(DisplayName = "Subtle"),
    Moderate        UMETA(DisplayName = "Moderate"),
    Pronounced      UMETA(DisplayName = "Pronounced"),
    Extreme         UMETA(DisplayName = "Extreme"),
    
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterQualityTier : uint8
{
    Crowd           UMETA(DisplayName = "Crowd - Low Detail"),
    Background      UMETA(DisplayName = "Background - Medium Detail"),
    Standard        UMETA(DisplayName = "Standard - High Detail"),
    Hero            UMETA(DisplayName = "Hero - Maximum Detail"),
    
    MAX             UMETA(Hidden)
};

/**
 * Structure for defining facial feature variations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFacialFeatureSet
{
    GENERATED_BODY()

    FFacialFeatureSet()
        : EyeShape(0.5f)
        , EyeSize(0.5f)
        , EyeDistance(0.5f)
        , NoseWidth(0.5f)
        , NoseLength(0.5f)
        , MouthWidth(0.5f)
        , LipThickness(0.5f)
        , JawWidth(0.5f)
        , CheekboneHeight(0.5f)
        , ForeheadHeight(0.5f)
        , AsymmetryFactor(0.1f)
    {
    }

    // Eye features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeShape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeDistance;

    // Nose features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseLength;

    // Mouth features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MouthWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LipThickness;

    // Face structure
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JawWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CheekboneHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ForeheadHeight;

    // Asymmetry for realism
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism", meta = (ClampMin = "0.0", ClampMax = "0.3"))
    float AsymmetryFactor;
};

/**
 * Structure for character visual storytelling elements
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterVisualStory
{
    GENERATED_BODY()

    FCharacterVisualStory()
        : SurvivalTime(0.0f)
        , StressLevel(0.0f)
        , HealthCondition(0.8f)
        , SunExposure(0.3f)
        , DirtAccumulation(0.2f)
        , bHasRecentInjuries(false)
        , bShowsSignsOfStarvation(false)
    {
    }

    // Time-based wear
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalTime; // Days in the Jurassic world

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StressLevel; // 0.0 = calm, 1.0 = extreme stress

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float HealthCondition; // 0.0 = near death, 1.0 = perfect health

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float SunExposure; // 0.0 = no exposure, 1.0 = heavily tanned/burned

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float DirtAccumulation; // 0.0 = clean, 1.0 = completely dirty

    // Specific conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bHasRecentInjuries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bShowsSignsOfStarvation;

    // Scars and markings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markings")
    TArray<FString> ScarDescriptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markings")
    TArray<FString> TattooDescriptions;

    // Clothing condition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> ClothingDamageAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float OverallClothingWear; // 0.0 = pristine, 1.0 = rags
};

/**
 * Structure for MetaHuman customization parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMetaHumanCustomization
{
    GENERATED_BODY()

    FMetaHumanCustomization()
        : Gender(ECharacterGender::Male)
        , SkinTone(ESkinTone::Medium)
        , EyeColor(FLinearColor::Blue)
        , HairColor(FLinearColor::Black)
        , QualityTier(ECharacterQualityTier::Standard)
        , CustomizationSeed(0)
        , bUseRandomVariation(true)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    ECharacterGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    ESkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    ECharacterQualityTier QualityTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 CustomizationSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    bool bUseRandomVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    FFacialFeatureSet FacialFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FCharacterVisualStory VisualStory;
};

/**
 * Jurassic Character Artist System
 * 
 * The master system for creating authentic, story-driven human characters.
 * Every face tells a story. Every detail has meaning. No clones allowed.
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicCharacterArtist : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core character creation functions
    UFUNCTION(BlueprintCallable, Category = "Character Artist")
    ACharacter* CreateCharacterFromArchetype(
        ECharacterArchetype ArchetypeType,
        const FVector& SpawnLocation,
        const FRotator& SpawnRotation = FRotator::ZeroRotator,
        bool bRandomizeAppearance = true
    );

    UFUNCTION(BlueprintCallable, Category = "Character Artist")
    ACharacter* CreateUniqueNamedCharacter(
        const FString& CharacterName,
        ECharacterArchetype BaseArchetype,
        const FMetaHumanCustomization& Customization,
        const FVector& SpawnLocation,
        const FRotator& SpawnRotation = FRotator::ZeroRotator
    );

    UFUNCTION(BlueprintCallable, Category = "Character Artist")
    void CustomizeExistingCharacter(
        ACharacter* Character,
        const FMetaHumanCustomization& Customization
    );

    // Visual storytelling functions
    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplyLifeStory(ACharacter* Character, const FCharacterVisualStory& Story);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplySurvivalEffects(ACharacter* Character, float DaysInWild, float StressLevel);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void AddBattleScars(ACharacter* Character, const TArray<FString>& ScarDescriptions);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplyEnvironmentalWear(ACharacter* Character, float SunExposure, float DirtLevel);

    // Diversity and variation management
    UFUNCTION(BlueprintCallable, Category = "Diversity")
    void EnsureCharacterDiversity(TArray<ACharacter*>& Characters);

    UFUNCTION(BlueprintCallable, Category = "Diversity")
    FMetaHumanCustomization GenerateDiverseCustomization(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Diversity")
    bool ValidateCharacterUniqueness(const FMetaHumanCustomization& Customization);

    // Quality and performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCharacterQualityTier(ACharacter* Character, ECharacterQualityTier QualityTier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCharactersForDistance(TArray<ACharacter*>& Characters, const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BatchUpdateCharacterLODs(const FVector& ViewerLocation, float DeltaTime);

    // Character database and analytics
    UFUNCTION(BlueprintCallable, Category = "Database")
    void RegisterCharacter(ACharacter* Character, const FMetaHumanCustomization& Customization);

    UFUNCTION(BlueprintCallable, Category = "Database")
    TArray<ACharacter*> GetCharactersByArchetype(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    FString GetCharacterDiversityReport() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    int32 GetTotalCharacterCount() const { return RegisteredCharacters.Num(); }

protected:
    // MetaHuman asset management
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaHuman Assets")
    TMap<ECharacterGender, TArray<TSoftObjectPtr<UMetaHumanIdentity>>> MetaHumanPresets;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaHuman Assets")
    TMap<ESkinTone, FLinearColor> SkinToneColors;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseEyeMaterial;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseHairMaterial;

    // Character registry
    UPROPERTY()
    TArray<TWeakObjectPtr<ACharacter>> RegisteredCharacters;

    UPROPERTY()
    TMap<FString, TWeakObjectPtr<ACharacter>> NamedCharacters;

    UPROPERTY()
    TMap<ECharacterArchetype, TArray<TWeakObjectPtr<ACharacter>>> CharactersByArchetype;

    // Diversity tracking
    UPROPERTY()
    TMap<ESkinTone, int32> SkinToneDistribution;

    UPROPERTY()
    TMap<ECharacterGender, int32> GenderDistribution;

    UPROPERTY()
    TMap<ECharacterAgeGroup, int32> AgeGroupDistribution;

    // Performance settings
    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    float LODUpdateInterval = 0.5f; // Seconds between LOD updates

    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    float CrowdQualityDistance = 2000.0f; // Distance at which characters become crowd quality

    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    int32 MaxHighQualityCharacters = 20; // Maximum number of high-quality characters at once

private:
    // Internal creation helpers
    void SetupCharacterMeshComponents(ACharacter* Character, const FMetaHumanCustomization& Customization);
    void ApplyFacialCustomization(ACharacter* Character, const FFacialFeatureSet& Features);
    void ApplyMaterialCustomization(ACharacter* Character, const FMetaHumanCustomization& Customization);
    void ApplyClothingAndAccessories(ACharacter* Character, ECharacterArchetype Archetype, const FCharacterVisualStory& Story);
    
    // Visual storytelling implementation
    void ApplySkinWeathering(ACharacter* Character, float Intensity);
    void ApplyClothingDamage(ACharacter* Character, const TArray<FString>& DamageAreas, float WearLevel);
    void AddFacialScars(ACharacter* Character, const TArray<FString>& ScarDescriptions);
    void ApplyMakeupAndDirt(ACharacter* Character, float DirtLevel);
    
    // Diversity management
    void BalanceDiversityMetrics();
    bool ShouldCreateDiverseVariant(ECharacterArchetype Archetype);
    FMetaHumanCustomization CreateDiverseVariant(ECharacterArchetype Archetype);
    
    // Performance optimization
    void UpdateCharacterLOD(ACharacter* Character, float DistanceToViewer);
    void OptimizeCharacterForCrowd(ACharacter* Character);
    ECharacterQualityTier DetermineOptimalQuality(float Distance, int32 CurrentHighQualityCount);
    
    // Utility functions
    FLinearColor GetRandomEyeColor() const;
    FLinearColor GetRandomHairColor() const;
    ESkinTone GetRandomSkinTone() const;
    FFacialFeatureSet GenerateRandomFacialFeatures(int32 Seed) const;
    
    // Timer for batch LOD updates
    FTimerHandle LODUpdateTimer;
    void PerformBatchLODUpdate();
    
    // Statistics
    mutable FCriticalSection StatisticsMutex;
    int32 TotalCharactersCreated = 0;
    float LastLODUpdateTime = 0.0f;
};

/**
 * Data Asset for Character Artist configuration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArtistDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** MetaHuman presets organized by gender and archetype */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TMap<ECharacterGender, TArray<TSoftObjectPtr<UMetaHumanIdentity>>> BasePresets;

    /** Archetype-specific MetaHuman overrides */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TMap<ECharacterArchetype, TArray<TSoftObjectPtr<UMetaHumanIdentity>>> ArchetypePresets;

    /** Material templates for different skin tones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<ESkinTone, TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    /** Clothing meshes organized by archetype */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TMap<ECharacterArchetype, TArray<TSoftObjectPtr<USkeletalMesh>>> ClothingMeshes;

    /** Hair groom assets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    TArray<TSoftObjectPtr<UGroomAsset>> HairStyles;

    /** Facial hair groom assets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    TArray<TSoftObjectPtr<UGroomAsset>> FacialHairStyles;

    /** Diversity targets for balanced character creation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    TMap<ESkinTone, float> SkinToneTargetPercentages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    TMap<ECharacterGender, float> GenderTargetPercentages;
};