#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "../SharedTypes.h"
#include "Char_TribalCharacterSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class EChar_AgeGroup : uint8
{
    YoungAdult  UMETA(DisplayName = "Young Adult"),
    Adult       UMETA(DisplayName = "Adult"),
    MiddleAged  UMETA(DisplayName = "Middle Aged"),
    Elder       UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_AgeGroup AgeGroup = EChar_AgeGroup::Adult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone = 0.5f; // 0.0 = light, 1.0 = dark

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 HairStyle = 0; // 0-5 different styles

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 FacialFeatures = 0; // 0-10 different face variations

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild = 0.5f; // 0.0 = lean, 1.0 = muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 ClothingVariation = 0; // Different animal hide patterns
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Strength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Agility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Intelligence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Survival = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SocialSkill = 50.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString CharacterName = TEXT("Tribal Survivor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    bool bIsFemale = false;

    // Equipment Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* ToolMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* AccessoryMesh;

    // Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RunSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHostile = false;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void EquipWeapon(class UStaticMesh* WeaponStaticMesh);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetAgeGroup(EChar_AgeGroup NewAgeGroup);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    FString GetCharacterDescription() const;

private:
    void InitializeComponents();
    void SetupDefaultMeshes();
    void ApplyAppearanceToMesh();
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UChar_TribalCharacterManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Character Manager")
    static AChar_TribalCharacter* SpawnTribalCharacter(UWorld* World, FVector Location, EChar_TribalRole Role = EChar_TribalRole::Hunter);

    UFUNCTION(BlueprintCallable, Category = "Character Manager")
    static void SpawnTribalGroup(UWorld* World, FVector CenterLocation, int32 GroupSize = 5);

    UFUNCTION(BlueprintCallable, Category = "Character Manager")
    static TArray<AChar_TribalCharacter*> GetAllTribalCharacters(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Character Manager")
    static void RandomizeAllTribalCharacters(UWorld* World);
};