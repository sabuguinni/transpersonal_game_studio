#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Char_TribalCharacter.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalType : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Child       UMETA(DisplayName = "Child"),
    Elder       UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class EChar_Gender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_Gender Gender = EChar_Gender::Male;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasWarPaint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTattoos = false;

    FChar_TribalAppearance()
    {
        Gender = EChar_Gender::Male;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyMass = 1.0f;
        Height = 1.0f;
        bHasWarPaint = false;
        bHasScars = false;
        bHasTattoos = false;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasAnimalHideVest = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasLeatherLoincloth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasBoneJewelry = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasFeatherHeaddress = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasStoneNecklace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);

    FChar_TribalClothing()
    {
        bHasAnimalHideVest = true;
        bHasLeatherLoincloth = true;
        bHasBoneJewelry = true;
        bHasFeatherHeaddress = false;
        bHasStoneNecklace = true;
        ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
};

/**
 * Tribal character class for Cretaceous period humans
 * Represents primitive humans surviving in the dinosaur age
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalCharacter();

protected:
    virtual void BeginPlay() override;

    // Camera system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // Character properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalType TribalType = EChar_TribalType::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FChar_TribalClothing Clothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName = TEXT("Tribal Hunter");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Age = 25;

    // Survival stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    // Skills
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float HuntingSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float CraftingSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float SurvivalSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float CombatSkill = 50.0f;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Movement functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);

    // Character functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetTribalType(EChar_TribalType NewType);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetClothing(const FChar_TribalClothing& NewClothing);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyWarPaint();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void RemoveWarPaint();

    // Survival functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float StaminaAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float StaminaAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void IncreaseHunger(float HungerAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DecreaseHunger(float HungerAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void IncreaseThirst(float ThirstAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DecreaseThirst(float ThirstAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void IncreaseFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DecreaseFear(float FearAmount);

    // Skill functions
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void IncreaseSkill(const FString& SkillName, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Skills")
    float GetSkillLevel(const FString& SkillName) const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Character")
    EChar_TribalType GetTribalType() const { return TribalType; }

    UFUNCTION(BlueprintPure, Category = "Character")
    FChar_TribalAppearance GetAppearance() const { return Appearance; }

    UFUNCTION(BlueprintPure, Category = "Character")
    FChar_TribalClothing GetClothing() const { return Clothing; }

    UFUNCTION(BlueprintPure, Category = "Camera")
    class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    UFUNCTION(BlueprintPure, Category = "Camera")
    class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
    // Input rates
    float BaseTurnRate = 45.0f;
    float BaseLookUpRate = 45.0f;

    // Internal functions
    void UpdateSurvivalStats(float DeltaTime);
    void ApplyAppearanceToMesh();
    void ApplyClothingToMesh();
};

#include "Char_TribalCharacter.generated.h"