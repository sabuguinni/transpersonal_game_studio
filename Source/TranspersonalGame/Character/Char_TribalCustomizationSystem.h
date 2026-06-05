#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialParameterCollection.h"
#include "Char_TribalCustomizationSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalHeritage : uint8
{
    CaveHunter      UMETA(DisplayName = "Cave Hunter"),
    PlainsStalker   UMETA(DisplayName = "Plains Stalker"),
    RiverGatherer   UMETA(DisplayName = "River Gatherer"),
    MountainClimber UMETA(DisplayName = "Mountain Climber")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Tall        UMETA(DisplayName = "Tall Scout")
};

USTRUCT(BlueprintType)
struct FChar_TribalMarkings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    FLinearColor PaintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    int32 PatternIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    float Intensity;

    FChar_TribalMarkings()
    {
        PaintColor = FLinearColor::Red;
        PatternIndex = 0;
        Intensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FChar_CustomizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heritage")
    EChar_TribalHeritage Heritage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_TribalMarkings FaceMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_TribalMarkings BodyMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString ClothingVariant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString WeaponSet;

    FChar_CustomizationData()
    {
        Heritage = EChar_TribalHeritage::CaveHunter;
        SkinTone = EChar_SkinTone::Tan;
        BodyBuild = EChar_BodyBuild::Muscular;
        ClothingVariant = TEXT("HideArmor");
        WeaponSet = TEXT("StoneSpear");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalCustomizationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalCustomizationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character customization data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_CustomizationData CustomizationData;

    // Material parameter collection for character materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialParameterCollection* CharacterMaterialParameters;

    // Skeletal mesh variants for different body builds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TMap<EChar_BodyBuild, USkeletalMesh*> BodyBuildMeshes;

    // Material instances for different skin tones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EChar_SkinTone, UMaterialInstance*> SkinToneMaterials;

    // Clothing material instances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<FString, UMaterialInstance*> ClothingMaterials;

    // Apply customization to character
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCustomization(AActor* TargetCharacter);

    // Generate random customization
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void GenerateRandomCustomization();

    // Set heritage and apply heritage-specific bonuses
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTribalHeritage(EChar_TribalHeritage NewHeritage);

    // Update tribal markings
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void UpdateTribalMarkings(const FChar_TribalMarkings& FaceMarkings, const FChar_TribalMarkings& BodyMarkings);

    // Get heritage description
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character Customization")
    FString GetHeritageDescription(EChar_TribalHeritage Heritage) const;

    // Get survival bonuses for heritage
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character Customization")
    TMap<FString, float> GetHeritageBonuses(EChar_TribalHeritage Heritage) const;

private:
    // Apply material parameters
    void ApplyMaterialParameters(USkeletalMeshComponent* MeshComponent);

    // Update character mesh based on body build
    void UpdateCharacterMesh(USkeletalMeshComponent* MeshComponent);

    // Apply tribal markings to materials
    void ApplyTribalMarkings(USkeletalMeshComponent* MeshComponent);
};