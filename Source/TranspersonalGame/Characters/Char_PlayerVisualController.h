#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Char_PlayerVisualController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PlayerVisualController : public APlayerController
{
    GENERATED_BODY()

public:
    AChar_PlayerVisualController();

protected:
    virtual void BeginPlay() override;

public:
    // Character visual configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TObjectPtr<USkeletalMesh> DefaultPlayerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TObjectPtr<UMaterialInterface> DefaultSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TObjectPtr<UMaterialInterface> DefaultClothingMaterial;

    // Camera configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraBoomLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraBoomOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUsePawnControlRotation;

    // Character appearance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_Gender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_AgeGroup AgeGroup;

    // Visual customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetupPlayerCharacterVisuals();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ApplyTribalRoleVisuals(EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetCharacterMesh(USkeletalMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetCharacterMaterial(UMaterialInterface* NewMaterial, int32 MaterialIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ConfigureThirdPersonCamera();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraBoomLength(float NewLength);

    // Character state management
    UFUNCTION(BlueprintCallable, Category = "Character State")
    void UpdateCharacterAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character State")
    bool IsCharacterVisible() const;

private:
    // Internal setup functions
    void LoadDefaultAssets();
    void SetupCameraComponents();
    void ApplyDefaultAppearance();

    // Asset references
    UPROPERTY()
    TArray<TObjectPtr<USkeletalMesh>> TribalMeshes;

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInterface>> TribalMaterials;

    // Component references
    UPROPERTY()
    TObjectPtr<class USpringArmComponent> CachedSpringArm;

    UPROPERTY()
    TObjectPtr<class UCameraComponent> CachedCamera;
};