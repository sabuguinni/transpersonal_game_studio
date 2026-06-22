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

public:
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

    // ── Movement helpers ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaRunDrainRate = 10.f;

    // ── Input ────────────────────────────────────────────────────────────────
    void MoveForward(float Value);
    void MoveRight(float Value);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
    void StartRun();
    void StopRun();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float BaseTurnRate = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float BaseLookUpRate = 45.f;

private:
    bool bIsRunning = false;
};
