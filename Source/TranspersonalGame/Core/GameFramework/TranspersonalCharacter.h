// TranspersonalCharacter.h — Agent #3 Core Systems Programmer
// Prehistoric survival game — human primitive vs dinosaur world
// CYCLE: PROD_CYCLE_AUTO_20260623_010
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ── Survival Component ──────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Camera ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // ── Movement ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement",
              meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    // ── Survival Stats (exposed for HUD) ────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    float Health;

    UPROPERTY(BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    float Hunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    float Thirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    float Stamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    float Fear;

    // ── Biome damage ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Biome")
    float VolcanicHeatDamagePerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Biome")
    float DesertHeatDamagePerSecond;

    // ── Input handlers ───────────────────────────────────────────────────────
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartSprint();
    void StopSprint();
    void StartCrouch();
    void StopCrouch();
    void Jump();

    // ── Survival helpers ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyBiomeDamage(float DeltaTime, bool bInVolcanic, bool bInDesert);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SyncSurvivalStats();

    // ── Death ────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Survival")
    void OnCharacterDeath();
    virtual void OnCharacterDeath_Implementation();

public:
    // Accessors
    UFUNCTION(BlueprintCallable, Category = "Survival")
    USurvivalComponent* GetSurvivalComponent() const { return SurvivalComp; }

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool GetIsSprinting() const { return bIsSprinting; }
};
