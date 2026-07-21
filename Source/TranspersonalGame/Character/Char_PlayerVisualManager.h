#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalCharacterAsset.h"
#include "Char_PlayerVisualManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAppearanceChanged, const UChar_TribalCharacterAsset*, NewAsset);

/**
 * Manages player character visual appearance, camera setup, and equipment
 * Handles third-person camera configuration and character mesh swapping
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisualManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisualManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Camera Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraBoomLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraBoomHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FRotator CameraBoomRotation = FRotator(-15.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraFOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUsePawnControlRotation = true;

    // Character Appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<UChar_TribalCharacterAsset> CurrentCharacterAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<TSoftObjectPtr<UChar_TribalCharacterAsset>> AvailableCharacterAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<USkeletalMesh> FallbackPlayerMesh;

    // Equipment Slots
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TMap<FString, USkeletalMeshComponent*> EquipmentComponents;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerAppearanceChanged OnPlayerAppearanceChanged;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetCharacterAsset(UChar_TribalCharacterAsset* NewAsset);

    UFUNCTION(BlueprintCallable, Category = "Character")
    UChar_TribalCharacterAsset* GetCurrentCharacterAsset() const;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetupThirdPersonCamera(USpringArmComponent* SpringArm, UCameraComponent* Camera);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void UpdateCameraSettings();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipItem(const FString& SlotName, USkeletalMesh* ItemMesh, const FName& SocketName);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipItem(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyCharacterMesh();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void CycleCharacterAsset();

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    USpringArmComponent* CachedSpringArm;

    UPROPERTY()
    UCameraComponent* CachedCamera;

    void InitializeOwnerComponents();
    void CreateEquipmentComponent(const FString& SlotName);
    void UpdateCharacterMaterials();
    void LoadCharacterAssetAsync();
};

#include "Char_PlayerVisualManager.generated.h"