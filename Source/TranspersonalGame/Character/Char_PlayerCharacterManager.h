#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Char_TribalWarriorAsset.h"
#include "Char_PlayerCharacterManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PlayerCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_PlayerCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Configuration")
    UChar_TribalWarriorAsset* WarriorAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraBoomLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    FRotator CameraBoomRotation = FRotator(-15.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float CurrentStamina = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MaxStamina = 80.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // Apply the warrior asset configuration to this character
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyWarriorConfiguration();

    // Set up the third-person camera
    UFUNCTION(BlueprintCallable, Category = "Camera Setup")
    void ConfigureThirdPersonCamera();

    // Get the character mesh component
    UFUNCTION(BlueprintCallable, Category = "Character")
    USkeletalMeshComponent* GetCharacterMesh() const { return CharacterMesh; }

    // Get the camera component
    UFUNCTION(BlueprintCallable, Category = "Camera")
    UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // Health and stamina management
    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    void SetStamina(float NewStamina);

    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    float GetStaminaPercentage() const;
};