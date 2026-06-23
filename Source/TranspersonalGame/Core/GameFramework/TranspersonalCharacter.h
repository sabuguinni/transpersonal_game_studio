// TranspersonalCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival game — player character with full survival integration

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

    // --- Survival Component ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // --- Camera ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // --- Movement ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsRunning;

    // --- Survival Stats (replicated from SurvivalComp for Blueprint access) ---
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentFear;

    // --- Biome environment modifiers ---
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float CurrentBiomeTemperature;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float CurrentBiomeHazardLevel;

private:
    // Input handlers
    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartRun();
    void StopRun();
    void StartJump();
    void StopJump();

    // Biome query — called every BiomeQueryInterval seconds
    void QueryBiomeEnvironment();
    float BiomeQueryTimer;

    UPROPERTY(EditAnywhere, Category = "Environment")
    float BiomeQueryInterval;
};
