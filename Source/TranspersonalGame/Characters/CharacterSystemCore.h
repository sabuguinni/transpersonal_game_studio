// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CharacterSystemCore.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Protagonist - Paleontologist"),
    TribalElder         UMETA(DisplayName = "Tribal Elder"),
    YoungHunter         UMETA(DisplayName = "Young Hunter"),
    WiseShaman          UMETA(DisplayName = "Wise Shaman"),
    ScarredSurvivor     UMETA(DisplayName = "Scarred Survivor"),
    CuriousChild        UMETA(DisplayName = "Curious Child"),
    StoicGuardian       UMETA(DisplayName = "Stoic Guardian"),
    MysteriousWanderer  UMETA(DisplayName = "Mysterious Wanderer")
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    African         UMETA(DisplayName = "African Heritage"),
    Asian           UMETA(DisplayName = "Asian Heritage"),
    European        UMETA(DisplayName = "European Heritage"),
    Indigenous      UMETA(DisplayName = "Indigenous Heritage"),
    Mixed           UMETA(DisplayName = "Mixed Heritage")
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float MuscleMass = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    FLinearColor HairColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float FaceWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float JawStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float NoseSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scars and Marks")
    TArray<FVector2D> ScarLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scars and Marks")
    TArray<float> ScarIntensities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age")
    float AgeProgression = 0.3f; // 0 = young, 1 = elderly

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    int32 ClothingVariant = 0;

    FCharacterVariationData()
    {
        ScarLocations.Empty();
        ScarIntensities.Empty();
    }
};

USTRUCT(BlueprintType)
struct FCharacterPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Traits")
    float Courage = 0.5f; // 0 = fearful, 1 = fearless

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Traits")
    float Wisdom = 0.5f; // 0 = naive, 1 = wise

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Traits")
    float Aggression = 0.5f; // 0 = peaceful, 1 = aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Traits")
    float Curiosity = 0.5f; // 0 = cautious, 1 = exploratory

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Friendliness = 0.5f; // 0 = hostile, 1 = friendly

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Leadership = 0.5f; // 0 = follower, 1 = leader

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalInstinct = 0.7f; // Higher in prehistoric setting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float TerritorialBehavior = 0.4f;
};

/**
 * Core character system that manages MetaHuman creation and variation
 * Handles protagonist and NPC generation with unique physical traits
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACharacterSystemCore : public ACharacter
{
    GENERATED_BODY()

public:
    ACharacterSystemCore();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character Definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    ECharacterArchetype CharacterArchetype = ECharacterArchetype::Protagonist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    ECharacterEthnicity Ethnicity = ECharacterEthnicity::Mixed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    FString CharacterName = TEXT("Dr. Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    FString BackgroundStory = TEXT("A paleontologist transported to the prehistoric world.");

    // Physical Variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    FCharacterVariationData PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FCharacterPersonality PersonalityTraits;

    // MetaHuman Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    USkeletalMeshComponent* MetaHumanBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    USkeletalMeshComponent* MetaHumanHead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    USkeletalMeshComponent* MetaHumanHair;

    // Clothing and Equipment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<UStaticMeshComponent*> ClothingPieces;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<UStaticMeshComponent*> EquipmentPieces;

    // Dynamic Materials
    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    UMaterialInstanceDynamic* SkinMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    UMaterialInstanceDynamic* EyeMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    UMaterialInstanceDynamic* HairMaterial;

    // Character Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void GenerateRandomCharacter();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void GenerateProtagonist();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyCharacterVariation(const FCharacterVariationData& VariationData);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyEthnicityTraits(ECharacterEthnicity TargetEthnicity);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyArchetypeTraits(ECharacterArchetype TargetArchetype);

    // Appearance Modification
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void UpdateSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void UpdateEyeColor(FLinearColor NewEyeColor);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void UpdateHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void AddScar(FVector2D Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetAgeProgression(float AgeValue);

    // Equipment Management
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipClothingPiece(UStaticMesh* ClothingMesh, FName SocketName);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipTool(UStaticMesh* ToolMesh, FName SocketName);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void RemoveAllEquipment();

    // Interaction System
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ExpressEmotion(const FString& EmotionName, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PlayGesture(const FString& GestureName);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsProtagonist() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetSurvivalExperience() const;

protected:
    // Internal helper functions
    void InitializeMetaHumanComponents();
    void CreateDynamicMaterials();
    void ApplyRandomVariations();
    FLinearColor GetEthnicSkinTone(ECharacterEthnicity Ethnicity) const;
    FCharacterPersonality GetArchetypePersonality(ECharacterArchetype Archetype) const;
    void UpdateMaterialParameters();

private:
    // Generation seed for consistent randomization
    int32 CharacterSeed = 0;
    
    // Performance tracking
    float LastUpdateTime = 0.0f;
    bool bNeedsVisualUpdate = false;
};