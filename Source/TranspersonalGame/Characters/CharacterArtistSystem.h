#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "MetaHumanCreator/Public/MetaHumanCreatorAPI.h"
#include "CharacterArtistSystem.generated.h"

/**
 * Character Artist System
 * Manages the creation, variation, and visual identity of all human characters
 * Uses MetaHuman Creator for base generation with procedural variation
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist - Dr. Paleontologist"),
    Survivor        UMETA(DisplayName = "Survivor - Weathered Human"),
    Tribal          UMETA(DisplayName = "Tribal - Indigenous Survivor"),
    Researcher      UMETA(DisplayName = "Researcher - Lost Scientist"),
    Explorer        UMETA(DisplayName = "Explorer - Adventure Seeker"),
    Elder           UMETA(DisplayName = "Elder - Wise Survivor"),
    Child           UMETA(DisplayName = "Child - Young Survivor"),
    Warrior         UMETA(DisplayName = "Warrior - Combat Survivor")
};

UENUM(BlueprintType)
enum class ECharacterCondition : uint8
{
    Fresh           UMETA(DisplayName = "Fresh - Recently Arrived"),
    Weathered       UMETA(DisplayName = "Weathered - Months of Survival"),
    Hardened        UMETA(DisplayName = "Hardened - Years of Survival"),
    Scarred         UMETA(DisplayName = "Scarred - Battle Marked"),
    Sick            UMETA(DisplayName = "Sick - Struggling Health"),
    Thriving        UMETA(DisplayName = "Thriving - Adapted Well")
};

USTRUCT(BlueprintType)
struct FCharacterVisualProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    ECharacterCondition SurvivalCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> ClothingPieces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    TArray<FString> ScarsAndMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    TArray<FString> Accessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstory")
    FString VisualBiography;

    FCharacterVisualProfile()
    {
        CharacterName = TEXT("Unknown");
        Archetype = ECharacterArchetype::Survivor;
        SurvivalCondition = ECharacterCondition::Weathered;
        Age = 35.0f;
        Height = 170.0f;
        Weight = 70.0f;
        SkinTone = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        EyeColor = FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);
        VisualBiography = TEXT("A survivor marked by time in this prehistoric world.");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterArtistSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterArtistSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    // Core Character Generation
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    class ACharacter* GenerateCharacterFromProfile(const FCharacterVisualProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterVisualProfile GenerateRandomProfile(ECharacterArchetype DesiredArchetype);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyWeatheringEffects(class USkeletalMeshComponent* MeshComponent, 
        ECharacterCondition Condition);

    // MetaHuman Integration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetupMetaHumanBase(class USkeletalMeshComponent* MeshComponent, 
        const FCharacterVisualProfile& Profile);

    // Clothing and Accessories
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void ApplyClothingSet(class USkeletalMeshComponent* MeshComponent, 
        ECharacterArchetype Archetype, ECharacterCondition Condition);

    // Diversity System
    UFUNCTION(BlueprintCallable, Category = "Diversity")
    FCharacterVisualProfile EnsureUniqueAppearance(const FCharacterVisualProfile& BaseProfile);

    // Character Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<FCharacterVisualProfile> GeneratedCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<class USkeletalMesh*> MetaHumanBaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<class UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<class UMaterialInterface*> ClothingMaterials;

private:
    // Internal generation helpers
    FLinearColor GenerateRealisticSkinTone();
    FLinearColor GenerateRealisticHairColor();
    FLinearColor GenerateRealisticEyeColor();
    TArray<FString> GenerateScarsForCondition(ECharacterCondition Condition);
    FString GenerateVisualBiography(const FCharacterVisualProfile& Profile);
};