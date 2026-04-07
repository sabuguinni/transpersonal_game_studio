#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TranspersonalGame/Core/ConsciousnessComponent.h"
#include "TranspersonalGameMode.h"
#include "TranspersonalPlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class TRANSPERSONALGAME_API ATranspersonalPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalPlayerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:    
    virtual void Tick(float DeltaTime) override;

    // Consciousness Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void EnterMeditativeState();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ExitMeditativeState();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TriggerBreathworkPractice(float Duration = 60.0f);

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    UConsciousnessComponent* GetConsciousnessComponent() const { return ConsciousnessComponent; }

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
    TObjectPtr<UConsciousnessComponent> ConsciousnessComponent;

    // Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MeditateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> BreathworkAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ConsciousnessShiftAction;

    // Input Functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartMeditation(const FInputActionValue& Value);
    void StopMeditation(const FInputActionValue& Value);
    void StartBreathwork(const FInputActionValue& Value);
    void TriggerConsciousnessShift(const FInputActionValue& Value);

    // Meditation State
    UPROPERTY(BlueprintReadOnly, Category = "Meditation")
    bool bIsMeditating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Meditation")
    float MeditationTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float MeditationIntensityRate = 5.0f; // Intensity gained per second while meditating

    // Breathwork State
    UPROPERTY(BlueprintReadOnly, Category = "Breathwork")
    bool bIsDoingBreathwork = false;

    UPROPERTY(BlueprintReadOnly, Category = "Breathwork")
    float BreathworkTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Breathwork")
    float BreathworkDuration = 60.0f;

    void UpdateMeditation(float DeltaTime);
    void UpdateBreathwork(float DeltaTime);
    void RegisterWithGameMode();
};