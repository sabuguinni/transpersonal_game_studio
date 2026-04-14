#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "../SharedTypes.h"
#include "CharacterSystem.generated.h"

// Character archetype definitions for the prehistoric world
UENUM(BlueprintType)
enum class EChar_CharacterArchetype : uint8
{
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout")
};

// Physical characteristics for character generation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Age = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FString HairStyle = TEXT("Braided");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    bool bHasTattoos = false;
};

// Clothing and equipment system
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingName = TEXT("Basic Hide");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> MaterialTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor PrimaryColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor SecondaryColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WearLevel = 0.0f; // 0.0 = new, 1.0 = heavily worn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> Accessories;
};

// Character personality and background
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PersonalityProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString CharacterName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString BackgroundStory = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Wisdom = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Compassion = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> Quirks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> Fears;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> Motivations;
};

/**
 * Core character system component that manages character appearance,
 * personality, and visual storytelling elements
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core character definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    EChar_CharacterArchetype CharacterArchetype = EChar_CharacterArchetype::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    FChar_PhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    FChar_ClothingSet ClothingSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Definition")
    FChar_PersonalityProfile PersonalityProfile;

    // MetaHuman integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftClassPtr<UAnimInstance> AnimationBlueprint;

    // Character generation and customization
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void GenerateRandomCharacter(EChar_CharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyPhysicalTraits();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyClothingSet();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterName(const FString& NewName);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void AddPersonalityQuirk(const FString& Quirk);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBackgroundStory(const FString& Story);

    // Visual storytelling
    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplyAgeEffects(float AgeYears);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplyWeatherEffects(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void AddBattleScars(int32 NumScars);

    // Character validation and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void ValidateCharacterSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void PreviewCharacterVariations();

private:
    // Internal character generation helpers
    void GeneratePhysicalTraitsForArchetype(EChar_CharacterArchetype Archetype);
    void GenerateClothingForArchetype(EChar_CharacterArchetype Archetype);
    void GeneratePersonalityForArchetype(EChar_CharacterArchetype Archetype);
    
    // Material and mesh management
    void UpdateCharacterMaterials();
    void ApplyDynamicMaterialParameters();
    
    // Reference to the skeletal mesh component
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;
};