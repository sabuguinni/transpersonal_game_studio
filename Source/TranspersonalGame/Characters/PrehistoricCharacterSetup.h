#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "PrehistoricCharacterSetup.generated.h"

/**
 * Survival stats for the prehistoric player character.
 * All values normalized 0.0-1.0 (1.0 = full, 0.0 = depleted).
 */
USTRUCT(BlueprintType)
struct FChar_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Temperature = 0.5f;
};

/**
 * Appearance data for the prehistoric character.
 * Used to define visual diversity across NPCs and the player.
 */
USTRUCT(BlueprintType)
struct FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.6f, 0.4f, 0.25f, 1.0f);

    UPROPERTY(BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.1f, 0.07f, 0.04f, 1.0f);

    UPROPERTY(BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 0.7f;

    UPROPERTY(BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = false;

    UPROPERTY(BlueprintReadWrite, Category = "Appearance")
    int32 ClothingVariant = 0;
};

/**
 * APrehistoricCharacterSetup
 * 
 * Third-person prehistoric survivor character with:
 * - WASD movement + jump + sprint
 * - Spring arm + follow camera
 * - Survival stats (health, hunger, thirst, stamina, fear, temperature)
 * - Visual appearance data for character diversity
 * 
 * Designed to work with UE5 mannequin skeletal mesh as placeholder
 * until final prehistoric character assets are imported.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricCharacterSetup : public ACharacter
{
    GENERATED_BODY()

public:
    APrehistoricCharacterSetup();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Camera ───────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ─── Survival Stats ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FChar_SurvivalStats SurvivalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerDrainRate = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstDrainRate = 0.008f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaDrainRate = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaRegenRate = 0.015f;

    // ─── Appearance ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_AppearanceData AppearanceData;

    // ─── Movement ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsSprinting = false;

    // ─── Functions ────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const { return SurvivalStats.Health > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsStarving() const { return SurvivalStats.Hunger < 0.15f; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsDehydrated() const { return SurvivalStats.Thirst < 0.15f; }

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyAppearance(const FChar_AppearanceData& NewAppearance);

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
    void TickSurvivalStats(float DeltaTime);

    float BaseTurnRate = 45.0f;
    float BaseLookUpRate = 45.0f;
};
