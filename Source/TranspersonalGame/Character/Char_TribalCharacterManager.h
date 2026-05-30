#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Char_TribalCharacterManager.generated.h"

// Tribal character appearance variations
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    // Physical characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    // Clothing and accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString ClothingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> Accessories;

    // Biome adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EBiomeType AdaptedBiome;

    FChar_TribalAppearance()
    {
        Height = 170.0f;
        Weight = 70.0f;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        ClothingType = TEXT("Animal Hide");
        WeaponType = TEXT("Stone Spear");
        AdaptedBiome = EBiomeType::Savanna;
    }
};

// Character role in tribal society
UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Child       UMETA(DisplayName = "Child")
};

// Character personality traits
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PersonalityTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Social;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    FChar_PersonalityTraits()
    {
        Courage = 0.5f;
        Aggression = 0.3f;
        Intelligence = 0.6f;
        Social = 0.7f;
        Curiosity = 0.5f;
    }
};

/**
 * Manages creation and customization of tribal characters for the Cretaceous survival world
 * Handles character appearance, role assignment, and biome-specific adaptations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character creation and management
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    class ATranspersonalCharacter* CreateTribalCharacter(const FChar_TribalAppearance& Appearance, EChar_TribalRole Role, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void GenerateRandomTribalCharacter(EBiomeType TargetBiome, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void CreateTribalGroup(EBiomeType Biome, const FVector& CenterLocation, int32 GroupSize = 5);

    // Character customization
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyBiomeAdaptation(class ATranspersonalCharacter* Character, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetCharacterRole(class ATranspersonalCharacter* Character, EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyPersonalityTraits(class ATranspersonalCharacter* Character, const FChar_PersonalityTraits& Traits);

    // Appearance generation
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    FChar_TribalAppearance GenerateRandomAppearance(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    FChar_PersonalityTraits GenerateRandomPersonality();

    // Character validation and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void TestCharacterCreation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void SpawnTestCharactersInAllBiomes();

protected:
    // Character templates and presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
    TArray<FChar_TribalAppearance> CharacterTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
    TMap<EBiomeType, FChar_TribalAppearance> BiomeSpecificTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
    TMap<EChar_TribalRole, FChar_PersonalityTraits> RolePersonalityDefaults;

    // Spawned character tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Management")
    TArray<class ATranspersonalCharacter*> ManagedCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCharactersPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CharacterSpawnRadius;

private:
    // Internal helper functions
    void InitializeCharacterTemplates();
    void InitializeBiomeTemplates();
    void InitializeRoleDefaults();
    
    FLinearColor GetBiomeAdaptedSkinTone(EBiomeType Biome);
    FString GetBiomeSpecificClothing(EBiomeType Biome);
    FString GetRoleSpecificWeapon(EChar_TribalRole Role);
    TArray<FString> GetRoleSpecificAccessories(EChar_TribalRole Role);
};