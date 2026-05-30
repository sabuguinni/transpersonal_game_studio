#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Char_TribalHumanSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Shaman      UMETA(DisplayName = "Shaman")
};

UENUM(BlueprintType)
enum class EChar_TribalClothing : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    WovenFibers     UMETA(DisplayName = "Woven Fibers"),
    BoneArmor       UMETA(DisplayName = "Bone Armor"),
    LeatherWraps    UMETA(DisplayName = "Leather Wraps"),
    FurCloak        UMETA(DisplayName = "Fur Cloak")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole Role = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalClothing ClothingType = EChar_TribalClothing::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Age = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTattoos = false;

    FChar_TribalAppearance()
    {
        Role = EChar_TribalRole::Hunter;
        ClothingType = EChar_TribalClothing::AnimalHide;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        Age = 25.0f;
        MuscleMass = 0.7f;
        bHasScars = true;
        bHasTattoos = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalHuman : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalHuman();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ToolComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* AccessoryComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Warmth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Fatigue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float HuntingSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float GatheringSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float CraftingSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float SurvivalSkill = 50.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyRoleBasedStats();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void EquipTribalTool(UStaticMesh* ToolMesh);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Skills")
    void GainExperience(EChar_TribalRole SkillType, float Amount);

    UFUNCTION(BlueprintPure, Category = "Tribal Character")
    FString GetRoleDescription() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsStarving() const { return Hunger < 20.0f; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsDehydrated() const { return Thirst < 20.0f; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsCold() const { return Warmth < 30.0f; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsExhausted() const { return Fatigue > 80.0f; }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UChar_TribalHumanSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    AChar_TribalHuman* SpawnTribalHuman(const FVector& Location, const FRotator& Rotation, EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void SpawnTribalGroup(const FVector& CenterLocation, int32 GroupSize, float SpreadRadius);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    TArray<AChar_TribalHuman*> GetAllTribalHumans();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void DistributeTribalHumansAcrossBiomes();

protected:
    UPROPERTY()
    TArray<AChar_TribalHuman*> SpawnedTribalHumans;

    UFUNCTION()
    FChar_TribalAppearance GenerateRandomAppearance(EChar_TribalRole Role);
};