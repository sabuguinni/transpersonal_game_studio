#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

// Forward declarations — avoid cross-module include chain
class UCameraComponent;
class USpringArmComponent;

/**
 * Survival stats for the prehistoric human player character.
 * All values normalized 0.0–1.0 (1.0 = full, 0.0 = critical/dead).
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 1.0f;

    /** Fear increases near predators; reduces decision-making accuracy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    /** Body temperature — hypothermia/hyperthermia risk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 0.5f;
};

/**
 * ATranspersonalCharacter — the playable prehistoric human survivor.
 *
 * Extends ACharacter with:
 *  - Third-person camera rig (spring arm + camera)
 *  - Survival stats (health, hunger, thirst, stamina, fear, temperature)
 *  - Inline survival tick (no separate component required for CDO safety)
 *  - WASD movement, sprint, jump wired to Enhanced Input
 *
 * Architecture note: SurvivalComponent was considered but survival logic is
 * kept inline here for CDO safety (no null-deref risk in headless editor).
 * If a separate component is needed later, extract after full compile validation.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Transpersonal Character"))
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ─── Camera ───────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ─── Survival Stats ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FCore_SurvivalStats SurvivalStats;

    /** Rate at which hunger depletes per second (0.0–1.0 scale) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDepletionRate = 0.002f;

    /** Rate at which thirst depletes per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDepletionRate = 0.003f;

    /** Stamina recovery rate per second when not sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 0.05f;

    /** Stamina depletion rate per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaSprintCost = 0.08f;

    // ─── Movement State ───────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed = 600.0f;

    // ─── Public API ───────────────────────────────────────────────────────────

    /** Apply damage to the character's health stat */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage_Survival(float Amount);

    /** Consume food — restores hunger */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    /** Drink water — restores thirst */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    /** Returns true if character is alive (health > 0) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    bool IsAlive() const;

    /** Set fear level (called by DinosaurAI proximity system) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFearLevel(float FearValue);

    /** Start sprinting — costs stamina */
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    /** Stop sprinting */
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

protected:
    /** Tick survival stats — hunger/thirst/stamina drain */
    void TickSurvivalStats(float DeltaTime);

    /** Handle character death */
    void OnCharacterDeath();

    // ─── Input Handlers ───────────────────────────────────────────────────────
    void MoveForward(float Value);
    void MoveRight(float Value);
    void TurnRate(float Value);
    void LookUpRate(float Value);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float BaseTurnRate = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float BaseLookUpRate = 45.0f;

private:
    bool bIsDead = false;
};
